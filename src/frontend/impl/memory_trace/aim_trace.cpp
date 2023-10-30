#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <vector>

#include "base/exception.h"
#include "base/request.h"
#include "frontend/frontend.h"

namespace Ramulator {

namespace fs = std::filesystem;

Request AiMTrace_request(-1, -1);

class AiMTrace : public IFrontEnd,
                 public Implementation {

    RAMULATOR_REGISTER_IMPLEMENTATION(IFrontEnd, AiMTrace, "AiMTrace", "AiM ISR trace.")

private:
    std::vector<Request> m_trace;

    int m_curr_trace_idx = 0;

    bool m_trace_reached_EOC = false;
    bool m_trace_reached_calledback = false;

    Logger_t m_logger;

    std::ifstream *trace_file;
    std::string file_path;

    bool remaining_req = false;

    int host_req_id = 0;

    std::function<void(Request &)> callback;

public:
    void init() override {
        std::string trace_path_str = param<std::string>("path")
                                         .desc("Path to the AiM host request trace file.")
                                         .required();
        m_clock_ratio = param<uint>("clock_ratio").required();

        m_logger = Logging::create_logger("AiMTrace");
        m_logger->info("Opening trace file {} ...", trace_path_str);
        init_trace(trace_path_str);
        remaining_req = false;
        callback = std::bind(&AiMTrace::receive, this, std::placeholders::_1);
    };

    void tick() override {
        if ((remaining_req == false) && (m_trace_reached_EOC == false)) {
            AiMTrace_request = get_host_request();
            remaining_req = true;
        }
        if (remaining_req == true) {
            bool request_sent = m_memory_system->send(AiMTrace_request);
            if (request_sent)
                remaining_req = false;
        }
    };

private:
    void init_trace(const std::string &file_path_str) {
        file_path = file_path_str;
        fs::path trace_path(file_path);
        if (!fs::exists(trace_path)) {
            throw ConfigurationError("Trace {} does not exist!", file_path);
        }

        trace_file = new std::ifstream(trace_path);
        if (!trace_file->is_open()) {
            throw ConfigurationError("Trace {} cannot be opened!", file_path);
        }

        m_curr_trace_idx = 0;
        m_trace_reached_EOC = false;
    }

    template <typename T>
    T token_decoder(std::string str) {
        if (str.compare(0, 2, "0x") == 0 | str.compare(0, 2, "0X") == 0) {
            return std::stoll(str.substr(2), nullptr, 16);
        } else {
            return std::stoll(str);
        }
    }

    void receive(Request &req) {
        assert(req.type == Request::Type::AIM);
        assert(req.opcode == Request::Opcode::ISR_EOC);
        m_logger->info("End-Of-Compute Called Back!");
        m_trace_reached_calledback = true;
    }

    Request get_host_request() {
        std::string line;

        if (m_trace_reached_EOC == true) {
            throw ConfigurationError("Trace: asking for host request while EOC reached!");
        }

        Request req(-1, -1);

        while (true) {
            try {
                std::getline(*trace_file, line);
                m_curr_trace_idx++;
            } catch (const std::runtime_error &ex) {
                std::cout << ex.what() << std::endl;
            }
            if (trace_file->eof()) {
                throw ConfigurationError("Trace: EOF reached while EOC not reached (trace does not have EOC host request)!");
            } else if (line.empty()) {
                // It's a white space
                continue;
            } else {
                std::vector<std::string> tokens;
                tokenize(tokens, line, ",");

                if (tokens.empty() == true) {
                    // It's an empty line
                    continue;
                }
                if (tokens[0] == "#") {
                    // It's a comment
                    continue;
                } else {
                    req.host_req_id = host_req_id++;

                    int token_idx = 0;

                    // printf("#%d:", m_curr_trace_idx);
                    // for (auto token : tokens) {
                    //     printf("\"%s\" ", token.c_str());
                    // }
                    // printf("\n");

                    // Decoding traxce type
                    req.type = AiMISRInfo::convert_str_to_type(tokens[token_idx++]);
                    req.type_id = (int)req.type;

                    if (req.type == Request::Type::AIM) {

                        // Decoding opcode
                        std::string ISR_opcode = tokens[token_idx++];
                        std::string prefix = "ISR_";
                        if (std::mismatch(prefix.begin(), prefix.end(), ISR_opcode.begin()).first != prefix.end())
                            ISR_opcode = prefix + ISR_opcode;
                        AiMISR aim_request = AiMISRInfo::convert_opcode_str_to_AiM_ISR(ISR_opcode);

                        if (aim_request.legal_fields.size() != tokens.size() - 2) {
                            throw ConfigurationError("Trace: aim request {} requires {} fields, but {} is specified in line: \n{}!",
                                                     AiMISRInfo::convert_AiM_opcode_to_str(aim_request.opcode),
                                                     aim_request.legal_fields.size(),
                                                     tokens.size() - 2,
                                                     line);
                        }

                        req.opcode = aim_request.opcode;

                        // Decoding other fields
#define DECODE_AND_SET_FIELD(name) \
    req.name = token_decoder<decltype(req.name)>(tokens[token_idx++]);

#define DECODE_AIM_HOST_REQ_FIELD(name) \
    DECODE_AND_SET_FIELD(name)          \
    aim_request.is_field_value_legal<decltype(req.name)>(AiMISR::Field::name, req.name);

#define DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(name)          \
    if (aim_request.is_field_legal(AiMISR::Field::name)) { \
        DECODE_AIM_HOST_REQ_FIELD(name)                    \
    }

                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(opsize)
                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(GPR_addr_0)
                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(GPR_addr_1)
                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(channel_mask)
                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(bank_index)
                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(row_addr)
                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(col_addr)
                        DECODE_AIM_HOST_REQ_FIELD_IF_NEEDED(thread_index)

                        if (req.opcode == Request::Opcode::ISR_EOC) {
                            m_trace_reached_EOC = true;
                            req.callback = callback;
                            m_logger->info("End-Of-Compute Reached!");
                        }
                    } else {

                        // Decoding opcode
                        req.mem_access_region = AiMISRInfo::convert_str_to_mem_access_region(tokens[token_idx++]);

                        if (req.mem_access_region == Request::MemAccessRegion::CFR) {
                            DECODE_AND_SET_FIELD(addr)
                            DECODE_AND_SET_FIELD(data)
                        } else if (req.mem_access_region == Request::MemAccessRegion::GPR) {
                            DECODE_AND_SET_FIELD(addr)
                        } else {
                            DECODE_AND_SET_FIELD(channel_mask)
                            DECODE_AND_SET_FIELD(bank_index)
                            DECODE_AND_SET_FIELD(row_addr)
                        }
                    }
                }

                return req;
            }
        }
    };

    // TODO: FIXME
    bool is_finished() override { return m_trace_reached_calledback; };
};

} // namespace Ramulator