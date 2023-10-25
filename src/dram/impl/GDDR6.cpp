#include "dram/dram.h"
#include "dram/lambdas.h"
#include <string>

namespace Ramulator {

class GDDR6 : public IDRAM, public Implementation {
    RAMULATOR_REGISTER_IMPLEMENTATION(IDRAM, GDDR6, "GDDR6", "GDDR6 Device Model")

public:
    inline static const std::map<std::string, Organization> org_presets = {
        //Table 19 for more info
        //    name         density   DQ   Ch Bg Ba   Ro     Co
        {"GDDR6_8Gb_x8", {8 << 10, 8, {2, 4, 4, 1 << 14, 1 << 11}}},
        {"GDDR6_8Gb_x16", {8 << 10, 16, {2, 4, 4, 1 << 14, 1 << 10}}},
        {"GDDR6_16Gb_x8", {16 << 10, 8, {2, 4, 4, 1 << 15, 1 << 11}}},
        {"GDDR6_16Gb_x16", {16 << 10, 16, {2, 4, 4, 1 << 14, 1 << 11}}},
        {"GDDR6_32Gb_x8", {32 << 10, 8, {2, 4, 4, 1 << 16, 1 << 11}}},
        {"GDDR6_32Gb_x16", {32 << 10, 16, {2, 4, 4, 1 << 15, 1 << 11}}},
    };

    inline static const std::map<std::string, std::vector<int>> timing_presets = {
        {"GDDR6_2000_1350mV_double", // name
         {2000,                      // rate
          8,                         // nBL
          24,                        // nCL
          26,                        // nRCDRD
          16,                        // nRCDWR
          26,                        // nRP
          53,                        // nRAS
          79,                        // nRC
          26,                        // nWR
          4,                         // nRTP
          6,                         // nCWL
          4,                         // nCCDS
          6,                         // nCCDL
          7,                         // nRRDS
          7,                         // nRRDL
          9,                         // nWTRS
          11,                        // nWTRL
          28,                        // nFAW
          210,                       // nRFC
          105,                       // nRFCpb
          14,                        // nRREFD
          3333,                      // nREFI
          570}},                     // tCK_ps

        {"GDDR6_2000_1250mV_double", // name
         {2000,                      // rate
          8,                         // nBL
          24,                        // nCL
          30,                        // nRCDRD
          19,                        // nRCDWR
          30,                        // nRP
          60,                        // nRAS
          89,                        // nRC
          30,                        // nWR
          4,                         // nRTP
          6,                         // nCWL
          4,                         // nCCDS
          6,                         // nCCDL
          11,                        // nRRDS
          11,                        // nRRDL
          9,                         // nWTRS
          11,                        // nWTRL
          42,                        // nFAW
          210,                       // nRFC
          105,                       // nRFCpb
          21,                        // nRREFD
          3333,                      // nREFI
          570}},                     // tCK_ps

        {"GDDR6_2000_1350mV_quad", // name
         {2000,                    // rate
          4,                       // nBL
          24,                      // nCL
          26,                      // nRCDRD
          16,                      // nRCDWR
          26,                      // nRP
          53,                      // nRAS
          79,                      // nRC
          26,                      // nWR
          4,                       // nRTP
          6,                       // nCWL
          4,                       // nCCDS
          6,                       // nCCDL
          7,                       // nRRDS
          7,                       // nRRDL
          9,                       // nWTRS
          11,                      // nWTRL
          28,                      // nFAW
          210,                     // nRFC
          105,                     // nRFCpb
          14,                      // nRREFD
          3333,                    // nREFI
          570}},                   // tCK_ps

        {"GDDR6_2000_1250mV_quad", // name
         {2000,                    // rate
          4,                       // nBL
          24,                      // nCL
          30,                      // nRCDRD
          19,                      // nRCDWR
          30,                      // nRP
          60,                      // nRAS
          89,                      // nRC
          30,                      // nWR
          4,                       // nRTP
          6,                       // nCWL
          4,                       // nCCDS
          6,                       // nCCDL
          11,                      // nRRDS
          11,                      // nRRDL
          9,                       // nWTRS
          11,                      // nWTRL
          42,                      // nFAW
          210,                     // nRFC
          105,                     // nRFCpb
          21,                      // nRREFD
          3333,                    // nREFI
          570}},                   // tCK_ps
    };

    /************************************************
   *                Organization
   ***********************************************/
    const int m_internal_prefetch_size = 8;

    inline static constexpr ImplDef m_levels = {
        "channel",
        "bankgroup",
        "bank",
        "row",
        "column",
    };

    /************************************************
   *             Requests & Commands
   ***********************************************/
    inline static constexpr ImplDef m_commands = {
        //figure 3
        "ACT",
        "PREA",
        "PRE",
        "RD",
        "WR",
        "RDA",
        "WRA",
        "REFab",
        "REFpb",
        "ACT4",
        "ACT16",
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
    };

    inline static const ImplLUT m_command_scopes = LUT(
        m_commands, m_levels, {
                                  {"REFab", "channel"},
                                  {"REFpb", "bank"},
                                  {"ACT16", "channel"},
                                  {"ACT4", "bankgroup"},
                                  {"ACT", "row"},
                                  {"PREA", "channel"},
                                  {"PRE4", "bankgroup"},
                                  {"PRE", "bank"},
                                  {"RD", "column"},
                                  {"WR", "column"},
                                  {"RDA", "column"},
                                  {"WRA", "column"},
                                  {"MAC", "column"},
                                  {"MAC16", "channel"},
                                  {"AF16", "channel"},
                                  {"EWMUL16", "channel"},
                                  {"RDCP", "column"},
                                  {"WRCP", "column"},
                                  {"WRGB", "channel"},
                                  {"RDMAC16", "channel"},
                                  {"RDAF16", "channel"},
                                  {"WRMAC16", "bank"},
                              });

    inline static const ImplLUT m_command_meta = LUT<DRAMCommandMeta>(
        m_commands, {
                        // open?   close?   access?  refresh?
                        {"ACT", {true, false, false, false}},
                        {"PREA", {false, true, false, false}},
                        {"PRE", {false, true, false, false}},
                        {"RD", {false, false, true, false}},
                        {"WR", {false, false, true, false}},
                        {"RDA", {false, true, true, false}},
                        {"WRA", {false, true, true, false}},
                        {"REFab", {false, false, false, true}}, //double check
                        {"REFpb", {false, false, false, true}},
                        {"ACT4", {true, false, false, false}},
                        {"ACT16", {true, false, false, false}},
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
                    });

    inline static constexpr ImplDef m_requests = {
        "read",
        "write",
        "all-bank-refresh",
        "PREsb"};

    inline static constexpr ImplDef m_aim_requests = {
        "MIN"
        "ISR_WR_SBK"
        "ISR_WR_GB"
        "ISR_WR_BIAS"
        "ISR_WR_AFLUT"
        "ISR_RD_MAC"
        "ISR_RD_AF"
        "ISR_RD_SBK"
        "ISR_COPY_BKGB"
        "ISR_COPY_GBBK"
        "ISR_MAC_SBK"
        "ISR_MAC_ABK"
        "ISR_AF"
        "ISR_EWMUL"
        "ISR_EWADD"
        "ISR_EOC"
        "MAX"};

    inline static const ImplLUT m_request_translations = LUT(
        m_requests, m_commands, {
                                    {"read", "RD"},                // Read single bank
                                    {"write", "WR"},               // Write single bank
                                    {"all-bank-refresh", "REFab"}, // Referesh all banks
                                    {"PREsb", "PRE"},              // Referesh single bank
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
                                        {"ISR_EOC", "UNKNOWN"},      // 16 - Unknown and illegal
                                        {"MAX", "UNKNOWN"},          // 17 - Unknown and illegal
                                    });

    /************************************************
   *                   Timing
   ***********************************************/
    //delete nCS
    inline static constexpr ImplDef m_timings = {
        "rate",
        "nBL", "nCL", "nRCDRD", "nRCDWR", "nRP", "nRAS", "nRC", "nWR", "nRTP", "nCWL",
        "nCCDS", "nCCDL",
        "nRRDS", "nRRDL",
        "nWTRS", "nWTRL",
        "nFAW",
        "nRFC", "nREFI", "nRREFD", "nRFCpb",
        "tCK_ps"};

    /************************************************
   *                 Node States
   ***********************************************/
    inline static constexpr ImplDef m_states = {
        "Opened", "Closed", "PowerUp", "N/A"};

    inline static const ImplLUT m_init_states = LUT(
        m_levels, m_states, {
                                {"channel", "N/A"},
                                {"bankgroup", "N/A"},
                                {"bank", "Closed"},
                                {"row", "Closed"},
                                {"column", "N/A"},
                            });

public:
    struct Node : public DRAMNodeBase<GDDR6> {
        Node(GDDR6 *dram, Node *parent, int level, int id) : DRAMNodeBase<GDDR6>(dram, parent, level, id){};
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
        assert(false); // Not implemented
        int channel_id = addr_vec[m_levels["channel"]];
        return m_channels[channel_id]->check_rowbuffer_hit(command, addr_vec, m_clk);
    };

private:
    void set_organization() {
        // Channel width
        m_channel_width = param_group("org").param<int>("channel_width").default_val(64);

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
        size_t _density = size_t(m_organization.count[m_levels["channel"]]) *
                          size_t(m_organization.count[m_levels["bankgroup"]]) *
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
            case 8:
                return 0;
            case 16:
                return 1;
            default:
                return -1;
            }
        }(m_organization.dq);

        int rate_id = [](int rate) -> int { //should low voltage operation be added here?
            switch (rate) {
            case 2000:
                return 0;
            default:
                return -1;
            }
        }(m_timing_vals("rate"));

        // Tables for secondary timings determined by the frequency, density, and DQ width.
        // Defined in the JEDEC standard (e.g., Table 169-170, JESD79-4C).

        //update these values
        constexpr int nRRDS_TABLE[2][1] = {
            // 2000
            {4}, // x8
            {5}, // x16
        };
        constexpr int nRRDL_TABLE[2][1] = {
            // 2000
            {5}, // x8
            {6}, // x16
        };
        constexpr int nFAW_TABLE[2][1] = {
            // 2000
            {20}, // x8
            {28}, // x16
        };

        if (dq_id != -1 && rate_id != -1) {
            m_timing_vals("nRRDS") = nRRDS_TABLE[dq_id][rate_id];
            m_timing_vals("nRRDL") = nRRDL_TABLE[dq_id][rate_id];
            m_timing_vals("nFAW") = nFAW_TABLE[dq_id][rate_id];
        }

        // Refresh timings
        // tRFC table (unit is nanosecond!)
        constexpr int tRFC_TABLE[3][3] = {
            //  4Gb   8Gb  16Gb
            {260, 360, 550}, // Normal refresh (tRFC1)
            {160, 260, 350}, // FGR 2x (tRFC2)
            {110, 160, 260}, // FGR 4x (tRFC4)
        };

        // tREFI(base) table (unit is nanosecond!)
        constexpr int tREFI_BASE = 7800;
        int density_id = [](int density_Mb) -> int {
            switch (density_Mb) {
            case 4096:
                return 0;
            case 8192:
                return 1;
            case 16384:
                return 2;
            default:
                return -1;
            }
        }(m_organization.density);

        m_timing_vals("nRFC") = JEDEC_rounding(tRFC_TABLE[0][density_id], tCK_ps);
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

        // Set read latency
        m_read_latency = m_timing_vals("nCL") + m_timing_vals("nBL");

        // AiM related timings

        /* CAS (RD) */
        // MAC or AF16 registers
        m_timing_vals("nCLREG") = 0;
        // Global Buffer (SRAM)
        m_timing_vals("nCLGB") = 1;

        /* CAS (RD) */
        // MAC or AF16 registers
        m_timing_vals("nCWLREG") = 1;
        // Global Buffer (SRAM)
        m_timing_vals("nCWLGB") = 1;

        // changing the direction of the external bus
        m_timing_vals("nWPRE") = 1;

// Populate the timing constraints
#define V(timing) (m_timing_vals(timing))
        populate_timingcons(this, {
                                      /****************************************************** Channel ******************************************************/
                                      // CAS <-> CAS
                                      /// External data bus occupancy
                                      /// AiM commands that transfer data on the external bus
                                      /// RD: RDMAC16 and RDAF16
                                      /// WR: WRGB and WRMAC16
                                      {.level = "channel", .preceding = {"RD", "RDA", "RDMAC16", "RDAF16"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16"}, .latency = V("nBL")},
                                      {.level = "channel", .preceding = {"WR", "WRA", "WRGB", "WRMAC16"}, .following = {"WR", "WRA", "WRGB", "WRMAC16"}, .latency = V("nBL")},

                                      /*** Rank (or different BankGroup) ***/

                                      // CAS <-> CAS
                                      /// nCCDS is the minimal latency for column commands that access to a different bank group
                                      /// AiM commands that transfer data on the bus shared between BGs
                                      /// RD: RDMAC16, RDAF16, and RDCP
                                      /// WR: WRGB, WRMAC16, and WRCP
                                      {.level = "channel", .preceding = {"RD", "RDA", "RDMAC16", "RDAF16", "RDCP"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16", "RDCP"}, .latency = V("nCCDS")},
                                      {.level = "channel", .preceding = {"WR", "WRA", "WRGB", "WRMAC16", "WRCP"}, .following = {"WR", "WRA", "WRGB", "WRMAC16", "WRCP"}, .latency = V("nCCDS")},

                                      /// nCCDL is the minimal latency for column commands that access to the same bank group
                                      /// AiM commands that transfer data on the bus shared inside a BG
                                      /// RD: RDMAC16, RDAF16, and RDCP
                                      /// WR: WRMAC16, and WRCP
                                      {.level = "channel", .preceding = {"RD", "RDA", "RDMAC16", "RDAF16", "RDCP"}, .following = {"RDMAC16", "RDAF16"}, .latency = V("nCCDL")},
                                      {.level = "channel", .preceding = {"RDMAC16", "RDAF16"}, .following = {"RD", "RDA", "RDMAC16", "RDAF16", "RDCP"}, .latency = V("nCCDL")},
                                      {.level = "channel", .preceding = {"WR", "WRA", "WRMAC16", "WRCP"}, .following = {"WRMAC16"}, .latency = V("nCCDL")},
                                      {.level = "channel", .preceding = {"WRMAC16"}, .following = {"WR", "WRA", "WRMAC16", "WRCP"}, .latency = V("nCCDL")},

                                      /// RD <-> WR
                                      /// Minimum Read to Write (READ or RDTR to WRITE or WRTR command delay)
                                      /// AiM commands that transfer data on the external bus
                                      /// RD: RDMAC16 and RDAF16
                                      /// WR: WRGB and WRMAC16
                                      /// The next timing is tRTW
                                      {.level = "channel", .preceding = {"RD", "RDA"}, .following = {"WR", "WRA"}, .latency = V("nCL") + V("nBL") + 3 - V("nCWL") + V("nWPRE")},
                                      {.level = "channel", .preceding = {"RDMAC16", "RDAF16"}, .following = {"WR", "WRA"}, .latency = V("nCLREG") + V("nBL") + 3 - V("nCWL") + V("nWPRE")},
                                      {.level = "channel", .preceding = {"RD", "RDA"}, .following = {"WRGB", "WRMAC16"}, .latency = V("nCL") + V("nBL") + 3 - V("nCWLREG") + V("nWPRE")},
                                      {.level = "channel", .preceding = {"RDMAC16", "RDAF16"}, .following = {"WRGB", "WRMAC16"}, .latency = V("nCLREG") + V("nBL") + 3 - V("nCWLREG") + V("nWPRE")},

                                      /// WR <-> RD
                                      /// Minimum Read after Write
                                      /// AiM commands that transfer data on the external bus, based on the bus shared between BGs
                                      /// RD: RDMAC16 and RDAF16
                                      /// WR: WRGB and WRMAC16
                                      {.level = "channel", .preceding = {"WR", "WRA"}, .following = {"RD", "RDA"}, .latency = V("nCWL") + V("nBL") + V("nWTRS")},
                                      {.level = "channel", .preceding = {"WR", "WRA"}, .following = {"RDMAC16", "RDAF16"}, .latency = V("nCWL") + V("nBL") + V("nWTRS")},
                                      {.level = "channel", .preceding = {"WRGB", "WRMAC16"}, .following = {"RD", "RDA"}, .latency = V("nCWLREG") + V("nBL") + V("nWTRS")},
                                      {.level = "channel", .preceding = {"WRGB", "WRMAC16"}, .following = {"RDMAC16", "RDAF16"}, .latency = V("nCWLREG") + V("nBL") + V("nWTRS")},

                                      /// AiM commands that transfer data on the external bus, based on the bus shared inside a BG
                                      /// RD: RDMAC16 and RDAF16
                                      /// WR: WRMAC16
                                      {.level = "channel", .preceding = {"WR", "WRA"}, .following = {"RDMAC16", "RDAF16"}, .latency = V("nCWL") + V("nBL") + V("nWTRL")},
                                      {.level = "channel", .preceding = {"WRMAC16"}, .following = {"RD", "RDA"}, .latency = V("nCWLREG") + V("nBL") + V("nWTRL")},
                                      {.level = "channel", .preceding = {"WRMAC16"}, .following = {"RDMAC16", "RDAF16"}, .latency = V("nCWLREG") + V("nBL") + V("nWTRL")},

                                      // What about the contention of the bus shared between BGs for:
                                      // ("RDCP" -> {"WR", "WRA", "WRGB", "WRMAC16"}) IDK. There is no internal READ to WRITE delay.
                                      // ("WRCP" -> {"RD", "RDA", "RDMAC16", "RDAF16"}) my guess is V("nWTRS")
                                      // ("RDCP" <-> "WRCP") IDK. There is no internal READ to WRITE delay.

                                      /// CAS <-> PREA
                                      /// AiM commands that read/write from/to a bank
                                      /// RD: RDCP, MAC, MAC16, AF16, and EWMUL16
                                      /// WR: WRCP and EWMUL16
                                      // "READ to PRECHARGE within the same bank group"
                                      {.level = "channel", .preceding = {"RD", "RDCP", "MAC", "MAC16", "AF16", "EWMUL16"}, .following = {"PREA"}, .latency = V("nRTP")},
                                      {.level = "channel", .preceding = {"MAC16", "AF16", "EWMUL16"}, .following = {"PRE", "PRE4"}, .latency = V("nRTP")},
                                      // "Not based on the GDDR6 document"
                                      {.level = "channel", .preceding = {"WR", "WRCP", "EWMUL16"}, .following = {"PREA"}, .latency = V("nCWL") + V("nBL") + V("nWR")}, // Not sure if we have to consider nBL for WRCP or EWMUL16
                                      {.level = "channel", .preceding = {"EWMUL16"}, .following = {"PRE", "PRE4"}, .latency = V("nCWL") + V("nBL") + V("nWR")},        // Not sure if we have to consider nBL for WRCP or EWMUL16

                                      /// RAS <-> RAS
                                      // "ACTIVATE to ACTIVATE in a different bank group"
                                      {.level = "channel", .preceding = {"ACT16"}, .following = {"ACT", "ACT4", "ACT16"}, .latency = V("nRRDS")},
                                      {.level = "channel", .preceding = {"ACT", "ACT4", "ACT16"}, .following = {"ACT"}, .latency = V("nRRDS")},
                                      // "ACTIVATE to ACTIVATE in the same bank group"
                                      {.level = "channel", .preceding = {"ACT", "ACT4", "ACT16"}, .following = {"ACT16"}, .latency = V("nRRDL")},
                                      {.level = "channel", .preceding = {"ACT16"}, .following = {"ACT", "ACT4", "ACT16"}, .latency = V("nRRDL")},
                                      //   {.level = "channel", .preceding = {"ACT"}, .following = {"ACT"}, .latency = V("nFAW"), .window = 4}, // Depricated because of the paper
                                      // "A minimum time, tRAS, must have elapsed between opening and closing a row."
                                      {.level = "channel", .preceding = {"ACT", "ACT4", "ACT16"}, .following = {"PREA"}, .latency = V("nRAS")},
                                      {.level = "channel", .preceding = {"ACT16"}, .following = {"PRE", "PRE4"}, .latency = V("nRAS")},
                                      // "After the PRECHARGE command, a subsequent command to the same bank cannot be issued until tRP is met."
                                      {.level = "channel", .preceding = {"PRE", "PRE4", "PREA"}, .following = {"ACT16"}, .latency = V("nRP")},
                                      {.level = "channel", .preceding = {"PREA"}, .following = {"ACT", "ACT4", "ACT16"}, .latency = V("nRP")},
                                      // "An ACTIVATE (ACT) command is required to be issued before the READ command to the same bank, and tRCDRD must be met."
                                      {.level = "channel", .preceding = {"ACT", "ACT4", "ACT16"}, .following = {"MAC16", "AF16", "EWMUL16"}, .latency = V("nRCDRD")},
                                      {.level = "channel", .preceding = {"ACT16"}, .following = {"RD", "RDA", "RDCP", "MAC", "MAC16", "AF16", "EWMUL16"}, .latency = V("nRCDRD")},
                                      // "An ACTIVATE (ACT) command is required to be issued before the WRITE command to the same bank, and tRCDWR must be met."
                                      {.level = "channel", .preceding = {"ACT", "ACT4", "ACT16"}, .following = {"EWMUL16"}, .latency = V("nRCDWR")},
                                      {.level = "channel", .preceding = {"ACT16"}, .following = {"WR", "WRA", "WRCP", "EWMUL16"}, .latency = V("nRCDWR")},

                                      /// RAS <-> REF
                                      // "All banks must be precharged prior to the REFab command."
                                      {.level = "channel", .preceding = {"ACT", "ACT4", "ACT16"}, .following = {"REFab"}, .latency = V("nRC")}, // Why nRC? Shouldn't it be nRAS + nRP? This never happens bevause ACT opens a row and REF's pre-requisite for an open row is PRE
                                      {.level = "channel", .preceding = {"PRE", "PRE4", "PREA"}, .following = {"REFab"}, .latency = V("nRP")},
                                      {.level = "channel", .preceding = {"RDA"}, .following = {"REFab"}, .latency = V("nRTP") + V("nRP")},
                                      {.level = "channel", .preceding = {"WRA"}, .following = {"REFab"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRP")},
                                      // "A minimum time tRFCab is required between two REFab commands. The same rule applies to any access command after the refresh operation."
                                      {.level = "channel", .preceding = {"REFab"}, .following = {"ACT", "ACT4", "ACT16"}, .latency = V("nRFC")},

                                      /// RAS <-> REFpb
                                      // "A minimum time tRRD is required between an ACTIVATE command and a REFpb command to a different bank."
                                      {.level = "channel", .preceding = {"ACT", "ACT4", "ACT16"}, .following = {"REFpb"}, .latency = V("nRRDL")}, // Why RRDL? Shouldn't it be RRDS?
                                      // "The selected bank must be precharged prior to the REFpb command"
                                      {.level = "channel", .preceding = {"ACT16"}, .following = {"REFpb"}, .latency = V("nRC")}, // Why nRC? Shouldn't it be nRAS + nRP? This never happens bevause ACT opens a row and REF's pre-requisite for an open row is PRE
                                      {.level = "channel", .preceding = {"PRE16"}, .following = {"REFpb"}, .latency = V("nRP")},
                                      // "A minimum time tRFCpb is required between a REFpb command and an access command to the same bank that follows"
                                      {.level = "channel", .preceding = {"REFpb"}, .following = {"ACT16"}, .latency = V("nRFCpb")},
                                      // "A minimum time tRREFD is required between a  REFpb command and an ACTIVATE command to a different bank"
                                      {.level = "channel", .preceding = {"REFpb"}, .following = {"ACT", "ACT4", "ACT16"}, .latency = V("nRREFD")},

                                      /****************************************************** Bank Group ******************************************************/
                                      /// CAS <-> CAS
                                      /// nCCDL is the minimal latency for column commands that access to the same bank group
                                      /// AiM commands that transfer data on the bus shared inside a BG
                                      /// RD: RDCP
                                      /// WR: WRCP
                                      {.level = "bankgroup", .preceding = {"RD", "RDA", "RDCP"}, .following = {"RD", "RDA", "RDCP"}, .latency = V("nCCDL")},
                                      {.level = "bankgroup", .preceding = {"WR", "WRA", "WRCP"}, .following = {"WR", "WRA", "WRCP"}, .latency = V("nCCDL")},

                                      /// WR <-> RD
                                      /// Minimum Read after Write
                                      /// AiM commands that transfer data on the external bus, based on the bus shared inside a BG
                                      /// RD:
                                      /// WR:
                                      {.level = "bankgroup", .preceding = {"WR", "WRA"}, .following = {"RD", "RDA"}, .latency = V("nCWL") + V("nBL") + V("nWTRL")},

                                      // Nothing for Read to Write?

                                      /// CAS <-> PRE4
                                      /// AiM commands that read/write from/to a bank
                                      /// RD: RDCP, MAC
                                      /// WR: WRCP
                                      // "READ to PRECHARGE within the same bank group"
                                      {.level = "bankgroup", .preceding = {"RD", "RDCP", "MAC"}, .following = {"PRE", "PRE4"}, .latency = V("nRTP")},
                                      // "Not based on the GDDR6 document"
                                      {.level = "bankgroup", .preceding = {"WR", "WRCP"}, .following = {"PRE", "PRE4"}, .latency = V("nCWL") + V("nBL") + V("nWR")},

                                      /// RAS <-> RAS
                                      // "ACTIVATE to ACTIVATE in the same bank group"
                                      {.level = "bankgroup", .preceding = {"ACT", "ACT4"}, .following = {"ACT", "ACT4"}, .latency = V("nRRDL")},
                                      // "A minimum time, tRAS, must have elapsed between opening and closing a row."
                                      {.level = "bankgroup", .preceding = {"ACT", "ACT4"}, .following = {"PRE4"}, .latency = V("nRAS")},
                                      {.level = "bankgroup", .preceding = {"ACT4"}, .following = {"PRE"}, .latency = V("nRAS")},
                                      // "After the PRECHARGE command, a subsequent command to the same bank cannot be issued until tRP is met."
                                      {.level = "bankgroup", .preceding = {"PRE", "PRE4"}, .following = {"ACT4"}, .latency = V("nRP")},
                                      {.level = "bankgroup", .preceding = {"PRE4"}, .following = {"ACT"}, .latency = V("nRP")},

                                      /// RAS <-> REFpb
                                      // "The selected bank must be precharged prior to the REFpb command"
                                      {.level = "bankgroup", .preceding = {"ACT4"}, .following = {"REFpb"}, .latency = V("nRC")}, // Why nRC? Shouldn't it be nRAS + nRP? This never happens bevause ACT opens a row and REF's pre-requisite for an open row is PRE
                                      {.level = "bankgroup", .preceding = {"PRE4"}, .following = {"REFpb"}, .latency = V("nRP")},
                                      // "A minimum time tRFCpb is required between a REFpb command and an access command to the same bank that follows"
                                      {.level = "bankgroup", .preceding = {"REFpb"}, .following = {"ACT4"}, .latency = V("nRFCpb")},

                                      /// CAS <-> RAS
                                      // "An ACTIVATE (ACT) command is required to be issued before the READ command to the same bank, and tRCDRD must be met."
                                      {.level = "bankgroup", .preceding = {"ACT4"}, .following = {"RD", "RDA", "RDCP", "MAC"}, .latency = V("nRCDRD")},
                                      // "An ACTIVATE (ACT) command is required to be issued before the WRITE command to the same bank, and tRCDWR must be met."
                                      {.level = "bankgroup", .preceding = {"ACT4"}, .following = {"WR", "WRA", "WRCP"}, .latency = V("nRCDWR")},

                                      /****************************************************** Bank ******************************************************/
                                      /// CAS <-> RAS
                                      // "An ACTIVATE (ACT) command is required to be issued before the READ command to the same bank, and tRCDRD must be met."
                                      {.level = "bank", .preceding = {"ACT"}, .following = {"RD", "RDA", "RDCP", "MAC"}, .latency = V("nRCDRD")},
                                      // "An ACTIVATE (ACT) command is required to be issued before the WRITE command to the same bank, and tRCDWR must be met."
                                      {.level = "bank", .preceding = {"ACT"}, .following = {"WR", "WRA", "WRCP"}, .latency = V("nRCDWR")},

                                      /// RAS <-> RAS
                                      // "A minimum time, tRAS, must have elapsed between opening and closing a row."
                                      {.level = "bank", .preceding = {"ACT"}, .following = {"PRE"}, .latency = V("nRAS")},
                                      // "After the PRECHARGE command, a subsequent command to the same bank cannot be issued until tRP is met."
                                      {.level = "bank", .preceding = {"PRE"}, .following = {"ACT"}, .latency = V("nRP")},

                                      {.level = "bank", .preceding = {"RDA"}, .following = {"ACT"}, .latency = V("nRTP") + V("nRP")},
                                      {.level = "bank", .preceding = {"WRA"}, .following = {"ACT"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRP")},

                                      /// RAS <-> REFpb
                                      // "The selected bank must be precharged prior to the REFpb command"
                                      {.level = "bank", .preceding = {"ACT"}, .following = {"REFpb"}, .latency = V("nRC")}, // Why nRC? Shouldn't it be nRAS + nRP? This never happens bevause ACT opens a row and REF's pre-requisite for an open row is PRE
                                      {.level = "bank", .preceding = {"PRE"}, .following = {"REFpb"}, .latency = V("nRP")},
                                      {.level = "bank", .preceding = {"RDA"}, .following = {"REFpb"}, .latency = V("nRTP") + V("nRP")},
                                      {.level = "bank", .preceding = {"WRA"}, .following = {"REFpb"}, .latency = V("nCWL") + V("nBL") + V("nWR") + V("nRP")},
                                      // "A minimum time tRFCpb is required between a REFpb command and an access command to the same bank that follows"
                                      {.level = "bank", .preceding = {"REFpb"}, .following = {"ACT"}, .latency = V("nRFCpb")},

                                  });
#undef V
    };

    void set_actions() {
        m_actions.resize(m_levels.size(), std::vector<ActionFunc_t<Node>>(m_commands.size()));

        // Channel Actions
        m_actions[m_levels["channel"]][m_commands["PREA"]] = Lambdas::Action::Channel::PREab<GDDR6>;
        m_actions[m_levels["channel"]][m_commands["ACT16"]] = Lambdas::Action::Channel::ACTab<GDDR6>;

        // Bank Group Actions
        m_actions[m_levels["bankgroup"]][m_commands["PRE4"]] = Lambdas::Action::BankGroup::PRE4b<GDDR6>;
        m_actions[m_levels["bankgroup"]][m_commands["ACT4"]] = Lambdas::Action::BankGroup::ACT4b<GDDR6>;

        // Bank actions
        m_actions[m_levels["bank"]][m_commands["ACT"]] = Lambdas::Action::Bank::ACT<GDDR6>;
        m_actions[m_levels["bank"]][m_commands["PRE"]] = Lambdas::Action::Bank::PRE<GDDR6>;
        m_actions[m_levels["bank"]][m_commands["RDA"]] = Lambdas::Action::Bank::PRE<GDDR6>;
        m_actions[m_levels["bank"]][m_commands["WRA"]] = Lambdas::Action::Bank::PRE<GDDR6>;
    };

    void set_preqs() {
        m_preqs.resize(m_levels.size(), std::vector<PreqFunc_t<Node>>(m_commands.size()));

        // Channel Actions
        m_preqs[m_levels["channel"]][m_commands["REFab"]] = Lambdas::Preq::Channel::RequireAllBanksClosed<GDDR6>;

        // Bank actions
        m_preqs[m_levels["bank"]][m_commands["RD"]] = Lambdas::Preq::Bank::RequireRowOpen<GDDR6>;
        m_preqs[m_levels["bank"]][m_commands["WR"]] = Lambdas::Preq::Bank::RequireRowOpen<GDDR6>;
        //m_preqs[m_levels["channel"]][m_commands["REFpb"]] = Lambdas::Preq::Bank::RequireAllBanksClosed<GDDR6>; // can RequireSameBanksClosed be used, or is RequireBankClosed needed?

        m_preqs[m_levels["bank"]][m_commands["RDCP"]] = Lambdas::Preq::Bank::RequireRowOpen<GDDR6>;
        m_preqs[m_levels["bank"]][m_commands["WRCP"]] = Lambdas::Preq::Bank::RequireRowOpen<GDDR6>;
        m_preqs[m_levels["bank"]][m_commands["MAC"]] = Lambdas::Preq::Bank::RequireRowOpen<GDDR6>;

        m_preqs[m_levels["channel"]][m_commands["MAC16"]] = Lambdas::Preq::Channel::RequireAllRowsOpen<GDDR6>;
        m_preqs[m_levels["channel"]][m_commands["AF16"]] = Lambdas::Preq::Channel::RequireAllRowsOpen<GDDR6>;
        m_preqs[m_levels["channel"]][m_commands["EWMUL16"]] = Lambdas::Preq::Channel::RequireAllRowsOpen<GDDR6>;
    };

    // Not implemented
    // void set_rowhits() {
    //     m_rowhits.resize(m_levels.size(), std::vector<RowhitFunc_t<Node>>(m_commands.size()));

    //     m_rowhits[m_levels["bank"]][m_commands["RD"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;
    //     m_rowhits[m_levels["bank"]][m_commands["WR"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;

    //     m_rowhits[m_levels["bank"]][m_commands["RDCP"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;
    //     m_rowhits[m_levels["bank"]][m_commands["WRCP"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;
    //     m_rowhits[m_levels["bank"]][m_commands["MAC"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;

    //     m_rowhits[m_levels["bank"]][m_commands["MAC16"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;
    //     m_rowhits[m_levels["bank"]][m_commands["AF16"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;
    //     m_rowhits[m_levels["bank"]][m_commands["EWMUL16"]] = Lambdas::RowHit::Bank::RDWR<GDDR6>;
    // }

    // Not implemented
    // void set_rowopens() {
    //     m_rowopens.resize(m_levels.size(), std::vector<RowhitFunc_t<Node>>(m_commands.size()));

    //     m_rowopens[m_levels["bank"]][m_commands["RD"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;
    //     m_rowopens[m_levels["bank"]][m_commands["WR"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;

    //     m_rowopens[m_levels["bank"]][m_commands["RDCP"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;
    //     m_rowopens[m_levels["bank"]][m_commands["WRCP"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;
    //     m_rowopens[m_levels["bank"]][m_commands["MAC"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;
    //     m_rowopens[m_levels["bank"]][m_commands["MAC16"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;
    //     m_rowopens[m_levels["bank"]][m_commands["AF16"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;
    //     m_rowopens[m_levels["bank"]][m_commands["EWMUL16"]] = Lambdas::RowOpen::Bank::RDWR<GDDR6>;
    // }

    void create_nodes() {
        int num_channels = m_organization.count[m_levels["channel"]];
        for (int i = 0; i < num_channels; i++) {
            Node *channel = new Node(this, nullptr, 0, i);
            m_channels.push_back(channel);
        }
    };
};

} // namespace Ramulator
