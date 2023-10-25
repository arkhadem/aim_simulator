#include "base/request.h"

namespace Ramulator {

Request::Request(Addr_t addr, Request::Type type) : addr(addr), type(type){};

Request::Request(AddrVec_t addr_vec, Request::Type type) : addr_vec(addr_vec), type(type){};

Request::Request(Addr_t addr, Request::Type type, int source_id, std::function<void(Request &)> callback) : addr(addr), type(type), source_id(source_id), callback(callback){};

bool Request::is_reader() {
    return (type == Type::Read) ||
           ((type == Type::AIM) &&
            (AiMISRInfo::convert_AiM_opcode_to_AiM_ISR(opcode).AiM_DMA_blocking == true));
}

} // namespace Ramulator
