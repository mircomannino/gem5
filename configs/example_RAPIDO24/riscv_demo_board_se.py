# Copyright (c) 2023 University of Siena
# Copyright (c) 2022 The Regents of the University of California
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""
This gem5 configuation script runs a binary on the
RISCVMatched prebuilt board found in src/python/gem5/prebuilt/riscvmatched/

Usage
-----

```
scons build/RISCV/gem5.opt
./build/RISCV/gem5.opt \
    configs/example_RAPIDO24//riscv_demo_board_se.py \
	--workload=/path/to/riscv/binary
```
"""

from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator
from gem5.prebuilt.riscvmatched.riscvmatched_board import RISCVMatchedBoard
from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.memory import DualChannelDDR4_2400
from gem5.components.processors.cpu_types import CPUTypes
from gem5.components.processors.simple_core import SimpleCore
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.cachehierarchies.classic.private_l1_private_l2_cache_no_ptwc_hierarchy import (
    PrivateL1PrivateL2CacheNoPTWCHierarchy,
)
from gem5.isas import ISA
from gem5.utils.requires import requires



requires(isa_required=ISA.RISCV)

import argparse

parser = argparse.ArgumentParser(
    description="A script which uses the RISCVMatchedBoard in SE mode."
)

# Additional options
parser.add_argument(
    "--workload",
    type=str,
    required=True,
    help="Path of the binary to run",
)
parser.add_argument(
    "--options",
    type=str,
    help="Options of the binary",
)
parser.add_argument(
    "--use-arch-pt",
    action="store_true",
    default=False,
    help="Enable the use of page table in SE mode. Default: False",
)
parser.add_argument(
    "--dtb-size",
    type=int,
    default=64,
    help="Number of data TLB entries. Default: 64"
)
parser.add_argument(
    "--itb-size",
    type=int,
    default=64,
    help="Number of data TLB entries. Default: 64"
)

args = parser.parse_args()

# Processor setup
processor = SimpleProcessor(
    cpu_type=CPUTypes.O3, isa=ISA.RISCV, num_cores=1
)
for core in processor.get_cores():
    core.get_mmu().itb.size = args.itb_size
    core.get_mmu().dtb.size = args.dtb_size

# Cache setup
cache_hierarchy = PrivateL1PrivateL2CacheNoPTWCHierarchy(
    l1d_size="32kB", l1i_size="32kB", l2_size="256kB"
)

# Memory setup
memory = DualChannelDDR4_2400(size="8GB")

# instantiate the riscv board
board = SimpleBoard(
    clk_freq = '3GHz',
    processor = processor,
    memory = memory,
    cache_hierarchy = cache_hierarchy
)

# set the riscv binary as the board workload
arguments = [option for option in (args.options).split()] if args.options != None else []
board.set_se_binary_workload(
    binary=CustomResource(args.workload),
    arguments=arguments,
    use_arch_pt = args.use_arch_pt
)

# run the simulation with the RISCV board
simulator = Simulator(board=board, full_system=False)
simulator.run()

print(
    "Exiting @ tick {} because {}.".format(
        simulator.get_current_tick(),
        simulator.get_last_exit_event_cause(),
    )
)
