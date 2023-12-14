# (c) Copyright 2022-2023, Advanced Micro Devices, Inc.
# 
# Permission is hereby granted, free of charge, to any person obtaining a 
# copy of this software and associated documentation files (the "Software"), 
# to deal in the Software without restriction, including without limitation 
# the rights to use, copy, modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
# DEALINGS IN THE SOFTWARE.
############################################################

# don't care about the reset performance
set_false_path -quiet -from [get_pins top_i/clock_reset/usr_?_psr/U0/ACTIVE_LOW_BSR_OUT_DFF[0].FDRE_BSR_N/C]
set_false_path -quiet -from [get_pins top_i/clock_reset/usr_?_psr/U0/ACTIVE_LOW_PR_OUT_DFF[0].FDRE_PER_N/C]

# SLR2 pblock
create_pblock pblock_SLR2
resize_pblock [get_pblocks pblock_SLR2] -add {CLOCKREGION_X0Y8:CLOCKREGION_X9Y10}
resize_pblock [get_pblocks pblock_SLR2] -add {CLOCKREGION_X0Y11:CLOCKREGION_X8Y11}

# SLR1 pblock
create_pblock pblock_SLR1
resize_pblock [get_pblocks pblock_SLR1] -add {CLOCKREGION_X0Y5:CLOCKREGION_X9Y7}

# SLR0 pblock
create_pblock pblock_SLR0
resize_pblock [get_pblocks pblock_SLR0] -add {CLOCKREGION_X0Y0:CLOCKREGION_X12Y0}
resize_pblock [get_pblocks pblock_SLR0] -add {CLOCKREGION_X0Y1:CLOCKREGION_X9Y4}
add_cells_to_pblock pblock_SLR0 [get_cells top_i/clock_reset]
add_cells_to_pblock pblock_SLR0 [get_cells -hierarchical -filter {PARENT =~ "top_i/base_logic" && NAME !~ "top_i/base_logic/pcie_slr*_sc"}]
add_cells_to_pblock pblock_SLR0 [get_cells top_i/base_logic/pcie_slr0_mgmt_sc]

# Indicate that SLR pblocks must strictly be obeyed.
set_property IS_SOFT FALSE [get_pblocks pblock_SLR*]

# BASE NSUs/NMUs
set_property LOC NOC_NSU512_X0Y0  [get_cells -filter {REF_NAME == NOC_NSU512} -of [get_pins -leaf -filter {DIRECTION == OUT} -of [get_nets -of [get_pins top_i/base_logic/pcie_slr0_mgmt_sc/S00_AXI_wvalid]]]]

