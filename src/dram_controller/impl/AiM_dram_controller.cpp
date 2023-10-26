#include "base/request.h"
#include "dram_controller/controller.h"
#include "memory_system/memory_system.h"
#include <cstdio>
#include <string>

namespace Ramulator {

class AiMDRAMController final : public IDRAMController, public Implementation {
    RAMULATOR_REGISTER_IMPLEMENTATION(IDRAMController, AiMDRAMController, "AiM", "AiM DRAM controller.");

private:
    std::deque<Request> pending; // A queue for read requests that are about to finish (callback after RL)

    ReqBuffer m_active_buffer;   // Buffer for requests being served. This has the highest priority
    ReqBuffer m_priority_buffer; // Buffer for high-priority requests (e.g., maintenance like refresh).
    ReqBuffer m_read_buffer;     // Read request buffer
    ReqBuffer m_write_buffer;    // Write request buffer
    ReqBuffer m_aim_buffer;      // AiM request buffer

    int m_row_addr_idx = -1;

    float m_wr_low_watermark;
    float m_wr_high_watermark;
    bool m_is_write_mode = false;

    std::vector<IControllerPlugin *> m_plugins;

    size_t s_num_row_hits = 0;
    size_t s_num_row_misses = 0;
    size_t s_num_row_conflicts = 0;

public:
    void init() override {
        m_wr_low_watermark = param<float>("wr_low_watermark").desc("Threshold for switching back to read mode.").default_val(0.2f);
        m_wr_high_watermark = param<float>("wr_high_watermark").desc("Threshold for switching to write mode.").default_val(0.8f);

        m_scheduler = create_child_ifce<IScheduler>();
        m_refresh = create_child_ifce<IRefreshManager>();

        if (m_config["plugins"]) {
            YAML::Node plugin_configs = m_config["plugins"];
            for (YAML::iterator it = plugin_configs.begin(); it != plugin_configs.end(); ++it) {
                m_plugins.push_back(create_child_ifce<IControllerPlugin>(*it));
            }
        }
    };

    void setup(IFrontEnd *frontend, IMemorySystem *memory_system) override {
        m_dram = memory_system->get_ifce<IDRAM>();
        m_row_addr_idx = m_dram->m_levels("row");
        m_priority_buffer.max_size = 512 * 3 + 32;
    };

    bool compare_addr_vec(Request req1, Request req2, int min_compared_level) {
        for (int level_idx = m_dram->m_levels("channel"); level_idx <= min_compared_level; level_idx++) {
            if (req1.addr_vec[level_idx] == -1)
                return true;
            if (req2.addr_vec[level_idx] == -1)
                return true;
            if (req1.addr_vec[level_idx] != req2.addr_vec[level_idx])
                return false;
        }
        return true;
    }

    bool send(Request &req) override {
        if (req.type == Request::Type::AIM) {
            if ((m_write_buffer.size() != 0) || (m_read_buffer.size() != 0))
                return false;
            req.final_command = m_dram->m_aim_request_translations((int)req.opcode);
        } else {
            if (m_aim_buffer.size() != 0)
                return false;
            req.final_command = m_dram->m_request_translations((int)req.type);
        }

        // // Forward existing write requests to incoming read requests
        // if (req.type == Request::Type::Read) {
        //     auto my_addr_comparator = [req](const Request &wreq) {
        //         return wreq.addr == req.addr;
        //     };
        //     auto my_addr_vec_comparator = [req, this](const Request &aim_req) {
        //         // The AiM request changes the bank data
        //         if (AiMISRInfo::convert_AiM_opcode_to_AiM_ISR(aim_req.opcode).target_level != "column")
        //             return false;
        //         return compare_addr_vec(aim_req, req, m_dram->m_levels("column"));
        //     };

        //     // Find the youngest write alias
        //     ReqBuffer::iterator write_alis_it = std::find_if(m_write_buffer.begin(), m_write_buffer.end(), my_addr_comparator);
        //     if (write_alis_it != m_write_buffer.end()) {

        //         // Find the youngest AiM collision
        //         ReqBuffer::iterator aim_alis_it = std::find_if(m_aim_buffer.begin(), m_aim_buffer.end(), my_addr_vec_comparator);

        //         // Write to Read forwarding only if [there is no AiM alias] or [write alias is younger than aim alias]
        //         if ((aim_alis_it != m_aim_buffer.end()) || (write_alis_it->AiM_req_id > aim_alis_it->AiM_req_id)) {

        //             // The request will depart at the next cycle
        //             req.depart = m_clk + 1;
        //             pending.push_back(req);
        //             return true;
        //         }
        //     }
        // }

        // Forward existing write requests to incoming read requests
        if (req.type == Request::Type::Read) {
            auto compare_addr = [req](const Request &wreq) {
                return wreq.addr == req.addr;
            };
            if (std::find_if(m_write_buffer.begin(), m_write_buffer.end(), compare_addr) != m_write_buffer.end()) {
                // The request will depart at the next cycle
                req.depart = m_clk + 1;
                pending.push_back(req);
                return true;
            }
        }

        // Else, enqueue them to corresponding buffer based on request type id
        bool is_success = false;
        req.arrive = m_clk;
        if (req.type == Request::Type::Read) {
            is_success = m_read_buffer.enqueue(req);
        } else if (req.type == Request::Type::Write) {
            is_success = m_write_buffer.enqueue(req);
        } else if (req.type == Request::Type::AIM) {
            is_success = m_aim_buffer.enqueue(req);
        } else {
            throw std::runtime_error("Invalid request type!");
        }
        if (!is_success) {
            // We could not enqueue the request
            req.arrive = -1;
            return false;
        }

        return true;
    };

    bool priority_send(Request &req) override {
        if (req.type == Request::Type::AIM)
            req.final_command = m_dram->m_aim_request_translations((int)req.opcode);
        else
            req.final_command = m_dram->m_request_translations((int)req.type);

        bool is_success = false;
        is_success = m_priority_buffer.enqueue(req);
        return is_success;
    }

    void tick() override {
        m_clk++;
        printf("[CH %d MC_CLK %ld]\n", m_channel_id, m_clk);

        // 1. Serve completed reads
        serve_completed_reads();

        m_refresh->tick();

        // 2. Try to find a request to serve.
        ReqBuffer::iterator req_it;
        ReqBuffer *buffer = nullptr;
        bool request_found = schedule_request(req_it, buffer);

        // // 3. Update all plugins
        // for (auto plugin : m_plugins) {
        //     plugin->update(request_found, req_it);
        // }

        // 4. Finally, issue the commands to serve the request
        if (request_found) {
            if (req_it->opcode == Request::Opcode::ISR_EOC) {
                req_it->depart = m_clk;
                pending.push_back(*req_it);
                buffer->remove(req_it);
                printf("EOC ready for callback\n");
            } else {
                // If we find a real request to serve
                printf("Issuing %s for %s\n", std::string(m_dram->m_commands(req_it->command)).c_str(), req_it->c_str());

                m_dram->issue_command(req_it->command, req_it->addr_vec);

                // If we are issuing the last command, set depart clock cycle and move the request to the pending queue
                if (req_it->command == req_it->final_command) {
                    if (req_it->is_reader()) {
                        req_it->depart = m_clk + m_dram->m_read_latency;
                        pending.push_back(*req_it);
                    }
                    // else if (req_it->type == Request::Type::Write) {
                    //     // TODO: Add code to update statistics
                    // }
                    buffer->remove(req_it);
                } else if (req_it->type != Request::Type::AIM) {
                    if (m_dram->m_command_meta(req_it->command).is_opening) {
                        m_active_buffer.enqueue(*req_it);
                        buffer->remove(req_it);
                    }
                }
            }
        }
    };

private:
    /**
     * @brief    Helper function to serve the completed read requests
     * @details
     * This function is called at the beginning of the tick() function.
     * It checks the pending queue to see if the top request has received data from DRAM.
     * If so, it finishes this request by calling its callback and poping it from the pending queue.
     */
    void serve_completed_reads() {
        if (pending.size()) {
            // Check the first pending request
            auto &req = pending[0];
            if (req.depart <= m_clk) {
                // Request received data from dram
                if (req.depart - req.arrive > 1) {
                    // Check if this requests accesses the DRAM or is being forwarded.
                    // TODO add the stats back
                }

                if (req.callback) {
                    // If the request comes from outside (e.g., processor), call its callback
                    req.callback(req);
                } else {
                    printf("Warning: %s doesn't have callback set but it is in the pending queue!\n", req.c_str());
                }
                // Finally, remove this request from the pending queue
                pending.pop_front();
            }
        };
    };

    /**
     * @brief    Checks if we need to switch to write mode
     * 
     */
    void set_write_mode() {
        if (!m_is_write_mode) {
            if ((m_write_buffer.size() > m_wr_high_watermark * m_write_buffer.max_size) || m_read_buffer.size() == 0) {
                m_is_write_mode = true;
            }
        } else {
            if ((m_write_buffer.size() < m_wr_low_watermark * m_write_buffer.max_size) && m_read_buffer.size() != 0) {
                m_is_write_mode = false;
            }
        }
    };

    /**
     * @brief    Helper function to find a request to schedule from the buffers.
     * 
     */
    bool schedule_request(ReqBuffer::iterator &req_it, ReqBuffer *&req_buffer) {
        bool request_found = false;
        // 2.1    First, check the act buffer to serve requests that are already activating (avoid useless ACTs)
        if (req_it = m_scheduler->get_best_request(m_active_buffer); req_it != m_active_buffer.end()) {
            if (m_dram->check_ready(req_it->command, req_it->addr_vec)) {
                request_found = true;
                req_buffer = &m_active_buffer;
            }
        }

        // 2.2    If no requests can be scheduled from the act buffer, check the rest of the buffers
        if (!request_found) {
            // 2.2.1    We first check the priority buffer to prioritize e.g., maintenance requests
            if (m_priority_buffer.size() != 0) {
                req_buffer = &m_priority_buffer;
                req_it = m_priority_buffer.begin();
                req_it->command = m_dram->get_preq_command(req_it->final_command, req_it->addr_vec);

                request_found = m_dram->check_ready(req_it->command, req_it->addr_vec);
                if ((request_found == false) & (m_priority_buffer.size() != 0)) {
                    return false;
                }
            }

            // 2.2.1    If no request to be scheduled in the priority buffer, check the read and write OR AiM buffers.
            if (!request_found) {
                if (m_aim_buffer.size() != 0) {
                    req_it = m_aim_buffer.begin();
                    if (req_it->opcode == Request::Opcode::ISR_EOC) {
                        req_buffer = &m_aim_buffer;
                        return true;
                    } else {
                        req_it->command = m_dram->get_preq_command(req_it->final_command, req_it->addr_vec);
                        request_found = m_dram->check_ready(req_it->command, req_it->addr_vec);
                        req_buffer = &m_aim_buffer;
                    }
                } else {
                    // Query the write policy to decide which buffer to serve
                    set_write_mode();
                    auto &buffer = m_is_write_mode ? m_write_buffer : m_read_buffer;
                    if (req_it = m_scheduler->get_best_request(buffer); req_it != buffer.end()) {
                        request_found = m_dram->check_ready(req_it->command, req_it->addr_vec);
                        req_buffer = &buffer;
                    }
                }
            }
        }

        // 2.3 If we find a request to schedule, we need to check if it will close an opened row in the active buffer.
        if (request_found) {
            if (m_dram->m_command_meta(req_it->command).is_closing) {
                std::vector<Addr_t> rowgroup((req_it->addr_vec).begin(), (req_it->addr_vec).begin() + m_row_addr_idx);

                // Search the active buffer with the row address (inkl. banks, etc.)
                for (auto _it = m_active_buffer.begin(); _it != m_active_buffer.end(); _it++) {
                    std::vector<Addr_t> _it_rowgroup(_it->addr_vec.begin(), _it->addr_vec.begin() + m_row_addr_idx);
                    if (rowgroup == _it_rowgroup) {
                        // Invalidate this scheduling outcome if we are to interrupt a request in the active buffer
                        request_found = false;
                    }
                }
            }
        }

        return request_found;
    }
};

} // namespace Ramulator