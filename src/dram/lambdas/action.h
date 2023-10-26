#ifndef RAMULATOR_DRAM_LAMBDAS_ACTION_H
#define RAMULATOR_DRAM_LAMBDAS_ACTION_H

#include <cassert>
#include <spdlog/spdlog.h>

#include "dram/node.h"

namespace Ramulator {
namespace Lambdas {

template <class>
inline constexpr bool false_v = false;

namespace Action {
namespace Bank {
template <class T>
void ACT(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[node->m_level + 1];
    assert(node->m_state == T::m_states["Closed"]);
    assert(node->m_row_state.size() == 0);
    node->m_state = T::m_states["Opened"];
    node->m_row_state[target_id] = T::m_states["Opened"];
};

template <class T>
void PRE(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    node->m_state = T::m_states["Closed"];
    node->m_row_state.clear();
};
} // namespace Bank

namespace BankGroup {
template <class T>
void PREsb(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[node->m_level + 1];
    typename T::Node *rank = node->m_parent_node;
    for (auto bg : rank->m_child_nodes) {
        for (auto bank : bg->m_child_nodes) {
            if (bank->m_node_id == target_id) {
                bank->m_state = T::m_states["Closed"];
                bank->m_row_state.clear();
            }
        }
    }
};

template <class T>
void SameBankActions(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[node->m_level + 1];
    AddrVec_t same_bank_addr(T::m_levels.size(), -1);
    same_bank_addr[T::m_levels["bank"]] = target_id;

    typename T::Node *rank = node->m_parent_node;
    for (auto bg : rank->m_child_nodes) {
        for (auto bank : bg->m_child_nodes) {
            if (bank->m_node_id == target_id) { // Ch  Ra  Bg     Ba      Ro  Co
                bank->update_timing(cmd, same_bank_addr, clk);
            }
        }
    }
}
template <class T>
void ACT4b(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    assert(node->m_level == T::m_levels["bankgroup"]);
    int target_id = addr_vec[T::m_levels["row"]];
    for (auto bank : node->m_child_nodes) {
        // assert(bank->m_state == T::m_states["Closed"]);
        // assert(bank->m_row_state.size() == 0);
        bank->m_state = T::m_states["Opened"];
        bank->m_row_state[target_id] = T::m_states["Opened"];
    }
}
template <class T>
void PRE4b(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    assert(node->m_level == T::m_levels["bankgroup"]);
    for (auto bank : node->m_child_nodes) {
        bank->m_state = T::m_states["Closed"];
        bank->m_row_state.clear();
    }
}
} // namespace BankGroup

namespace Rank {
template <class T>
void PREab(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    if constexpr (T::m_levels["bank"] - T::m_levels["rank"] == 1) {
        for (auto bank : node->m_child_nodes) {
            bank->m_state = T::m_states["Closed"];
            bank->m_row_state.clear();
        }
    } else if constexpr (T::m_levels["bank"] - T::m_levels["rank"] == 2) {
        for (auto bg : node->m_child_nodes) {
            for (auto bank : bg->m_child_nodes) {
                bank->m_state = T::m_states["Closed"];
                bank->m_row_state.clear();
            }
        }
    } else {
        static_assert(
            false_v<T>,
            "[Action::Rank] Unsupported organization. Please write your own PREab function.");
    }
};

template <class T>
void PREsb(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[node->m_level + 1];
    for (auto bg : node->m_child_nodes) {
        for (auto bank : bg->m_child_nodes) {
            if (bank->m_node_id == target_id) {
                bank->m_state = T::m_states["Closed"];
                bank->m_row_state.clear();
            }
        }
    }
};

} // namespace Rank

namespace Channel {
// TODO: Make these nicer...
template <class T>
void ACTab(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    int target_id = addr_vec[T::m_levels["row"]];
    if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 2) {
        for (auto bg : node->m_child_nodes) {
            for (auto bank : bg->m_child_nodes) {
                // assert(bank->m_state == T::m_states["Closed"]);
                // assert(bank->m_row_state.size() == 0);
                bank->m_state = T::m_states["Opened"];
                bank->m_row_state[target_id] = T::m_states["Opened"];
            }
        }
    } else if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 3) {
        for (auto pc : node->m_child_nodes) {
            for (auto bg : pc->m_child_nodes) {
                for (auto bank : bg->m_child_nodes) {
                    // assert(bank->m_state == T::m_states["Closed"]);
                    // assert(bank->m_row_state.size() == 0);
                    bank->m_state = T::m_states["Opened"];
                    bank->m_row_state[target_id] = T::m_states["Opened"];
                }
            }
        }
    } else {
        static_assert(
            false_v<T>,
            "[Action::Rank] Unsupported organization. Please write your own PREab function.");
    }
};
template <class T>
void PREab(typename T::Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
    if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 2) {
        for (auto bg : node->m_child_nodes) {
            for (auto bank : bg->m_child_nodes) {
                bank->m_state = T::m_states["Closed"];
                bank->m_row_state.clear();
            }
        }
    } else if constexpr (T::m_levels["bank"] - T::m_levels["channel"] == 3) {
        for (auto pc : node->m_child_nodes) {
            for (auto bg : pc->m_child_nodes) {
                for (auto bank : bg->m_child_nodes) {
                    bank->m_state = T::m_states["Closed"];
                    bank->m_row_state.clear();
                }
            }
        }
    } else {
        static_assert(
            false_v<T>,
            "[Action::Rank] Unsupported organization. Please write your own PREab function.");
    }
};
} // namespace Channel
} // namespace Action
} // namespace Lambdas
}; // namespace Ramulator

#endif // RAMULATOR_DRAM_LAMBDAS_ACTION_H