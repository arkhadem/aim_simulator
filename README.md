# SK hynix Accelerator-in-Memory (AiM) Simulator

This repository provides a trace-driven simulator for AiM PIM architecutre [1-4] based on Ramulator 2.0 trace-driven simulator [5].

## Description

### Modules

To enable AiM PIM capabilities in GDDR6 memory channels, we augmented Ramulator 2.0 with the following modules:

1. AiM ISR (Instruction Set Register) Instruction Definitions
  - AiM ISR instructions are issued by the host to the DRAM system.
2. AiM Trace-Driven Frontend
  - The trace-driven frontend reads ISR instructions from a trace file.
3. AiM DRAM System
  - Decodes ISR instructions, unrolls them, and generates multiple memory requests across multiple channels.
    - Uses the `opcode` and `channel_mask` fields to determine the unrolling factor (*i.e.*, the number of consecutive DRAM columns) and the target memory channels.
  - Includes configuration registers (CFRs) and general-purpose registers (GPRs). Each GPR is 256 bits, matching GDDR6 access granularity, and can store 16 FP16 elements.
4. AiM DRAM Controller
  - Decodes and issues DRAM commands to memory banks.
  - Contains a Global Buffer, which houses 256-bit registers for MAC and element-wise operations.
  - To manage data dependencies between AiM and conventional memory accesses, the controller:
    - Processes AiM memory requests in order.
    - Blocks new AiM or conventional memory requests if there is an outstanding request of the opposite type in the buffers.
5. AiM GDDR6 Memory
  - Contains timing constraints, prerequisites, and actions for AiM DRAM commands.

For more information about AiM, please refer to [1-4].

### Implemented ISRs

The following table lists all supported instructions:

| Type | Instruction | Detail |
| :-----: | :-------------: | :----: |
| AiM | WR_SBK GPR_0 channel_mask bank row | write 256b from GPR to a single bank |
| AiM | WR_ABK GPR_0 channel_mask row | write 256b from GPR to 16 banks |
| AiM | WR_GB opsize GPR_0 channel_mask | write 256b from GPR to GB |
| AiM | WR_BIAS GPR_0 channel_mask | write 256b from GPR to 16b MAC registers of 16 banks |
| AiM | RD_MAC GPR_0 channel_mask | read 256b from 16b MAC registers of 16 banks to GPR |
| AiM | RD_AF GPR_0 channel_mask | read 256b from 16b AF registers of 16 banks to GPR |
| AiM | RD_SBK GPR_0 channel_mask bank row | write 256b from a single bank to GPR |
| AiM | COPY_BKGB opsize channel_mask bank row | copy 256b from 16 banks to GB |
| AiM | COPY_GBBK opsize channel_mask bank row | copy 256b from GB to 16 banks |
| AiM | MAC_SBK opsize channel_mask bank row | perform 256b MAC in a single bank |
| AiM | MAC_ABK opsize channel_mask row | perform 256b MAC in all banks |
| AiM | AF channel_mask | perform activation function in all banks |
| AiM | EWMUL opsize channel_mask row | perform element-wise multiplication in 4 banks |
| AiM | EWADD opsize GPR_0 GPR_1 | perform element-wise addition using GPRs |
| AiM | SYNC | barrier between ISR instructions |
| AiM | EOC | end of compute, finishes simulation |
| W | MEM channel_id bank row | conventional write access to a bank |
| R | MEM channel_id bank row | conventional read access from a bank |
| W | GPR GPR_0 | write 256b to GPR |
| R | GPR GPR_0 | read 256b from GPR |
| W | CFR CFR_0 data | configure CFR with data |

Notes:
- `channel_mask` must specify only one channel for `WR_ABK` AiM ISR (*e.g.,* 1, 2, 4, etc).
- `RD_MAC`, `RD_AF`, `SYNC`, and `EOC` block the DRAM system until executed by DRAM controller. This blocking mechanism ensures data hazards are properly managed between GPR reads and writes.

### Trace File

Each line of the trace shal contain a comment (`# COMMENT`), AiM ISR instruction (`AiM [Instruction]`), or conventional read and write GPR/CFR/memory access (`R|W [instruction]`).
Please refer to `test/all_isr.trace` to find trace examples.

## Getting Started with AiM Simulator

### Dependencies

AiM simulator is implemented by Ramulator 2.0 which requires `g++-12` or `clang++-15` for compilation.

### build

Clone the repository:
```bash
  $ git clone git@github.com:arkhadem/aim_simulator.git
  $ cd aim_simulator
```

Build the simulator:
```bash
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make -j
  $ cd ..
```

### Run
```bash
  $ ./build/ramulator2 -f [config.yaml] -t [your.trace]
```

Please refer to `test/example.yaml` to find a config file example.

**NOTE:** The current version of AiM simulator supports 32 channels (this is not configurable).

## Citation

If you use AiM simulator, please cite the following paper as well as Ramulator 2.0 [5]:

> Yufeng Gu, Alireza Khadem, Sumanth Umesh, Ning Liang, Xavier Servot, Onur Mutlu, Ravi Iyer, and Reetuparna Das.
> *PIM is All You Need: A CXL-Enabled GPU-Free System for LLM Inference*,
> In 2025 International Conference on Architectural Support for Programming Languages and Operating Systems (ASPLOS)

```
@inproceedings{cent,
  title={PIM is All You Need: A CXL-Enabled GPU-Free System for LLM Inference},
  author={Gu, Yufeng and Khadem, Alireza and Umesh, Sumanth, and Liang, Ning and Servot, Xavier and Mutlu, Onur and Iyer, Ravi and and Das, Reetuparna},
  booktitle={2025 International Conference on Architectural Support for Programming Languages and Operating Systems (ASPLOS)}, 
  year={2025}
}
```

## Issues and bug reporting

We appreciate any feedback and suggestions from the community.
Feel free to raise an issue or submit a pull request on Github.
For assistance in using CENT, please contact: Yufeng Gu (yufenggu@umich.edu) and Alireza Khadem (arkhadem@umich.edu)

## Licensing

This repository is available under a [MIT license](/LICENSE).
please refer to [Ramulator](https://github.com/CMU-SAFARI/ramulator) for its corresponding license.

## Acknowledgement

This work was supported in part by the NSF under the CAREER-1652294 and NSF-1908601 awards and by Intel gift.

## References

[1] Y. Kwon et. al., "System architecture and software stack for GDDR6-AiM,". In 2022 IEEE Hot Chips 34 Symposium (HCS).

[2] Y. Kwon et al., "Memory-Centric Computing with SK Hynix's Domain-Specific Memory," 2023 IEEE Hot Chips 35 Symposium (HCS).

[3] S. Lee et. al., "A 1ynm 1.25 v 8gb, 16gb/s/pin gddr6-based accelerator-in-memory supporting 1tflops mac operation and various activation functions for deep-learning applications," In 2022 IEEE International Solid-State Circuits Conference (ISSCC).

[4] D. Kwon et al., "A 1ynm 1.25V 8Gb 16Gb/s/Pin GDDR6-Based Accelerator-in-Memory Supporting 1TFLOPS MAC Operation and Various Activation Functions for Deep Learning Application," in 2023 IEEE Journal of Solid-State Circuits (JSSC).

[5] H. Luo et. al., "Ramulator 2.0: A Modern, Modular, and Extensible DRAM Simulator," In 2024 IEEE Computuer Architecture Letter.