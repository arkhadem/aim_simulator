#include "dram/dram.h"
#include "dram/lambdas.h"

namespace Ramulator {

class LPDDR5 : public IDRAM, public Implementation {
    RAMULATOR_REGISTER_IMPLEMENTATION(IDRAM, LPDDR5, "LPDDR5", "LPDDR5 Device Model")

public:
    inline static const std::map<std::string, Organization> org_presets = {
        //   name           density   DQ   Ch Ra Bg Ba   Ro     Co
        {"LPDDR5_2Gb_x16", {2 << 10, 16, {1, 1, 4, 4, 1 << 13, 1 << 10}}},
        {"LPDDR5_4Gb_x16", {4 << 10, 16, {1, 1, 4, 4, 1 << 14, 1 << 10}}},
        {"LPDDR5_8Gb_x16", {8 << 10, 16, {1, 1, 4, 4, 1 << 15, 1 << 10}}},
        {"LPDDR5_16Gb_x16", {16 << 10, 16, {1, 1, 4, 4, 1 << 16, 1 << 10}}},
        {"LPDDR5_32Gb_x16", {32 << 10, 16, {1, 1, 4, 4, 1 << 17, 1 << 10}}},
        {"LPDDR5_AiM_org", {32 << 10, 16, {32, 1, 4, 4, 1 << 17, 1 << 10}}},
    };

    inline static const std::map<std::string, std::vector<int>> timing_presets = {
        {"LPDDR5_6400", // name
         {
             6400,   // rate
             4,      // nBL
             20,     // nCL
             15,     // nRCD
             0,      // nRCDRDMAC
             0,      // nRCDEWMUL
             0,      // nRCDRDAF
             0,      // nRCDRDCP
             17,     // nRCDWRCP
             17,     // nRPab
             15,     // nRPpb
             34,     // nRAS
             30,     // nRC
             28,     // nWR
             4,      // nRTP
             11,     // nCWL
             4,      // nCCD
             4,      // nRRD
             5,      // nWTRS
             10,     // nWTRL
             16,     // nFAW
             2,      // nPPD
             -1,     // nRFCab
             -1,     // nRFCpb
             -1,     // nREFI
             -1,     // nPBR2PBR
             -1,     // nPBR2ACT
             2,      // nCS
             0,      // nCLREG
             0,      // nCLGB
             0,      // nCWLREG
             0,      // nCWLGB
             0,      // nWPRE
             0,      // nMODCH
             1250}}, // tCK_ps

        {"LPDDR5_AiM_timing", // name
         {
             6400,   // rate
             4,      // nBL
             20,     // nCL
             15,     // nRCD
             56,     // nRCDRDMAC (from GDDR6)
             25,     // nRCDEWMUL (from GDDR6)
             86,     // nRCDRDAF (from GDDR6)
             66,     // nRCDRDCP (from GDDR6)
             48,     // nRCDWRCP (from GDDR6)
             17,     // nRPab
             15,     // nRPpb
             34,     // nRAS
             30,     // nRC
             28,     // nWR
             4,      // nRTP
             11,     // nCWL
             4,      // nCCD
             4,      // nRRD
             5,      // nWTRS
             10,     // nWTRL
             16,     // nFAW
             2,      // nPPD
             -1,     // nRFCab
             -1,     // nRFCpb
             -1,     // nREFI
             -1,     // nPBR2PBR
             -1,     // nPBR2ACT
             2,      // nCS
             0,      // nCLREG (from GDDR6)
             1,      // nCLGB (from GDDR6)
             1,      // nCWLREG (from GDDR6)
             1,      // nCWLGB (from GDDR6)
             1,      // nWPRE (from GDDR6)
             32,     // nMODCH (from GDDR6)
             1250}}, // tCK_ps
    };

    /************************************************
   *                Organization
   ***********************************************/
    const int m_internal_prefetch_size = 8;

    inline static constexpr ImplDef m_levels = {
        "channel",
        "rank",
        "bankgroup",
        "bank",
        "row",
        "column",
    };

    /************************************************
   *             Requests & Commands
   ***********************************************/
    inline static constexpr ImplDef m_commands = {
        "ACT-1",
        "ACT-2",
        "PRE",
        "PREA",
        "CASRD",
        "CASWR", // WCK2CK Sync
        "CASWRGB",
        "CASWRMAC16",
        "CASRDMAC16",
        "CASRDAF16",
        "CASWRA16",
        "RD",
        "WR",
        "RDA",
        "WRA",
        "REFab",
        "REFpb",
        "RFMab",
        "RFMpb",
        "ACT4-1",
        "ACT16-1",
        "ACT4-2",
        "ACT16-2",
        "PRE4",
        "MAC",
        "MAC16",
        "AF16",
        "EWMUL16",
        "RDCP",
        "WRCP",
        "WRGB",
        "RDMAC16",
        "RDAF16",
        "WRMAC16",
        "WRA16",
        "TMOD",
        "SYNC",
        "EOC",
        "UNKNOWN"};

    inline static const ImplLUT m_command_scopes = LUT(
        m_commands, m_levels, {
                                  {"ACT-1", "row"},
                                  {"ACT-2", "row"},
                                  {"PRE", "bank"},
                                  {"PREA", "rank"},
                                  {"CASRD", "rank"},
                                  {"CASWR", "rank"},
                                  {"CASWRGB", "rank"},
                                  {"CASWRMAC16", "rank"},
                                  {"CASRDMAC16", "rank"},
                                  {"CASRDAF16", "rank"},
                                  {"CASWRA16", "rank"},
                                  {"RD", "column"},
                                  {"WR", "column"},
                                  {"RDA", "column"},
                                  {"WRA", "column"},
                                  {"REFab", "rank"},
                                  {"REFpb", "rank"},
                                  {"RFMab", "rank"},
                                  {"RFMpb", "rank"},
                                  {"ACT16-1", "rank"},
                                  {"ACT4-1", "bankgroup"},
                                  {"ACT16-2", "rank"},
                                  {"ACT4-2", "bankgroup"},
                                  {"PRE4", "bankgroup"},
                                  {"MAC", "column"},
                                  {"MAC16", "rank"},
                                  {"AF16", "rank"},
                                  {"EWMUL16", "rank"},
                                  {"RDCP", "column"},
                                  {"WRCP", "column"},
                                  {"WRGB", "rank"},
                                  {"RDMAC16", "rank"},
                                  {"RDAF16", "rank"},
                                  {"WRMAC16", "rank"},
                                  {"WRA16", "rank"},
                                  {"TMOD", "rank"},
                                  {"SYNC", "rank"},
                                  {"EOC", "rank"},
                              });

    inline static const ImplLUT m_command_meta = LUT<DRAMCommandMeta>(
        m_commands, {
                        // open?   close?   access?  refresh?
                        {"ACT-1", {false, false, false, false}},
                        {"ACT-2", {true, false, false, false}},
                        {"PRE", {false, true, false, false}},
                        {"PREA", {false, true, false, false}},
                        {"CASRD", {false, false, false, false}},
                        {"CASWR", {false, false, false, false}},
                        {"CASWRGB", {false, false, false, false}},
                        {"CASWRMAC16", {false, false, false, false}},
                        {"CASRDMAC16", {false, false, false, false}},
                        {"CASRDAF16", {false, false, false, false}},
                        {"CASWRA16", {false, false, false, false}},
                        {"RD", {false, false, true, false}},
                        {"WR", {false, false, true, false}},
                        {"RDA", {false, true, true, false}},
                        {"WRA", {false, true, true, false}},
                        {"REFab", {false, false, false, true}},
                        {"REFpb", {false, false, false, true}},
                        {"RFMab", {false, false, false, true}},
                        {"RFMpb", {false, false, false, true}},
                        {"ACT4-1", {false, false, false, false}},
                        {"ACT16-1", {false, false, false, false}},
                        {"ACT4-2", {true, false, false, false}},
                        {"ACT16-2", {true, false, false, false}},
                        {"PRE4", {false, true, false, false}},
                        {"MAC", {false, false, true, false}},
                        {"MAC16", {false, false, true, false}},
                        {"AF16", {false, false, false, false}},
                        {"EWMUL16", {false, false, true, false}},
                        {"RDCP", {false, false, true, false}},
                        {"WRCP", {false, false, true, false}},
                        {"WRGB", {false, false, false, false}},
                        {"RDMAC16", {false, false, false, false}},
                        {"RDAF16", {false, false, false, false}},
                        {"WRMAC16", {false, false, false, false}},
                        {"WRA16", {false, true, true, false}},
                        {"TMOD", {false, false, false, false}},
                        {"SYNC", {false, false, false, false}},
                        {"EOC", {false, false, false, false}},
                    });

    inline static constexpr ImplDef m_requests = {
        "read16",
        "write16",
        "all-bank-refresh",
        "per-bank-refresh"};

    inline static constexpr ImplDef m_aim_requests = {
        "MIN",
        "ISR_WR_SBK",
        "ISR_WR_GB",
        "ISR_WR_BIAS",
        "ISR_WR_AFLUT",
        "ISR_RD_MAC",
        "ISR_RD_AF",
        "ISR_RD_SBK",
        "ISR_COPY_BKGB",
        "ISR_COPY_GBBK",
        "ISR_MAC_SBK",
        "ISR_MAC_ABK",
        "ISR_AF",
        "ISR_EWMUL",
        "ISR_EWADD",
        "ISR_WR_ABK",
        "ISR_SYNC",
        "ISR_EOC",
        "MAX"};

    inline static const ImplLUT m_request_translations = LUT(
        m_requests, m_commands, {
                                    {"read16", "RD"},              // Read single bank
                                    {"write16", "WR"},             // Write single bank
                                    {"all-bank-refresh", "REFab"}, // Referesh all banks
                                    {"per-bank-refresh", "REFpb"}, // Referesh single bank
                                });

    inline static const ImplLUT m_aim_request_translations = LUT(
        m_aim_requests, m_commands, {
                                        {"MIN", "UNKNOWN"},          // 0 - Unknown and illegal
                                        {"ISR_WR_SBK", "WR"},        // 1 - Write single bank
                                        {"ISR_WR_GB", "WRGB"},       // 2 - Write global buffer
                                        {"ISR_WR_BIAS", "WRMAC16"},  // 3 - Write all MAC registers
                                        {"ISR_WR_AFLUT", "UNKNOWN"}, // 4 - Unknown and illegal
                                        {"ISR_RD_MAC", "RDMAC16"},   // 5 - Read all MAC registers
                                        {"ISR_RD_AF", "RDAF16"},     // 6 - Read all AF16 registers
                                        {"ISR_RD_SBK", "RD"},        // 7 - Read single bank
                                        {"ISR_COPY_BKGB", "RDCP"},   // 8 - Copy from a bank to the global buffer
                                        {"ISR_COPY_GBBK", "WRCP"},   // 9 - Copy from the global buffer to a bank
                                        {"ISR_MAC_SBK", "MAC"},      // 10 - MAC single bank
                                        {"ISR_MAC_ABK", "MAC16"},    // 11 - MAC all bank
                                        {"ISR_AF", "AF16"},          // 12 - AF16 all banks
                                        {"ISR_EWMUL", "EWMUL16"},    // 14 - EWMUL16 all banks or 1 bank group
                                        {"ISR_EWADD", "UNKNOWN"},    // 15 - Unknown and illegal
                                        {"ISR_WR_ABK", "WRA16"},     // 1 - Write single bank
                                        {"ISR_SYNC", "SYNC"},        // 16 - Unknown and illegal
                                        {"ISR_EOC", "EOC"},          // 16 - Unknown and illegal
                                        {"MAX", "UNKNOWN"},          // 17 - Unknown and illegal
                                    });

    /************************************************
   *                   Timing
   ***********************************************/
    inline static constexpr ImplDef m_timings = {
        "rate",
        "nBL",
        "nCL",
        "nRCD",
        "nRCDRDMAC",
        "nRCDEWMUL",
        "nRCDRDAF",
        "nRCDRDCP",
        "nRCDWRCP",
        "nRPab",
        "nRPpb",
        "nRAS",
        "nRC",
        "nWR",
        "nRTP",
        "nCWL",
        "nCCD",
        "nRRD",
        "nWTRS",
        "nWTRL",
        "nFAW",
        "nPPD",
        "nRFCab",
        "nRFCpb",
        "nREFI",
        "nPBR2PBR",
        "nPBR2ACT",
        "nCS",
        "nCLREG",
        "nCLGB",
        "nCWLREG",
        "nCWLGB",
        "nWPRE",
        "nMODCH",
        "tCK_ps"};

    /************************************************
   *                 Node States
   ***********************************************/
    inline static constexpr ImplDef m_states = {
        //    ACT-1       ACT-2
        "Pre-Opened", "Opened", "Closed", "PowerUp", "N/A"};

    inline static const ImplLUT m_init_states = LUT(
        m_levels, m_states, {
                                {"channel", "N/A"},
                                {"rank", "PowerUp"},
                                {"bankgroup", "N/A"},
                                {"bank", "Closed"},
                                {"row", "Closed"},
                                {"column", "N/A"},
                            });

public:
    struct Node : public DRAMNodeBase<LPDDR5> {
        Clk_t m_final_synced_cycle = -1; // Extra CAS Sync command needed for RD/WR after this cycle

        Node(LPDDR5 *dram, Node *parent, int level, int id) : DRAMNodeBase<LPDDR5>(dram, parent, level, id) {};
    };
    std::vector<Node *> m_channels;

    FuncMatrix<ActionFunc_t<Node>> m_actions;
    FuncMatrix<PreqFunc_t<Node>> m_preqs;
    FuncMatrix<RowhitFunc_t<Node>> m_rowhits;
    FuncMatrix<RowopenFunc_t<Node>> m_rowopens;

public:
    void tick() override {
        m_clk++;
    };

    void init() override {
        RAMULATOR_DECLARE_SPECS();
        set_organization();
        set_timing_vals();

        set_actions();
        set_preqs();
        // set_rowhits();
        // set_rowopens();

        create_nodes();
    };

    void issue_command(int command, const AddrVec_t &addr_vec) override {
        int channel_id = addr_vec[m_levels["channel"]];
        m_channels[channel_id]->update_timing(command, addr_vec, m_clk);
        m_channels[channel_id]->update_states(command, addr_vec, m_clk);
        switch (command) {
        case m_commands["WRA16"]:
        case m_commands["PREA"]: {
            m_open_rows[channel_id] = 0;
            break;
        }
        case m_commands["PRE4"]: {
            int bankgroup_id = addr_vec[m_levels["bankgroup"]];
            for (int bank_id = bankgroup_id * 4; bank_id < (bankgroup_id + 1) * 4; bank_id++) {
                m_open_rows[channel_id] &= ~((uint16_t)(1 << bank_id));
            }
            break;
        }
        case m_commands["PRE"]:
        case m_commands["RDA"]:
        case m_commands["WRA"]: {
            int bank_id = addr_vec[m_levels["bank"]];
            m_open_rows[channel_id] &= ~((uint16_t)(1 << bank_id));
            break;
        }
        case m_commands["ACT16-2"]: {
            m_open_rows[channel_id] = (uint16_t)(0xFFFF);
            break;
        }
        case m_commands["ACT4-2"]: {
            int bankgroup_id = addr_vec[m_levels["bankgroup"]];
            for (int bank_id = bankgroup_id * 4; bank_id < (bankgroup_id + 1) * 4; bank_id++) {
                m_open_rows[channel_id] |= (uint16_t)(1 << bank_id);
            }
            break;
        }
        case m_commands["ACT-2"]: {
            int bank_id = addr_vec[m_levels["bank"]];
            m_open_rows[channel_id] |= (uint16_t)(1 << bank_id);
            break;
        }
        }
    };

    int get_preq_command(int command, const AddrVec_t &addr_vec) override {
        int channel_id = addr_vec[m_levels["channel"]];
        return m_channels[channel_id]->get_preq_command(command, addr_vec, m_clk);
    };

    bool check_ready(int command, const AddrVec_t &addr_vec) override {
        int channel_id = addr_vec[m_levels["channel"]];
        return m_channels[channel_id]->check_ready(command, addr_vec, m_clk);
    };

    bool check_rowbuffer_hit(int command, const AddrVec_t &addr_vec) override {
        int channel_id = addr_vec[m_levels["channel"]];
        return m_channels[channel_id]->check_rowbuffer_hit(command, addr_vec, m_clk);
    };

private:
    void set_organization() {
        // Channel width
        m_channel_width = param_group("org").param<int>("channel_width").default_val(32);

        // Organization
        m_organization.count.resize(m_levels.size(), -1);

        // Load organization preset if provided
        if (auto preset_name = param_group("org").param<std::string>("preset").optional()) {
            if (org_presets.count(*preset_name) > 0) {
                m_organization = org_presets.at(*preset_name);
            } else {
                throw ConfigurationError("Unrecognized organization preset \"{}\" in {}!", *preset_name, get_name());
            }
        }

        // Override the preset with any provided settings
        if (auto dq = param_group("org").param<int>("dq").optional()) {
            m_organization.dq = *dq;
        }

        for (int i = 0; i < m_levels.size(); i++) {
            auto level_name = m_levels(i);
            if (auto sz = param_group("org").param<int>(level_name).optional()) {
                m_organization.count[i] = *sz;
            }
        }

        if (auto density = param_group("org").param<int>("density").optional()) {
            m_organization.density = *density;
        }

        // Sanity check: is the calculated chip density the same as the provided one?
        size_t _density = size_t(m_organization.count[m_levels["bankgroup"]]) *
                          size_t(m_organization.count[m_levels["bank"]]) *
                          size_t(m_organization.count[m_levels["row"]]) *
                          size_t(m_organization.count[m_levels["column"]]) *
                          size_t(m_organization.dq);
        _density >>= 20;
        if (m_organization.density != _density) {
            throw ConfigurationError(
                "Calculated {} chip density {} Mb does not equal the provided density {} Mb!",
                get_name(),
                _density,
                m_organization.density);
        }
    };

    void set_timing_vals() {
        m_timing_vals.resize(m_timings.size(), -1);
        m_command_latencies.resize(m_commands.size(), -1);

        // Load timing preset if provided
        bool preset_provided = false;
        if (auto preset_name = param_group("timing").param<std::string>("preset").optional()) {
            if (timing_presets.count(*preset_name) > 0) {
                m_timing_vals = timing_presets.at(*preset_name);
                preset_provided = true;
            } else {
                throw ConfigurationError("Unrecognized timing preset \"{}\" in {}!", *preset_name, get_name());
            }
        }

        // Check for rate (in MT/s), and if provided, calculate and set tCK (in picosecond)
        if (auto dq = param_group("timing").param<int>("rate").optional()) {
            if (preset_provided) {
                throw ConfigurationError("Cannot change the transfer rate of {} when using a speed preset !", get_name());
            }
            m_timing_vals("rate") = *dq;
        }
        int tCK_ps = 1E6 / (m_timing_vals("rate") / 2);
        m_timing_vals("tCK_ps") = tCK_ps;

        // Load the organization specific timings
        int dq_id = [](int dq) -> int {
            switch (dq) {
            case 16:
                return 0;
            default:
                return -1;
            }
        }(m_organization.dq);

        int rate_id = [](int rate) -> int {
            switch (rate) {
            case 6400:
                return 0;
            default:
                return -1;
            }
        }(m_timing_vals("rate"));

        // Refresh timings
        // tRFC table (unit is nanosecond!)
        constexpr int tRFCab_TABLE[4] = {
            //  2Gb   4Gb   8Gb  16Gb
            130,
            180,
            210,
            280,
        };

        constexpr int tRFCpb_TABLE[4] = {
            //  2Gb   4Gb   8Gb  16Gb
            60,
            90,
            120,
            140,
        };

        constexpr int tPBR2PBR_TABLE[4] = {
            //  2Gb   4Gb   8Gb  16Gb
            60,
            90,
            90,
            90,
        };

        constexpr int tPBR2ACT_TABLE[4] = {
            //  2Gb   4Gb   8Gb  16Gb
            8,
            8,
            8,
            8,
        };

        // tREFI(base) table (unit is nanosecond!)
        constexpr int tREFI_BASE = 3906;
        int density_id = [](int density_Mb) -> int {
            switch (density_Mb) {
            case 2048:
                return 0;
            case 4096:
                return 1;
            case 8192:
                return 2;
            case 16384:
                return 3;
            default:
                return -1;
            }
        }(m_organization.density);

        m_timing_vals("nRFCab") = JEDEC_rounding(tRFCab_TABLE[density_id], tCK_ps);
        m_timing_vals("nRFCpb") = JEDEC_rounding(tRFCpb_TABLE[density_id], tCK_ps);
        m_timing_vals("nPBR2PBR") = JEDEC_rounding(tPBR2PBR_TABLE[density_id], tCK_ps);
        m_timing_vals("nPBR2ACT") = JEDEC_rounding(tPBR2ACT_TABLE[density_id], tCK_ps);
        m_timing_vals("nREFI") = JEDEC_rounding(tREFI_BASE, tCK_ps);

        // Overwrite timing parameters with any user-provided value
        // Rate and tCK should not be overwritten
        for (int i = 1; i < m_timings.size() - 1; i++) {
            auto timing_name = std::string(m_timings(i));

            if (auto provided_timing = param_group("timing").param<int>(timing_name).optional()) {
                // Check if the user specifies in the number of cycles (e.g., nRCD)
                m_timing_vals(i) = *provided_timing;
            } else if (auto provided_timing = param_group("timing").param<float>(timing_name.replace(0, 1, "t")).optional()) {
                // Check if the user specifies in nanoseconds (e.g., tRCD)
                m_timing_vals(i) = JEDEC_rounding(*provided_timing, tCK_ps);
            }
        }

        // Check if there is any uninitialized timings
        for (int i = 0; i < m_timing_vals.size(); i++) {
            if (m_timing_vals(i) == -1) {
                throw ConfigurationError("In \"{}\", timing {} is not specified!", get_name(), m_timings(i));
            }
        }

        // The followings are directly from GDDR6
        m_timing_vals("nCLREG") = 0;
        m_timing_vals("nCLGB") = 1;
        m_timing_vals("nCWLREG") = 1;
        m_timing_vals("nCWLGB") = 1;
        m_timing_vals("nWPRE") = 1;

        // Set read latency
        m_read_latency = m_timing_vals("nCL") + m_timing_vals("nBL");
        // The followings are directly from GDDR6
        m_command_latencies("WR") = m_timing_vals("nCWL") + m_timing_vals("nBL");
        m_command_latencies("WRGB") = m_timing_vals("nCWLGB") + m_timing_vals("nBL");
        m_command_latencies("WRMAC16") = m_timing_vals("nCWLREG") + m_timing_vals("nBL");
        m_command_latencies("RDMAC16") = m_timing_vals("nCLREG") + m_timing_vals("nBL");
        m_command_latencies("RDAF16") = m_timing_vals("nCLREG") + m_timing_vals("nBL");
        m_command_latencies("RD") = m_timing_vals("nCL") + m_timing_vals("nBL");
        m_command_latencies("RDCP") = 1;
        m_command_latencies("WRCP") = 1;
        m_command_latencies("MAC") = 1;
        m_command_latencies("MAC16") = 1;
        m_command_latencies("AF16") = 1;
        m_command_latencies("EWMUL16") = 1;
        m_command_latencies("WRA16") = m_timing_vals("nCWL") + m_timing_vals("nBL") + m_timing_vals("nRPab");
        m_command_latencies("SYNC") = 1;
        m_command_latencies("EOC") = 1;

// Populate the timing constraints
#define V(timing) (m_timing_vals(timing))
        populate_timingcons(this, {
                                      /*** Channel ***/
                                      // CAS <-> CAS
                                      /// Data bus occupancy
                                      {.level = "channel", .preceding = {"RD", "RDA", "RDMAC16", "RDAF16"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nBL")},
                                      {.level = "channel", .preceding = {"WR", "WRA", "WRA16", "WRGB", "WRMAC16"}, .following = {"WR", "WRA", "WRA16", "WRGB", "WRMAC16"}, .latency = V("nBL")},

                                      /*** Rank (or different BankGroup) ***/
                                      // CAS <-> CAS
                                      {.level = "rank", .preceding = {"RD", "RDA", "MAC", "MAC16", "RDCP", "RDMAC16", "RDAF16"}, .following = {"RD", "RDA", "MAC", "MAC16", "RDCP", "RDMAC16", "RDAF16"}, .latency = V("nCCD")},
                                      {.level = "rank", .preceding = {"WR", "WRA", "WRA16", "WRGB", "WRCP", "WRMAC16"}, .following = {"WR", "WRA", "WRA16", "WRGB", "WRCP", "WRMAC16"}, .latency = V("nCCD")},
                                      {.level = "rank", .preceding = {"RD", "RDA", "MAC", "RDCP", "RDMAC16", "RDAF16", "MAC16", "EWMUL16"}, .following = {"RDMAC16", "RDAF16", "MAC16", "EWMUL16"}, .latency = V("nCCD")},
                                      {.level = "rank", .preceding = {"RDMAC16", "RDAF16", "MAC16", "EWMUL16"}, .following = {"RD", "RDA", "MAC", "RDCP", "RDMAC16", "RDAF16", "MAC16", "EWMUL16"}, .latency = V("nCCD")},
                                      {.level = "rank", .preceding = {"WR", "WRA", "WRCP", "WRA16", "WRMAC16", "EWMUL16"}, .following = {"WRA16", "WRMAC16", "EWMUL16"}, .latency = V("nCCD")},
                                      {.level = "rank", .preceding = {"WRA16", "WRMAC16", "EWMUL16"}, .following = {"WR", "WRA", "WRCP", "WRA16", "WRMAC16", "EWMUL16"}, .latency = V("nCCD")},

                                      /// RD <-> WR, Minimum Read to Write, Assuming tWPRE = 1 tCK
                                      {.level = "rank", .preceding = {"RD", "RDA"}, .following = {"WR", "WRA"}, .latency = V("nCL") + V("nBL") + 2 - V("nCWL")},
                                      {.level = "rank", .preceding = {"RD", "RDA"}, .following = {"WRA16"}, .latency = V("nCL") + V("nBL") + 2 - V("nCWL")},
                                      {.level = "rank", .preceding = {"RDMAC16", "RDAF16"}, .following = {"WR", "WRA"}, .latency = V("nCLREG") + V("nBL") + 2 - V("nCWL")},
                                      {.level = "rank", .preceding = {"RDMAC16", "RDAF16"}, .following = {"WRA16"}, .latency = V("nCLREG") + V("nBL") + 2 - V("nCWL")},
                                      {.level = "rank", .preceding = {"RD", "RDA"}, .following = {"WRGB"}, .latency = V("nCL") + V("nBL") + 2 - V("nCWLGB")},
                                      {.level = "rank", .preceding = {"RD", "RDA"}, .following = {"WRMAC16"}, .latency = V("nCL") + V("nBL") + 2 - V("nCWLREG")},
                                      {.level = "rank", .preceding = {"RDMAC16", "RDAF16"}, .following = {"WRGB"}, .latency = V("nCLREG") + V("nBL") + 2 - V("nCWLGB")},
                                      {.level = "rank", .preceding = {"RDMAC16", "RDAF16"}, .following = {"WRMAC16"}, .latency = V("nCLREG") + V("nBL") + 2 - V("nCWLREG")},

                                      /// WR <-> RD, Minimum Read after Write
                                      {.level = "rank", .preceding = {"WR", "WRA"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nCWL") + V("nBL") + V("nWTRS")},
                                      {.level = "rank", .preceding = {"WRA16"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nCWL") + V("nBL") + V("nWTRS")},
                                      {.level = "rank", .preceding = {"WRGB"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nCWLGB") + V("nBL") + V("nWTRS")},
                                      {.level = "rank", .preceding = {"WRMAC16"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nCWLREG") + V("nBL") + V("nWTRS")},
                                      {.level = "rank", .preceding = {"WR", "WRA"}, .following = {"RDMAC16", "RDAF16"}, .latency = V("nCWL") + V("nBL") + V("nWTRL")},
                                      {.level = "rank", .preceding = {"WRA16"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nCWL") + V("nBL") + V("nWTRL")},
                                      {.level = "rank", .preceding = {"WRMAC16"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nCWLREG") + V("nBL") + V("nWTRL")},

                                      // TODO
                                      /// CAS <-> CAS between sibling ranks, nCS (rank switching) is needed for new DQS
                                      {.level = "rank", .preceding = {"RD", "RDA"}, .following = {"RD", "RDA", "WR", "WRA"}, .latency = V("nBL") + V("nCS"), .is_sibling = true},
                                      {.level = "rank", .preceding = {"WR", "WRA"}, .following = {"RD", "RDA"}, .latency = V("nCL") + V("nBL") + V("nCS") - V("nCWL"), .is_sibling = true},

                                      /// CAS <-> PREab
                                      {.level = "rank", .preceding = {"RD", "RDCP", "MAC", "MAC16", "AF16", "EWMUL16"}, .following = {"PREA"}, .latency = V("nRTP")},
                                      {.level = "rank", .preceding = {"MAC16", "AF16", "EWMUL16"}, .following = {"PRE", "PRE4"}, .latency = V("nRTP")},
                                      {.level = "rank", .preceding = {"WR", "WRCP"}, .following = {"PREA"}, .latency = V("nCWL") + V("nBL") + V("nWR")},
                                      {.level = "rank", .preceding = {"EWMUL16"}, .following = {"PRE", "PRE4", "PREA"}, .latency = V("nCWL") + V("nWR")},

                                      /// RAS <-> RAS
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1"}, .following = {"ACT-1", "ACT4-1", "REFpb"}, .latency = V("nRRD")},
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"ACT16-1"}, .latency = V("nRRD")},
                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"ACT-1", "ACT4-1", "ACT16-1", "REFpb"}, .latency = V("nRRD")},
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"ACT16-1"}, .latency = V("nRC")},
                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"ACT-1", "ACT4-1", "ACT16-1", "REFpb"}, .latency = V("nRC")},

                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"PREA"}, .latency = V("nRAS")},
                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"PRE", "PRE4"}, .latency = V("nRAS")},

                                      {.level = "rank", .preceding = {"PRE"}, .following = {"ACT16-1"}, .latency = V("nRPpb")},
                                      {.level = "rank", .preceding = {"PRE4", "PREA"}, .following = {"ACT-1", "ACT4-1", "ACT16-1"}, .latency = V("nRPab")},

                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"RD", "RDA", "WR", "WRA"}, .latency = V("nRCD")},
                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"MAC"}, .latency = V("nRCDRDMAC")},
                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"RDCP"}, .latency = V("nRCDRDCP")},
                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"WRCP"}, .latency = V("nRCDWRCP")},
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"MAC16"}, .latency = V("nRCDRDMAC")},
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"AF16"}, .latency = V("nRCDRDAF")},
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"EWMUL16"}, .latency = V("nRCDEWMUL")},
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"WRA16"}, .latency = V("nRCD")},

                                      {.level = "rank", .preceding = {"RDA"}, .following = {"ACT16-1"}, .latency = V("nRTP") + V("nRPpb")},
                                      {.level = "rank", .preceding = {"WRA"}, .following = {"ACT16-1"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRPpb")},
                                      {.level = "rank", .preceding = {"WRA16"}, .following = {"ACT-1", "ACT4-1", "ACT16-1"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRPab")},

                                      /// RAS <-> REF

                                      {.level = "rank", .preceding = {"REFpb"}, .following = {"REFpb"}, .latency = V("nPBR2PBR")},
                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"REFpb"}, .latency = V("nPBR2ACT")},

                                      {.level = "rank", .preceding = {"ACT-1", "ACT4-1", "ACT16-1"}, .following = {"REFab"}, .latency = V("nRC")},
                                      {.level = "rank", .preceding = {"ACT16-1"}, .following = {"REFpb"}, .latency = V("nRC")},
                                      {.level = "rank", .preceding = {"PRE"}, .following = {"REFab"}, .latency = V("nRPpb")},
                                      {.level = "rank", .preceding = {"PREA"}, .following = {"REFab", "REFpb"}, .latency = V("nRPab")},
                                      {.level = "rank", .preceding = {"PRE4"}, .following = {"REFab"}, .latency = V("nRPab")}, // there could be a new nRPbg
                                      {.level = "rank", .preceding = {"RDA"}, .following = {"REFab"}, .latency = V("nRPpb") + V("nRTP")},
                                      {.level = "rank", .preceding = {"WRA"}, .following = {"REFab"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRPpb")},
                                      {.level = "rank", .preceding = {"WRA16"}, .following = {"REFpb", "REFab"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRPab")},
                                      {.level = "rank", .preceding = {"REFab"}, .following = {"REFab", "REFpb", "ACT-1", "ACT4-1", "ACT16-1"}, .latency = V("nRFCab")},
                                      {.level = "rank", .preceding = {"REFpb"}, .following = {"ACT16-1"}, .latency = V("nRFCpb")},

                                      {.level = "rank", .preceding = {"TMOD"}, .following = {"ACT-1", "ACT-2", "PRE", "PREA", "CASRD", "CASWR", "CASWRGB", "CASWRMAC16", "CASRDMAC16", "CASRDAF16", "CASWRA16", "RD", "WR", "RDA", "WRA", "REFab", "REFpb", "RFMab", "RFMpb", "ACT16-1", "ACT4-1", "ACT16-2", "ACT4-2", "PRE4", "MAC", "MAC16", "AF16", "EWMUL16", "RDCP", "WRCP", "WRGB", "RDMAC16", "RDAF16", "WRMAC16", "WRA16", "SYNC", "EOC"}, .latency = V("nMODCH")},

                                      /****************************************************** Bank Group ******************************************************/
                                      /// CAS <-> CAS
                                      {.level = "bankgroup", .preceding = {"RD", "RDA", "MAC", "RDCP"}, .following = {"RD", "RDA", "MAC", "RDCP"}, .latency = V("nCCD")},
                                      {.level = "bankgroup", .preceding = {"WR", "WRA", "WRCP"}, .following = {"WR", "WRA", "WRCP"}, .latency = V("nCCD")},

                                      /// WR <-> RD
                                      {.level = "bankgroup", .preceding = {"WR", "WRA"}, .following = {"RD", "RDA"}, .latency = V("nCWL") + V("nBL") + V("nWTRL")},

                                      /// CAS <-> PRE4
                                      {.level = "bankgroup", .preceding = {"RD", "RDCP", "MAC"}, .following = {"PRE"}, .latency = V("nRTP")},
                                      {.level = "bankgroup", .preceding = {"WR", "WRCP"}, .following = {"PRE4"}, .latency = V("nCWL") + V("nBL") + V("nWR")},

                                      /// RAS <-> RAS
                                      {.level = "bankgroup", .preceding = {"ACT-1", "ACT4-1"}, .following = {"ACT-1", "ACT4-1"}, .latency = V("nRRD")},
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"ACT-1", "ACT4-1"}, .latency = V("nRC")},
                                      {.level = "bankgroup", .preceding = {"ACT-1", "ACT4-1"}, .following = {"ACT4-1"}, .latency = V("nRC")},
                                      {.level = "bankgroup", .preceding = {"ACT-1", "ACT4-1"}, .following = {"PRE4"}, .latency = V("nRAS")},
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"PRE"}, .latency = V("nRAS")},
                                      {.level = "bankgroup", .preceding = {"PRE"}, .following = {"ACT4-1"}, .latency = V("nRPpb")},
                                      {.level = "bankgroup", .preceding = {"PRE4"}, .following = {"ACT-1", "ACT4-1"}, .latency = V("nRPab")},
                                      {.level = "bankgroup", .preceding = {"RDA"}, .following = {"ACT4-1"}, .latency = V("nRTP") + V("nRPpb")},
                                      {.level = "bankgroup", .preceding = {"WRA"}, .following = {"ACT4-1"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRPpb")},

                                      /// RAS <-> REFpb
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"REFpb"}, .latency = V("nRC")},
                                      {.level = "bankgroup", .preceding = {"PRE4"}, .following = {"REFpb"}, .latency = V("nRPab")},
                                      {.level = "bankgroup", .preceding = {"REFpb"}, .following = {"ACT4-1"}, .latency = V("nRFCpb")},

                                      /// CAS <-> RAS
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"MAC"}, .latency = V("nRCDRDMAC")},
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"RDCP"}, .latency = V("nRCDRDCP")},
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"RD", "RDA"}, .latency = V("nRCD")},
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"WRCP"}, .latency = V("nRCDWRCP")},
                                      {.level = "bankgroup", .preceding = {"ACT4-1"}, .following = {"WR", "WRA"}, .latency = V("nRCD")},

                                      /****************************************************** Bank ******************************************************/
                                      /// CAS <-> RAS
                                      {.level = "bank", .preceding = {"ACT-1"}, .following = {"RD", "RDA", "WR", "WRA"}, .latency = V("nRCD")},
                                      {.level = "bank", .preceding = {"ACT-1"}, .following = {"MAC"}, .latency = V("nRCDRDMAC")},
                                      {.level = "bank", .preceding = {"ACT-1"}, .following = {"RDCP"}, .latency = V("nRCDRDCP")},
                                      {.level = "bank", .preceding = {"ACT-1"}, .following = {"WRCP"}, .latency = V("nRCDWRCP")},
                                      {.level = "bank", .preceding = {"RD", "RDCP", "MAC"}, .following = {"PRE"}, .latency = V("nRTP")},
                                      {.level = "bank", .preceding = {"WR", "WRCP"}, .following = {"PRE"}, .latency = V("nCWL") + V("nBL") + V("nWR")},

                                      /// RAS <-> RAS
                                      {.level = "bank", .preceding = {"ACT-1"}, .following = {"ACT-1"}, .latency = V("nRC")},
                                      {.level = "bank", .preceding = {"ACT-1"}, .following = {"PRE"}, .latency = V("nRAS")},
                                      {.level = "bank", .preceding = {"PRE"}, .following = {"PRE"}, .latency = V("nRPpb")},
                                      {.level = "bank", .preceding = {"PRE"}, .following = {"ACT-1"}, .latency = V("nRPpb")},
                                      {.level = "bank", .preceding = {"RDA"}, .following = {"ACT-1"}, .latency = V("nRTP") + V("nRPpb")},
                                      {.level = "bank", .preceding = {"WRA"}, .following = {"ACT-1"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRPpb")},

                                      /// RAS <-> REFpb
                                      {.level = "bank", .preceding = {"ACT-1"}, .following = {"REFpb"}, .latency = V("nRC")},
                                      {.level = "bank", .preceding = {"PRE"}, .following = {"REFpb"}, .latency = V("nRPpb")},
                                      {.level = "bank", .preceding = {"RDA"}, .following = {"REFpb"}, .latency = V("nRTP") + V("nRPpb")},
                                      {.level = "bank", .preceding = {"WRA"}, .following = {"REFpb"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRPpb")},
                                      {.level = "bank", .preceding = {"REFpb"}, .following = {"ACT-1"}, .latency = V("nRFCpb")},

                                  });
#undef V
    };

    void set_actions() {
        m_actions.resize(m_levels.size(), std::vector<ActionFunc_t<Node>>(m_commands.size()));

        // Rank Actions
        m_actions[m_levels["rank"]][m_commands["WRA16"]] = Lambdas::Action::Channel::PREab<LPDDR5>;
        m_actions[m_levels["rank"]][m_commands["PREA"]] = Lambdas::Action::Rank::PREab<LPDDR5>;
        m_actions[m_levels["rank"]][m_commands["ACT16-1"]] = [](Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
            int target_id = addr_vec[node->m_level + 3];
            for (auto bg : node->m_child_nodes) {
                for (auto bank : bg->m_child_nodes) {
                    bank->m_state = m_states["Pre-Opened"];
                    bank->m_row_state[target_id] = m_states["Pre-Opened"];
                }
            }
        };
        m_actions[m_levels["channel"]][m_commands["ACT16-2"]] = Lambdas::Action::Channel::ACTab<LPDDR5>;

        assert(m_timings["nCL"] == m_timing_vals("nCL"));
        assert(m_timings["nCWL"] == m_timing_vals("nCWL"));
        assert(m_timings["nBL"] == m_timing_vals("nBL"));
        assert(m_timings["nCWLGB"] == m_timing_vals("nCWLGB"));
        assert(m_timings["nCWLREG"] == m_timing_vals("nCWLREG"));
        assert(m_timings["nCLREG"] == m_timing_vals("nCLREG"));

#define ACTION_DEF(OP)                                                                \
    m_actions[m_levels["rank"]]                                                       \
             [m_commands["CAS" #OP]] = [this](Node *node, int cmd,                    \
                                              const AddrVec_t &addr_vec, Clk_t clk) { \
                 node->m_final_synced_cycle = clk + m_command_latencies(#OP) + 1;     \
             };                                                                       \
                                                                                      \
    m_actions[m_levels["rank"]]                                                       \
             [m_commands[#OP]] = [this](Node *node, int cmd,                          \
                                        const AddrVec_t &addr_vec, Clk_t clk) {       \
                 node->m_final_synced_cycle = clk + m_command_latencies(#OP);         \
             }

        ACTION_DEF(RD);
        ACTION_DEF(WR);
        ACTION_DEF(WRGB);
        ACTION_DEF(WRMAC16);
        ACTION_DEF(RDMAC16);
        ACTION_DEF(RDAF16);
        ACTION_DEF(WRA16);

        // Bank Group Actions
        m_actions[m_levels["bankgroup"]][m_commands["ACT4-1"]] = [](Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
            int target_id = addr_vec[node->m_level + 2];
            for (auto bank : node->m_child_nodes) {
                bank->m_state = m_states["Pre-Opened"];
                bank->m_row_state[target_id] = m_states["Pre-Opened"];
            }
        };
        m_actions[m_levels["bankgroup"]][m_commands["ACT4-2"]] = Lambdas::Action::BankGroup::ACT4b<LPDDR5>;
        m_actions[m_levels["bankgroup"]][m_commands["PRE4"]] = Lambdas::Action::BankGroup::PRE4b<LPDDR5>;

        // Bank actions
        m_actions[m_levels["bank"]][m_commands["ACT-1"]] = [](Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
            int target_id = addr_vec[node->m_level + 1];
            node->m_state = m_states["Pre-Opened"];
            node->m_row_state[target_id] = m_states["Pre-Opened"];
        };
        m_actions[m_levels["bank"]][m_commands["ACT-2"]] = Lambdas::Action::Bank::ACT<LPDDR5>;
        m_actions[m_levels["bank"]][m_commands["PRE"]] = Lambdas::Action::Bank::PRE<LPDDR5>;
        // TODO: RDA and WRA actions may need to also update the sync time
        m_actions[m_levels["bank"]][m_commands["RDA"]] = Lambdas::Action::Bank::PRE<LPDDR5>;
        m_actions[m_levels["bank"]][m_commands["WRA"]] = Lambdas::Action::Bank::PRE<LPDDR5>;
    };

    void set_preqs() {
        m_preqs.resize(m_levels.size(), std::vector<PreqFunc_t<Node>>(m_commands.size()));

        // Rank Preqs
        m_preqs[m_levels["rank"]][m_commands["REFab"]] = Lambdas::Preq::Rank::RequireAllBanksClosed<LPDDR5>;
        m_preqs[m_levels["rank"]][m_commands["RFMab"]] = Lambdas::Preq::Rank::RequireAllBanksClosed<LPDDR5>;

        m_preqs[m_levels["rank"]][m_commands["REFpb"]] = [this](Node *node, int cmd, const AddrVec_t &addr_vec, Clk_t clk) {
            int target_id = addr_vec[node->m_level + 1];
            int target_bank_id = target_id;
            int another_target_bank_id = target_id + 8;

            for (auto bg : node->m_child_nodes) {
                for (auto bank : bg->m_child_nodes) {
                    int num_banks_per_bg = m_organization.count[m_levels["bank"]];
                    int flat_bankid = bank->m_node_id + bg->m_node_id * num_banks_per_bg;
                    if (flat_bankid == target_id || flat_bankid == another_target_bank_id) {
                        switch (node->m_state) {
                        case m_states["Pre-Opened"]:
                            return m_commands["PRE"];
                        case m_states["Opened"]:
                            return m_commands["PRE"];
                        }
                    }
                }
            }

            return cmd;
        };

        m_preqs[m_levels["rank"]][m_commands["RFMpb"]] = m_preqs[m_levels["rank"]][m_commands["REFpb"]];

#define PREQ_BANK_ACT_SYNC_DEF(OP, CASOP)                                                    \
    m_preqs[m_levels["bank"]]                                                                \
           [m_commands[#OP]] = [](Node *node, int cmd,                                       \
                                  const AddrVec_t &addr_vec, Clk_t clk) {                    \
               int target_id = addr_vec[m_levels["row"]];                                    \
               switch (node->m_state) {                                                      \
               case m_states["Closed"]:                                                      \
                   return m_commands["ACT-1"];                                               \
               case m_states["Pre-Opened"]:                                                  \
                   return m_commands["ACT-2"];                                               \
               case m_states["Opened"]: {                                                    \
                   if (node->m_row_state.find(target_id) !=                                  \
                       node->m_row_state.end()) {                                            \
                       Node *rank = node->m_parent_node->m_parent_node;                      \
                       if (rank->m_final_synced_cycle < clk) {                               \
                           return m_commands[#CASOP]; /* CAS‑prefixed form */                \
                       } else {                                                              \
                           return cmd;                                                       \
                       }                                                                     \
                   } else {                                                                  \
                       return m_commands["PRE"];                                             \
                   }                                                                         \
               }                                                                             \
               default:                                                                      \
                   spdlog::error("[Preq::Bank] Invalid bank state for an " #OP " command!"); \
                   std::exit(-1);                                                            \
               }                                                                             \
           }
        PREQ_BANK_ACT_SYNC_DEF(RD, CASRD);
        PREQ_BANK_ACT_SYNC_DEF(WR, CASWR);
        PREQ_BANK_ACT_SYNC_DEF(RDA, CASRD); // TODO: not sure if we should use CASRD for RDA
        PREQ_BANK_ACT_SYNC_DEF(WRA, CASWR); // TODO: not sure if we should use CASWR for WRA

#define PREQ_BANK_ACT_DEF(OP)                                                                \
    m_preqs[m_levels["bank"]]                                                                \
           [m_commands[#OP]] = [](Node *node, int cmd,                                       \
                                  const AddrVec_t &addr_vec, Clk_t clk) {                    \
               int target_id = addr_vec[m_levels["row"]];                                    \
               switch (node->m_state) {                                                      \
               case m_states["Closed"]:                                                      \
                   return m_commands["ACT-1"];                                               \
               case m_states["Pre-Opened"]:                                                  \
                   return m_commands["ACT-2"];                                               \
               case m_states["Opened"]: {                                                    \
                   if (node->m_row_state.find(target_id) !=                                  \
                       node->m_row_state.end()) {                                            \
                       return cmd;                                                           \
                   } else {                                                                  \
                       return m_commands["PRE"];                                             \
                   }                                                                         \
               }                                                                             \
               default:                                                                      \
                   spdlog::error("[Preq::Bank] Invalid bank state for an " #OP " command!"); \
                   std::exit(-1);                                                            \
               }                                                                             \
           }

        PREQ_BANK_ACT_DEF(RDCP);
        PREQ_BANK_ACT_DEF(WRCP);
        PREQ_BANK_ACT_DEF(MAC);

#define PREQ_RANK_ACT_DEF(OP)                                                                        \
    m_preqs[m_levels["rank"]]                                                                        \
           [m_commands[#OP]] = [](Node *node, int cmd,                                               \
                                  const AddrVec_t &addr_vec, Clk_t clk) {                            \
               int target_id = addr_vec[m_levels["row"]];                                            \
               bool any_closed = false;                                                              \
               bool any_pre_opened = false;                                                          \
               bool any_open_diff = false;                                                           \
                                                                                                     \
               for (auto bg : node->m_child_nodes) {                                                 \
                   for (auto bank : bg->m_child_nodes) {                                             \
                       switch (bank->m_state) {                                                      \
                       case m_states["Closed"]:                                                      \
                           any_closed = true;                                                        \
                           break;                                                                    \
                       case m_states["Pre-Opened"]:                                                  \
                           any_pre_opened = true;                                                    \
                           break;                                                                    \
                       case m_states["Opened"]:                                                      \
                           if (bank->m_row_state.find(target_id) ==                                  \
                               bank->m_row_state.end())                                              \
                               any_open_diff = true;                                                 \
                           break;                                                                    \
                       default:                                                                      \
                           spdlog::error("[Preq::Bank] Invalid bank state for an " #OP " command!"); \
                           std::exit(-1);                                                            \
                       }                                                                             \
                   }                                                                                 \
               }                                                                                     \
                                                                                                     \
               if (any_open_diff)                                                                    \
                   return m_commands["PREA"];                                                        \
               if (any_closed)                                                                       \
                   return m_commands["ACT16-1"];                                                     \
               if (any_pre_opened)                                                                   \
                   return m_commands["ACT16-2"];                                                     \
               return cmd;                                                                           \
           }

        PREQ_RANK_ACT_DEF(WRA16);
        PREQ_RANK_ACT_DEF(MAC16);
        PREQ_RANK_ACT_DEF(AF16);
        PREQ_RANK_ACT_DEF(EWMUL16);

#define PREQ_RANK_SYNC_DEF(OP, CASOP)                                     \
    m_preqs[m_levels["rank"]]                                             \
           [m_commands[#OP]] = [](Node *node, int cmd,                    \
                                  const AddrVec_t &addr_vec, Clk_t clk) { \
               if (node->m_final_synced_cycle < clk) {                    \
                   return m_commands[#CASOP];                             \
               } else {                                                   \
                   return cmd;                                            \
               }                                                          \
           }

        PREQ_RANK_SYNC_DEF(WRGB, CASWRGB);
        PREQ_RANK_SYNC_DEF(WRMAC16, CASWRMAC16);
        PREQ_RANK_SYNC_DEF(RDMAC16, CASRDMAC16);
        PREQ_RANK_SYNC_DEF(RDAF16, CASRDAF16);
        PREQ_RANK_SYNC_DEF(WRA16, CASWRA16);
    };

    // Not implemented
    // void set_rowhits() {
    //     m_rowhits.resize(m_levels.size(), std::vector<RowhitFunc_t<Node>>(m_commands.size()));

    //     m_rowhits[m_levels["bank"]][m_commands["RD"]] = Lambdas::RowHit::Bank::RDWR<LPDDR5>;
    //     m_rowhits[m_levels["bank"]][m_commands["WR"]] = Lambdas::RowHit::Bank::RDWR<LPDDR5>;
    // }

    // Not implemented
    // void set_rowopens() {
    //     m_rowopens.resize(m_levels.size(), std::vector<RowhitFunc_t<Node>>(m_commands.size()));

    //     m_rowopens[m_levels["bank"]][m_commands["RD"]] = Lambdas::RowOpen::Bank::RDWR<LPDDR5>;
    //     m_rowopens[m_levels["bank"]][m_commands["WR"]] = Lambdas::RowOpen::Bank::RDWR<LPDDR5>;
    // }

    void create_nodes() {
        int num_channels = m_organization.count[m_levels["channel"]];
        for (int i = 0; i < num_channels; i++) {
            Node *channel = new Node(this, nullptr, 0, i);
            m_channels.push_back(channel);
            m_open_rows.push_back(0);
        }
    };
};

} // namespace Ramulator
