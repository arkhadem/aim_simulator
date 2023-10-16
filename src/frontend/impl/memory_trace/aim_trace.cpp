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

#define ISR_SIZE (1 << 21)
#define CFR_SIZE (1 << 14)
#define GPR_SIZE (1 << 19)

#define CFR_ADDR_MIN (ISR_SIZE)
#define CFR_ADDR_MAX (ISR_SIZE + CFR_SIZE - 1)
#define GPR_ADDR_MIN (ISR_SIZE + CFR_SIZE)
#define GPR_ADDR_MAX (ISR_SIZE + CFR_SIZE + GPR_SIZE - 1)
#define MEM_ADDR_MIN (ISR_SIZE + CFR_SIZE + GPR_SIZE)

enum mem_access_t {
    ISR,
    CFR,
    GPR,
    MEM,
    MAX
};

namespace fs = std::filesystem;

class AiMHostRequest {

public:
    enum class Field {
        opsize,
        GPR_addr_0,
        GPR_addr_1,
        channel_mask,
        bank_index,
        row_addr,
        col_addr,
        thread_index
    };

    AiMHostRequest(Ramulator::Request::Opcode opcode_, std::vector<Field> legal_fields_) : opcode(opcode_), legal_fields(legal_fields_) {}

    Ramulator::Request::Opcode opcode;

    std::vector<Field> legal_fields;

    template <typename T>
    void is_field_legal(Field field, T value) {
        if (legal_fields.empty() or std::count(legal_fields.begin(), legal_fields.end(), field) == false) {
            if (value != (T)-1) {
                throw ConfigurationError("Trace: opcode {} does not accept field {}!", (int)opcode, (int)field);
            }
        } else if (value == (T)-1) {
            throw ConfigurationError("Trace: opcode {} must be provided with field {}!", (int)opcode, (int)field);
        }
    }
};

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

    static std::map<std::string, AiMHostRequest> str_to_aim_map;
    static std::map<std::string, Ramulator::Request::Type> str_to_type;
    static std::map<std::string, Ramulator::Request::MemAccessRegion> str_to_mem_access_region;

public:
    void init() override {
        std::string trace_path_str = param<std::string>("path")
                                         .desc("Path to the AiM host request trace file.")
                                         .required();
        m_clock_ratio = param<uint>("clock_ratio").required();

        m_logger = Logging::create_logger("AiMTrace");
        m_logger->info("Opening trace file {} ...", trace_path_str);
        init_trace(trace_path_str);
    };

    void tick() override {
        const Request &t = m_trace[m_curr_trace_idx];
        bool request_sent = m_memory_system->send(
            {t.addr, t.is_write ? Request::Type::Write : Request::Type::Read});
        if (request_sent) {
            m_curr_trace_idx = (m_curr_trace_idx + 1) % m_trace_length;
            m_trace_count++;
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

        str_to_aim_map["ISR_WR_SBK"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_WR_SBK,
                                                      {AiMHostRequest::Field::opsize,
                                                       AiMHostRequest::Field::GPR_addr_0,
                                                       AiMHostRequest::Field::channel_mask,
                                                       AiMHostRequest::Field::bank_index,
                                                       AiMHostRequest::Field::row_addr,
                                                       AiMHostRequest::Field::col_addr});
        str_to_aim_map["ISR_WR_GB"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_WR_GB,
                                                     {AiMHostRequest::Field::opsize,
                                                      AiMHostRequest::Field::GPR_addr_0,
                                                      AiMHostRequest::Field::channel_mask,
                                                      AiMHostRequest::Field::col_addr});
        str_to_aim_map["ISR_WR_BIAS"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_WR_BIAS,
                                                       {AiMHostRequest::Field::opsize,
                                                        AiMHostRequest::Field::GPR_addr_0,
                                                        AiMHostRequest::Field::channel_mask});
        str_to_aim_map["ISR_WR_AFLUT"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_WR_AFLUT,
                                                        {AiMHostRequest::Field::opsize});
        str_to_aim_map["ISR_RD_MAC"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_RD_MAC,
                                                      {AiMHostRequest::Field::opsize,
                                                       AiMHostRequest::Field::GPR_addr_0,
                                                       AiMHostRequest::Field::channel_mask,
                                                       AiMHostRequest::Field::thread_index});
        str_to_aim_map["ISR_RD_AF"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_RD_AF,
                                                     {AiMHostRequest::Field::opsize});
        str_to_aim_map["ISR_RD_SBK"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_RD_SBK,
                                                      {AiMHostRequest::Field::opsize,
                                                       AiMHostRequest::Field::channel_mask,
                                                       AiMHostRequest::Field::bank_index,
                                                       AiMHostRequest::Field::row_addr,
                                                       AiMHostRequest::Field::col_addr});
        str_to_aim_map["ISR_COPY_BKGB"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_COPY_BKGB,
                                                         {AiMHostRequest::Field::opsize,
                                                          AiMHostRequest::Field::channel_mask,
                                                          AiMHostRequest::Field::bank_index,
                                                          AiMHostRequest::Field::row_addr,
                                                          AiMHostRequest::Field::col_addr});
        str_to_aim_map["ISR_COPY_GBBK"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_COPY_GBBK,
                                                         {AiMHostRequest::Field::opsize,
                                                          AiMHostRequest::Field::channel_mask,
                                                          AiMHostRequest::Field::bank_index,
                                                          AiMHostRequest::Field::row_addr,
                                                          AiMHostRequest::Field::col_addr});
        str_to_aim_map["ISR_MAC_SBK"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_MAC_SBK,
                                                       {AiMHostRequest::Field::opsize,
                                                        AiMHostRequest::Field::channel_mask,
                                                        AiMHostRequest::Field::bank_index,
                                                        AiMHostRequest::Field::row_addr,
                                                        AiMHostRequest::Field::col_addr,
                                                        AiMHostRequest::Field::thread_index});
        str_to_aim_map["ISR_MAC_ABK"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_MAC_ABK,
                                                       {AiMHostRequest::Field::opsize,
                                                        AiMHostRequest::Field::channel_mask,
                                                        AiMHostRequest::Field::row_addr,
                                                        AiMHostRequest::Field::col_addr,
                                                        AiMHostRequest::Field::thread_index});
        str_to_aim_map["ISR_AF"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_AF,
                                                  {AiMHostRequest::Field::opsize});
        str_to_aim_map["ISR_EWMUL"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_EWMUL,
                                                     {AiMHostRequest::Field::opsize,
                                                      AiMHostRequest::Field::channel_mask,
                                                      AiMHostRequest::Field::bank_index,
                                                      AiMHostRequest::Field::row_addr,
                                                      AiMHostRequest::Field::col_addr});
        str_to_aim_map["ISR_EWADD"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_EWADD,
                                                     {AiMHostRequest::Field::opsize,
                                                      AiMHostRequest::Field::GPR_addr_0,
                                                      AiMHostRequest::Field::GPR_addr_1});
        str_to_aim_map["ISR_EOC"] = AiMHostRequest(Ramulator::Request::Opcode::ISR_EOC,
                                                   {});

        str_to_type["R"] = Ramulator::Request::Type::Read;
        str_to_type["W"] = Ramulator::Request::Type::Write;
        str_to_type["AiM"] = Ramulator::Request::Type::AIM;

        str_to_mem_access_region["GPR"] = Ramulator::Request::MemAccessRegion::GPR;
        str_to_mem_access_region["CFR"] = Ramulator::Request::MemAccessRegion::CFR;
        str_to_mem_access_region["MEM"] = Ramulator::Request::MemAccessRegion::MEM;

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

    Request get_host_AiM_request() {
        std::string line;

        if (m_trace_reached_EOC == true) {
            throw ConfigurationError("Trace: asking for host AiM request while EOC reached!");
        }

        while (true) {
            try {
                std::getline(*trace_file, line);
                m_curr_trace_idx++;
            } catch (const std::runtime_error &ex) {
                std::cout << ex.what() << std::endl;
            }
            if (trace_file->eof()) {
                throw ConfigurationError("Trace: EOF reached while EOC not reached (trace does not have EOC host AiM request)!");
            } else if (line.empty()) {
                // It's a white space
                m_curr_trace_idx++;
                continue;
            } else {
                Request req();

                std::vector<std::string> tokens;
                tokenize(tokens, line, ",");
                if (tokens[0] == "#") {
                    // It's a comment
                    continue;
                } else {
                    int token_idx = 0;

                    // Decoding trace type
                    std::string type_str = tokens[token_idx++];
                    if (str_to_type.find(type_str) == str_to_type.end()) {
                        throw ConfigurationError("Trace: unknown type {}!", type_str);
                    }
                    req.type = str_to_type[type_str];

                    if (req.type == Ramulator::Request::Type::AIM) {

                        // Decoding opcode
                        std::string opcode_str = tokens[token_idx++];
                        if (str_to_aim_map.find(opcode_str) == str_to_aim_map.end()) {
                            throw ConfigurationError("Trace: unknown opcode {}!", opcode_str);
                        }

                        AiMHostRequest aim_request = str_to_aim_map[opcode_str];

                        req.opcode = aim_request.opcode;

                        // Decoding other fields
#define DECODE_AND_SET_FIELD(name) \
    req.name = token_decoder<decltype(req.name)>(tokens[token_idx++]);

#define DECODE_AIM_HOST_REQ_FIELD(name) \
    DECODE_AND_SET_FIELD(name)          \
    aim_request.is_field_legal<decltype(req.name)>(AiMHostRequest::Field::name, req.name);

                        DECODE_AIM_HOST_REQ_FIELD(opsize)
                        DECODE_AIM_HOST_REQ_FIELD(GPR_addr_0)
                        DECODE_AIM_HOST_REQ_FIELD(GPR_addr_1)
                        DECODE_AIM_HOST_REQ_FIELD(channel_mask)
                        DECODE_AIM_HOST_REQ_FIELD(bank_index)
                        DECODE_AIM_HOST_REQ_FIELD(row_addr)
                        DECODE_AIM_HOST_REQ_FIELD(col_addr)
                        DECODE_AIM_HOST_REQ_FIELD(thread_index)

                        if (req.opcode == Ramulator::Request::Opcode::ISR_EOC) {
                            m_trace_reached_EOC = true;
                            m_logger->info("End-Of-Compute Reached!");
                        }
                    } else {

                        // Decoding opcode
                        std::string mem_access_region_str = tokens[token_idx++];
                        if (str_to_mem_access_region.find(mem_access_region_str) == str_to_mem_access_region.end()) {
                            throw ConfigurationError("Trace: unknown mem_access_region {}!", mem_access_region_str);
                        }
                        req.mem_access_region = str_to_mem_access_region[mem_access_region_str];

                        DECODE_AND_SET_FIELD(addr)
                        DECODE_AND_SET_FIELD(addr_max)
                        DECODE_AND_SET_FIELD(data)

                        if (req.mem_access_region == Ramulator::Request::MemAccessRegion::CFR) {
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
            }
        }
    };

    // TODO: FIXME
    bool is_finished() override { return m_trace_reached_EOC; };
};

} // namespace Ramulator