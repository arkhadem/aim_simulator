#include "base/request.h"

namespace Ramulator {

Request::Request(Addr_t addr, int type_id) : addr(addr), type_id(type_id), type((Type)type_id){};

Request::Request(AddrVec_t addr_vec, int type_id) : addr_vec(addr_vec), type_id(type_id), type((Type)type_id){};

Request::Request(Addr_t addr, int type_id, int source_id, std::function<void(Request &)> callback) : addr(addr), type_id(type_id), type((Type)type_id), source_id(source_id), callback(callback){};

bool Request::is_reader() {
    return (type == Type::Read) ||
           ((type == Type::AIM) &&
            (AiMISRInfo::convert_AiM_opcode_to_AiM_ISR(opcode).AiM_DMA_blocking == true));
}

const char *Request::c_str() {
    std::stringstream req_stream;
    char *name = new char[1024];
    if (type == Type::AIM) {
        req_stream << "Request[Type(" << AiMISRInfo::convert_AiM_opcode_to_str(opcode) << "), ";
        if (opsize != -1)
            req_stream << "Opsize(" << opsize << "), ";
        if (GPR_addr_0 != -1)
            req_stream << "GPR0(" << GPR_addr_0 << "), ";
        if (GPR_addr_1 != -1)
            req_stream << "GPR1(" << GPR_addr_1 << "), ";
        if (channel_mask != -1)
            req_stream << "CHMask(" << channel_mask << "), ";
        if (bank_index != -1)
            req_stream << "BA(" << bank_index << "), ";
        if (row_addr != -1)
            req_stream << "RO(" << row_addr << "), ";
        if (col_addr != -1)
            req_stream << "CO(" << col_addr << "), ";
        if (thread_index != -1)
            req_stream << "Tid(" << thread_index << "), ";
    } else {
        if (type == Type::Read) {
            req_stream << "Request[Type(Read), Region(";
        } else if (type == Type::Write) {
            req_stream << "Request[Type(Write), Region(";
        }
        if (mem_access_region == MemAccessRegion::MEM)
            req_stream << "MEM), ";
        else if (mem_access_region == MemAccessRegion::GPR)
            req_stream << "GPR), ";
        else
            req_stream << "CFR), ";
    }
    if (host_req_id != -1)
        req_stream << "hostID(" << host_req_id << "), ";
    if (AiM_req_id != -1)
        req_stream << "AiMID(" << AiM_req_id << "), ";
    if (addr != -1)
        req_stream << "Address(0x" << std::hex << addr << std::dec << "), ";
    if (addr_vec.size() != 0) {
        req_stream << "Address Vec(";
        if (addr_vec[0] != -1)
            req_stream << "CH: " << addr_vec[0] << ", ";
        if (addr_vec[1] != -1)
            req_stream << "BG: " << addr_vec[1] << ", ";
        if (addr_vec[2] != -1)
            req_stream << "BA: " << addr_vec[2] << ", ";
        if (addr_vec[3] != -1)
            req_stream << "RO: " << addr_vec[3] << ", ";
        if (addr_vec[4] != -1)
            req_stream << "CO: " << addr_vec[4] << "), ";
        req_stream << "), ";
    }
    req_stream << "]";

    strcpy(name, req_stream.str().c_str());
    return name;
}

} // namespace Ramulator
