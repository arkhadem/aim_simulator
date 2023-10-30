#ifndef RAMULATOR_BASE_REQUEST_H
#define RAMULATOR_BASE_REQUEST_H

#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "base/base.h"

namespace Ramulator {

struct Request {
    Addr_t addr = -1;
    Data_t data;
    AddrVec_t addr_vec{};
    int host_req_id = -1;
    int AiM_req_id = -1;

    // Basic request id convention
    // 0 = Read, 1 = Write. The device spec defines all others
    enum class Type {
        Read = 0,
        Write = 1,
        AIM = 2,
        MAX = 3
    } type;

    enum class MemAccessRegion {
        MIN,
        GPR,
        CFR,
        MEM,
        MAX
    } mem_access_region;

    // ISR Command Opcodes
    enum class Opcode {
        MIN = 0,
        ISR_WR_SBK = 1,    // Write [op_size * 256 bits] from [GPR * 32] to [a single bank] of channel [#channel_address]
        ISR_WR_GB = 2,     // Write the same [op_size * 256 bits] starting from [GPR * 32]
                           // to the [Global Buffer] of [channel_mask] channels
                           // [NOT_IMPLEMENTED] source could also be [host]
        ISR_WR_BIAS = 3,   // Write [opsize * 16 x 16-bits] bits from [GPR * 32]
                           // to [MAC accumulator of 16 banks] of [opsize or channel_mask] channels
        ISR_WR_AFLUT = 4,  // Write activation function data from [host] to [?]
        ISR_RD_MAC = 5,    // Read data from [MAC accumulator of all banks] to [GPR]
        ISR_RD_AF = 6,     // Read data from [AF results of all banks] to [GPR]
        ISR_RD_SBK = 7,    // [NOT_LISTED] Read data from [a single bank] to [host]
        ISR_COPY_BKGB = 8, // Copy data from [a single bank] to [the Global Buffer]
        ISR_COPY_GBBK = 9, // Copy data from [the Global Buffer] to [a single bank]
        ISR_MAC_SBK = 10,  // Perform MAC operation between [a single bank] and [Global Buffer]
        ISR_MAC_ABK = 11,  // Perform MAC operation between [all banks] and [Global Buffer]
        ISR_AF = 12,       // Perform Activation Function operation on [all banks]
        ISR_EWMUL = 13,    // Element wise multiplication between 2 banks of 1 or all bank group(s)
        ISR_EWADD = 14,    // Element wise multiplication between 2 GPR addresses
        ISR_EOC = 15,      // End of compute for the current kernel
        MAX = 16
        // ISR_WR_HBK,    // [NOT_IMPLEMENTED] Write data from [GPR] to [8 banks]
        // ISR_WR_ABK,    // [NOT_IMPLEMENTED] Write data from [GPR] to [all banks]
        // ISR_WR_GPR,    // [NOT_IMPLEMENTED] Write data from [host] to [GPR]
        // ISR_MAC_HBK,   // [NOT_IMPLEMENTED] Perform MAC operation between [8 banks] and [Global Buffer]
    } opcode;

    // [NOT_IMPLEMENTED] Increament order for ISR_WR_SBK, ISR_WR_HBK, and ISR_WR_ABK operations
    // struct IncreamentOrder {
    //     enum : int {
    //         IO_ROW_COL, // {ROW_ADDR[13:0], COL_ADDR[5:0]}++
    //         IO_BK_COL,  // {BK_ADDR[3:0], COL_ADDR[5:0]}++
    //         IO_COL_CH,  // {COL_ADDR[3:0], BK_ADDR[5:0]}++
    //         MAX
    //     };
    // };

    int32_t opsize = -1;

    // [NOT_IMPLEMENTED] Source of ISR_WR_GB is host (false) or GPR (true)
    // bool use_GPR = false;

    // GPR address 0 USED for: ISR_WR_SBK, ISR_WR_GB, ISR_WR_BIAS, ISR_RD_MAC, ISR_EWADD
    // GPR address 1 USED for: ISR_EWADD
    Addr_t GPR_addr_0 = -1;
    Addr_t GPR_addr_1 = -1;

    // This request will be broadcasted/multicasted to the channels
    // whose bit is set in channel mask.
    // NOT USED in ISR_EWADD.
    // Channel mask must show 1 channel in ISR_WR_SBK and ISR_RD_SBK ISRs
    int32_t channel_mask = -1;

    // This request will be sent to a specific bank. USED only in single-bank ISRs, i.e.,
    // ISR_WR_SBK, ISR_RD_SBK, ISR_COPY_BKGB, ISR_COPY_GBBK, ISR_MAC_SBK, and ISR_EWMUL
    int16_t bank_index = -1;

    // Bank row and column address, NOT USED for the operations without DRAM bank source/dst:
    // ISR_WR_BIAS, ISR_WR_AFLUT, ISR_RD_MAC, ISR_RD_AF, ISR_AF, and ISR_EWADD
    // In addition, row_addr is NOT USED for ISR_WR_GB
    int32_t row_addr = -1;
    int32_t col_addr = -1;

    // Thread (register) index (0 or 1) for MAC and AF results
    int8_t thread_index = -1;

    // Broadcast only USED for ISR_MAC_SBK and ISR_MAC_ABK
    // vector data for MAC is from GB (0) or next bank (1)
    int16_t broadcast = -1;

    // AFM only USED for ISR_AF
    // Activation Function mode selects AF (0-7)
    int16_t afm = -1;

    // ewmul bank group only USED for ISR_MAC_ABK
    // EWMUL in one bank group (0) or all bank groups (1)
    int16_t ewmul_bg = -1;

    int source_id = -1; // An identifier for where the request is coming from (e.g., which core)

    int command = -1;       // The command that need to be issued to progress the request
    int final_command = -1; // The final command that is needed to finish the request

    Clk_t arrive = -1; // Clock cycle when the request arrive at the memory controller
    Clk_t depart = -1; // Clock cycle when the request depart the memory controller

    int type_id = -1;

    std::function<void(Request &)> callback;

    Request(Addr_t addr, int type_id);
    Request(AddrVec_t addr_vec, int type_id);
    Request(Addr_t addr, int type_id, int source_id, std::function<void(Request &)> callback);

    const char *c_str();

    bool is_reader();
};

class AiMISR {

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

    std::map<Field, std::string> field_to_str = {
        {Field::opsize, "opsize"},
        {Field::GPR_addr_0, "GPR_addr_0"},
        {Field::GPR_addr_1, "GPR_addr_1"},
        {Field::channel_mask, "channel_mask"},
        {Field::bank_index, "bank_index"},
        {Field::row_addr, "row_addr"},
        {Field::col_addr, "col_addr"},
        {Field::thread_index, "thread_index"},
    };

    AiMISR(Request::Opcode opcode_ = Request::Opcode::MAX,
           std::vector<Field> legal_fields_ = {},
           bool channel_count_eq_one_ = false,
           bool AiM_DMA_blocking_ = false,
           std::string target_level_ = "")
        : opcode(opcode_),
          legal_fields(legal_fields_),
          channel_count_eq_one(channel_count_eq_one_),
          AiM_DMA_blocking(AiM_DMA_blocking_),
          target_level(target_level_) {}

    Request::Opcode opcode;

    std::vector<Field> legal_fields;

    bool channel_count_eq_one;
    bool AiM_DMA_blocking;

    std::string target_level;

    bool is_field_legal(Field field) {
        if (std::count(legal_fields.begin(), legal_fields.end(), field))
            return true;
        return false;
    }

    template <typename T>
    void is_field_value_legal(Field field, T value);
};

class AiMISRInfo {
private:
    static std::map<std::string, AiMISR> opcode_str_to_aim_ISR;
    static std::map<Request::Opcode, std::string> aim_opcode_to_str;

    static std::map<std::string, Request::Type> str_to_type;
    static std::map<Request::Type, std::string> type_to_str;

    static std::map<std::string, Request::MemAccessRegion> str_to_mem_access_region;
    static std::map<Request::MemAccessRegion, std::string> mem_access_region_to_str;

public:
    static void init() {
        aim_opcode_to_str[Request::Opcode::ISR_WR_SBK] = "ISR_WR_SBK";
        opcode_str_to_aim_ISR["ISR_WR_SBK"] = AiMISR(Request::Opcode::ISR_WR_SBK,
                                                     {AiMISR::Field::opsize,
                                                      AiMISR::Field::GPR_addr_0,
                                                      AiMISR::Field::channel_mask,
                                                      AiMISR::Field::bank_index,
                                                      AiMISR::Field::row_addr},
                                                     true,    // channel_count_eq_one
                                                     false,   // AiM_DMA_blocking
                                                     "column" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_WR_GB] = "ISR_WR_GB";
        opcode_str_to_aim_ISR["ISR_WR_GB"] = AiMISR(Request::Opcode::ISR_WR_GB,
                                                    {
                                                        AiMISR::Field::opsize,
                                                        AiMISR::Field::GPR_addr_0,
                                                        AiMISR::Field::channel_mask,
                                                    },
                                                    false,    // channel_count_eq_one
                                                    false,    // AiM_DMA_blocking
                                                    "channel" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_WR_BIAS] = "ISR_WR_BIAS";
        opcode_str_to_aim_ISR["ISR_WR_BIAS"] = AiMISR(Request::Opcode::ISR_WR_BIAS,
                                                      {
                                                          AiMISR::Field::GPR_addr_0,
                                                          AiMISR::Field::channel_mask,
                                                      },
                                                      false, // channel_count_eq_one
                                                      false, // AiM_DMA_blocking
                                                      "bank" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_WR_AFLUT] = "ISR_WR_AFLUT";
        opcode_str_to_aim_ISR["ISR_WR_AFLUT"] = AiMISR(Request::Opcode::ISR_WR_AFLUT,
                                                       {
                                                           AiMISR::Field::opsize,
                                                       },
                                                       false,   // channel_count_eq_one
                                                       false,   // AiM_DMA_blocking
                                                       "column" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_RD_MAC] = "ISR_RD_MAC";
        opcode_str_to_aim_ISR["ISR_RD_MAC"] = AiMISR(Request::Opcode::ISR_RD_MAC,
                                                     {
                                                         AiMISR::Field::GPR_addr_0,
                                                         AiMISR::Field::channel_mask,
                                                     },
                                                     false, // channel_count_eq_one
                                                     true,  // AiM_DMA_blocking
                                                     "bank" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_RD_AF] = "ISR_RD_AF";
        opcode_str_to_aim_ISR["ISR_RD_AF"] = AiMISR(Request::Opcode::ISR_RD_AF,
                                                    {
                                                        AiMISR::Field::GPR_addr_0,
                                                        AiMISR::Field::channel_mask,
                                                    },
                                                    false, // channel_count_eq_one
                                                    true,  // AiM_DMA_blocking
                                                    "bank" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_RD_SBK] = "ISR_RD_SBK";
        opcode_str_to_aim_ISR["ISR_RD_SBK"] = AiMISR(Request::Opcode::ISR_RD_SBK,
                                                     {
                                                         AiMISR::Field::opsize,
                                                         AiMISR::Field::GPR_addr_0,
                                                         AiMISR::Field::channel_mask,
                                                         AiMISR::Field::bank_index,
                                                         AiMISR::Field::row_addr,
                                                     },
                                                     true,    // channel_count_eq_one
                                                     true,    // AiM_DMA_blocking
                                                     "column" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_COPY_BKGB] = "ISR_COPY_BKGB";
        opcode_str_to_aim_ISR["ISR_COPY_BKGB"] = AiMISR(Request::Opcode::ISR_COPY_BKGB,
                                                        {
                                                            AiMISR::Field::opsize,
                                                            AiMISR::Field::channel_mask,
                                                            AiMISR::Field::bank_index,
                                                            AiMISR::Field::row_addr,
                                                        },
                                                        false,   // channel_count_eq_one
                                                        false,   // AiM_DMA_blocking
                                                        "column" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_COPY_GBBK] = "ISR_COPY_GBBK";
        opcode_str_to_aim_ISR["ISR_COPY_GBBK"] = AiMISR(Request::Opcode::ISR_COPY_GBBK,
                                                        {
                                                            AiMISR::Field::opsize,
                                                            AiMISR::Field::channel_mask,
                                                            AiMISR::Field::bank_index,
                                                            AiMISR::Field::row_addr,
                                                        },
                                                        false,   // channel_count_eq_one
                                                        false,   // AiM_DMA_blocking
                                                        "column" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_MAC_SBK] = "ISR_MAC_SBK";
        opcode_str_to_aim_ISR["ISR_MAC_SBK"] = AiMISR(Request::Opcode::ISR_MAC_SBK,
                                                      {
                                                          AiMISR::Field::opsize,
                                                          AiMISR::Field::channel_mask,
                                                          AiMISR::Field::bank_index,
                                                          AiMISR::Field::row_addr,
                                                      },
                                                      false, // channel_count_eq_one
                                                      false, // AiM_DMA_blocking
                                                      "bank" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_MAC_ABK] = "ISR_MAC_ABK";
        opcode_str_to_aim_ISR["ISR_MAC_ABK"] = AiMISR(Request::Opcode::ISR_MAC_ABK,
                                                      {
                                                          AiMISR::Field::opsize,
                                                          AiMISR::Field::channel_mask,
                                                          AiMISR::Field::row_addr,
                                                      },
                                                      false, // channel_count_eq_one
                                                      false, // AiM_DMA_blocking
                                                      "bank" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_AF] = "ISR_AF";
        opcode_str_to_aim_ISR["ISR_AF"] = AiMISR(Request::Opcode::ISR_AF,
                                                 {
                                                     AiMISR::Field::channel_mask,
                                                 },
                                                 false, // channel_count_eq_one
                                                 false, // AiM_DMA_blocking
                                                 "bank" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_EWMUL] = "ISR_EWMUL";
        opcode_str_to_aim_ISR["ISR_EWMUL"] = AiMISR(Request::Opcode::ISR_EWMUL,
                                                    {
                                                        AiMISR::Field::opsize,
                                                        AiMISR::Field::channel_mask,
                                                        AiMISR::Field::row_addr,
                                                    },
                                                    false,   // channel_count_eq_one
                                                    false,   // AiM_DMA_blocking
                                                    "column" // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_EWADD] = "ISR_EWADD";
        opcode_str_to_aim_ISR["ISR_EWADD"] = AiMISR(Request::Opcode::ISR_EWADD,
                                                    {AiMISR::Field::opsize,
                                                     AiMISR::Field::GPR_addr_0,
                                                     AiMISR::Field::GPR_addr_1},
                                                    false, // channel_count_eq_one
                                                    false, // AiM_DMA_blocking
                                                    "DMA"  // target_level
        );

        aim_opcode_to_str[Request::Opcode::ISR_EOC] = "ISR_EOC";
        opcode_str_to_aim_ISR["ISR_EOC"] = AiMISR(Request::Opcode::ISR_EOC,
                                                  {},
                                                  false, // channel_count_eq_one
                                                  true,  // AiM_DMA_blocking
                                                  "DMA"  // target_level
        );

        str_to_type["R"] = Request::Type::Read;
        type_to_str[Request::Type::Read] = "R";
        str_to_type["W"] = Request::Type::Write;
        type_to_str[Request::Type::Write] = "W";
        str_to_type["AiM"] = Request::Type::AIM;
        type_to_str[Request::Type::AIM] = "AiM";

        str_to_mem_access_region["GPR"] = Request::MemAccessRegion::GPR;
        mem_access_region_to_str[Request::MemAccessRegion::GPR] = "GPR";
        str_to_mem_access_region["CFR"] = Request::MemAccessRegion::CFR;
        mem_access_region_to_str[Request::MemAccessRegion::CFR] = "CFR";
        str_to_mem_access_region["MEM"] = Request::MemAccessRegion::MEM;
        mem_access_region_to_str[Request::MemAccessRegion::MEM] = "MEM";
    }

    static bool type_valid(std::string type_str) {
        if (str_to_type.size() == 0) {
            printf("AiMISRInfo not initialized!");
            exit(-1);
        }

        if (str_to_type.find(type_str) == str_to_type.end())
            return false;
        return true;
    }

    static bool type_valid(Request::Type type) {
        if (type_to_str.size() == 0) {
            printf("AiMISRInfo not initialized!");
            exit(-1);
        }

        if (type_to_str.find(type) == type_to_str.end())
            return false;
        return true;
    }

    static Request::Type convert_str_to_type(std::string type_str) {
        if (type_valid(type_str) == false) {
            printf("Trace: unknown type %s!", type_str.c_str());
            exit(-1);
        }
        return str_to_type[type_str];
    }

    static std::string convert_type_to_str(Request::Type type) {
        if (type_valid(type) == false) {
            printf("Trace: unknown type %d!", (int)type);
            exit(-1);
        }
        return type_to_str[type];
    }

    static bool AiM_opcode_valid(std::string AiM_opcode_str) {
        if (opcode_str_to_aim_ISR.size() == 0) {
            printf("AiMISRInfo not initialized!");
            exit(-1);
        }

        if (opcode_str_to_aim_ISR.find(AiM_opcode_str) == opcode_str_to_aim_ISR.end())
            return false;
        return true;
    }

    static bool AiM_opcode_valid(Request::Opcode AiM_opcode) {
        if (aim_opcode_to_str.size() == 0) {
            printf("AiMISRInfo not initialized!");
            exit(-1);
        }

        if (aim_opcode_to_str.find(AiM_opcode) == aim_opcode_to_str.end())
            return false;
        return true;
    }

    static AiMISR convert_opcode_str_to_AiM_ISR(std::string AiM_opcode_str) {
        if (AiM_opcode_valid(AiM_opcode_str) == false) {
            printf("Trace: unknown AiM opcode %s!", AiM_opcode_str.c_str());
            exit(-1);
        }
        return opcode_str_to_aim_ISR[AiM_opcode_str];
    }

    static AiMISR convert_AiM_opcode_to_AiM_ISR(Request::Opcode AiM_opcode) {
        return opcode_str_to_aim_ISR[convert_AiM_opcode_to_str(AiM_opcode)];
    }

    static std::string convert_AiM_opcode_to_str(Request::Opcode AiM_opcode) {
        if (AiM_opcode_valid(AiM_opcode) == false) {
            printf("Trace: unknown AiM opcode %d!", (int)AiM_opcode);
            exit(-1);
        }
        return aim_opcode_to_str[AiM_opcode];
    }

    static bool mem_access_region_valid(std::string mem_access_region_str) {
        if (str_to_mem_access_region.size() == 0) {
            printf("AiMISRInfo not initialized!");
            exit(-1);
        }

        if (str_to_mem_access_region.find(mem_access_region_str) == str_to_mem_access_region.end())
            return false;
        return true;
    }

    static bool mem_access_region_valid(Request::MemAccessRegion mem_access_region) {
        if (mem_access_region_to_str.size() == 0) {
            printf("AiMISRInfo not initialized!");
            exit(-1);
        }

        if (mem_access_region_to_str.find(mem_access_region) == mem_access_region_to_str.end())
            return false;
        return true;
    }

    static Request::MemAccessRegion convert_str_to_mem_access_region(std::string mem_access_region_str) {
        if (mem_access_region_valid(mem_access_region_str) == false) {
            printf("Trace: unknown mem_access_region %s!", mem_access_region_str.c_str());
            exit(-1);
        }
        return str_to_mem_access_region[mem_access_region_str];
    }

    static std::string convert_mem_access_region_to_str(Request::MemAccessRegion mem_access_region) {
        if (mem_access_region_valid(mem_access_region) == false) {
            printf("Trace: unknown mem_access_region %d!", (int)mem_access_region);
            exit(-1);
        }
        return mem_access_region_to_str[mem_access_region];
    }
};

static AiMISRInfo AiM_host_request_info();

struct ReqBuffer {
    std::list<Request> buffer;
    size_t max_size = 32;

    using iterator = std::list<Request>::iterator;
    iterator begin() { return buffer.begin(); };
    iterator end() { return buffer.end(); };

    size_t size() const { return buffer.size(); }

    bool enqueue(const Request &request) {
        if (buffer.size() <= max_size) {
            buffer.push_back(request);
            return true;
        } else {
            return false;
        }
    }

    void remove(iterator it) {
        buffer.erase(it);
    }
};

template <typename T>
void AiMISR::is_field_value_legal(AiMISR::Field field, T value) {
    if (is_field_legal(field)) {
        if (value == (T)-1) {
            printf("Trace: opcode %s must be provided with field %s!", AiMISRInfo::convert_AiM_opcode_to_str(opcode).c_str(), field_to_str[field].c_str());
            exit(-1);
        }
    } else {
        if (value != (T)-1) {
            printf("Trace: opcode %s does not accept field %s!", AiMISRInfo::convert_AiM_opcode_to_str(opcode).c_str(), field_to_str[field].c_str());
            exit(-1);
        }
    }
}

} // namespace Ramulator

#endif // RAMULATOR_BASE_REQUEST_H