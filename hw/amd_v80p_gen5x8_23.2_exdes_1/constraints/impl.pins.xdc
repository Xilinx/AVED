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

# xcv80-lsva4737-2MHP-e-S pins constraints XDC

set_property -dict { PACKAGE_PIN BR15    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_act_n[0]"]     ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_ACT_B - IO_L18N_XCC_N6P1_M0P37_700
set_property -dict { PACKAGE_PIN BP15    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[0]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A0 - IO_L18P_XCC_N6P0_M0P36_700
set_property -dict { PACKAGE_PIN BU8     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[1]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A1 - IO_L17N_N5P5_M0P35_700
set_property -dict { PACKAGE_PIN BT9     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[2]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A2 - IO_L17P_N5P4_M0P34_700
set_property -dict { PACKAGE_PIN BR14    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[3]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A3 - IO_L20P_N6P4_M0P40_700
set_property -dict { PACKAGE_PIN BN10    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[4]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A4 - IO_L12P_GC_XCC_N4P0_M0P24_700
set_property -dict { PACKAGE_PIN BT6     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[5]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A5 - IO_L26P_N8P4_M0P52_700
set_property -dict { PACKAGE_PIN BR8     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[6]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A6 - IO_L24P_GC_XCC_N8P0_M0P48_700
set_property -dict { PACKAGE_PIN BP7     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[7]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A7 - IO_L6N_GC_XCC_N2P1_M0P13_700
set_property -dict { PACKAGE_PIN BR6     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[8]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A8 - IO_L25N_N8P3_M0P51_700
set_property -dict { PACKAGE_PIN BN14    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[9]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A9 - IO_L19P_N6P2_M0P38_700
set_property -dict { PACKAGE_PIN BR12    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[10]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A10 - IO_L21N_XCC_N7P1_M0P43_700
set_property -dict { PACKAGE_PIN BR7     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[11]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A11 - IO_L25P_N8P2_M0P50_700
set_property -dict { PACKAGE_PIN BN2     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[12]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A12 - IO_L0N_XCC_N0P1_M0P1_700
set_property -dict { PACKAGE_PIN BT8     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[13]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_A13 - IO_L24N_GC_XCC_N8P1_M0P49_700
set_property -dict { PACKAGE_PIN BU10    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[14]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_WE_B - IO_L16N_N5P3_M0P33_700
set_property -dict { PACKAGE_PIN BR9     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[15]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_CAS_B - IO_L14N_N4P5_M0P29_700
set_property -dict { PACKAGE_PIN BN13    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_adr[16]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_RAS_B - IO_L19N_N6P3_M0P39_700
set_property -dict { PACKAGE_PIN BP13    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_ba[0]"]        ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_BA0 - IO_L20N_N6P5_M0P41_700
set_property -dict { PACKAGE_PIN BN9     IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_ba[1]"]        ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_BA1 - IO_L12N_GC_XCC_N4P1_M0P25_700
set_property -dict { PACKAGE_PIN BT13    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_bg[0]"]        ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_BG0 - IO_L21P_XCC_N7P0_M0P42_700
set_property -dict { PACKAGE_PIN BT11    IOSTANDARD DIFF_SSTL12                       } [get_ports "CH0_DDR4_0_0_ck_t[0]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_CK_T0 - IO_L15P_XCC_N5P0_M0P30_700
set_property -dict { PACKAGE_PIN BN12    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_cke[0]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_CKE0 - IO_L23P_N7P4_M0P46_700
set_property -dict { PACKAGE_PIN BP10    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_cs_n[0]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_CS_B0 - IO_L14P_N4P4_M0P28_700
set_property -dict { PACKAGE_PIN BY13    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[0]"]      ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B0 - IO_L18P_XCC_N6P0_M0P90_701
set_property -dict { PACKAGE_PIN BU13    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[1]"]      ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B1 - IO_L12P_GC_XCC_N4P0_M0P78_701
set_property -dict { PACKAGE_PIN CC7     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[2]"]      ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B2 - IO_L6P_GC_XCC_N2P0_M0P120_702
set_property -dict { PACKAGE_PIN CD9     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[3]"]      ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B3 - IO_L12P_GC_XCC_N4P0_M0P132_702
set_property -dict { PACKAGE_PIN CA4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[4]"]      ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B4 - IO_L9P_GC_XCC_N3P0_M0P72_701
set_property -dict { PACKAGE_PIN BV5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[5]"]      ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B5 - IO_L0P_XCC_N0P0_M0P54_701
set_property -dict { PACKAGE_PIN CE2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[6]"]      ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B6 - IO_L3P_XCC_N1P0_M0P114_702
set_property -dict { PACKAGE_PIN BN3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[7]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B7 - IO_L0P_XCC_N0P0_M0P0_700
set_property -dict { PACKAGE_PIN BN7     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dm_n[8]"]      ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DM_B8 - IO_L6P_GC_XCC_N2P0_M0P12_700
set_property -dict { PACKAGE_PIN BY12    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[0]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ0 - IO_L19N_N6P3_M0P93_701
set_property -dict { PACKAGE_PIN CA11    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[1]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ1 - IO_L22P_N7P2_M0P98_701
set_property -dict { PACKAGE_PIN CA10    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[2]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ2 - IO_L22N_N7P3_M0P99_701
set_property -dict { PACKAGE_PIN CB12    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[3]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ3 - IO_L20N_N6P5_M0P95_701
set_property -dict { PACKAGE_PIN CB8     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[4]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ4 - IO_L23N_N7P5_M0P101_701
set_property -dict { PACKAGE_PIN CA13    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[5]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ5 - IO_L19P_N6P2_M0P92_701
set_property -dict { PACKAGE_PIN CA9     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[6]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ6 - IO_L23P_N7P4_M0P100_701
set_property -dict { PACKAGE_PIN CB13    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[7]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ7 - IO_L20P_N6P4_M0P94_701
set_property -dict { PACKAGE_PIN BV8     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[8]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ8 - IO_L17P_N5P4_M0P88_701
set_property -dict { PACKAGE_PIN BV9     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[9]"]        ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ9 - IO_L16P_N5P2_M0P86_701
set_property -dict { PACKAGE_PIN BV12    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[10]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ10 - IO_L13N_N4P3_M0P81_701
set_property -dict { PACKAGE_PIN BW9     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[11]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ11 - IO_L16N_N5P3_M0P87_701
set_property -dict { PACKAGE_PIN BV7     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[12]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ12 - IO_L17N_N5P5_M0P89_701
set_property -dict { PACKAGE_PIN BV10    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[13]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ13 - IO_L14N_N4P5_M0P83_701
set_property -dict { PACKAGE_PIN BV13    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[14]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ14 - IO_L13P_N4P2_M0P80_701
set_property -dict { PACKAGE_PIN BW11    IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[15]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ15 - IO_L14P_N4P4_M0P82_701
set_property -dict { PACKAGE_PIN CD5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[16]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ16 - IO_L10P_N3P2_M0P128_702
set_property -dict { PACKAGE_PIN CF4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[17]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ17 - IO_L8N_N2P5_M0P125_702
set_property -dict { PACKAGE_PIN CD4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[18]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ18 - IO_L10N_N3P3_M0P129_702
set_property -dict { PACKAGE_PIN CF5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[19]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ19 - IO_L7N_N2P3_M0P123_702
set_property -dict { PACKAGE_PIN CC4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[20]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ20 - IO_L11N_N3P5_M0P131_702
set_property -dict { PACKAGE_PIN CG5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[21]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ21 - IO_L8P_N2P4_M0P124_702
set_property -dict { PACKAGE_PIN CC5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[22]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ22 - IO_L11P_N3P4_M0P130_702
set_property -dict { PACKAGE_PIN CE6     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[23]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ23 - IO_L7P_N2P2_M0P122_702
set_property -dict { PACKAGE_PIN CJ9     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[24]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ24 - IO_L16P_N5P2_M0P140_702
set_property -dict { PACKAGE_PIN CJ6     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[25]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ25 - IO_L17N_N5P5_M0P143_702
set_property -dict { PACKAGE_PIN CF9     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[26]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ26 - IO_L14P_N4P4_M0P136_702
set_property -dict { PACKAGE_PIN CJ7     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[27]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ27 - IO_L17P_N5P4_M0P142_702
set_property -dict { PACKAGE_PIN CE8     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[28]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ28 - IO_L13N_N4P3_M0P135_702
set_property -dict { PACKAGE_PIN CH8     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[29]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ29 - IO_L16N_N5P3_M0P141_702
set_property -dict { PACKAGE_PIN CE9     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[30]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ30 - IO_L13P_N4P2_M0P134_702
set_property -dict { PACKAGE_PIN CF8     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[31]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ31 - IO_L14N_N4P5_M0P137_702
set_property -dict { PACKAGE_PIN CB2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[32]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ32 - IO_L10N_N3P3_M0P75_701
set_property -dict { PACKAGE_PIN CA5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[33]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ33 - IO_L7N_N2P3_M0P69_701
set_property -dict { PACKAGE_PIN CB3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[34]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ34 - IO_L10P_N3P2_M0P74_701
set_property -dict { PACKAGE_PIN CB5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[35]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ35 - IO_L8N_N2P5_M0P71_701
set_property -dict { PACKAGE_PIN CA1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[36]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ36 - IO_L11P_N3P4_M0P76_701
set_property -dict { PACKAGE_PIN BY6     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[37]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ37 - IO_L7P_N2P2_M0P68_701
set_property -dict { PACKAGE_PIN CB1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[38]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ38 - IO_L11N_N3P5_M0P77_701
set_property -dict { PACKAGE_PIN CA6     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[39]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ39 - IO_L8P_N2P4_M0P70_701
set_property -dict { PACKAGE_PIN BV2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[40]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ40 - IO_L5N_N1P5_M0P65_701
set_property -dict { PACKAGE_PIN BY3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[41]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ41 - IO_L2N_N0P5_M0P59_701
set_property -dict { PACKAGE_PIN BW1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[42]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ42 - IO_L4N_N1P3_M0P63_701
set_property -dict { PACKAGE_PIN BW4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[43]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ43 - IO_L1N_N0P3_M0P57_701
set_property -dict { PACKAGE_PIN BV3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[44]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ44 - IO_L5P_N1P4_M0P64_701
set_property -dict { PACKAGE_PIN BY4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[45]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ45 - IO_L2P_N0P4_M0P58_701
set_property -dict { PACKAGE_PIN BW2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[46]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ46 - IO_L4P_N1P2_M0P62_701
set_property -dict { PACKAGE_PIN BW5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[47]"]       ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ47 - IO_L1P_N0P2_M0P56_701
set_property -dict { PACKAGE_PIN CD2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[48]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ48 - IO_L1P_N0P2_M0P110_702
set_property -dict { PACKAGE_PIN CF3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[49]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ49 - IO_L4P_N1P2_M0P116_702
set_property -dict { PACKAGE_PIN CD1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[50]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ50 - IO_L1N_N0P3_M0P111_702
set_property -dict { PACKAGE_PIN CF2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[51]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ51 - IO_L4N_N1P3_M0P117_702
set_property -dict { PACKAGE_PIN CE3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[52]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ52 - IO_L2N_N0P5_M0P113_702
set_property -dict { PACKAGE_PIN CG3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[53]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ53 - IO_L5N_N1P5_M0P119_702
set_property -dict { PACKAGE_PIN CE4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[54]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ54 - IO_L2P_N0P4_M0P112_702
set_property -dict { PACKAGE_PIN CH4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[55]"]       ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ55 - IO_L5P_N1P4_M0P118_702
set_property -dict { PACKAGE_PIN BP2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[56]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ56 - IO_L1P_N0P2_M0P2_700
set_property -dict { PACKAGE_PIN BU2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[57]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ57 - IO_L4N_N1P3_M0P9_700
set_property -dict { PACKAGE_PIN BR2     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[58]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ58 - IO_L2P_N0P4_M0P4_700
set_property -dict { PACKAGE_PIN BT1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[59]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ59 - IO_L5P_N1P4_M0P10_700
set_property -dict { PACKAGE_PIN BP1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[60]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ60 - IO_L1N_N0P3_M0P3_700
set_property -dict { PACKAGE_PIN BU1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[61]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ61 - IO_L5N_N1P5_M0P11_700
set_property -dict { PACKAGE_PIN BR1     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[62]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ62 - IO_L2N_N0P5_M0P5_700
set_property -dict { PACKAGE_PIN BU3     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[63]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ63 - IO_L4P_N1P2_M0P8_700
set_property -dict { PACKAGE_PIN BN5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[64]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ64 - IO_L8P_N2P4_M0P16_700
set_property -dict { PACKAGE_PIN BP5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[65]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ65 - IO_L7N_N2P3_M0P15_700
set_property -dict { PACKAGE_PIN BP6     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[66]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ66 - IO_L7P_N2P2_M0P14_700
set_property -dict { PACKAGE_PIN BP4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[67]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ67 - IO_L11P_N3P4_M0P22_700
set_property -dict { PACKAGE_PIN BT4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[68]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ68 - IO_L10N_N3P3_M0P21_700
set_property -dict { PACKAGE_PIN BN4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[69]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ69 - IO_L8N_N2P5_M0P17_700
set_property -dict { PACKAGE_PIN BU5     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[70]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ70 - IO_L10P_N3P2_M0P20_700
set_property -dict { PACKAGE_PIN BR4     IOSTANDARD POD12                             } [get_ports "CH0_DDR4_0_0_dq[71]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQ71 - IO_L11N_N3P5_M0P23_700
set_property -dict { PACKAGE_PIN CB11    IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[0]"]     ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T0 - IO_L21P_XCC_N7P0_M0P96_701
set_property -dict { PACKAGE_PIN BY11    IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[1]"]     ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T1 - IO_L15P_XCC_N5P0_M0P84_701
set_property -dict { PACKAGE_PIN CH5     IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[2]"]     ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T2 - IO_L9P_GC_XCC_N3P0_M0P126_702
set_property -dict { PACKAGE_PIN CH9     IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[3]"]     ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T3 - IO_L15P_XCC_N5P0_M0P138_702
set_property -dict { PACKAGE_PIN CB7     IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[4]"]     ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T4 - IO_L6P_GC_XCC_N2P0_M0P66_701
set_property -dict { PACKAGE_PIN BY2     IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[5]"]     ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T5 - IO_L3P_XCC_N1P0_M0P60_701
set_property -dict { PACKAGE_PIN CC3     IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[6]"]     ;# Bank 702 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T6 - IO_L0P_XCC_N0P0_M0P108_702
set_property -dict { PACKAGE_PIN BR3     IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[7]"]     ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T7 - IO_L3P_XCC_N1P0_M0P6_700
set_property -dict { PACKAGE_PIN BU7     IOSTANDARD DIFF_POD12                        } [get_ports "CH0_DDR4_0_0_dqs_t[8]"]     ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_DQS_T8 - IO_L9P_GC_XCC_N3P0_M0P18_700
set_property -dict { PACKAGE_PIN BP11    IOSTANDARD SSTL12                            } [get_ports "CH0_DDR4_0_0_odt[0]"]       ;# Bank 700 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_ODT0 - IO_L23N_N7P5_M0P47_700
set_property -dict { PACKAGE_PIN BW7     IOSTANDARD LVCMOS12                          } [get_ports "CH0_DDR4_0_0_reset_n[0]"]   ;# Bank 701 VCCO  - VR_1V2_VCCO_DDR4 -Net CH0_DDR4_0_0_RESET_B - IO_L25P_N8P2_M0P104_701


set_property -dict { PACKAGE_PIN BY9     IOSTANDARD LVDS15                            } [get_ports "sys_clk0_0_clk_p"]          ;# Bank 701 VCCO - VR_1V2_VCCO_DDR4                       - IO_L24P_GC_XCC_N8P0_M0P102_701

set_property -dict { PACKAGE_PIN N18                                                  } [get_ports "hbm_ref_clk_0_clk_p"]   ;# Bank 800 " C4CCIO_PAD1_0_800
set_property -dict { PACKAGE_PIN N19                                                  } [get_ports "hbm_ref_clk_0_clk_n"]   ;# Bank 800 " C4CCIO_PAD1_0_800
set_property -dict { PACKAGE_PIN N38                                                  } [get_ports "hbm_ref_clk_1_clk_p"]   ;# Bank 801 " C4CCIO_PAD1_1_801
set_property -dict { PACKAGE_PIN N37                                                  } [get_ports "hbm_ref_clk_1_clk_n"]   ;# Bank 801 " C4CCIO_PAD1_1_801

# SMBus is not currently implemented
#set_property -dict { PACKAGE_PIN CG6     IOSTANDARD LVCMOS12    SLEW SLOW   DRIVE 8   } [get_ports "smbus_0_scl_io"]        ;# Bank 702 VCCO - VR_1V2_VCCO_DDR4
#set_property -dict { PACKAGE_PIN CH7     IOSTANDARD LVCMOS12    SLEW SLOW   DRIVE 8   } [get_ports "smbus_0_sda_io"]        ;# Bank 702 VCCO - VR_1V2_VCCO_DDR4
