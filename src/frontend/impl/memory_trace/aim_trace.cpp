#include <cassert>
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

class AiMTrace : public IFrontEnd,
                 public Implementation {

    RAMULATOR_REGISTER_IMPLEMENTATION(IFrontEnd, AiMTrace, "AiMTrace", "AiM ISR trace.")

private:
    std::vector<Request> m_trace;

    size_t m_curr_trace_idx = 0;

    bool m_trace_reached_EOC = false;

    Logger_t m_logger;

    std::ifstream *trace_file;
    std::string file_path;

    Request req;
    bool remaining_req = false;

    int host_req_id = 0;

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
    };

    void tick() override {
        if (m_trace_reached_EOC == false) {
            if (remaining_req == false) {
                req = get_host_request();
                remaining_req = true;
            }
            bool request_sent = m_memory_system->send(req);
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

    Request get_host_request() {
        std::string line;

        if (m_trace_reached_EOC == true) {
            throw ConfigurationError("Trace: asking for host request while EOC reached!");
        }

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
                m_curr_trace_idx++;
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
                    Request req(-1, Request::Type::MAX);

                    req.host_req_id = host_req_id++;

                    int token_idx = 0;

                    // Decoding trace type
                    req.type = AiMISRInfo::convert_str_to_type(tokens[token_idx++]);

                    if (req.type == Request::Type::AIM) {

                        // Decoding opcode
                        AiMISR aim_request = AiMISRInfo::convert_opcode_str_to_AiM_ISR(tokens[token_idx++]);

                        req.opcode = aim_request.opcode;

                        // Decoding other fields
#define DECODE_AND_SET_FIELD(name) \
    req.name = token_decoder<decltype(req.name)>(tokens[token_idx++]);

#define DECODE_AIM_HOST_REQ_FIELD(name) \
    DECODE_AND_SET_FIELD(name)          \
    aim_request.is_field_value_legal<decltype(req.name)>(AiMISR::Field::name, req.name);

                        DECODE_AIM_HOST_REQ_FIELD(opsize)
                        DECODE_AIM_HOST_REQ_FIELD(GPR_addr_0)
                        DECODE_AIM_HOST_REQ_FIELD(GPR_addr_1)
                        DECODE_AIM_HOST_REQ_FIELD(channel_mask)
                        DECODE_AIM_HOST_REQ_FIELD(bank_index)
                        DECODE_AIM_HOST_REQ_FIELD(row_addr)
                        DECODE_AIM_HOST_REQ_FIELD(col_addr)
                        DECODE_AIM_HOST_REQ_FIELD(thread_index)

                        if (req.opcode == Request::Opcode::ISR_EOC) {
                            m_trace_reached_EOC = true;
                            m_logger->info("End-Of-Compute Reached!");
                        }
                    } else {

                        // Decoding opcode
                        req.mem_access_region = AiMISRInfo::convert_str_to_mem_access_region(tokens[token_idx++]);

                        DECODE_AND_SET_FIELD(addr)
                        DECODE_AND_SET_FIELD(addr_max)
                        DECODE_AND_SET_FIELD(data)

                        if (req.mem_access_region == Request::MemAccessRegion::CFR) {
                            if (req.data == -1) {
                                throw ConfigurationError("Trace: CFR value not determined!");
                            }
                        } else {
                            if (req.data != -1) {
                                throw ConfigurationError("Trace: GPR/MEM value must be -1!");
                            }
                        }
                    }
                }

                return req;
            }
        }
    };

    // TODO: FIXME
    bool is_finished() override { return m_trace_reached_EOC; };
};

} // namespace Ramulator