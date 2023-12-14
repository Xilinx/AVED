# (c) Copyright 2023, Advanced Micro Devices, Inc.
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
## xbtest physical constraints

# SLR2 pblock
add_cells_to_pblock -quiet pblock_SLR2 [get_cells -quiet top_i/xbtest/krnl_memtest_hbm_*]
add_cells_to_pblock -quiet pblock_SLR2 [get_cells -quiet top_i/xbtest/krnl_gtyp_prbs_test_03_02_1]
add_cells_to_pblock -quiet pblock_SLR2 [get_cells -quiet top_i/xbtest/krnl_powertest_slr2*]
add_cells_to_pblock -quiet pblock_SLR2 [get_cells -quiet top_i/xbtest/pcie_slr2_sc]
add_cells_to_pblock -quiet pblock_SLR2 [get_cells -quiet top_i/xbtest/plram_slr2]

# SLR1 pblock
add_cells_to_pblock -quiet pblock_SLR1 [get_cells -quiet top_i/xbtest/krnl_gtm_prbs_test_04_01_1]
add_cells_to_pblock -quiet pblock_SLR1 [get_cells -quiet top_i/xbtest/krnl_powertest_slr1*]
add_cells_to_pblock -quiet pblock_SLR1 [get_cells -quiet top_i/xbtest/pcie_slr1_sc]
add_cells_to_pblock -quiet pblock_SLR1 [get_cells -quiet top_i/xbtest/plram_slr1]

# SLR0 pblock
add_cells_to_pblock -quiet pblock_SLR0 [get_cells -quiet top_i/xbtest/krnl_verify*]
add_cells_to_pblock -quiet pblock_SLR0 [get_cells -quiet top_i/xbtest/krnl_memtest_ddr_*]
add_cells_to_pblock -quiet pblock_SLR0 [get_cells -quiet top_i/xbtest/krnl_gtyp_prbs_test_01_00_1]
add_cells_to_pblock -quiet pblock_SLR0 [get_cells -quiet top_i/xbtest/krnl_powertest_slr0*]
add_cells_to_pblock -quiet pblock_SLR0 [get_cells -quiet top_i/xbtest/pcie_slr0_sc]
add_cells_to_pblock -quiet pblock_SLR0 [get_cells -quiet top_i/xbtest/plram_slr0]

# DDR NMUs
set_property -quiet LOC NOC_NMU512_X2Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU512} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_ddr_*/m01_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU512_X3Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU512} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_ddr_*/m02_axi_wvalid]]]]

# PCIE NSUs
set_property -quiet LOC NOC_NSU512_X2Y14  [get_cells -quiet -filter {REF_NAME == NOC_NSU512} -of [get_pins -quiet -leaf -filter {DIRECTION == OUT} -of [get_nets -of [get_pins -quiet top_i/xbtest/pcie_slr2_sc/S00_AXI_wvalid]]]]
set_property -quiet LOC NOC_NSU512_X1Y10  [get_cells -quiet -filter {REF_NAME == NOC_NSU512} -of [get_pins -quiet -leaf -filter {DIRECTION == OUT} -of [get_nets -of [get_pins -quiet top_i/xbtest/pcie_slr1_sc/S00_AXI_wvalid]]]]
set_property -quiet LOC NOC_NSU512_X3Y1   [get_cells -quiet -filter {REF_NAME == NOC_NSU512} -of [get_pins -quiet -leaf -filter {DIRECTION == OUT} -of [get_nets -of [get_pins -quiet top_i/xbtest/pcie_slr0_sc/S00_AXI_wvalid]]]]


# HBM NMUs
set_property -quiet LOC NOC_NMU_HBM2E_X0Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m01_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X1Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m02_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X2Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m03_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X3Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m04_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X4Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m05_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X5Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m06_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X6Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m07_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X7Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m08_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X8Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m09_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X9Y0  [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m10_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X10Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m11_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X11Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m12_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X12Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m13_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X13Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m14_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X14Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m15_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X15Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m16_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X16Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m17_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X17Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m18_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X18Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m19_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X19Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m20_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X20Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m21_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X21Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m22_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X22Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m23_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X23Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m24_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X24Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m25_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X25Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m26_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X26Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m27_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X27Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m28_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X28Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m29_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X29Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m30_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X30Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m31_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X31Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m32_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X32Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m33_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X33Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m34_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X34Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m35_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X35Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m36_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X36Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m37_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X37Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m38_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X38Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m39_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X39Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m40_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X40Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m41_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X41Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m42_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X42Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m43_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X43Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m44_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X44Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m45_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X45Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m46_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X46Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m47_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X47Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m48_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X48Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m49_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X49Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m50_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X50Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m51_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X51Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m52_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X52Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m53_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X53Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m54_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X54Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m55_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X55Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m56_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X56Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m57_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X57Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m58_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X58Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m59_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X59Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m60_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X60Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m61_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X61Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m62_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X62Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m63_axi_wvalid]]]]
set_property -quiet LOC NOC_NMU_HBM2E_X63Y0 [get_cells -quiet -filter {REF_NAME == NOC_NMU_HBM2E} -of [get_pins -quiet -leaf -filter {DIRECTION == IN} -of [get_nets -quiet -of [get_pins -quiet */xbtest/krnl_memtest_hbm_*/m64_axi_wvalid]]]]
