#ifndef RAMULATOR_BASE_REQUEST_H
#define RAMULATOR_BASE_REQUEST_H

#include <list>
#include <string>
#include <vector>

#include "base/base.h"

namespace Ramulator {

struct Request {
    Addr_t addr = -1;
    Addr_t addr_max = -1;
    Data_t data = -1;
    AddrVec_t addr_vec{};

    // Basic request id convention
    // 0 = Read, 1 = Write. The device spec defines all others
    enum class Type {
        Read = 0,
        Write = 1,
        AIM = 2,
        MAX = 3
    } type;

    enum class MemAccessRegion {
        GPR,
        CFR,
        MEM,
        MAX
    } mem_access_region;

    // ISR Command Opcodes
    enum class Opcode {
        ISR_WR_SBK, // Write [op_size * 256 bits] from [GPR * 32] to [a single bank] of channel [#channel_address]

        ISR_WR_GB, // Write the same [op_size * 256 bits] starting from [GPR * 32]
                   // to the [Global Buffer] of [channel_mask] channels
                   // [NOT_IMPLEMENTED] source could also be [host]

        ISR_WR_BIAS, // Write [opsize * 16 x 16-bits] bits from [GPR * 32]
                     // to [MAC accumulator of 16 banks] of [opsize or channel_mask] channels

        ISR_WR_AFLUT,  // Write activation function data from [host] to [?]
        ISR_RD_MAC,    // Read data from [MAC accumulator of all banks] to [GPR]
        ISR_RD_AF,     // Read data from [AF results of all banks] to [GPR]
        ISR_RD_SBK,    // [NOT_LISTED] Read data from [a single bank] to [host]
        ISR_COPY_BKGB, // Copy data from [a single bank] to [the Global Buffer]
        ISR_COPY_GBBK, // Copy data from [the Global Buffer] to [a single bank]
        ISR_MAC_SBK,   // Perform MAC operation between [a single bank] and [Global Buffer]
        ISR_MAC_ABK,   // Perform MAC operation between [all banks] and [Global Buffer]
        ISR_AF,        // Perform Activation Function operation on [all banks]
        ISR_EWMUL,     // Element wise multiplication between 2 banks of 1 or all bank group(s)
        ISR_EWADD,     // Element wise multiplication between 2 GPR addresses
        ISR_EOC,       // End of compute for the current kernel
        MAX
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

    uint16_t opsize = -1;

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
    uint16_t channel_mask = -1;

    // This request will be sent to a specific bank. USED only in single-bank ISRs, i.e.,
    // ISR_WR_SBK, ISR_RD_SBK, ISR_COPY_BKGB, ISR_COPY_GBBK, ISR_MAC_SBK, and ISR_EWMUL
    uint16_t bank_index = -1;

    // Bank row and column address, NOT USED for the operations without DRAM bank source/dst:
    // ISR_WR_BIAS, ISR_WR_AFLUT, ISR_RD_MAC, ISR_RD_AF, ISR_AF, and ISR_EWADD
    // In addition, row_addr is NOT USED for ISR_WR_GB
    uint32_t row_addr = -1;
    uint32_t col_addr = -1;

    // Thread (register) index (0 or 1) for MAC and AF results
    uint8_t thread_index = -1;

    int source_id = -1; // An identifier for where the request is coming from (e.g., which core)

    int command = -1;       // The command that need to be issued to progress the request
    int final_command = -1; // The final command that is needed to finish the request

    Clk_t arrive = -1; // Clock cycle when the request arrive at the memory controller
    Clk_t depart = -1; // Clock cycle when the request depart the memory controller

    std::function<void(Request &)> callback;

    Request();
    Request(Addr_t addr, Type type);
    Request(AddrVec_t addr_vec, Type type);
    Request(Addr_t addr, Type type, int source_id, std::function<void(Request &)> callback);
};

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

} // namespace Ramulator

#endif // RAMULATOR_BASE_REQUEST_H