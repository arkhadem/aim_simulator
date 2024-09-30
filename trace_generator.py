filename = "trace_test.txt"
# # op_size_tile should be at most 32 to not overflow the request buffer of the DRAM controller
# op_size_tile = 16
# total_bursts = 64
# with open(filename, "w") as f:
#     for burst_group in range(int(total_bursts / op_size_tile)):
#         # outermost loop because of row buffer hit
#         for row in range(16):
#             # for bank-level parallelism between bank groups, banks are interleaved
#             for bank in [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15]:
#                 # Channel inner-most loop for one channel to not block other channels receiveing requests
#                 for channel in range(32):
#                     hex_channel = hex(1 << channel)
#                     f.write(f"AiM RD_SBK {op_size_tile} 0 {hex_channel} {bank} {row}\n")
#     f.write("AiM EOC\n")

# op_size_tile should be at most 32 to not overflow the request buffer of the DRAM controller
total_bursts = 64
hex_channel = 0xFFFFFFFF
with open(filename, "w") as f:
    # outermost loop because of row buffer hit
    for row in range(16):
        # to be bounded by TCCDS rather than TCCDL, column loop is before the bank loop
        for column in range(total_bursts):
            # for bank-level parallelism between bank groups, banks are interleaved
            for bank in [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15]:
                f.write(f"AiM WR_SBK 0 {hex_channel} {bank} {row}\n")
    f.write("AiM EOC\n")