# Integration of RISC-V Page Table Walk in gem5 SE Mode
This branch enables the use of page table walk in SE mode (RISC-V ISA).

## Differences
All the differences, compared to gem5 v23.0.1.0, are shown in ```gem5.diff``` file.

## Usage
The configuration file includes additional options:
```bash
  --workload WORKLOAD  Path of the binary to run
  --options OPTIONS    Options of the binary
  --use-arch-pt        Enable the use of page table in SE mode Default: False
  --dtb-size DTB_SIZE  Number of data TLB entries. Default: 64
  --itb-size ITB_SIZE  Number of instruction TLB entries. Default: 64
```
The most important option is ```--use-arch-pt``` that enables the use of the page table walk in gem5 SE.

An example of configuration script can be found in ```configs/example_RAPIDO24/riscv_demo_board.py```

## Example
The following command can be used to run a simulation with the demo board:
```
build/RISCV/gem5.opt \
    configs/example_RAPIDO24/riscv_demo_board_se.py \
    --workload <path/to/your/binary> \
    --options <options of your binary> \
    --use-arch-pt \
    --dtlb-size 1024 \
    --itlb-size 1024
```
