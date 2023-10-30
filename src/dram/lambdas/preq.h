#ifndef RAMULATOR_DRAM_LAMBDAS_PREQ_H
#define RAMULATOR_DRAM_LAMBDAS_PREQ_H

#include "base/type.h"
#include <cassert>
#include <cstdio>
#include <spdlog/spdlog.h>

namespace Ramulator {
namespace Lambdas {
namespace Preq {
namespace Bank {
template <class T>
int RequireRowOpen(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[node->m_level + 1];
    switch (node->m_state) {
    case T::m_states["Closed"]:
        return T::m_commands["ACT"];
    case T::m_states["Opened"]: {
        if (node->m_row_state.find(target_id) != node->m_row_state.end()) {
            return cmd;
        } else {
            return T::m_commands["PRE"];
        }
    }
    default: {
        spdlog::error("[Preq::Bank] Invalid bank state for an RD/WR command!");
        std::exit(-1);
    }
    }
};

template <class T>
int RequireBankClosed(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    switch (node->m_state) {
    case T::m_states["Closed"]:
        return cmd;
    case T::m_states["Opened"]:
        return T::m_commands["PRE"];
    default: {
        spdlog::error("[Preq::Bank] Invalid bank state for an RD/WR command!");
        std::exit(-1);
    }
    }
};
} // namespace Bank

namespace Rank {
template <class T>
int RequireAllBanksClosed(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    if constexpr (T::m_levels["bank"] - T::m_levels["rank"] == 1) {
        for (auto bank : node->m_child_nodes) {
            if (bank->m_state == T::m_states["Closed"]) {
                continue;
            } else {
                return T::m_commands["PREA"];
            }
        }
    } else if constexpr (T::m_levels["bank"] - T::m_levels["rank"] == 2) {
        for (auto bg : node->m_child_nodes) {
            for (auto bank : bg->m_child_nodes) {
                if (bank->m_state == T::m_states["Closed"]) {
                    continue;
                } else {
                    return T::m_commands["PREA"];
                }
            }
        }
    }
    return cmd;
};

template <class T>
int RequireSameBanksClosed(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[node->m_level + 1];
    bool all_banks_ready = true;
    typename T::Node *rank = node->m_parent_node;
    for (auto bg : rank->m_child_nodes) {
        for (auto bank : bg->m_child_nodes) {
            if (bank->m_node_id == target_id) {
                all_banks_ready &= (bank->m_state == T::m_states["Closed"]);
            }
        }
    }
    if (all_banks_ready) {
        return cmd;
    } else {
        return T::m_commands["PREsb"];
    }
};
} // namespace Rank
namespace Channel {
template <class T>
int RequireAllRowsOpen(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[T::m_levels["row"]];
    // printf("RequireAllRowsOpen for CMD (%s), row %d\n", std::string(T::m_commands(cmd)).c_str(), addr_vec[T::m_levels["row"]]);
    assert(target_id != -1);
    bool any_closed = false;
    if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 2) {
        for (auto bg : node->m_child_nodes) {
            for (auto bank : bg->m_child_nodes) {
                if (bank->m_state == T::m_states["Closed"]) {
                    any_closed = true;
                    // printf("ch[%d] bg[%d] ba[%d] is closed!\n", node->m_node_id, bg->m_node_id, bank->m_node_id);
                } else {
                    if (bank->m_row_state.find(target_id) == bank->m_row_state.end()) {
                        // printf("ch[%d] bg[%d] ba[%d] is opened with another row!\n", node->m_node_id, bg->m_node_id, bank->m_node_id);
                        return T::m_commands["PREA"];
                    }
                }
            }
        }
    } else if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 3) {
        for (auto pc : node->m_child_nodes) {
            for (auto bg : pc->m_child_nodes) {
                for (auto bank : bg->m_child_nodes) {
                    if (bank->m_state == T::m_states["Closed"]) {
                        any_closed = true;
                    } else {
                        if (bank->m_row_state.find(target_id) == bank->m_row_state.end())
                            return T::m_commands["PREA"];
                    }
                }
            }
        }
    }
    if (any_closed == true)
        return T::m_commands["ACT16"];
    return cmd;
};
template <class T>
int RequireAllBanksClosed(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 2) {
        for (auto bg : node->m_child_nodes) {
            for (auto bank : bg->m_child_nodes) {
                if (bank->m_state == T::m_states["Closed"]) {
                    continue;
                } else {
                    return T::m_commands["PREA"];
                }
            }
        }
    } else if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 3) {
        for (auto pc : node->m_child_nodes) {
            for (auto bg : pc->m_child_nodes) {
                for (auto bank : bg->m_child_nodes) {
                    if (bank->m_state == T::m_states["Closed"]) {
                        continue;
                    } else {
                        return T::m_commands["PREA"];
                    }
                }
            }
        }
    }
    return cmd;
};
} // namespace Channel
} // namespace Preq
} // namespace Lambdas
}; // namespace Ramulator

#endif // RAMULATOR_DRAM_LAMBDAS_PREQ_H