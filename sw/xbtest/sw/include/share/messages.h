// Copyright (C) 2022 Xilinx, Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _MESSAGES_H
#define _MESSAGES_H

namespace xbtest
{

#define PARSE_MESSAGES() \
PARSE_MESSAGE(MSG_GEN_001) \
PARSE_MESSAGE(MSG_GEN_002) \
PARSE_MESSAGE(MSG_GEN_003) \
PARSE_MESSAGE(MSG_GEN_004) \
PARSE_MESSAGE(MSG_GEN_005) \
PARSE_MESSAGE(MSG_GEN_006) \
PARSE_MESSAGE(MSG_GEN_007) \
PARSE_MESSAGE(MSG_GEN_008) \
PARSE_MESSAGE(MSG_GEN_009) \
PARSE_MESSAGE(MSG_GEN_010) \
PARSE_MESSAGE(MSG_GEN_011) \
PARSE_MESSAGE(MSG_GEN_012) \
PARSE_MESSAGE(MSG_GEN_013) \
PARSE_MESSAGE(MSG_GEN_014) \
PARSE_MESSAGE(MSG_GEN_015) \
PARSE_MESSAGE(MSG_GEN_016) \
PARSE_MESSAGE(MSG_GEN_017) \
PARSE_MESSAGE(MSG_GEN_018) \
PARSE_MESSAGE(MSG_GEN_019) \
PARSE_MESSAGE(MSG_GEN_020) \
PARSE_MESSAGE(MSG_GEN_021) \
PARSE_MESSAGE(MSG_GEN_022) \
PARSE_MESSAGE(MSG_GEN_023) \
PARSE_MESSAGE(MSG_GEN_024) \
PARSE_MESSAGE(MSG_GEN_025) \
PARSE_MESSAGE(MSG_GEN_026) \
PARSE_MESSAGE(MSG_GEN_027) \
PARSE_MESSAGE(MSG_GEN_028) \
PARSE_MESSAGE(MSG_GEN_029) \
PARSE_MESSAGE(MSG_GEN_030) \
PARSE_MESSAGE(MSG_GEN_031) \
PARSE_MESSAGE(MSG_GEN_032) \
PARSE_MESSAGE(MSG_GEN_033) \
PARSE_MESSAGE(MSG_GEN_034) \
PARSE_MESSAGE(MSG_GEN_035) \
PARSE_MESSAGE(MSG_GEN_036) \
PARSE_MESSAGE(MSG_GEN_037) \
PARSE_MESSAGE(MSG_GEN_038) \
PARSE_MESSAGE(MSG_GEN_039) \
PARSE_MESSAGE(MSG_GEN_040) \
PARSE_MESSAGE(MSG_GEN_041) \
PARSE_MESSAGE(MSG_GEN_042) \
PARSE_MESSAGE(MSG_GEN_043) \
PARSE_MESSAGE(MSG_GEN_044) \
PARSE_MESSAGE(MSG_GEN_045) \
PARSE_MESSAGE(MSG_GEN_046) \
PARSE_MESSAGE(MSG_GEN_047) \
PARSE_MESSAGE(MSG_GEN_048) \
PARSE_MESSAGE(MSG_GEN_049) \
PARSE_MESSAGE(MSG_GEN_050) \
PARSE_MESSAGE(MSG_GEN_051) \
PARSE_MESSAGE(MSG_GEN_052) \
PARSE_MESSAGE(MSG_GEN_053) \
PARSE_MESSAGE(MSG_GEN_054) \
PARSE_MESSAGE(MSG_GEN_055) \
PARSE_MESSAGE(MSG_GEN_056) \
PARSE_MESSAGE(MSG_GEN_057) \
PARSE_MESSAGE(MSG_GEN_058) \
PARSE_MESSAGE(MSG_GEN_059) \
PARSE_MESSAGE(MSG_GEN_060) \
PARSE_MESSAGE(MSG_GEN_061) \
PARSE_MESSAGE(MSG_GEN_062) \
PARSE_MESSAGE(MSG_GEN_063) \
PARSE_MESSAGE(MSG_GEN_064) \
PARSE_MESSAGE(MSG_GEN_065) \
PARSE_MESSAGE(MSG_GEN_066) \
PARSE_MESSAGE(MSG_GEN_067) \
PARSE_MESSAGE(MSG_GEN_068) \
PARSE_MESSAGE(MSG_GEN_069) \
PARSE_MESSAGE(MSG_GEN_070) \
PARSE_MESSAGE(MSG_GEN_071) \
PARSE_MESSAGE(MSG_GEN_072) \
PARSE_MESSAGE(MSG_GEN_073) \
PARSE_MESSAGE(MSG_GEN_074) \
PARSE_MESSAGE(MSG_GEN_075) \
PARSE_MESSAGE(MSG_GEN_076) \
PARSE_MESSAGE(MSG_GEN_077) \
PARSE_MESSAGE(MSG_GEN_078) \
PARSE_MESSAGE(MSG_GEN_079) \
PARSE_MESSAGE(MSG_GEN_080) \
PARSE_MESSAGE(MSG_GEN_081) \
\
PARSE_MESSAGE(MSG_CMN_001) \
PARSE_MESSAGE(MSG_CMN_002) \
PARSE_MESSAGE(MSG_CMN_003) \
PARSE_MESSAGE(MSG_CMN_004) \
PARSE_MESSAGE(MSG_CMN_005) \
PARSE_MESSAGE(MSG_CMN_006) \
PARSE_MESSAGE(MSG_CMN_007) \
PARSE_MESSAGE(MSG_CMN_008) \
PARSE_MESSAGE(MSG_CMN_009) \
PARSE_MESSAGE(MSG_CMN_010) \
PARSE_MESSAGE(MSG_CMN_011) \
PARSE_MESSAGE(MSG_CMN_012) \
PARSE_MESSAGE(MSG_CMN_013) \
PARSE_MESSAGE(MSG_CMN_014) \
PARSE_MESSAGE(MSG_CMN_015) \
PARSE_MESSAGE(MSG_CMN_016) \
PARSE_MESSAGE(MSG_CMN_017) \
PARSE_MESSAGE(MSG_CMN_018) \
PARSE_MESSAGE(MSG_CMN_019) \
PARSE_MESSAGE(MSG_CMN_020) \
PARSE_MESSAGE(MSG_CMN_021) \
PARSE_MESSAGE(MSG_CMN_022) \
PARSE_MESSAGE(MSG_CMN_023) \
PARSE_MESSAGE(MSG_CMN_024) \
PARSE_MESSAGE(MSG_CMN_025) \
PARSE_MESSAGE(MSG_CMN_026) \
PARSE_MESSAGE(MSG_CMN_027) \
PARSE_MESSAGE(MSG_CMN_028) \
PARSE_MESSAGE(MSG_CMN_029) \
PARSE_MESSAGE(MSG_CMN_030) \
PARSE_MESSAGE(MSG_CMN_031) \
PARSE_MESSAGE(MSG_CMN_032) \
PARSE_MESSAGE(MSG_CMN_033) \
PARSE_MESSAGE(MSG_CMN_034) \
PARSE_MESSAGE(MSG_CMN_035) \
PARSE_MESSAGE(MSG_CMN_036) \
PARSE_MESSAGE(MSG_CMN_037) \
PARSE_MESSAGE(MSG_CMN_038) \
PARSE_MESSAGE(MSG_CMN_039) \
PARSE_MESSAGE(MSG_CMN_040) \
PARSE_MESSAGE(MSG_CMN_041) \
PARSE_MESSAGE(MSG_CMN_042) \
PARSE_MESSAGE(MSG_CMN_043) \
PARSE_MESSAGE(MSG_CMN_044) \
PARSE_MESSAGE(MSG_CMN_045) \
PARSE_MESSAGE(MSG_CMN_046) \
PARSE_MESSAGE(MSG_CMN_047) \
PARSE_MESSAGE(MSG_CMN_048) \
PARSE_MESSAGE(MSG_CMN_049) \
PARSE_MESSAGE(MSG_CMN_050) \
PARSE_MESSAGE(MSG_CMN_051) \
PARSE_MESSAGE(MSG_CMN_052) \
PARSE_MESSAGE(MSG_CMN_053) \
PARSE_MESSAGE(MSG_CMN_054) \
\
PARSE_MESSAGE(MSG_ITF_001) \
PARSE_MESSAGE(MSG_ITF_002) \
PARSE_MESSAGE(MSG_ITF_003) \
PARSE_MESSAGE(MSG_ITF_004) \
PARSE_MESSAGE(MSG_ITF_005) \
PARSE_MESSAGE(MSG_ITF_006) \
PARSE_MESSAGE(MSG_ITF_007) \
PARSE_MESSAGE(MSG_ITF_008) \
PARSE_MESSAGE(MSG_ITF_009) \
PARSE_MESSAGE(MSG_ITF_010) \
PARSE_MESSAGE(MSG_ITF_011) \
PARSE_MESSAGE(MSG_ITF_012) \
PARSE_MESSAGE(MSG_ITF_013) \
PARSE_MESSAGE(MSG_ITF_014) \
PARSE_MESSAGE(MSG_ITF_015) \
PARSE_MESSAGE(MSG_ITF_016) \
PARSE_MESSAGE(MSG_ITF_017) \
PARSE_MESSAGE(MSG_ITF_018) \
PARSE_MESSAGE(MSG_ITF_019) \
PARSE_MESSAGE(MSG_ITF_020) \
PARSE_MESSAGE(MSG_ITF_021) \
PARSE_MESSAGE(MSG_ITF_022) \
PARSE_MESSAGE(MSG_ITF_023) \
PARSE_MESSAGE(MSG_ITF_024) \
PARSE_MESSAGE(MSG_ITF_025) \
PARSE_MESSAGE(MSG_ITF_026) \
PARSE_MESSAGE(MSG_ITF_027) \
PARSE_MESSAGE(MSG_ITF_028) \
PARSE_MESSAGE(MSG_ITF_029) \
PARSE_MESSAGE(MSG_ITF_030) \
PARSE_MESSAGE(MSG_ITF_031) \
PARSE_MESSAGE(MSG_ITF_032) \
PARSE_MESSAGE(MSG_ITF_033) \
PARSE_MESSAGE(MSG_ITF_034) \
PARSE_MESSAGE(MSG_ITF_035) \
PARSE_MESSAGE(MSG_ITF_036) \
PARSE_MESSAGE(MSG_ITF_037) \
PARSE_MESSAGE(MSG_ITF_038) \
PARSE_MESSAGE(MSG_ITF_039) \
PARSE_MESSAGE(MSG_ITF_040) \
PARSE_MESSAGE(MSG_ITF_041) \
PARSE_MESSAGE(MSG_ITF_044) \
PARSE_MESSAGE(MSG_ITF_045) \
PARSE_MESSAGE(MSG_ITF_046) \
PARSE_MESSAGE(MSG_ITF_047) \
PARSE_MESSAGE(MSG_ITF_048) \
PARSE_MESSAGE(MSG_ITF_049) \
PARSE_MESSAGE(MSG_ITF_050) \
PARSE_MESSAGE(MSG_ITF_051) \
PARSE_MESSAGE(MSG_ITF_052) \
PARSE_MESSAGE(MSG_ITF_053) \
PARSE_MESSAGE(MSG_ITF_054) \
PARSE_MESSAGE(MSG_ITF_055) \
PARSE_MESSAGE(MSG_ITF_056) \
PARSE_MESSAGE(MSG_ITF_057) \
PARSE_MESSAGE(MSG_ITF_058) \
PARSE_MESSAGE(MSG_ITF_059) \
PARSE_MESSAGE(MSG_ITF_060) \
PARSE_MESSAGE(MSG_ITF_061) \
PARSE_MESSAGE(MSG_ITF_062) \
PARSE_MESSAGE(MSG_ITF_063) \
PARSE_MESSAGE(MSG_ITF_064) \
PARSE_MESSAGE(MSG_ITF_065) \
PARSE_MESSAGE(MSG_ITF_066) \
PARSE_MESSAGE(MSG_ITF_067) \
PARSE_MESSAGE(MSG_ITF_068) \
PARSE_MESSAGE(MSG_ITF_069) \
PARSE_MESSAGE(MSG_ITF_070) \
PARSE_MESSAGE(MSG_ITF_071) \
PARSE_MESSAGE(MSG_ITF_072) \
PARSE_MESSAGE(MSG_ITF_073) \
PARSE_MESSAGE(MSG_ITF_074) \
PARSE_MESSAGE(MSG_ITF_075) \
PARSE_MESSAGE(MSG_ITF_076) \
PARSE_MESSAGE(MSG_ITF_077) \
PARSE_MESSAGE(MSG_ITF_078) \
PARSE_MESSAGE(MSG_ITF_079) \
PARSE_MESSAGE(MSG_ITF_080) \
PARSE_MESSAGE(MSG_ITF_081) \
PARSE_MESSAGE(MSG_ITF_082) \
PARSE_MESSAGE(MSG_ITF_083) \
PARSE_MESSAGE(MSG_ITF_084) \
PARSE_MESSAGE(MSG_ITF_085) \
PARSE_MESSAGE(MSG_ITF_086) \
PARSE_MESSAGE(MSG_ITF_087) \
PARSE_MESSAGE(MSG_ITF_088) \
PARSE_MESSAGE(MSG_ITF_089) \
PARSE_MESSAGE(MSG_ITF_090) \
PARSE_MESSAGE(MSG_ITF_091) \
PARSE_MESSAGE(MSG_ITF_092) \
PARSE_MESSAGE(MSG_ITF_093) \
PARSE_MESSAGE(MSG_ITF_094) \
PARSE_MESSAGE(MSG_ITF_095) \
PARSE_MESSAGE(MSG_ITF_096) \
PARSE_MESSAGE(MSG_ITF_097) \
PARSE_MESSAGE(MSG_ITF_098) \
PARSE_MESSAGE(MSG_ITF_099) \
PARSE_MESSAGE(MSG_ITF_100) \
PARSE_MESSAGE(MSG_ITF_101) \
PARSE_MESSAGE(MSG_ITF_102) \
PARSE_MESSAGE(MSG_ITF_103) \
PARSE_MESSAGE(MSG_ITF_104) \
PARSE_MESSAGE(MSG_ITF_105) \
PARSE_MESSAGE(MSG_ITF_106) \
PARSE_MESSAGE(MSG_ITF_107) \
PARSE_MESSAGE(MSG_ITF_108) \
PARSE_MESSAGE(MSG_ITF_109) \
PARSE_MESSAGE(MSG_ITF_110) \
PARSE_MESSAGE(MSG_ITF_111) \
PARSE_MESSAGE(MSG_ITF_112) \
PARSE_MESSAGE(MSG_ITF_113) \
PARSE_MESSAGE(MSG_ITF_114) \
PARSE_MESSAGE(MSG_ITF_115) \
PARSE_MESSAGE(MSG_ITF_116) \
PARSE_MESSAGE(MSG_ITF_117) \
PARSE_MESSAGE(MSG_ITF_118) \
PARSE_MESSAGE(MSG_ITF_119) \
PARSE_MESSAGE(MSG_ITF_120) \
PARSE_MESSAGE(MSG_ITF_121) \
PARSE_MESSAGE(MSG_ITF_122) \
PARSE_MESSAGE(MSG_ITF_123) \
PARSE_MESSAGE(MSG_ITF_124) \
PARSE_MESSAGE(MSG_ITF_125) \
PARSE_MESSAGE(MSG_ITF_126) \
PARSE_MESSAGE(MSG_ITF_127) \
PARSE_MESSAGE(MSG_ITF_128) \
PARSE_MESSAGE(MSG_ITF_129) \
PARSE_MESSAGE(MSG_ITF_130) \
PARSE_MESSAGE(MSG_ITF_131) \
PARSE_MESSAGE(MSG_ITF_132) \
PARSE_MESSAGE(MSG_ITF_133) \
PARSE_MESSAGE(MSG_ITF_134) \
PARSE_MESSAGE(MSG_ITF_135) \
PARSE_MESSAGE(MSG_ITF_136) \
PARSE_MESSAGE(MSG_ITF_137) \
PARSE_MESSAGE(MSG_ITF_138) \
PARSE_MESSAGE(MSG_ITF_139) \
PARSE_MESSAGE(MSG_ITF_140) \
PARSE_MESSAGE(MSG_ITF_141) \
PARSE_MESSAGE(MSG_ITF_142) \
PARSE_MESSAGE(MSG_ITF_143) \
PARSE_MESSAGE(MSG_ITF_144) \
PARSE_MESSAGE(MSG_ITF_145) \
PARSE_MESSAGE(MSG_ITF_146) \
PARSE_MESSAGE(MSG_ITF_147) \
PARSE_MESSAGE(MSG_ITF_148) \
PARSE_MESSAGE(MSG_ITF_149) \
PARSE_MESSAGE(MSG_ITF_150) \
PARSE_MESSAGE(MSG_ITF_151) \
PARSE_MESSAGE(MSG_ITF_152) \
PARSE_MESSAGE(MSG_ITF_153) \
PARSE_MESSAGE(MSG_ITF_154) \
PARSE_MESSAGE(MSG_ITF_155) \
PARSE_MESSAGE(MSG_ITF_156) \
PARSE_MESSAGE(MSG_ITF_157) \
PARSE_MESSAGE(MSG_ITF_158) \
PARSE_MESSAGE(MSG_ITF_159) \
PARSE_MESSAGE(MSG_ITF_160) \
PARSE_MESSAGE(MSG_ITF_161) \
PARSE_MESSAGE(MSG_ITF_162) \
PARSE_MESSAGE(MSG_ITF_163) \
PARSE_MESSAGE(MSG_ITF_164) \
PARSE_MESSAGE(MSG_ITF_165) \
PARSE_MESSAGE(MSG_ITF_166) \
PARSE_MESSAGE(MSG_ITF_167) \
PARSE_MESSAGE(MSG_ITF_168) \
PARSE_MESSAGE(MSG_ITF_169) \
PARSE_MESSAGE(MSG_ITF_170) \
PARSE_MESSAGE(MSG_ITF_171) \
PARSE_MESSAGE(MSG_ITF_172) \
PARSE_MESSAGE(MSG_ITF_173) \
PARSE_MESSAGE(MSG_ITF_174) \
PARSE_MESSAGE(MSG_ITF_175) \
PARSE_MESSAGE(MSG_ITF_176) \
PARSE_MESSAGE(MSG_ITF_177) \
PARSE_MESSAGE(MSG_ITF_178) \
PARSE_MESSAGE(MSG_ITF_179) \
PARSE_MESSAGE(MSG_ITF_180) \
PARSE_MESSAGE(MSG_ITF_181) \
PARSE_MESSAGE(MSG_ITF_182) \
PARSE_MESSAGE(MSG_ITF_183) \
PARSE_MESSAGE(MSG_ITF_184) \
PARSE_MESSAGE(MSG_ITF_185) \
PARSE_MESSAGE(MSG_ITF_186) \
\
PARSE_MESSAGE(MSG_JPR_001) \
PARSE_MESSAGE(MSG_JPR_002) \
PARSE_MESSAGE(MSG_JPR_003) \
PARSE_MESSAGE(MSG_JPR_004) \
PARSE_MESSAGE(MSG_JPR_005) \
PARSE_MESSAGE(MSG_JPR_006) \
PARSE_MESSAGE(MSG_JPR_007) \
PARSE_MESSAGE(MSG_JPR_008) \
PARSE_MESSAGE(MSG_JPR_009) \
PARSE_MESSAGE(MSG_JPR_010) \
PARSE_MESSAGE(MSG_JPR_011) \
PARSE_MESSAGE(MSG_JPR_012) \
PARSE_MESSAGE(MSG_JPR_013) \
PARSE_MESSAGE(MSG_JPR_014) \
\
PARSE_MESSAGE(MSG_MIO_001) \
PARSE_MESSAGE(MSG_MIO_002) \
PARSE_MESSAGE(MSG_MIO_003) \
PARSE_MESSAGE(MSG_MIO_004) \
PARSE_MESSAGE(MSG_MIO_005) \
PARSE_MESSAGE(MSG_MIO_006) \
PARSE_MESSAGE(MSG_MIO_007) \
PARSE_MESSAGE(MSG_MIO_008) \
PARSE_MESSAGE(MSG_MIO_009) \
PARSE_MESSAGE(MSG_MIO_010) \
PARSE_MESSAGE(MSG_MIO_011) \
PARSE_MESSAGE(MSG_MIO_012) \
PARSE_MESSAGE(MSG_MIO_013) \
PARSE_MESSAGE(MSG_MIO_014) \
PARSE_MESSAGE(MSG_MIO_015) \
PARSE_MESSAGE(MSG_MIO_016) \
PARSE_MESSAGE(MSG_MIO_017) \
PARSE_MESSAGE(MSG_MIO_018) \
PARSE_MESSAGE(MSG_MIO_019) \
PARSE_MESSAGE(MSG_MIO_020) \
PARSE_MESSAGE(MSG_MIO_021) \
PARSE_MESSAGE(MSG_MIO_022) \
PARSE_MESSAGE(MSG_MIO_023) \
PARSE_MESSAGE(MSG_MIO_024) \
PARSE_MESSAGE(MSG_MIO_025) \
PARSE_MESSAGE(MSG_MIO_026) \
PARSE_MESSAGE(MSG_MIO_027) \
PARSE_MESSAGE(MSG_MIO_028) \
PARSE_MESSAGE(MSG_MIO_029) \
PARSE_MESSAGE(MSG_MIO_030) \
PARSE_MESSAGE(MSG_MIO_031) \
PARSE_MESSAGE(MSG_MIO_032) \
\
\
PARSE_MESSAGE(MSG_DMA_001) \
PARSE_MESSAGE(MSG_DMA_002) \
PARSE_MESSAGE(MSG_DMA_003) \
PARSE_MESSAGE(MSG_DMA_004) \
PARSE_MESSAGE(MSG_DMA_005) \
PARSE_MESSAGE(MSG_DMA_006) \
PARSE_MESSAGE(MSG_DMA_007) \
PARSE_MESSAGE(MSG_DMA_008) \
PARSE_MESSAGE(MSG_DMA_009) \
PARSE_MESSAGE(MSG_DMA_010) \
PARSE_MESSAGE(MSG_DMA_011) \
PARSE_MESSAGE(MSG_DMA_012) \
PARSE_MESSAGE(MSG_DMA_013) \
PARSE_MESSAGE(MSG_DMA_014) \
PARSE_MESSAGE(MSG_DMA_015) \
PARSE_MESSAGE(MSG_DMA_016) \
PARSE_MESSAGE(MSG_DMA_017) \
PARSE_MESSAGE(MSG_DMA_018) \
PARSE_MESSAGE(MSG_DMA_019) \
PARSE_MESSAGE(MSG_DMA_020) \
PARSE_MESSAGE(MSG_DMA_021) \
PARSE_MESSAGE(MSG_DMA_022) \
PARSE_MESSAGE(MSG_DMA_023) \
PARSE_MESSAGE(MSG_DMA_024) \
PARSE_MESSAGE(MSG_DMA_025) \
PARSE_MESSAGE(MSG_DMA_026) \
PARSE_MESSAGE(MSG_DMA_027) \
PARSE_MESSAGE(MSG_DMA_028) \
PARSE_MESSAGE(MSG_DMA_029) \
PARSE_MESSAGE(MSG_DMA_030) \
PARSE_MESSAGE(MSG_DMA_031) \
PARSE_MESSAGE(MSG_DMA_032) \
PARSE_MESSAGE(MSG_DMA_033) \
\
PARSE_MESSAGE(MSG_P2P_001) \
PARSE_MESSAGE(MSG_P2P_002) \
PARSE_MESSAGE(MSG_P2P_003) \
PARSE_MESSAGE(MSG_P2P_004) \
PARSE_MESSAGE(MSG_P2P_005) \
PARSE_MESSAGE(MSG_P2P_006) \
PARSE_MESSAGE(MSG_P2P_007) \
PARSE_MESSAGE(MSG_P2P_008) \
PARSE_MESSAGE(MSG_P2P_009) \
PARSE_MESSAGE(MSG_P2P_010) \
PARSE_MESSAGE(MSG_P2P_011) \
PARSE_MESSAGE(MSG_P2P_012) \
PARSE_MESSAGE(MSG_P2P_013) \
PARSE_MESSAGE(MSG_P2P_014) \
PARSE_MESSAGE(MSG_P2P_015) \
PARSE_MESSAGE(MSG_P2P_016) \
PARSE_MESSAGE(MSG_P2P_017) \
PARSE_MESSAGE(MSG_P2P_018) \
PARSE_MESSAGE(MSG_P2P_019) \
PARSE_MESSAGE(MSG_P2P_020) \
PARSE_MESSAGE(MSG_P2P_021) \
PARSE_MESSAGE(MSG_P2P_022) \
PARSE_MESSAGE(MSG_P2P_023) \
PARSE_MESSAGE(MSG_P2P_024) \
PARSE_MESSAGE(MSG_P2P_025) \
PARSE_MESSAGE(MSG_P2P_026) \
PARSE_MESSAGE(MSG_P2P_027) \
PARSE_MESSAGE(MSG_P2P_028) \
PARSE_MESSAGE(MSG_P2P_029) \
PARSE_MESSAGE(MSG_P2P_030) \
PARSE_MESSAGE(MSG_P2P_031) \
PARSE_MESSAGE(MSG_P2P_032) \
PARSE_MESSAGE(MSG_P2P_033) \
PARSE_MESSAGE(MSG_P2P_034) \
PARSE_MESSAGE(MSG_P2P_035) \
PARSE_MESSAGE(MSG_P2P_036) \
PARSE_MESSAGE(MSG_P2P_037) \
PARSE_MESSAGE(MSG_P2P_038) \
PARSE_MESSAGE(MSG_P2P_039) \
PARSE_MESSAGE(MSG_P2P_040) \
PARSE_MESSAGE(MSG_P2P_041) \
\
PARSE_MESSAGE(MSG_VER_001) \
PARSE_MESSAGE(MSG_VER_002) \
PARSE_MESSAGE(MSG_VER_003) \
PARSE_MESSAGE(MSG_VER_004) \
PARSE_MESSAGE(MSG_VER_005) \
PARSE_MESSAGE(MSG_VER_006) \
PARSE_MESSAGE(MSG_VER_007) \
PARSE_MESSAGE(MSG_VER_008) \
PARSE_MESSAGE(MSG_VER_009) \
PARSE_MESSAGE(MSG_VER_010) \
PARSE_MESSAGE(MSG_VER_011) \
PARSE_MESSAGE(MSG_VER_012) \
PARSE_MESSAGE(MSG_VER_013) \
PARSE_MESSAGE(MSG_VER_014) \
PARSE_MESSAGE(MSG_VER_015) \
\
PARSE_MESSAGE(MSG_ETH_001) \
PARSE_MESSAGE(MSG_ETH_002) \
PARSE_MESSAGE(MSG_ETH_003) \
PARSE_MESSAGE(MSG_ETH_004) \
PARSE_MESSAGE(MSG_ETH_005) \
PARSE_MESSAGE(MSG_ETH_006) \
PARSE_MESSAGE(MSG_ETH_007) \
PARSE_MESSAGE(MSG_ETH_008) \
PARSE_MESSAGE(MSG_ETH_009) \
PARSE_MESSAGE(MSG_ETH_010) \
PARSE_MESSAGE(MSG_ETH_011) \
PARSE_MESSAGE(MSG_ETH_012) \
PARSE_MESSAGE(MSG_ETH_013) \
PARSE_MESSAGE(MSG_ETH_014) \
PARSE_MESSAGE(MSG_ETH_015) \
PARSE_MESSAGE(MSG_ETH_016) \
PARSE_MESSAGE(MSG_ETH_017) \
PARSE_MESSAGE(MSG_ETH_018) \
PARSE_MESSAGE(MSG_ETH_019) \
PARSE_MESSAGE(MSG_ETH_020) \
PARSE_MESSAGE(MSG_ETH_021) \
PARSE_MESSAGE(MSG_ETH_022) \
PARSE_MESSAGE(MSG_ETH_023) \
PARSE_MESSAGE(MSG_ETH_024) \
PARSE_MESSAGE(MSG_ETH_025) \
PARSE_MESSAGE(MSG_ETH_026) \
PARSE_MESSAGE(MSG_ETH_027) \
PARSE_MESSAGE(MSG_ETH_028) \
PARSE_MESSAGE(MSG_ETH_029) \
PARSE_MESSAGE(MSG_ETH_030) \
PARSE_MESSAGE(MSG_ETH_031) \
PARSE_MESSAGE(MSG_ETH_032) \
PARSE_MESSAGE(MSG_ETH_033) \
PARSE_MESSAGE(MSG_ETH_034) \
PARSE_MESSAGE(MSG_ETH_035) \
PARSE_MESSAGE(MSG_ETH_036) \
PARSE_MESSAGE(MSG_ETH_037) \
PARSE_MESSAGE(MSG_ETH_038) \
PARSE_MESSAGE(MSG_ETH_039) \
PARSE_MESSAGE(MSG_ETH_040) \
PARSE_MESSAGE(MSG_ETH_041) \
PARSE_MESSAGE(MSG_ETH_042) \
PARSE_MESSAGE(MSG_ETH_043) \
PARSE_MESSAGE(MSG_ETH_044) \
PARSE_MESSAGE(MSG_ETH_045) \
PARSE_MESSAGE(MSG_ETH_046) \
PARSE_MESSAGE(MSG_ETH_047) \
PARSE_MESSAGE(MSG_ETH_048) \
PARSE_MESSAGE(MSG_ETH_049) \
PARSE_MESSAGE(MSG_ETH_050) \
PARSE_MESSAGE(MSG_ETH_051) \
PARSE_MESSAGE(MSG_ETH_052) \
PARSE_MESSAGE(MSG_ETH_053) \
PARSE_MESSAGE(MSG_ETH_054) \
PARSE_MESSAGE(MSG_ETH_055) \
PARSE_MESSAGE(MSG_ETH_056) \
PARSE_MESSAGE(MSG_ETH_057) \
PARSE_MESSAGE(MSG_ETH_058) \
PARSE_MESSAGE(MSG_ETH_059) \
PARSE_MESSAGE(MSG_ETH_060) \
PARSE_MESSAGE(MSG_ETH_061) \
PARSE_MESSAGE(MSG_ETH_062) \
PARSE_MESSAGE(MSG_ETH_063) \
PARSE_MESSAGE(MSG_ETH_064) \
PARSE_MESSAGE(MSG_ETH_065) \
PARSE_MESSAGE(MSG_ETH_066) \
PARSE_MESSAGE(MSG_ETH_067) \
PARSE_MESSAGE(MSG_ETH_068) \
PARSE_MESSAGE(MSG_ETH_069) \
PARSE_MESSAGE(MSG_ETH_070) \
PARSE_MESSAGE(MSG_ETH_071) \
PARSE_MESSAGE(MSG_ETH_072) \
PARSE_MESSAGE(MSG_ETH_073) \
PARSE_MESSAGE(MSG_ETH_074) \
PARSE_MESSAGE(MSG_ETH_075) \
PARSE_MESSAGE(MSG_ETH_076) \
PARSE_MESSAGE(MSG_ETH_077) \
PARSE_MESSAGE(MSG_ETH_078) \
PARSE_MESSAGE(MSG_ETH_079) \
PARSE_MESSAGE(MSG_ETH_080) \
PARSE_MESSAGE(MSG_ETH_081) \
PARSE_MESSAGE(MSG_ETH_082) \
PARSE_MESSAGE(MSG_ETH_083) \
PARSE_MESSAGE(MSG_ETH_084) \
PARSE_MESSAGE(MSG_ETH_085) \
PARSE_MESSAGE(MSG_ETH_086) \
PARSE_MESSAGE(MSG_ETH_087) \
PARSE_MESSAGE(MSG_ETH_088) \
PARSE_MESSAGE(MSG_ETH_089) \
\
PARSE_MESSAGE(MSG_PWR_001) \
PARSE_MESSAGE(MSG_PWR_002) \
PARSE_MESSAGE(MSG_PWR_003) \
PARSE_MESSAGE(MSG_PWR_004) \
PARSE_MESSAGE(MSG_PWR_005) \
PARSE_MESSAGE(MSG_PWR_006) \
PARSE_MESSAGE(MSG_PWR_007) \
PARSE_MESSAGE(MSG_PWR_008) \
PARSE_MESSAGE(MSG_PWR_009) \
PARSE_MESSAGE(MSG_PWR_010) \
PARSE_MESSAGE(MSG_PWR_011) \
PARSE_MESSAGE(MSG_PWR_012) \
PARSE_MESSAGE(MSG_PWR_013) \
PARSE_MESSAGE(MSG_PWR_014) \
PARSE_MESSAGE(MSG_PWR_015) \
PARSE_MESSAGE(MSG_PWR_016) \
PARSE_MESSAGE(MSG_PWR_017) \
PARSE_MESSAGE(MSG_PWR_018) \
PARSE_MESSAGE(MSG_PWR_019) \
PARSE_MESSAGE(MSG_PWR_020) \
PARSE_MESSAGE(MSG_PWR_021) \
PARSE_MESSAGE(MSG_PWR_022) \
PARSE_MESSAGE(MSG_PWR_023) \
PARSE_MESSAGE(MSG_PWR_024) \
PARSE_MESSAGE(MSG_PWR_025) \
PARSE_MESSAGE(MSG_PWR_026) \
PARSE_MESSAGE(MSG_PWR_027) \
PARSE_MESSAGE(MSG_PWR_028) \
PARSE_MESSAGE(MSG_PWR_029) \
PARSE_MESSAGE(MSG_PWR_030) \
PARSE_MESSAGE(MSG_PWR_031) \
PARSE_MESSAGE(MSG_PWR_032) \
PARSE_MESSAGE(MSG_PWR_033) \
PARSE_MESSAGE(MSG_PWR_034) \
PARSE_MESSAGE(MSG_PWR_035) \
PARSE_MESSAGE(MSG_PWR_036) \
PARSE_MESSAGE(MSG_PWR_037) \
PARSE_MESSAGE(MSG_PWR_038) \
PARSE_MESSAGE(MSG_PWR_039) \
PARSE_MESSAGE(MSG_PWR_040) \
PARSE_MESSAGE(MSG_PWR_041) \
PARSE_MESSAGE(MSG_PWR_042) \
PARSE_MESSAGE(MSG_PWR_043) \
PARSE_MESSAGE(MSG_PWR_044) \
PARSE_MESSAGE(MSG_PWR_045) \
PARSE_MESSAGE(MSG_PWR_046) \
PARSE_MESSAGE(MSG_PWR_047) \
PARSE_MESSAGE(MSG_PWR_048) \
PARSE_MESSAGE(MSG_PWR_049) \
PARSE_MESSAGE(MSG_PWR_050) \
PARSE_MESSAGE(MSG_PWR_051) \
PARSE_MESSAGE(MSG_PWR_052) \
PARSE_MESSAGE(MSG_PWR_053) \
PARSE_MESSAGE(MSG_PWR_054) \
PARSE_MESSAGE(MSG_PWR_055) \
PARSE_MESSAGE(MSG_PWR_056) \
PARSE_MESSAGE(MSG_PWR_057) \
PARSE_MESSAGE(MSG_PWR_058) \
PARSE_MESSAGE(MSG_PWR_059) \
PARSE_MESSAGE(MSG_PWR_060) \
PARSE_MESSAGE(MSG_PWR_061) \
\
PARSE_MESSAGE(MSG_MGT_001) \
PARSE_MESSAGE(MSG_MGT_002) \
PARSE_MESSAGE(MSG_MGT_003) \
PARSE_MESSAGE(MSG_MGT_004) \
PARSE_MESSAGE(MSG_MGT_005) \
PARSE_MESSAGE(MSG_MGT_006) \
PARSE_MESSAGE(MSG_MGT_007) \
PARSE_MESSAGE(MSG_MGT_008) \
PARSE_MESSAGE(MSG_MGT_009) \
PARSE_MESSAGE(MSG_MGT_010) \
PARSE_MESSAGE(MSG_MGT_011) \
PARSE_MESSAGE(MSG_MGT_012) \
PARSE_MESSAGE(MSG_MGT_013) \
PARSE_MESSAGE(MSG_MGT_014) \
PARSE_MESSAGE(MSG_MGT_015) \
PARSE_MESSAGE(MSG_MGT_016) \
PARSE_MESSAGE(MSG_MGT_017) \
PARSE_MESSAGE(MSG_MGT_018) \
PARSE_MESSAGE(MSG_MGT_019) \
PARSE_MESSAGE(MSG_MGT_020) \
PARSE_MESSAGE(MSG_MGT_021) \
PARSE_MESSAGE(MSG_MGT_022) \
PARSE_MESSAGE(MSG_MGT_023) \
PARSE_MESSAGE(MSG_MGT_024) \
\
PARSE_MESSAGE(MSG_MEM_001) \
PARSE_MESSAGE(MSG_MEM_002) \
PARSE_MESSAGE(MSG_MEM_003) \
PARSE_MESSAGE(MSG_MEM_004) \
PARSE_MESSAGE(MSG_MEM_005) \
PARSE_MESSAGE(MSG_MEM_006) \
PARSE_MESSAGE(MSG_MEM_007) \
PARSE_MESSAGE(MSG_MEM_008) \
PARSE_MESSAGE(MSG_MEM_009) \
PARSE_MESSAGE(MSG_MEM_010) \
PARSE_MESSAGE(MSG_MEM_011) \
PARSE_MESSAGE(MSG_MEM_012) \
PARSE_MESSAGE(MSG_MEM_013) \
PARSE_MESSAGE(MSG_MEM_014) \
PARSE_MESSAGE(MSG_MEM_015) \
PARSE_MESSAGE(MSG_MEM_016) \
PARSE_MESSAGE(MSG_MEM_017) \
PARSE_MESSAGE(MSG_MEM_018) \
PARSE_MESSAGE(MSG_MEM_019) \
PARSE_MESSAGE(MSG_MEM_020) \
PARSE_MESSAGE(MSG_MEM_021) \
PARSE_MESSAGE(MSG_MEM_022) \
PARSE_MESSAGE(MSG_MEM_023) \
PARSE_MESSAGE(MSG_MEM_024) \
PARSE_MESSAGE(MSG_MEM_025) \
PARSE_MESSAGE(MSG_MEM_026) \
PARSE_MESSAGE(MSG_MEM_027) \
PARSE_MESSAGE(MSG_MEM_028) \
PARSE_MESSAGE(MSG_MEM_029) \
PARSE_MESSAGE(MSG_MEM_030) \
PARSE_MESSAGE(MSG_MEM_031) \
PARSE_MESSAGE(MSG_MEM_032) \
PARSE_MESSAGE(MSG_MEM_033) \
PARSE_MESSAGE(MSG_MEM_034) \
PARSE_MESSAGE(MSG_MEM_035) \
PARSE_MESSAGE(MSG_MEM_036) \
PARSE_MESSAGE(MSG_MEM_037) \
PARSE_MESSAGE(MSG_MEM_038) \
PARSE_MESSAGE(MSG_MEM_039) \
PARSE_MESSAGE(MSG_MEM_040) \
PARSE_MESSAGE(MSG_MEM_041) \
PARSE_MESSAGE(MSG_MEM_042) \
PARSE_MESSAGE(MSG_MEM_043) \
PARSE_MESSAGE(MSG_MEM_044) \
PARSE_MESSAGE(MSG_MEM_045) \
PARSE_MESSAGE(MSG_MEM_046) \
PARSE_MESSAGE(MSG_MEM_047) \
PARSE_MESSAGE(MSG_MEM_048) \
PARSE_MESSAGE(MSG_MEM_049) \
PARSE_MESSAGE(MSG_MEM_050) \
PARSE_MESSAGE(MSG_MEM_051) \
PARSE_MESSAGE(MSG_MEM_052) \
PARSE_MESSAGE(MSG_MEM_053) \
PARSE_MESSAGE(MSG_MEM_054) \
\
PARSE_MESSAGE(MSG_DEBUG_EXCEPTION) \
PARSE_MESSAGE(MSG_DEBUG_TESTCASE) \
PARSE_MESSAGE(MSG_DEBUG_SETUP) \
PARSE_MESSAGE(MSG_DEBUG_OPENCL) \
PARSE_MESSAGE(MSG_DEBUG_PARSING) \
\
PARSE_MESSAGE(MSG_DEF_001) \
PARSE_MESSAGE(MSG_DEF_002) \
PARSE_MESSAGE(MSG_DEF_003) \
PARSE_MESSAGE(MSG_DEF_004) \
PARSE_MESSAGE(MSG_DEF_005) \
PARSE_MESSAGE(MSG_DEF_006) \
PARSE_MESSAGE(MSG_DEF_007) \
PARSE_MESSAGE(MSG_DEF_008) \
PARSE_MESSAGE(MSG_DEF_009) \
PARSE_MESSAGE(MSG_DEF_010) \
PARSE_MESSAGE(MSG_DEF_011) \
PARSE_MESSAGE(MSG_DEF_012) \
PARSE_MESSAGE(MSG_DEF_013) \

#define MSG_GEN_001 Message_t{ \
    .id                             = "GEN_001", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Command line option -<option> provided more than once", \
    .msg_edit                       = "Command line option -%s provided more than once", \
    .detail                         = "A command line option has been set more than once in the command line", \
    .resolution                     = "Check the options in the command line", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_002 Message_t{ \
    .id                             = "GEN_002", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of help message>", \
    .msg_edit                       = "%s", \
    .detail                         = "Provided by the host application" \
}
#define MSG_GEN_003 Message_t{ \
    .id                             = "GEN_003", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of version message>", \
    .msg_edit                       = "%s", \
    .detail                         = "The version of host application" \
}
#define MSG_GEN_004 Message_t{ \
    .id                             = "GEN_004", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Command line option -<option> requires an argument", \
    .msg_edit                       = "Command line option -%s requires an argument", \
    .detail                         = "A command line option has been provided without an expected argument", \
    .resolution                     = "Check the options in the command line", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_005 Message_t{ \
    .id                             = "GEN_005", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Unknown command line option -<option>", \
    .msg_edit                       = "Unknown command line option -%s", \
    .detail                         = "An unknown command line option has been provided", \
    .resolution                     = "Check the options in the command line", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_006 Message_t{ \
    .id                             = "GEN_006", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Found <number of options> invalid command line option(s): <list of invalid options found>", \
    .msg_edit                       = "Found %s invalid command line option(s): %s", \
    .detail                         = "Invalid command line option(s) have been provided", \
    .resolution                     = "Check the options in the command line", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_007 Message_t{ \
    .id                             = "GEN_007", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "No test performed", \
    .msg_edit                       = "", \
    .detail                         = "No test is performed because the options -v and/or -h have been provided in the command line" \
}
#define MSG_GEN_008 Message_t{ \
    .id                             = "GEN_008", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Required command line option not found -<option>", \
    .msg_edit                       = "Required command line option not found: -%s", \
    .detail                         = "A required option has not been provided", \
    .resolution                     = "Check the options in the command line", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_009 Message_t{ \
    .id                             = "GEN_009", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "For help, try command line option: -h", \
    .msg_edit                       = "", \
    .detail                         = "Message printed when any error is detected in the command line" \
}
#define MSG_GEN_010 Message_t{ .id = "GEN_010", .state = "obsolete" }
#define MSG_GEN_011 Message_t{ .id = "GEN_011", .state = "obsolete" }
#define MSG_GEN_012 Message_t{ \
    .id                             = "GEN_012", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "User abort received", \
    .msg_edit                       = "", \
    .detail                         = "An interruption has been received by the host application", \
    .resolution                     = "Check if the SIGINT signal has been sent to the host application (for example if CTRL+C has been issued) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_013 Message_t{ \
    .id                             = "GEN_013", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Setting parameter <parameter name> with value (<parameter value>) provided in command line", \
    .msg_edit                       = "Setting parameter %s with value (%s) provided in command line", \
    .detail                         = "The value of the parameter has been set via the command line and, if applicable, overwrites any value provided in the equivalent test JSON file parameter", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_GEN_014 Message_t{ \
    .id                             = "GEN_014", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid value (<parameter value>) provided in command line for parameter <parameter name>. <info>", \
    .msg_edit                       = "Invalid value (%s) provided in command line for parameter %s. %s", \
    .detail                         = "An invalid value has been provided for the parameter via the command line", \
    .resolution                     = "Check the parameter value provided is in the supported range", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_015 Message_t{ .id = "GEN_015", .state = "obsolete" }
#define MSG_GEN_016 Message_t{ \
    .id                             = "GEN_016", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of system configuration header>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message is the header of host application displaying system configuration" \
}
#define MSG_GEN_017 Message_t{ \
    .id                             = "GEN_017", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of system configuration footer>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message is the footer of host application displaying system configuration" \
}
#define MSG_GEN_018 Message_t{ \
    .id                             = "GEN_018", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of summary footer>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message is the footer of host application displaying summary information" \
}
#define MSG_GEN_019 Message_t{ \
    .id                             = "GEN_019", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<test name> test unknown state", \
    .msg_edit                       = "%s test unknown state", \
    .detail                         = "A testcase has ended in an unknown state", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_020 Message_t{ \
    .id                             = "GEN_020", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<test name> test aborted", \
    .msg_edit                       = "%s test aborted", \
    .detail                         = "A testcase has ended after being aborted", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_021 Message_t{ \
    .id                             = "GEN_021", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<test name> test failed", \
    .msg_edit                       = "%s test failed", \
    .detail                         = "A testcase has ended and failed", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_022 Message_t{ \
    .id                             = "GEN_022", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<test name> test passed", \
    .msg_edit                       = "%s test passed", \
    .detail                         = "A testcase has ended and passed", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_023 Message_t{ \
    .id                             = "GEN_023", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "RESULT: SOME TESTS FAILED", \
    .msg_edit                       = "", \
    .detail                         = "One or more testcases have failed", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_024 Message_t{ \
    .id                             = "GEN_024", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "RESULT: ALL TESTS PASSED", \
    .msg_edit                       = "", \
    .detail                         = "All testcases have ended and passed", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_025 Message_t{ \
    .id                             = "GEN_025", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Internal exception caught: <exception>", \
    .msg_edit                       = "Internal exception caught: %s", \
    .detail                         = "An exception occurred during the SW execution and has been caught. The description of the exception is displayed", \
    .resolution                     = "Check the preceding error messages printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_026 Message_t{ \
    .id                             = "GEN_026", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "No test passed", \
    .msg_edit                       = "", \
    .detail                         = "This error message highlights the fact that no check has been reported as successful", \
    .resolution                     = "Check the preceding messages printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_027 Message_t{ .id = "GEN_027", .state = "obsolete" }
#define MSG_GEN_028 Message_t{ .id = "GEN_028", .state = "obsolete" }
#define MSG_GEN_029 Message_t{ \
    .id                             = "GEN_029", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Could not find installed xbtest HW design compatible with selected card, identified by <identifier name> = <identifier value>", \
    .msg_edit                       = "Could not find installed xbtest HW design compatible with selected card, identified by %s = %s", \
    .detail                         = "The host application could not find any compatible installed HW designs for the selected card", \
    .resolution                     = "Check the correct HW package is installed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_030 Message_t{ \
    .id                             = "GEN_030", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Pre-canned test <test name> not found. Available pre-canned tests are: <test list>", \
    .msg_edit                       = "Pre-canned test %s not found. Available pre-canned tests are: %s", \
    .detail                         = "The pre-canned test provided is not available for selected card", \
    .resolution                     = "Correct the pre-canned test to run", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_031 Message_t{ .id = "GEN_031", .state = "obsolete" }
#define MSG_GEN_032 Message_t{ \
    .id                             = "GEN_032", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Selected card does not have a valid parameter: <param name>. Check you are targeting a card compatible with the SW version you are using", \
    .msg_edit                       = "Selected card does not have a valid parameter: %s. Check you are targeting a card compatible with the SW version you are using", \
    .detail                         = "xbtest is trying to run on a card which is not supported", \
    .resolution                     = "Check the card selected and/or check the parameter in xbutil examine output", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_033 Message_t{ \
    .id                             = "GEN_033", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<message info>", \
    .msg_edit                       = "%s", \
    .detail                         = "Message info is displayed as requested via in the command line option -m" \
}
#define MSG_GEN_034 Message_t{ \
    .id                             = "GEN_034", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-<option> command line option requires -<option> command line options", \
    .msg_edit                       = "-%s command line option requires -%s command line options", \
    .detail                         = "This message is displayed when a command line options is used but the required combination of another command line option is missing", \
    .resolution                     = "Print help using -h command line option", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_035 Message_t{ \
    .id                             = "GEN_035", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "xbtest host application is terminated by another process. An Alveo board reset has potentially been issued", \
    .msg_edit                       = "", \
    .detail                         = "Bus error signal (SIGBUS) has been received by the host application, causing it to terminate", \
    .resolution                     = "The card may have gone e.g. overpower/temperature. Check dmesg for more details", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_036 Message_t{ \
    .id                             = "GEN_036", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Command line -l and -L are incompatible", \
    .msg_edit                       = "", \
    .detail                         = "-l is used to define a log directory, while -L is used to disable log. Both options cannot be used simultaneously", \
    .resolution                     = "Only use -l or -L, but not both", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_037 Message_t{ \
    .id                             = "GEN_037", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Memory xbtest HW IP <IP name> targeting host memory <tag> found in HW design, but allocated memory is 0. xbtest HW IP will be ignored", \
    .msg_edit                       = "Memory xbtest HW IP %s targeting host memory %s found in HW design, but allocated memory is 0. xbtest HW IP will be ignored", \
    .detail                         = "The host application will skip the verify task and not be able to run the memory testcase", \
    .resolution                     = "Enable host memory if needed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_038 Message_t{ \
    .id                             = "GEN_038", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Cannot run <memory name> memory test provided in test JSON file as allocated memory is 0", \
    .msg_edit                       = "Cannot run %s memory test provided in test JSON file as allocated memory is 0", \
    .detail                         = "When the allocated memory is null, the host application is not able to run the memory testcase", \
    .resolution                     = "Enable host memory to be able to run this testcase", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_039 Message_t{ \
    .id                             = "GEN_039", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of system configuration header>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message is the header of host application displaying system configuration in the console and log file", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_040 Message_t{ \
    .id                             = "GEN_040", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of summary footer>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message is the footer of host application log displaying summary in the console and log file", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_041 Message_t{ .id = "GEN_041", .state = "obsolete" }
#define MSG_GEN_042 Message_t{ \
    .id                             = "GEN_042", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of invalid xbtest HW designs>", \
    .msg_edit                       = "%s", \
    .detail                         = "Invalid xbtest HW design name and location are displayed" \
}
#define MSG_GEN_043 Message_t{ \
    .id                             = "GEN_043", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The same BDF (<bdf>) was provided multiple times in command line", \
    .msg_edit                       = "The same BDF (%s) was provided multiple times in command line", \
    .detail                         = "The same BDF can be provided only once with command line option -d", \
    .resolution                     = "Check the list of BDF provided in command line", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_044 Message_t{ \
    .id                             = "GEN_044", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-<option> command line option cannot be combined with -D command line option", \
    .msg_edit                       = "-%s command line option cannot be combined with -D command line option", \
    .detail                         = "-D is used to select the card configuration JSON file, while -d, -j, and -c are used to set the card configuration via the command line", \
    .resolution                     = "Configure the cards only using command line options or only using the card configuration JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_045 Message_t{ \
    .id                             = "GEN_045", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The first option of the command line must be -d when running xbtest on multiple cards", \
    .msg_edit                       = "", \
    .detail                         = "The command line options defining the card configuration must be entered after the -d option", \
    .resolution                     = "Check the order of the command line options", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_046 Message_t{ \
    .id                             = "GEN_046", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "COMMON RESULT: ABORTED", \
    .msg_edit                       = "", \
    .detail                         = "Abort was received", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_047 Message_t{ \
    .id                             = "GEN_047", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "COMMON RESULT: SOME TESTS FAILED FOR SOME CARDS", \
    .msg_edit                       = "", \
    .detail                         = "One or more testcases have failed for one or more cards", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_048 Message_t{ \
    .id                             = "GEN_048", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "COMMON RESULT: ALL TESTS PASSED FOR ALL CARDS", \
    .msg_edit                       = "", \
    .detail                         = "All testcases have ended and passed for all cards", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_049 Message_t{ \
    .id                             = "GEN_049", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of xbtest common card configuration>", \
    .msg_edit                       = "%s", \
    .detail                         = "The card configuration provided in command line or in card configuration JSON file is displayed before running the tests", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_GEN_050 Message_t{ \
    .id                             = "GEN_050", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Test <idx> of <number of tests> failed", \
    .msg_edit                       = "Test %s of %s failed", \
    .detail                         = "One of the tests has failed on a card", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_051 Message_t{ \
    .id                             = "GEN_051", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Test <idx> of <number of tests> passed", \
    .msg_edit                       = "Test %s of %s passed", \
    .detail                         = "One of the tests has passed on a card" \
}
#define MSG_GEN_052 Message_t{ \
    .id                             = "GEN_052", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Test <idx> of <number of tests> aborted", \
    .msg_edit                       = "Test %s of %s aborted", \
    .detail                         = "One of the tests has aborted on a card", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_053 Message_t{ .id = "GEN_053", .state = "obsolete" }
#define MSG_GEN_054 Message_t{ \
    .id                             = "GEN_054", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Total number of <parameter> (<value>) specified in command line (via the options <options>) is greater than maximum authorized <maximum>", \
    .msg_edit                       = "Total number of %s (%s) specified in command line (via the options %s) is greater than maximum authorized %s", \
    .detail                         = "Incorrect definition of the command line", \
    .resolution                     = "Correct command line by reduce number of parameters", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_055 Message_t{ \
    .id                             = "GEN_055", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Tests aborted", \
    .msg_edit                       = "", \
    .detail                         = "Tests has been aborted on a card", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_056 Message_t{ \
    .id                             = "GEN_056", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Some tests failed", \
    .msg_edit                       = "", \
    .detail                         = "One of the tests has failed on a card", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_057 Message_t{ \
    .id                             = "GEN_057", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "All tests passed", \
    .msg_edit                       = "", \
    .detail                         = "One of the tests has passed on a card" \
}
#define MSG_GEN_058 Message_t{ \
    .id                             = "GEN_058", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Option (<opt>) is provided in more than once via command line and/or card configuration JSON file args for test <idx>", \
    .msg_edit                       = "Option (-%s) is provided in more than once via command line and/or card configuration JSON file args for test %s", \
    .detail                         = "The same option cannot be set multiple times via command line and/or card configuration JSON file", \
    .resolution                     = "Remove duplicate", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_059 Message_t{ \
    .id                             = "GEN_059", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The last option of the command line cannot be -d when running xbtest on multiple cards", \
    .msg_edit                       = "", \
    .detail                         = "The command line options defining the card configuration must be entered after the -d option", \
    .resolution                     = "Check the order of the command line options", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_060 Message_t{ \
    .id                             = "GEN_060", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Running xbtest in test <test_idx> of <num_test>. Elapsed time <time> seconds", \
    .msg_edit                       = "Running xbtest in test %s of %s. Elapsed time %s seconds", \
    .detail                         = "Reporting a test status" \
}
#define MSG_GEN_061 Message_t{ \
    .id                             = "GEN_061", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Card status. Power = <power>; Temperature = <temperature>", \
    .msg_edit                       = "Card status. Power = %s; Temperature = %s", \
    .detail                         = "Reporting a card status" \
}
#define MSG_GEN_062 Message_t{ \
    .id                             = "GEN_062", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "All card status stopped", \
    .msg_edit                       = "", \
    .detail                         = "Reporting end of card status" \
}
#define MSG_GEN_063 Message_t{ .id = "GEN_063", .state = "obsolete" }
#define MSG_GEN_064 Message_t{ \
    .id                             = "GEN_064", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Starting test <test_idx> of <num_test>. Executing: <sys_cmd>", \
    .msg_edit                       = "Starting test %s of %s. Executing: %s", \
    .detail                         = "Reporting which test is starting on which card with its xbtest SW command" \
}
#define MSG_GEN_065 Message_t{ \
    .id                             = "GEN_065", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Tests aborted: <tests results>", \
    .msg_edit                       = "Tests aborted: %s", \
    .detail                         = "Summary of card tests results. Tests were aborted", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_066 Message_t{ \
    .id                             = "GEN_066", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Some tests failed: <tests results>", \
    .msg_edit                       = "Some tests failed: %s", \
    .detail                         = "Summary of card tests results. Some tests have failed", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_067 Message_t{ \
    .id                             = "GEN_067", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "All tests passed: <tests results>", \
    .msg_edit                       = "All tests passed: %s", \
    .detail                         = "Summary of card tests results. All tests have passed", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_068 Message_t{ \
    .id                             = "GEN_068", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Host memory must be enabled for NoDMA HW designs", \
    .msg_edit                       = "", \
    .detail                         = "The host application requires host memory to be enabled for any NoDMA HW designs", \
    .resolution                     = "Enable host memory", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_069 Message_t{ \
    .id                             = "GEN_069", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-<option> command line option is not valid when multiple tests or multiple cards where provided in the command line", \
    .msg_edit                       = "-%s command line option is not valid when multiple tests or multiple cards where provided in the command line", \
    .detail                         = "No test is run when these options are provided as they are used to get information from the host application", \
    .resolution                     = "Provide a single card and no test in command line for these options", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_070 Message_t{ .id = "GEN_070", .state = "obsolete" }
#define MSG_GEN_071 Message_t{ \
    .id                             = "GEN_071", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-D command line option cannot be combined with <opt> command line option", \
    .msg_edit                       = "-D command line option cannot be combined with %s command line option", \
    .detail                         = "-T or -N is used to set P2P test, while -D used to set the card configuration JSON file", \
    .resolution                     = "To run a P2P test, configure the cards only using command line", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_072 Message_t{ \
    .id                             = "GEN_072", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-d command line option is required when P2P is set with <opt> command line option", \
    .msg_edit                       = "-d command line option is required when P2P is set with %s command line option", \
    .detail                         = "-d command line option is required to set P2P test", \
    .resolution                     = "Select the source card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_073 Message_t{ \
    .id                             = "GEN_073", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-d command line option provided more than once with <opt> command line option", \
    .msg_edit                       = "-d command line option provided more than once with %s command line option", \
    .detail                         = "-d command line option can be used only once to select the P2P source card to set", \
    .resolution                     = "Select the only one source card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_074 Message_t{ \
    .id                             = "GEN_074", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "P2P source card -d (<d>) should not match P2P target card -T (<T>)", \
    .msg_edit                       = "P2P source card -d (%s) should not match P2P target card -T (%s)", \
    .detail                         = "Source and target P2P cards cannot be the same", \
    .resolution                     = "Select different source and target P2P cards", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_075 Message_t{ \
    .id                             = "GEN_075", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-<option> command line option must be provided when P2P target is a card", \
    .msg_edit                       = "-%s command line option must be provided when P2P target is a card", \
    .detail                         = "HW design and card definition JSON file must be specified to run P2P test between two cards", \
    .resolution                     = "Set missing option", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_076 Message_t{ \
    .id                             = "GEN_076", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<opt> command line option was provided to set <target> but <testcase> testcase is not specified in test JSON file", \
    .msg_edit                       = "%s command line option was provided to set %s but %s testcase is not specified in test JSON file", \
    .detail                         = "The testcase must be specified in test JSON file when the command line is specified", \
    .resolution                     = "Set missing testcase in test JSON file or do not use -T command line option", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_077 Message_t{ \
    .id                             = "GEN_077", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<testcase> testcase is specified in test JSON file but <opt> command line option was not provided to set <value>", \
    .msg_edit                       = "%s testcase is specified in test JSON file but %s command line option was not provided to set %s", \
    .detail                         = "The command line option must be provided with correct P2P target when testcase is specified in test JSON file", \
    .resolution                     = "Correct or set missing command line option or do not use testcase in test JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_078 Message_t{ \
    .id                             = "GEN_078", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<testcase> testcase is specified in test JSON file but P2P feature is not <supported/enabled> on <source/target> card", \
    .msg_edit                       = "%s testcase is specified in test JSON file but P2P feature is not %s on %s card", \
    .detail                         = "To run a P2P testcase, P2P feature must be enabled on card", \
    .resolution                     = "Check P2P status of selected card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_079 Message_t{ \
    .id                             = "GEN_079", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<testcase> testcase is specified in test JSON file but source card is NoDMA", \
    .msg_edit                       = "%s testcase is specified in test JSON file but source card is NoDMA", \
    .detail                         = "To run a P2P testcase, the source card must have a DMA engine", \
    .resolution                     = "Check HW design running on source card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_080 Message_t{ \
    .id                             = "GEN_080", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The host application does not have <write/read> permission for file: <file>", \
    .msg_edit                       = "The host application does not have %s permission for file: %s", \
    .detail                         = "The host application requires write or read permission for a file", \
    .resolution                     = "Check permissions", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_GEN_081 Message_t{ \
    .id                             = "GEN_081", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "-T command line option cannot be combined with -N command line option", \
    .msg_edit                       = "", \
    .detail                         = "-T is used to select the P2P target card, while -N is used to select P2P NVMe path", \
    .resolution                     = "Configure P2P test only for a card (p2p_card testcase) or a NVMe path (p2p_nvme testcase)", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}

#define MSG_CMN_001 Message_t{ \
    .id                             = "CMN_001", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid value (<parameter value>) for parameter: <parameter name>. Supported values are: <list of supported values>", \
    .msg_edit                       = "Invalid value (%s) for parameter: %s. Supported values are: %s", \
    .detail                         = "A wrong value has been provided. The supported values are reported", \
    .resolution                     = "Check documentation for the parameter", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_002 Message_t{ .id = "CMN_002", .state = "obsolete" }
#define MSG_CMN_003 Message_t{ .id = "CMN_003", .state = "obsolete" }
#define MSG_CMN_004 Message_t{ \
    .id                             = "CMN_004", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Pre-Setup & test/task configuration", \
    .msg_edit                       = "", \
    .detail                         = "The test (or task) is entering its pre-setup phase which includes, at least, the verification of its configuration & settings", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_005 Message_t{ \
    .id                             = "CMN_005", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Post Teardown", \
    .msg_edit                       = "", \
    .detail                         = "The test or the task is entering its post teardown phase", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_006 Message_t{ \
    .id                             = "CMN_006", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Abort received", \
    .msg_edit                       = "", \
    .detail                         = "The test has received an abort command", \
    .resolution                     = "Check if the abort was intentional" \
}
#define MSG_CMN_007 Message_t{ \
    .id                             = "CMN_007", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Stop received", \
    .msg_edit                       = "", \
    .detail                         = "The test task has received a stop command" \
}
#define MSG_CMN_008 Message_t{ \
    .id                             = "CMN_008", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Launch test/task at <timestamp>", \
    .msg_edit                       = "Launch test/task at %s", \
    .detail                         = "Launch test/task confirmation" \
}
#define MSG_CMN_009 Message_t{ \
    .id                             = "CMN_009", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Test/task aborted at <timestamp>", \
    .msg_edit                       = "Test/task aborted at %s", \
    .detail                         = "Test/task has aborted due to a previous failure", \
    .resolution                     = "Check error and failure messages available" \
}
#define MSG_CMN_010 Message_t{ \
    .id                             = "CMN_010", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Test/task finished with error(s) at <timestamp>", \
    .msg_edit                       = "Test/task finished with error(s) at %s", \
    .detail                         = "Test/task has completed but error(s) have been detected", \
    .resolution                     = "Check error messages available" \
}
#define MSG_CMN_011 Message_t{ \
    .id                             = "CMN_011", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Test/task finished successfully at <timestamp>", \
    .msg_edit                       = "Test/task finished successfully at %s", \
    .detail                         = "Test/task has completed without encountering any error or failure" \
}
#define MSG_CMN_012 Message_t{ \
    .id                             = "CMN_012", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Setting parameter <parameter name> to its default value <parameter value>", \
    .msg_edit                       = "Setting parameter %s to its default value %s", \
    .detail                         = "During the parsing of the test JSON file, the listed parameter has not been overwritten. It keeps its default value, which is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_013 Message_t{ \
    .id                             = "CMN_013", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Overwriting default value of parameter <parameter name> with <parameter value>", \
    .msg_edit                       = "Overwriting default value of parameter %s with %s", \
    .detail                         = "The listed parameter has been overwritten with the value found in the test JSON file", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_014 Message_t{ \
    .id                             = "CMN_014", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The value (<parameter value>) for parameter <parameter name> is below its minimum authorised value. The valid range for this parameter is [<min value>,<max value>]", \
    .msg_edit                       = "The value (%s) for parameter %s is below its minimum authorised value. The valid range for this parameter is [%s,%s]", \
    .detail                         = "The value supplied in the test JSON file or in the command line for the listed parameter is out of range. The supported range is displayed along with the failure message", \
    .resolution                     = "Provide a value within the valid range", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_015 Message_t{ \
    .id                             = "CMN_015", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The value (<parameter value>) for parameter <parameter name> is above its maximum authorised value. The valid range for this parameter is [<min value>,<max value>]", \
    .msg_edit                       = "The value (%s) for parameter %s is above its maximum authorised value. The valid range for this parameter is [%s,%s]", \
    .detail                         = "The value supplied in the test JSON file or in the command line for the listed parameter is out of range. The supported range is displayed along the failure message", \
    .resolution                     = "Provide a value within the valid range", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_016 Message_t{ \
    .id                             = "CMN_016", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The value (<parameter value>) for parameter <parameter name> cannot be converted into the parameter type", \
    .msg_edit                       = "The value (%s) for parameter %s cannot be converted into the parameter type", \
    .detail                         = "The value supplied in the test JSON file for the listed parameter is not of the correct type. Check documentation for the type definition of the parameter", \
    .resolution                     = "Provide a value with the correct type: e.g. boolean, integer", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_017 Message_t{ \
    .id                             = "CMN_017", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Low & high threshold mistake. The value of low threshold <low threshold name> (<low threshold value>) is not smaller than the value of high threshold <high threshold name> (<high threshold value>).", \
    .msg_edit                       = "Low & high threshold mistake. The value of low threshold %s (%s) is not smaller than the value of high threshold %s (%s).", \
    .detail                         = "The thresholds definition of the parameter is not correct.  The low value must be smaller than the high value", \
    .resolution                     = "Provide a low threshold below the high one", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_018 Message_t{ \
    .id                             = "CMN_018", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Could not open or create the file: <file name>", \
    .msg_edit                       = "Could not open or create the file: %s", \
    .detail                         = "The file could not be opened or created", \
    .resolution                     = "Check permission (& path) and/or if the file is already opened by another application", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_019 Message_t{ \
    .id                             = "CMN_019", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "The following output file already exists on disk and its content will be overwritten: <file>", \
    .msg_edit                       = "The following output file already exists on disk and its content will be overwritten: %s", \
    .detail                         = "The output file is already existing. Its previous content is erased and will be overwritten during the test", \
    .resolution                     = "Are you sure that you want this file to be overwritten? If yes, you can ignore this warning. If not, make sure you rename your output file path (see -l command line option) as now the previous content has been overwritten", \
    .display_classic_console        = false \
}
#define MSG_CMN_020 Message_t{ \
    .id                             = "CMN_020", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "The following output file will be used to store values/results: <file>", \
    .msg_edit                       = "The following output file will be used to store values/results: %s", \
    .detail                         = "Displays name of output file used to store values", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_021 Message_t{ \
    .id                             = "CMN_021", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of test configuration>", \
    .msg_edit                       = "%s", \
    .detail                         = "Displays the test configuration", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_022 Message_t{ \
    .id                             = "CMN_022", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Checking test sequence parameters", \
    .msg_edit                       = "", \
    .detail                         = "Start to check the test sequence parameters according to the test configuration" \
}
#define MSG_CMN_023 Message_t{ \
    .id                             = "CMN_023", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "All test settings of the test sequence are correct", \
    .msg_edit                       = "", \
    .detail                         = "Result of the check of the test sequence values", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_024 Message_t{ \
    .id                             = "CMN_024", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Some test settings supplied in the test sequence are not correct", \
    .msg_edit                       = "", \
    .detail                         = "Result of the of the test sequence values check", \
    .resolution                     = "Check message displayed prior to this for detailed description of the errors", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_025 Message_t{ \
    .id                             = "CMN_025", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid settings provided for test <test index>", \
    .msg_edit                       = "Invalid settings provided for test %s", \
    .detail                         = "This is a summary message which lists the 20 first tests containing error", \
    .resolution                     = "Check message displayed prior to this for detailed description of the errors", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_026 Message_t{ .id = "CMN_026", .state = "obsolete" }
#define MSG_CMN_027 Message_t{ \
    .id                             = "CMN_027", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Watchdog has been triggered during previous test, but it's safe to start this test", \
    .msg_edit                       = "", \
    .detail                         = "The watchdog has been triggered during a previous test but it's safe to start this test as the xbtest HW IP has been stopped by the watchdog", \
    .resolution                     = "Check why the watchdog triggered. Did you interrupt previous test?", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_028 Message_t{ \
    .id                             = "CMN_028", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Test already running. By trying to start another test, this can have caused error(s) in currently running test", \
    .msg_edit                       = "", \
    .detail                         = "Test already running on this xbtest HW IP. Trying to start another test can cause error(s) in currently running test. If no tests are running, the card could be in an unknown state. First re-validate it, then try running xbtest again", \
    .resolution                     = "Do not target the same board with 2 different tests. Check the documentation about how to target a different card using option -d", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_029 Message_t{ \
    .id                             = "CMN_029", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Watchdog has been triggered during previous test", \
    .msg_edit                       = "", \
    .detail                         = "Watchdog has been triggered during previous test", \
    .resolution                     = "Check that the previous test has terminated", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_030 Message_t{ \
    .id                             = "CMN_030", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Another test tried to access xbtest HW IP <IP name>. This can have caused errors to this test", \
    .msg_edit                       = "Another test tried to access xbtest HW IP %s. This might have caused errors to this test", \
    .detail                         = "Another test tried to access the xbtest HW IP. This might have caused an error to this test", \
    .resolution                     = "Do not target the same board with 2 different tests. Check the documentation about how to target different card using option -d ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_031 Message_t{ \
    .id                             = "CMN_031", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Watchdog alarm detected. This can have caused error to this test", \
    .msg_edit                       = "", \
    .detail                         = "The watchdog triggered during the test, meaning the xbtest SW pauses for at least 10 seconds", \
    .resolution                     = "Check that the processors are not heavily loaded", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_032 Message_t{ \
    .id                             = "CMN_032", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Start Test <test index>: <test parameters>", \
    .msg_edit                       = "Start Test %s: %s", \
    .detail                         = "Displays the started test from the test sequence" \
}
#define MSG_CMN_033 Message_t{ \
    .id                             = "CMN_033", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "End Test: <test index>", \
    .msg_edit                       = "End Test: %s", \
    .detail                         = "Displays the completed test from the test sequence, this message could be a pass or an error", \
    .resolution                     = "If this message is an error, check previous message to identify why the test failed" \
}
#define MSG_CMN_034 Message_t{ \
    .id                             = "CMN_034", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<list of test sequence parameters>", \
    .msg_edit                       = "%s", \
    .detail                         = "Displays the parameters of the starting test sequence" \
}
#define MSG_CMN_035 Message_t{ .id = "CMN_035", .state = "obsolete" }
#define MSG_CMN_036 Message_t{ \
    .id                             = "CMN_036", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The nominal/default value (<parameter value>) for parameter <parameter name> is out of range [<min value>,<max value>]", \
    .msg_edit                       = "The nominal/default value (%s) for parameter %s is out of range [%s,%s]", \
    .detail                         = "The default value defined for the parameter is wrong. This default value may be defined into the card definition JSON file", \
    .resolution                     = "Check that the value supplied in the card definition JSON file is within range", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_037 Message_t{ .id = "CMN_037", .state = "obsolete" }
#define MSG_CMN_038 Message_t{ \
    .id                             = "CMN_038", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "AP 2 clock monitor should not be present", \
    .msg_edit                       = "", \
    .detail                         = "This xbtest HW IP should not contain any AP 2 clock monitor", \
    .resolution                     = "Check that you're using a valid HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_CMN_039 Message_t{ \
    .id                             = "CMN_039", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<clock name> monitor should be present", \
    .msg_edit                       = "%s monitor should be present", \
    .detail                         = "This xbtest HW IP should contain clock monitoring for detecting clock throttling", \
    .resolution                     = "Check that you're using a valid HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_CMN_040 Message_t{ \
    .id                             = "CMN_040", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "No clock throttling detected for <clock name>", \
    .msg_edit                       = "No clock throttling detected for %s", \
    .detail                         = "Clock monitor did not detect any clock throttling", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_041 Message_t{ \
    .id                             = "CMN_041", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "<clock name> monitor detects that throttling was activated, this may have negative impact on your results", \
    .msg_edit                       = "%s monitor detects that throttling was activated, this may have negative impact on your results", \
    .detail                         = "The automatic clock throttling has been enabled, this will affect results/performances of all xbtest HW IPs as they won't run at their nominal/expected frequencies", \
    .resolution                     = "Make sure that the card is working within its power and temperature limits", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_CMN_042 Message_t{ \
    .id                             = "CMN_042", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<list of extra test parameters>", \
    .msg_edit                       = "%s", \
    .detail                         = "Displays extra (or default) parameters of the test. These parameters may be fixed by the SW according to the HW design loaded and some may potentially be overwritten, check documentation", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_CMN_043 Message_t{ \
    .id                             = "CMN_043", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Overwriting value of parameter <parameter>", \
    .msg_edit                       = "Overwriting value of parameter %s", \
    .detail                         = "The listed parameter has been overwritten with the values found in the test JSON file" \
}
#define MSG_CMN_044 Message_t{ \
    .id                             = "CMN_044", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Number of tests (<number of tests>) in test_sequence override for <override level> does not equal the number of tests (<number of tests>) in global test_sequence", \
    .msg_edit                       = "Number of tests (%s) in test_sequence override for %s does not equal the number of tests (%s) in global test_sequence", \
    .detail                         = "The host application only supports test_sequence overwrite when the number of parameters is the same as the original test sequence", \
    .resolution                     = "Check the test_sequence override", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_045 Message_t{ \
    .id                             = "CMN_045", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameter <parameter> for <override level> value (<value>) of test <test index> in test_sequence overwrite does not equal the value (<value>) in global test_sequence", \
    .msg_edit                       = "Parameter %s for %s value (%s) of test %s in test_sequence overwrite does not equal the value (%s) in global test_sequence", \
    .detail                         = "The host application only supports test override with same value as the value in original test sequence for this parameter", \
    .resolution                     = "Check the test_sequence override", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_046 Message_t{ \
    .id                             = "CMN_046", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "For <override level>, use in test <test index> the parameters: <test parameters>", \
    .msg_edit                       = "For %s, use in test %s the parameters: %s", \
    .detail                         = "Displays the started test from the test sequence override" \
}
#define MSG_CMN_047 Message_t{ \
    .id                             = "CMN_047", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<number of tests> tests provided in test sequence: <test sequence>", \
    .msg_edit                       = "%s tests provided in test sequence: %s", \
    .detail                         = "Displays the configuration provided for each test of the test sequences" \
}
#define MSG_CMN_048 Message_t{ \
    .id                             = "CMN_048", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "<tab><number of seconds> seconds remaining <info>", \
    .msg_edit                       = "\t%s seconds remaining %s", \
    .detail                         = "Status of the ongoing test. This is displayed on a regular basis. info can be displayed when applicable" \
}
#define MSG_CMN_049 Message_t{ \
    .id                             = "CMN_049", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Test duration reached", \
    .msg_edit                       = "", \
    .detail                         = "Status of the ongoing test, displayed at the end of test" \
}
#define MSG_CMN_050 Message_t{ \
    .id                             = "CMN_050", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The parameter <parameter name> must not been defined under this circumstance: <reason>", \
    .msg_edit                       = "The parameter %s must not been defined under this circumstance: %s ", \
    .detail                         = "The listed parameter should not be used in the described condition", \
    .resolution                     = "Remove the parameter", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_051 Message_t{ \
    .id                             = "CMN_051", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Test is not running in optimal conditions as <justification>. Write and read %s results could be affected", \
    .msg_edit                       = "Test is not running in optimal conditions as %s. Write and read %s results could be affected", \
    .detail                         = "The memory/dma testcase is not running in optimal conditions based on the test JSON file configuration (duration, burst/block size, rate, number of outstanding transactions, buffer size, etc) ", \
    .resolution                     = "Review the justification provided in this message and if needed, re-run the testcase with updated test JSON file parameters. You can also force the BW or latency check by setting check_bw or check_latency test JSON file parameter", \
    .display_classic_console        = false \
}
#define MSG_CMN_052 Message_t{ \
    .id                             = "CMN_052", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Check of write and read <bw/lat> is disabled as memory test is not running in optimal conditions", \
    .msg_edit                       = "Check of write and read %s is disabled as memory test is not running in optimal conditions", \
    .detail                         = "The memory/dma testcase is not running in optimal conditions based on the test JSON file configuration (duration, burst/block size, rate, number of outstanding transactions, buffer size, etc) ", \
    .resolution                     = "Review the justification provided in this message and if needed, re-run the testcase with updated test JSON file parameters. You can also force the BW or latency check by setting check_bw or check_latency test JSON file parameter", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_CMN_053 Message_t{ \
    .id                             = "CMN_053", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The following output file already exists on disk, use the force command line option (-f) to overwrite: <file>", \
    .msg_edit                       = "The following output file already exists on disk, use the force command line option (-f) to overwrite: %s", \
    .detail                         = "The output file is already existing. By default the host application cannot re-use file already existing", \
    .resolution                     = "Make sure you rename your file path (see -l command line option) or if are you sure that you want this file to be overwritten, then see -f command line option", \
    .display_classic_console        = false \
}
#define MSG_CMN_054 Message_t{ \
    .id                             = "CMN_054", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The card <power/temperature> should be defined only once in the card definition JSON file", \
    .msg_edit                       = "The card %s should be defined only once in the card definition JSON file", \
    .detail                         = "The override of card power or temperature cannot be defined multiple time in the card definition JSON file", \
    .resolution                     = "Make sure you define the override only once", \
    .display_classic_console        = false \
}
#define MSG_ITF_001 Message_t{ \
    .id                             = "ITF_001", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Setting parameter <parameter name> with provided value: <parameter value>", \
    .msg_edit                       = "Setting parameter %s with provided value: %s", \
    .detail                         = "Display parameter used to configure the device interface", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_002 Message_t{ \
    .id                             = "ITF_002", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameter <parameter name> must be defined in test JSON file or in command line", \
    .msg_edit                       = "Parameter %s must be defined in test JSON file or in command line", \
    .detail                         = "A required parameter has not been provided via command line or via test JSON file", \
    .resolution                     = "Check the parameter has been defined via command line or via test JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_003 Message_t{ \
    .id                             = "ITF_003", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "File for <source file> does not exist <file name>", \
    .msg_edit                       = "File for %s does not exist %s", \
    .detail                         = "A required source file was not found", \
    .resolution                     = "Check that the file exists", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_004 Message_t{ .id = "ITF_004", .state = "obsolete" }
#define MSG_ITF_005 Message_t{ \
    .id                             = "ITF_005", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No Xilinx cl::Platform available.", \
    .msg_edit                       = "", \
    .detail                         = "No Xilinx board has been detected by the host application", \
    .resolution                     = "Check the output of the 'xbutil examine' command", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_006 Message_t{ .id = "ITF_006", .state = "obsolete" }
#define MSG_ITF_007 Message_t{ .id = "ITF_007", .state = "obsolete" }
#define MSG_ITF_008 Message_t{ \
    .id                             = "ITF_008", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using <card>: <card name>", \
    .msg_edit                       = "Using %s: %s", \
    .detail                         = "The following board has been selected using provided BDF", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_ITF_009 Message_t{ \
    .id                             = "ITF_009", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Loading HW design. This could take up to <time> seconds", \
    .msg_edit                       = "Loading HW design. This could take up to %s seconds", \
    .detail                         = "The host application starts to download the HW design to the card", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_ITF_010 Message_t{ .id = "ITF_010", .state = "obsolete" }
#define MSG_ITF_011 Message_t{ \
    .id                             = "ITF_011", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to get the device information: <info>", \
    .msg_edit                       = "Failed to get the device information: %s", \
    .detail                         = "The host application was not able to get a device information from card driver API", \
    .resolution                     = "Check the preceding error message printed before this error and check the output of card driver API", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_012 Message_t{ .id = "ITF_012", .state = "obsolete" }
#define MSG_ITF_013 Message_t{ \
    .id                             = "ITF_013", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to convert <JSON source> member <member name>", \
    .msg_edit                       = "Failed to convert %s member %s", \
    .detail                         = "The host application was not able to convert the string value of the member to a number", \
    .resolution                     = "Check the JSON output", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_014 Message_t{ .id = "ITF_014", .state = "obsolete" }
#define MSG_ITF_015 Message_t{ \
    .id                             = "ITF_015", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<Board and system information>", \
    .msg_edit                       = "%s", \
    .detail                         = "After the HW design has been successfully downloaded, information extracted from card driver API output is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_016 Message_t{ \
    .id                             = "ITF_016", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "MEM_TOPOLOGY count", \
    .msg_edit                       = "MEM_TOPOLOGY %s count: %s", \
    .detail                         = "Reporting the number of memory banks available for type: DDR, HBM..." \
}
#define MSG_ITF_017 Message_t{ \
    .id                             = "ITF_017", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Measured HW design download time (<measured time> seconds) greater than expected limit (<expected limit> seconds) ", \
    .msg_edit                       = "Measured HW design download time (%s seconds) greater than expected limit (%s seconds) ", \
    .detail                         = "Although the HW design has been successfully loaded, the duration of the HW design download was greater than the expected limit, specified by the card definition JSON file", \
    .resolution                     = "Check PCIe workload, for example, DMA transfers, other HW design download. Note: if xbtest is re-run, this error might not be raised again as an HW design is not re-loaded when it is already programmed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_018 Message_t{ \
    .id                             = "ITF_018", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Measured HW design download time (<measured time> seconds) within tolerance (<expected limit> seconds) ", \
    .msg_edit                       = "Measured HW design download time (%s seconds) within tolerance (%s seconds) ", \
    .detail                         = "The duration of the HW design download was lower than the expected limit, specified by the card definition JSON file", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_019 Message_t{ .id = "ITF_019", .state = "obsolete" }
#define MSG_ITF_020 Message_t{ \
    .id                             = "ITF_020", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Checking HW design clock frequencies", \
    .msg_edit                       = "", \
    .detail                         = "The host application checks the clock frequencies in the downloaded HW design matched the frequencies specified by the card definition JSON file", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_021 Message_t{ .id = "ITF_021", .state = "obsolete" }
#define MSG_ITF_022 Message_t{ .id = "ITF_022", .state = "obsolete" }
#define MSG_ITF_023 Message_t{ \
    .id                             = "ITF_023", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Setup xbtest HW IPs", \
    .msg_edit                       = "", \
    .detail                         = "The host application will setup the valid xbtest HW IPs that are found in the HW design, creating xbtest HW IPs and buffers", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_024 Message_t{ \
    .id                             = "ITF_024", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No compatible xbtest HW IP found in HW design", \
    .msg_edit                       = "", \
    .detail                         = "The host application has not found any xbtest HW IPs in the HW design", \
    .resolution                     = "Check the HW design used", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_025 Message_t{ .id = "ITF_025", .state = "obsolete" }
#define MSG_ITF_026 Message_t{ .id = "ITF_026", .state = "obsolete" }
#define MSG_ITF_027 Message_t{ .id = "ITF_027", .state = "obsolete" }
#define MSG_ITF_028 Message_t{ .id = "ITF_028", .state = "obsolete" }
#define MSG_ITF_029 Message_t{ .id = "ITF_029", .state = "obsolete" }
#define MSG_ITF_030 Message_t{ \
    .id                             = "ITF_030", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Found <IP quantity> <IP type> xbtest HW IP(s) ", \
    .msg_edit                       = "Found %s %s xbtest HW IP(s) ", \
    .detail                         = "The number of valid xbtest HW IPs found in the HW design is reported per xbtest HW IP type", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_031 Message_t{ \
    .id                             = "ITF_031", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Unknown xbtest HW IPs found in HW design: <unknown IP names>", \
    .msg_edit                       = "Unknown xbtest HW IPs found in HW design: %s", \
    .detail                         = "The name of each unknown xbtest HW IP found in the HW design is reported", \
    .resolution                     = "Check the HW design used", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_ITF_032 Message_t{ \
    .id                             = "ITF_032", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to get xbtest metadata in section <section>", \
    .msg_edit                       = "Failed to get xbtest metadata in section %s", \
    .detail                         = "The host application was not able to get necessary xbtest metadata", \
    .resolution                     = "Check content of xbtest metadata section", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_033 Message_t{ .id = "ITF_033", .state = "obsolete" }
#define MSG_ITF_034 Message_t{ .id = "ITF_034", .state = "obsolete" }
#define MSG_ITF_035 Message_t{ .id = "ITF_035", .state = "obsolete" }
#define MSG_ITF_036 Message_t{ \
    .id                             = "ITF_036", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Required memory <tag/index> <value> not found in MEM_TOPOLOGY", \
    .msg_edit                       = "Required memory %s %s not found in MEM_TOPOLOGY", \
    .detail                         = "The host application was not able to find memory in MEM_TOPOLOGY", \
    .resolution                     = "Check the memory tag was used in HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_037 Message_t{ \
    .id                             = "ITF_037", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Required memory <tag/index> <memory tag> found unused in MEM_TOPOLOGY", \
    .msg_edit                       = "Required memory %s %s found unused in MEM_TOPOLOGY", \
    .detail                         = "The host application has found memory in MEM_TOPOLOGY but the memory is not enabled", \
    .resolution                     = "Check the memory tag was used in HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_038 Message_t{ \
    .id                             = "ITF_038", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Card driver API error: <OpenCL error (function, arguments, return code ...)>", \
    .msg_edit                       = "Card driver API error: %s", \
    .detail                         = "Card driver API implementation failed.", \
    .resolution                     = "Check the error message content, and errors reported before, e.g. errors can be reported by driver in STDERR which are not visible in xbtest.log file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_039 Message_t{ \
    .id                             = "ITF_039", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Get xbtest metadata in section <section>", \
    .msg_edit                       = "Get xbtest metadata in section %s", \
    .detail                         = "The host application gets an xbtest metadata section", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_040 Message_t{ .id = "ITF_040", .state = "obsolete" }
#define MSG_ITF_041 Message_t{ .id = "ITF_041", .state = "obsolete" }
#define MSG_ITF_042 Message_t{ .id = "ITF_042", .state = "obsolete" }
#define MSG_ITF_043 Message_t{ .id = "ITF_043", .state = "obsolete" }
#define MSG_ITF_044 Message_t{ \
    .id                             = "ITF_044", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Required parameter not found in <source JSON>: <parameter name>", \
    .msg_edit                       = "Required parameter not found in %s: %s", \
    .detail                         = "A required JSON parameter is not found", \
    .resolution                     = "Check JSON content. For external commands, try and run the command in another terminal", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_045 Message_t{ \
    .id                             = "ITF_045", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using <source file>: <file name>", \
    .msg_edit                       = "Using %s: %s", \
    .detail                         = "Displays the name of the file being read", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_046 Message_t{ \
    .id                             = "ITF_046", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Reporting card driver <info> API JSON output for card at <bdf> below: <output>", \
    .msg_edit                       = "Reporting card driver %s API JSON output for card at %s below:\n%s", \
    .detail                         = "The card driver API JSON output is reported when it is invalid (not parsable as JSON, missing node, node value conversion failure) ", \
    .resolution                     = "Check JSON content. For external commands, try and run the command in another terminal" \
}
#define MSG_ITF_047 Message_t{ .id = "ITF_047", .state = "obsolete" }
#define MSG_ITF_048 Message_t{ .id = "ITF_048", .state = "obsolete" }
#define MSG_ITF_049 Message_t{ .id = "ITF_049", .state = "obsolete" }
#define MSG_ITF_050 Message_t{ .id = "ITF_050", .state = "obsolete" }
#define MSG_ITF_051 Message_t{ .id = "ITF_051", .state = "obsolete" }
#define MSG_ITF_052 Message_t{ \
    .id                             = "ITF_052", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of xbtest SW download time configuration>", \
    .msg_edit                       = "%s", \
    .detail                         = "The xbtest SW download time configuration is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_053 Message_t{ .id = "ITF_053", .state = "obsolete" }
#define MSG_ITF_054 Message_t{ .id = "ITF_054", .state = "obsolete" }
#define MSG_ITF_055 Message_t{ \
    .id                             = "ITF_055", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of xbtest SW temperature sources>", \
    .msg_edit                       = "%s", \
    .detail                         = "The xbtest SW temperature sources configuration is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_056 Message_t{ .id = "ITF_056", .state = "obsolete" }
#define MSG_ITF_057 Message_t{ \
    .id                             = "ITF_057", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of xbtest SW power sources>", \
    .msg_edit                       = "%s", \
    .detail                         = "The xbtest SW power sources configuration is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_058 Message_t{ \
    .id                             = "ITF_058", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of xbtest SW memory configuration>", \
    .msg_edit                       = "%s", \
    .detail                         = "The xbtest SW memory configuration is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_059 Message_t{ .id = "ITF_059", .state = "obsolete" }
#define MSG_ITF_060 Message_t{ .id = "ITF_060", .state = "obsolete" }
#define MSG_ITF_061 Message_t{ .id = "ITF_061", .state = "obsolete" }
#define MSG_ITF_062 Message_t{ .id = "ITF_062", .state = "obsolete" }
#define MSG_ITF_063 Message_t{ .id = "ITF_063", .state = "obsolete" }
#define MSG_ITF_064 Message_t{ .id = "ITF_064", .state = "obsolete" }
#define MSG_ITF_065 Message_t{ .id = "ITF_065", .state = "obsolete" }
#define MSG_ITF_066 Message_t{ .id = "ITF_066", .state = "obsolete" }
#define MSG_ITF_067 Message_t{ \
    .id                             = "ITF_067", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Wrong number of parameters in test JSON file for <test type test_sequence[seq idx]>: <number of parameters found>. Expecting <expected number of parameters> parameters", \
    .msg_edit                       = "Wrong number of parameters in test JSON file for %s: %s. Expecting %s parameters", \
    .detail                         = "The number of parameters of the test sequence in the test JSON file is not valid", \
    .resolution                     = "Check  the test sequence in the test JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_068 Message_t{ .id = "ITF_068", .state = "obsolete" }
#define MSG_ITF_069 Message_t{ .id = "ITF_069", .state = "obsolete" }
#define MSG_ITF_070 Message_t{ .id = "ITF_070", .state = "obsolete" }
#define MSG_ITF_071 Message_t{ .id = "ITF_071", .state = "obsolete" }
#define MSG_ITF_072 Message_t{ .id = "ITF_072", .state = "obsolete" }
#define MSG_ITF_073 Message_t{ \
    .id                             = "ITF_073", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid parameter(s) found in test JSON file for <test type test_sequence[seq idx]>. Check error message(s) above", \
    .msg_edit                       = "Invalid parameter(s) found in test JSON file for %s. Check error message(s) above", \
    .detail                         = "Error(s) were detected in the test sequence being parsed", \
    .resolution                     = "Check the error messages before this error occurs", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_074 Message_t{ .id = "ITF_074", .state = "obsolete" }
#define MSG_ITF_075 Message_t{ \
    .id                             = "ITF_075", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid parameters were found in test <test type> configuration. Check error message(s) above", \
    .msg_edit                       = "Invalid parameters were found in test %s configuration. Check error message(s) above", \
    .detail                         = "The host application has detected errors in the test configuration and has aborted the test configuration check", \
    .resolution                     = "Check the test configuration", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_076 Message_t{ \
    .id                             = "ITF_076", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Wrong value in test parameter <parameter name>: <parameter value>. Expecting <expected values>", \
    .msg_edit                       = "Wrong value in test parameter %s: %s. Expecting %s", \
    .detail                         = "A wrong value has been detected for this parameter while parsing test parameters", \
    .resolution                     = "Check the test parameter value", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_077 Message_t{ \
    .id                             = "ITF_077", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of test JSON file definition>", \
    .msg_edit                       = "%s", \
    .detail                         = "The test JSON file definition is printed when a wrong/missing parameter is detected" \
}
#define MSG_ITF_078 Message_t{ \
    .id                             = "ITF_078", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Creating log directory <directory name>", \
    .msg_edit                       = "Creating log directory %s", \
    .detail                         = "The host application will use a log directory to store information such as messages, results, temporary files", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_079 Message_t{ \
    .id                             = "ITF_079", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to create log directory <log directory name>: <error>", \
    .msg_edit                       = "Failed to create log directory %s: %s", \
    .detail                         = "The host application was not able to create the log directory using mkdir command", \
    .resolution                     = "Check permissions", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_080 Message_t{ .id = "ITF_080", .state = "obsolete" }
#define MSG_ITF_081 Message_t{ .id = "ITF_081", .state = "obsolete" }
#define MSG_ITF_082 Message_t{ .id = "ITF_082", .state = "obsolete" }
#define MSG_ITF_083 Message_t{ \
    .id                             = "ITF_083", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to create xbtest log file <log file name>", \
    .msg_edit                       = "Failed to create xbtest log file %s", \
    .detail                         = "The host application was not able to create the log file", \
    .resolution                     = "Check permissions", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_084 Message_t{ \
    .id                             = "ITF_084", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "HW design download time: <download time> seconds", \
    .msg_edit                       = "HW design download time: %s seconds", \
    .detail                         = "The host application reports the time taken to download the HW design on the board", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_085 Message_t{ \
    .id                             = "ITF_085", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "For more information on supported test JSON parameters, try xbtest \"-g <testcase/task>\" command line option", \
    .msg_edit                       = "", \
    .detail                         = "The test JSON file contains invalid parameters. Use \"-g <testcase/task>\" option, with <testcase/task> the name of the testcase or task, to get more info on the test JSON file format and supported parameters" \
}
#define MSG_ITF_086 Message_t{ \
    .id                             = "ITF_086", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No card found for device BDF: <provided device BDF>", \
    .msg_edit                       = "No card found for device BDF: %s", \
    .detail                         = "The host application failed to find a card with a device BDF matching the provided one", \
    .resolution                     = "Check provided device BDF match the device BDF of the selected card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_087 Message_t{ .id = "ITF_087", .state = "obsolete" }
#define MSG_ITF_088 Message_t{ \
    .id                             = "ITF_088", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of xbtest SW GT configuration>", \
    .msg_edit                       = "%s", \
    .detail                         = "The xbtest SW GT configuration is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_089 Message_t{ .id = "ITF_089", .state = "obsolete" }
#define MSG_ITF_090 Message_t{ .id = "ITF_090", .state = "obsolete" }
#define MSG_ITF_091 Message_t{ \
    .id                             = "ITF_091", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Incorrect output for command <command>. Reporting output below:<command output>", \
    .msg_edit                       = "Incorrect output for command %s. Reporting output below:\n%s", \
    .detail                         = "The host application has executed an external command but its output in stdout was incorrect", \
    .resolution                     = "Try and run the command in another terminal", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_092 Message_t{ \
    .id                             = "ITF_092", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid array in card definition JSON file. Array indexes must be defined back-to-back. Found invalid index <index> in array <array name>", \
    .msg_edit                       = "Invalid array in card definition JSON file. Array indexes must be defined back-to-back. Found invalid index %s in array %s", \
    .detail                         = "The index of arrays in card definition JSON file must be consecutive. Example of invalid indexes: 0, 1, 3", \
    .resolution                     = "Check card definition JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_093 Message_t{ \
    .id                             = "ITF_093", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Incorrect test JSON file. It could be that the card definition JSON file is being used instead of the test JSON file", \
    .msg_edit                       = "", \
    .detail                         = "The file provided as test JSON file is not correct", \
    .resolution                     = "Check test JSON file is being used", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_094 Message_t{ \
    .id                             = "ITF_094", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid <test_sequence> definition. At least one test must me defined in a test sequence", \
    .msg_edit                       = "Invalid %s definition. At least one test must me defined in a test sequence", \
    .detail                         = "It is invalid to use an empty test sequence", \
    .resolution                     = "Remove the testcase from the test JSON file or define a test in test sequence", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_095 Message_t{ .id = "ITF_095", .state = "obsolete" }
#define MSG_ITF_096 Message_t{ .id = "ITF_096", .state = "obsolete" }
#define MSG_ITF_097 Message_t{ \
    .id                             = "ITF_097", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Definition of <parameter> (<value>) is present in USER_METADATA but was not found in card definition JSON file", \
    .msg_edit                       = "Definition of %s (%s) is present in USER_METADATA but was not found in card definition JSON file", \
    .detail                         = "The host application first gets HW configuration in USER_METADATA which is then, in some cases, completed with other parameters from card definition JSON file", \
    .resolution                     = "Check the required parameters are provided in card definition JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_098 Message_t{ \
    .id                             = "ITF_098", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Definition of <parameter> (<value>) is present more than once in card definition JSON file", \
    .msg_edit                       = "Definition of %s (%s) is present more than once in card definition JSON file", \
    .detail                         = "Multiple definition of a parameter/configuration is found in the card definition JSON file", \
    .resolution                     = "Check the parameters provided in card definition JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_099 Message_t{ \
    .id                             = "ITF_099", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<node> node is not supported in card definition JSON file for memory (<name>) ", \
    .msg_edit                       = "%s node is not supported in card definition JSON file for memory (%s) ", \
    .detail                         = "Host memory is not supported in DMA test, while board memory is", \
    .resolution                     = "For host memory, remove the node in the definition of this memory in card definition JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_100 Message_t{ \
    .id                             = "ITF_100", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of xbtest HW IP configuration>", \
    .msg_edit                       = "%s", \
    .detail                         = "The xbtest HW IP configuration is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_101 Message_t{ \
    .id                             = "ITF_101", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Fail to generate xbtest SW configuration for <BDF>", \
    .msg_edit                       = "Fail to generate xbtest SW configuration for %s", \
    .detail                         = "xbtest SW configuration, based on xbtest metadata, card definition JSON file and card driver API output, cannot be generated", \
    .resolution                     = "Check the content of card definition JSON file, xbtest metadata, version of card driver being used", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_102 Message_t{ .id = "ITF_102", .state = "obsolete" }
#define MSG_ITF_103 Message_t{ \
    .id                             = "ITF_103", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<testcase> testcase specified in test JSON file but no xbtest HW IP present in HW design to run it: <detail>", \
    .msg_edit                       = "%s testcase specified in test JSON file but no xbtest HW IP present in HW design to run it: %s", \
    .detail                         = "The HW design being used does not contains the xbtest HW IP required to run the testcase specified in the test JSON", \
    .resolution                     = "Check the HW design being used, or remove the testcase not supported from test JSON", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_104 Message_t{ \
    .id                             = "ITF_104", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "In test JSON file <path to parameter>: invalid type found for the definition of the test iteration parameters. Expected <type>", \
    .msg_edit                       = "In test JSON file %s: invalid type found for the definition of the test iteration parameters. Expected %s", \
    .detail                         = "This parameter should be defined as an array (JSON_NODE_ARRAY) of objects (JSON_NODE_OBJECT). Array of arrays has been deprecated.", \
    .resolution                     = "Check the test sequence in the test JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_105 Message_t{ \
    .id                             = "ITF_105", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Failed to create <tag> memory (memory index <idx>) extra OpenCL buffer <size> Bytes <OpenCL code>. Remaining size of memory not allocated is <size> Bytes (<percent> %). Clearing extra memory previously allocated", \
    .msg_edit                       = "Failed to create %s memory (memory index %s) extra OpenCL buffer of size %s Bytes %s. Remaining size of memory not allocated is %s Bytes (%s %). Clearing extra memory previously allocated", \
    .detail                         = "The host application create required extra buffers. Depending on the targeted memory type this could lead to incorrect test results", \
    .resolution                     = "Check OpenCL return code to determine the source of the error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_106 Message_t{ \
    .id                             = "ITF_106", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to download HW design on device <device name> selected with BDF <device BDF> <OpenCL code>", \
    .msg_edit                       = "Failed to download HW design on device %s selected with BDF %s %s", \
    .detail                         = "The host application is not able to download the HW design on the card", \
    .resolution                     = "Check the targeted card with provided BDF is compatible with the supplied HW design (for example, check interface_uuid of HW design and XSA match) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_107 Message_t{ \
    .id                             = "ITF_107", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Installed HW design <name> is ignored: <reason>", \
    .msg_edit                       = "Installed HW design %s is ignored: %s", \
    .detail                         = "The host application detected an invalid installed HW design. It is ignored", \
    .resolution                     = "Re-install HW design so missing or corrupted files will be corrected", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_108 Message_t{ \
    .id                             = "ITF_108", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No valid installed HW design found", \
    .msg_edit                       = "", \
    .detail                         = "The host application has not detected any valid installed HW designs", \
    .resolution                     = "Check HW design installation", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_109 Message_t{ \
    .id                             = "ITF_109", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid value (<parameter value>) provided in card configuration JSON file for parameter <parameter name>. <info>", \
    .msg_edit                       = "Invalid value (%s) provided in card configuration JSON file for parameter %s. %s", \
    .detail                         = "An invalid value has been provided for the parameter via the card configuration JSON file", \
    .resolution                     = "Check the parameter value provided is in the supported range", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_110 Message_t{ \
    .id                             = "ITF_110", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid definition of parameter <parameter name>. Cannot combined for the same test the parameters <param> and <param>", \
    .msg_edit                       = "Invalid definition of parameter %s. Cannot combined for the same test the parameters %s and %s", \
    .detail                         = "Incorrect definition of the test was provided via the card configuration JSON file", \
    .resolution                     = "Check the card configuration JSON file provided is correctly defined", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_111 Message_t{ \
    .id                             = "ITF_111", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to parsed card configuration JSON file", \
    .msg_edit                       = "", \
    .detail                         = "The content of card configuration JSON file is not correct", \
    .resolution                     = "Check the preceding error messages", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_112 Message_t{ \
    .id                             = "ITF_112", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No configuration provided in card configuration JSON file", \
    .msg_edit                       = "", \
    .detail                         = "At least on configuration must be defined in card configuration JSON file", \
    .resolution                     = "Update your file with correct configuration(s) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_113 Message_t{ \
    .id                             = "ITF_113", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to execute test <test_idx> of <num_test> (<error>) with command: <sys_cmd>", \
    .msg_edit                       = "Failed to execute test %s of %s (%s) with command: %s", \
    .detail                         = "The host application failed to execute a test on a card", \
    .resolution                     = "Try and run the command in another terminal", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_114 Message_t{ \
    .id                             = "ITF_114", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Total size (<size>) of the list <name> specified in card configuration JSON file is greater than maximum authorized (<maximum>) ", \
    .msg_edit                       = "Total size (%s) of the list %s specified in card configuration JSON file is greater than maximum authorized (%s) ", \
    .detail                         = "Incorrect definition of the card configuration JSON file", \
    .resolution                     = "Update your file with correct configuration(s) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_115 Message_t{ \
    .id                             = "ITF_115", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No element found in the list <name> specified in card configuration JSON file", \
    .msg_edit                       = "No element found in the list %s specified in card configuration JSON file", \
    .detail                         = "Incorrect definition of the card configuration JSON file", \
    .resolution                     = "Update your file with correct configuration(s) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_116 Message_t{ .id = "ITF_116", .state = "obsolete" }
#define MSG_ITF_117 Message_t{ .id = "ITF_117", .state = "obsolete" }
#define MSG_ITF_118 Message_t{ \
    .id                             = "ITF_118", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid defintion of threshold <threshold name> for sensor <name>: minimum threshold (<min>) is greater than maximum threshold (<max>) ", \
    .msg_edit                       = "Invalid defintion of threshold %s for sensor %s: minimum threshold (%s) is greater than maximum threshold (%s) ", \
    .detail                         = "Incorrect definition of a sensor was provided in test JSON file", \
    .resolution                     = "Update your file with correct configuration(s) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_119 Message_t{ \
    .id                             = "ITF_119", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Total size (<size>) of the list <name> specified in test JSON file is greater than maximum authorized (<maximum>) ", \
    .msg_edit                       = "Total size (%s) of the list %s specified in test JSON file is greater than maximum authorized (%s) ", \
    .detail                         = "Incorrect definition of the test JSON file", \
    .resolution                     = "Update your file with correct configuration(s) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_120 Message_t{ \
    .id                             = "ITF_120", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "PLRAM memory size (<memory size> Bytes) at memory index <idx> is too small to create sub-buffer for all xbtest HW IPs. Required PLRAM size is <required size> Bytes", \
    .msg_edit                       = "PLRAM memory size (%s Bytes) at memory index %s is too small to create sub-buffer for all xbtest HW IPs. Required PLRAM size is %s Bytes", \
    .detail                         = "Unable to create sub-buffers for all xbtest HW IPs as the PLRAM is too small", \
    .resolution                     = "Increase the size of this PLRAM in the HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_121 Message_t{ \
    .id                             = "ITF_121", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Although the <memory name> memory allocated is <allocated> MB, xbtest requires some space for internal operation, so the memory testcase will only test <actual> MB", \
    .msg_edit                       = "Although the %s memory allocated is %s MB, xbtest requires some space for internal operation, so the memory testcase will only test %s MB", \
    .detail                         = "The Memory xbtest HW IP will generate traffic targeting a memory area smaller than the memory size", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ITF_122 Message_t{ \
    .id                             = "ITF_122", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "xbtest requires some space in <name> memory for internal operation but the size allocated is too small for this memory to be used in the memory testcase as actual size is <actual> MB", \
    .msg_edit                       = "xbtest requires some space in %s memory for internal operation but the size allocated is too small for this memory to be used in the memory testcase as the actual size is %s MB", \
    .detail                         = "The memory size was not correctly allocated", \
    .resolution                     = "Increase the size of this memory", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_123 Message_t{ \
    .id                             = "ITF_123", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "xbtest DMA testcase is not supported for the targeted card", \
    .msg_edit                       = "", \
    .detail                         = "xbtest DMA testcase is not supported for the targeted card" \
}
#define MSG_ITF_124 Message_t{ \
    .id                             = "ITF_124", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Definition of <source type> source <source id> overwritten with value provided in test JSON file", \
    .msg_edit                       = "Definition of %s source %s overwritten with value provided in test JSON file", \
    .detail                         = "Some sensor sources, depending on targeted card, are monitored by default and their definition can be overwritten in test JSON file" \
}
#define MSG_ITF_125 Message_t{ \
    .id                             = "ITF_125", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to extract optional parameter found in <source JSON>: <parameter name>", \
    .msg_edit                       = "Failed to extract optional parameter found in %s: %s", \
    .detail                         = "An optional JSON parameter is found but unable to extract its value", \
    .resolution                     = "Check JSON content, for example check the parameter type", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_126 Message_t{ \
    .id                             = "ITF_126", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Cannot provide test JSON <write/read> parameter <param name> for memory testcase when mode is <mode>", \
    .msg_edit                       = "Cannot provide test JSON %s parameter %s for memory testcase when mode is %s", \
    .detail                         = "Some parameters cannot be provided depending on the mode.", \
    .resolution                     = "Correct the test JSON", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_127 Message_t{ \
    .id                             = "ITF_127", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "All test JSON parameters <param name> must be provided when one of these parameters is provided", \
    .msg_edit                       = "All test JSON parameters %s must be provided when one of these parameters is provided", \
    .detail                         = "Some parameters cannot be provided without other parameters", \
    .resolution                     = "Correct the test JSON", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_128 Message_t{ \
    .id                             = "ITF_128", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Array type is used for the definition of the test iteration parameters of <path to parameter>. This type has been deprecated, please use the object type definition", \
    .msg_edit                       = "Array type is used for the definition of the test iteration parameters of %s. This type has been deprecated, please use the object type definition", \
    .detail                         = "This parameter should be defined as an array (JSON_NODE_ARRAY) of objects (JSON_NODE_OBJECT). Array of arrays has been deprecated.", \
    .resolution                     = "Check the test sequence in the test JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_129 Message_t{ \
    .id                             = "ITF_129", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name>: Sensor name (<name>) provided more than once", \
    .msg_edit                       = "%s: Sensor name (%s) provided more than once", \
    .detail                         = "The same sensor name cannot be provided multiple times", \
    .resolution                     = "Check the sensor definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_130 Message_t{ \
    .id                             = "ITF_130", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Exception caught while executing xrt::device::get_info<<info>> for card at <bdf>: <exception>", \
    .msg_edit                       = "Exception caught while executing xrt::device::get_info<%s> for card at %s: %s", \
    .detail                         = "The host application was not able to get necessary device information throught the XRT API", \
    .resolution                     = "Check connectivity to the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_131 Message_t{ \
    .id                             = "ITF_131", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Empty JSON string device info returned from card driver API for card at <bdf>", \
    .msg_edit                       = "Empty JSON string device info returned from card driver API for card at %s", \
    .detail                         = "The host application has called card driver API but its JSON string return value was empty", \
    .resolution                     = "Check card driver API", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_132 Message_t{ \
    .id                             = "ITF_132", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Adding new <source type> source <source id> using definition provided in test JSON file", \
    .msg_edit                       = "Adding new %s source %s using definition provided in test JSON file", \
    .detail                         = "New sensor sources can be defined in test JSON file" \
}
#define MSG_ITF_133 Message_t{ \
    .id                             = "ITF_133", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Provided <source type> source <source id> definition in <JSON> not found in device info", \
    .msg_edit                       = "Provided %s source %s definition in %s not found in device info", \
    .detail                         = "Sensor sources provided must exist in device info", \
    .resolution                     = "Try xbtest -d <bdf> -g device_mgmt to get supported sensor sources for the targeted card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_134 Message_t{ \
    .id                             = "ITF_134", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Exception caught while creating xrt::device for card at <bdf>: <exception>", \
    .msg_edit                       = "Exception caught while creating xrt::device for card at %s: %s", \
    .detail                         = "The host application was not able to create the xrt::device of XRT API", \
    .resolution                     = "Check connectivity to the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_135 Message_t{ \
    .id                             = "ITF_135", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Thresholds definition provided in test JSON file for <source type> source <source id> will be ignored as source is not present on selected card", \
    .msg_edit                       = "Thresholds definition provided in test JSON file for %s source %s will be ignored as source is not present on selected card", \
    .detail                         = "Sensor source provided exists in device info but is flagged as not present for selected card", \
    .resolution                     = "Remove thresholds definition for this sensor", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_136 Message_t{ \
    .id                             = "ITF_136", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Unable to get device info <info> for card at <bdf> as device is offline, the card might be in process of being reset", \
    .msg_edit                       = "Unable to get device info %s for card at %s as device is offline, the card might be in process of being reset", \
    .detail                         = "The host application cannot get the device info when the card is offline", \
    .resolution                     = "Check if an Alveo card reset have occured", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_137 Message_t{ \
    .id                             = "ITF_137", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using HW design generated for internal release", \
    .msg_edit                       = "", \
    .detail                         = "The host application reports the HW design provided is released internally", \
    .resolution                     = "", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_ITF_138 Message_t{ \
    .id                             = "ITF_138", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "<info> (<version>) configuration running on the card does not match expected (<expected_version>) according to device information", \
    .msg_edit                       = "%s (%s) configuration running on the card does not match expected (%s) according to device information", \
    .detail                         = "The host application detected unexpected information in device info API", \
    .resolution                     = "Check if the version of configuration running on the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_139 Message_t{ .id = "ITF_139", .state = "obsolete" }
#define MSG_ITF_140 Message_t{ \
    .id                             = "ITF_140", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Memory index (<Memory index>) of interface connection for argument (<argument index>) not found in MEM_TOPOLOGY metadata for xbtest HW IP <IP name>", \
    .msg_edit                       = "Memory index (%s) of interface connection for argument (%s) not found in MEM_TOPOLOGY metadata for xbtest HW IP <%s>", \
    .detail                         = "Host application cannot find in MEM_TOPOLOGY the memory associated to a connection of a xbtest HW IP, required to set the xbtest HW IP argument", \
    .resolution                     = "Check the content xbtest metadata", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_141 Message_t{ \
    .id                             = "ITF_141", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No xbtest HW IP connection for argument (<argument index>) found in CONNECTIVITY metadata for xbtest HW IP (<IP name>) with IP index <IP index>", \
    .msg_edit                       = "No xbtest HW IP connection for argument (%s) found in CONNECTIVITY metadata for xbtest HW IP (%s) with IP index <IP index>", \
    .detail                         = "Host application cannot find a connection of a xbtest HW IP, required to set the xbtest HW IP argument", \
    .resolution                     = "Check the content of xbtest metadata", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_142 Message_t{ \
    .id                             = "ITF_142", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Memory index (<Memory index>) of interface connection not found in card driver API for xbtest HW IP <IP name>", \
    .msg_edit                       = "Memory index (%s) of interface connection not found in card driver API for xbtest HW IP %s", \
    .detail                         = "Host application cannot find in card driver API the memory associated to a connection of a xbtest HW IP, required to set the xbtest HW IP argument", \
    .resolution                     = "Check the content of xbtest metadata", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_143 Message_t{ \
    .id                             = "ITF_143", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Memory index (<Memory index>) of interface connection not found in MEM_TOPOLOGY metadata for xbtest HW IP <IP name>", \
    .msg_edit                       = "Memory index (%s) of interface connection not found in MEM_TOPOLOGY metadata for xbtest HW IP %s", \
    .detail                         = "Host application cannot find in MEM_TOPOLOGY the memory associated to a connection of a xbtest HW IP, required to set the xbtest HW IP argument", \
    .resolution                     = "Check the content of xbtest metadata", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_144 Message_t{ \
    .id                             = "ITF_144", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to configure m00_axi interface for xbtest HW IP (<IP name>). Unable to find a buffer created with memory index <Memory index>", \
    .msg_edit                       = "Failed to configure m00_axi interface for xbtest HW IP (%s). Unable to find a buffer created with memory index %s", \
    .detail                         = "Host application cannot find in MEM_TOPOLOGY the memory associated to a connection of a xbtest HW IP, required to set the xbtest HW IP argument", \
    .resolution                     = "Check the content of xbtest metadata", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_145 Message_t{ \
    .id                             = "ITF_145", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Too many (<severity>) <ID> reported, stop reporting them", \
    .msg_edit                       = "Too many (%s) %s reported, stop reporting them", \
    .detail                         = "Host application stopped reporting a message", \
    .resolution                     = "Check the reason why the message specified was reported too many times", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_146 Message_t{ \
    .id                             = "ITF_146", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "The following output directory already exists on disk and will be overwritten: <directory>", \
    .msg_edit                       = "The following output directory already exists on disk and will be overwritten: %s", \
    .detail                         = "The output directory is already existing. Its previous content is not deleted but some of the files contained in the directory will be overwritten during the test", \
    .resolution                     = "Are you sure that you want this directory to be overwritten? If yes, you can ignore this warning. If not, make sure you rename your output directory (see -l command line option) as now this directory has been re-used (see -f command line option) ", \
    .display_classic_console        = false \
}
#define MSG_ITF_147 Message_t{ \
    .id                             = "ITF_147", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The following output directory already exists on disk, use the force command line option (-f) to overwrite: <directory>", \
    .msg_edit                       = "The following output directory already exists on disk, use the force command line option (-f) to overwrite: %s", \
    .detail                         = "The output directory is already existing. By default the host application cannot re-use directory already existing", \
    .resolution                     = "Make sure you rename your output directory (see -l command line option) or if are you sure that you want this directory to be re-used, then see -f command line option", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_148 Message_t{ \
    .id                             = "ITF_148", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The following output directory cannot be created, path already exists and is not a directory: <directory>", \
    .msg_edit                       = "The following output directory cannot be created, path already exists and is not a directory: %s", \
    .detail                         = "A file is already existing and the host application cannot use it as output directory", \
    .resolution                     = "Make sure you rename your output directory (see -l command line option) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_149 Message_t{ \
    .id                             = "ITF_149", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Verify xbtest HW IP was not found in the HW design", \
    .msg_edit                       = "", \
    .detail                         = "The HW design must contain a Verify xbtest HW IP", \
    .resolution                     = "Check HW design generation and content for the Verify xbtest HW IP", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_150 Message_t{ \
    .id                             = "ITF_150", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<testcase> testcase specified in test JSON file but: <detail>", \
    .msg_edit                       = "%s testcase specified in test JSON file but: %s", \
    .detail                         = "The testcase specified cannot be run on target card", \
    .resolution                     = "Check the card being used, or remove the testcase not supported from test JSON", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_151 Message_t{ \
    .id                             = "ITF_151", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Clock frequency not found for clock <name> in CLOCK_FREQ_TOPOLOGY. Clock frequency from USER_METADATA will be used", \
    .msg_edit                       = "Clock frequency not found for clock %s in CLOCK_FREQ_TOPOLOGY. Clock frequency from USER_METADATA will be used", \
    .detail                         = "The actual value of clock frequency used in HW design was not found, using the value set up at build time in xbtest configuration instead. Results may be affected", \
    .resolution                     = "Check that the actual clock frequency and the user metadata frequency match", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_152 Message_t{ \
    .id                             = "ITF_152", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Stop accessing xbtest HW IP due to previous card driver API error", \
    .msg_edit                       = "", \
    .detail                         = "Card driver API failed.", \
    .resolution                     = "Check the driver error message content, and errors reported before, e.g. errors can be reported by driver in STDERR which are not visible in xbtest.log file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_153 Message_t{ \
    .id                             = "ITF_153", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Exception caught while loading HW design with xrt::device for card at: <exception>", \
    .msg_edit                       = "Exception caught while loading HW design with xrt::device for card at: %s", \
    .detail                         = "The host application was not able to load HW design with xrt::device of XRT API", \
    .resolution                     = "Check connectivity to the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_154 Message_t{ \
    .id                             = "ITF_154", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Exception caught while creating xrt::kernel <name>: <exception>", \
    .msg_edit                       = "Exception caught while creating xrt::kernel %s: %s", \
    .detail                         = "The host application was not able to create xrt::kernel", \
    .resolution                     = "Check connectivity to the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_155 Message_t{ \
    .id                             = "ITF_155", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Exception caught while starting xrt::kernel <name>: <exception>", \
    .msg_edit                       = "Exception caught while starting xrt::kernel %s: %s", \
    .detail                         = "The host application was not able to start xrt::kernel of XRT API", \
    .resolution                     = "Check connectivity to the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_156 Message_t{ \
    .id                             = "ITF_156", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "xrt::kernel <name> execution failed with state: <state> return code: <ret>", \
    .msg_edit                       = "xrt::kernel %s execution failed with sate: %s and return code: %s", \
    .detail                         = "The PS kernel returned an unexpected error", \
    .resolution                     = "Check connectivity to the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_157 Message_t{ \
    .id                             = "ITF_157", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Exception caught while stopping/waiting xrt::kernel <name> to complete: <exception>", \
    .msg_edit                       = "Exception caught while stopping/waiting xrt::kernel %s to complete: %s", \
    .detail                         = "The host application was not able to stop/wait xrt::kernel to complete", \
    .resolution                     = "Check connectivity to the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_158 Message_t{ \
    .id                             = "ITF_158", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Root privileges are required to run xbtest in no driver mode", \
    .msg_edit                       = "", \
    .detail                         = "The host application requires root privileges to communicate with the card", \
    .resolution                     = "Run xbtest with root privileges", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_159 Message_t{ \
    .id                             = "ITF_159", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to access <resource> <file>: <error>", \
    .msg_edit                       = "Failed to access %s %s: %s", \
    .detail                         = "The host application uses this resources to communicate with the card", \
    .resolution                     = "Check this file is accessible with root privileges", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_160 Message_t{ \
    .id                             = "ITF_160", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to create mapping to system's physical memory at offset 0x<offset>: <error>", \
    .msg_edit                       = "Failed to create mapping to system's physical memory at offset 0x%s: %s", \
    .detail                         = "The host application uses this resources to communicate with the card", \
    .resolution                     = "Check communication with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_161 Message_t{ \
    .id                             = "ITF_161", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to <write/read> <qty> data bytes <from/to> the card at start address 0x<address> (PCIe BAR <idx>) for <target> using <details>", \
    .msg_edit                       = "Failed to %s %s data bytes %s the card at start address 0x%s (PCIe BAR %s) for %s using %s", \
    .detail                         = "The host application was unable to communicate with the card via PCIe BAR peek-poke", \
    .resolution                     = "Check communication with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_162 Message_t{ \
    .id                             = "ITF_162", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to set xbtest HW IP <IP name> <arg type> argument <arg idx>", \
    .msg_edit                       = "Failed to set xbtest HW IP %s %s argument %s", \
    .detail                         = "The arguments are used to pass commands to the xbtest HW IP", \
    .resolution                     = "Check communications with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_163 Message_t{ \
    .id                             = "ITF_163", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to execute xbtest HW IP <IP name>", \
    .msg_edit                       = "Failed to execute xbtest HW IP %s", \
    .detail                         = "The xbtest HW IPs are executed to perfom a command", \
    .resolution                     = "Check communications with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_164 Message_t{ \
    .id                             = "ITF_164", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "AMI failed to <write/read> data <to/from> PLRAM for xbtest HW IP <IP name>. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to %s data %s PLRAM for xbtest HW IP %s. AMI error message: %s", \
    .detail                         = "The host application was unable to access PLRAM via AMI API", \
    .resolution                     = "Check communications with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_165 Message_t{ \
    .id                             = "ITF_165", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to read <resource> <file>. Content is not as expected", \
    .msg_edit                       = "Failed to read %s %s. Content is not as expected", \
    .detail                         = "The host application uses this resources to communicate with the card", \
    .resolution                     = "Check the content of this file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_166 Message_t{ \
    .id                             = "ITF_166", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to get xbtest HW IP <IP name> idle state", \
    .msg_edit                       = "Failed to get xbtest HW IP %s idle state", \
    .detail                         = "The host application was unable to transfer data to/from card", \
    .resolution                     = "Check communications with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_167 Message_t{ \
    .id                             = "ITF_167", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "xbtest HW IP <IP name> is not in idle state. xbtest HW IP was deadlocked? Please reset your card", \
    .msg_edit                       = "xbtest HW IP %s is not in idle state. xbtest HW IP was deadlocked? Please reset your card", \
    .detail                         = "The host application cannot use xbtest HW IP that has not returned in idle state", \
    .resolution                     = "Check your hardware", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_168 Message_t{ .id = "ITF_168", .state = "obsolete" }
#define MSG_ITF_169 Message_t{ \
    .id                             = "ITF_169", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to delete mapping to system's physical memory at offset 0x<offset>: <error>", \
    .msg_edit                       = "Failed to delete mapping to system's physical memory at offset 0x%s: %s", \
    .detail                         = "The host application uses this resources to communicate with the card", \
    .resolution                     = "Check communication with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_170 Message_t{ \
    .id                             = "ITF_170", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to close descriptor to system's physical memory: <error>", \
    .msg_edit                       = "Failed to close descriptor to system's physical memory: %s", \
    .detail                         = "The host application uses this resources to communicate with the card", \
    .resolution                     = "Check this file is accessible with root privileges", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_171 Message_t{ \
    .id                             = "ITF_171", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to find base address of design IP instance: <instance>", \
    .msg_edit                       = "Failed to find base address of design IP instance: %s", \
    .detail                         = "The host application uses this resources to communicate with the card", \
    .resolution                     = "Check this IP is designed in design metadata", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_172 Message_t{ \
    .id                             = "ITF_172", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "AMI failed to return <sensor_type> for sensor <sensor_name>. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to return %s for sensor %s. AMI error message: %s", \
    .detail                         = "The sensor doesn't report the requested information", \
    .resolution                     = "Check with AMI tool if the sensor is actually present", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_173 Message_t{ \
    .id                             = "ITF_173", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "AMI failed to return <info> for sensor <sensor_name>. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to return %s for sensor %s. AMI error message: %s", \
    .detail                         = "AMI didn't return the information requested for the listed sensor", \
    .resolution                     = "Check with AMI tool if the sensor is actually present", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_174 Message_t{ \
    .id                             = "ITF_174", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "AMI returned invalid sensor type for <sensor_name>", \
    .msg_edit                       = "AMI returned invalid sensor type for %s", \
    .detail                         = "Sensor is invalid", \
    .resolution                     = "Check with AMI tool if the sensor is actually present", \
    .display_classic_console        = false, \
    .display_dynamic_console        = false, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_175 Message_t{ \
    .id                             = "ITF_175", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Bad <type> sensor status (<value>) for <sensor_name>", \
    .msg_edit                       = "Bad %s sensor status (%s) for %s", \
    .detail                         = "Sensor status is invalid", \
    .resolution                     = "Check if the sensor is actually present", \
    .display_classic_console        = false, \
    .display_dynamic_console        = false, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_176 Message_t{ \
    .id                             = "ITF_176", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "AMI sensor type not fully decoded for <sensor_name>. Remaining types are: <value not decoded>", \
    .msg_edit                       = "AMI sensor type not fully decoded for %s. Remaining types are: %s", \
    .detail                         = "AMI sensor contains other fields but they are ignored by host application", \
    .resolution                     = "Check sensor definition. Supported AMI sensor types are: voltage, current, power and temperature", \
    .display_classic_console        = false, \
    .display_dynamic_console        = false, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_177 Message_t{ \
    .id                             = "ITF_177", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "No sensor reported by AMI", \
    .msg_edit                       = "", \
    .detail                         = "No Sensor reported by AMI", \
    .resolution                     = "Check with AMI tool if sensors are actually present", \
    .display_classic_console        = false, \
    .display_dynamic_console        = false, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_178 Message_t{ \
    .id                             = "ITF_178", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "AMI failed to create AMI device. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to create AMI device. AMI error message: %s", \
    .detail                         = "AMI is not able to create object representing AMI device", \
    .resolution                     = "Check with AMI tool if card is actually present and detected", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_179 Message_t{ \
    .id                             = "ITF_179", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "AMI failed to list the sensors. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to list the sensors. AMI error message: %s", \
    .detail                         = "AMI cannot list the sensors available for the card", \
    .resolution                     = "Check with AMI tool if card and sensors are actually present and detected", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_180 Message_t{ \
    .id                             = "ITF_180", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "AMI failed to return <info>. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to return %s. AMI error message: %s", \
    .detail                         = "The host application is not able to query an information to AMI", \
    .resolution                     = "Refer to AMI error message to resolve the error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_181 Message_t{ \
    .id                             = "ITF_181", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to download PDI on card <card name> selected with BDF <card BDF>. AMI error message: <AMI message>", \
    .msg_edit                       = "Failed to download PDI on card %s selected with BDF %s. AMI error message: %s", \
    .detail                         = "The host application is not able to download the PDI on the card", \
    .resolution                     = "Check the targeted card with provided BDF is compatible with the supplied PDI", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_182 Message_t{ \
    .id                             = "ITF_182", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to get AMI build version. AMI error message: <AMI message>", \
    .msg_edit                       = "Failed to get AMI build version. AMI error message: %s", \
    .detail                         = "The host application is not able to query build version to AMI", \
    .resolution                     = "Refer to AMI error message to resolve the error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_183 Message_t{ \
    .id                             = "ITF_183", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "AMI failed to enable access to AMI device. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to enable access to AMI device. AMI error message: %s", \
    .detail                         = "AMI is not able to provide access to AMI device", \
    .resolution                     = "Check with AMI tool if card is actually present and detected", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_184 Message_t{ \
    .id                             = "ITF_184", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "AMI failed to <get/set> sensor refresh rate. AMI error message: <AMI message>", \
    .msg_edit                       = "AMI failed to %s sensor refresh rate. AMI error message: %s", \
    .detail                         = "AMI is not able to provide access to sensor refresh rate", \
    .resolution                     = "Check with AMI tool if card is actually present and detected", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_185 Message_t{ \
    .id                             = "ITF_185", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to access driver. Check driver is installed/loaded", \
    .detail                         = "The host application requires access to driver. Refer to installation manual of your card", \
    .resolution                     = "Install/load driver before running host application", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ITF_186 Message_t{ \
    .id                             = "ITF_186", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to remove: <path>. Error: <err>", \
    .msg_edit                       = "Failed to remove: %s. Error: %s", \
    .detail                         = "The host application was not able to remove already existing file or directory", \
    .resolution                     = "Try to remove the directory manually and re-run host application", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}

#define MSG_JPR_001 Message_t{ \
    .id                             = "JPR_001", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: JSONReader error: <JSONReader error>", \
    .msg_edit                       = "%s parser: JSONReader error: %s", \
    .detail                         = "json-glib JSONReader error", \
    .resolution                     = "Check the json-glib JSONReader error message content", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_002 Message_t{ .id = "JPR_002", .state = "obsolete" }
#define MSG_JPR_003 Message_t{ \
    .id                             = "JPR_003", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: unable to find node <node title>", \
    .msg_edit                       = "%s parser: unable to find node %s", \
    .detail                         = "The host application was not able to find the specified node in the JSON content", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_004 Message_t{ \
    .id                             = "JPR_004", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: value in node <node title> is not of expected type <expected type>", \
    .msg_edit                       = "%s parser: value in node %s is not of expected type %s", \
    .detail                         = "The host application was able to find the specified node in the JSON content, but the JSON node type is not as expected", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_005 Message_t{ \
    .id                             = "JPR_005", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: unable to find value in node <node title>", \
    .msg_edit                       = "%s parser: unable to find value in node %s", \
    .detail                         = "The host application was able to find the specified node in the JSON content, but the JSON node does not contain a JSON value (JSON_NODE_VALUE) as expected", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_006 Message_t{ \
    .id                             = "JPR_006", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: unable to find <node type> value in node <node title>", \
    .msg_edit                       = "%s parser: unable to find %s value in node %s", \
    .detail                         = "The host application was able to find the specified node in the JSON content, but the JSON value type (G_TYPE_BOOLEAN, G_TYPE_INT64, G_TYPE_DOUBLE, G_TYPE_STRING) in the JSON node is not as expected", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_007 Message_t{ \
    .id                             = "JPR_007", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: failed to convert value (<value>) in node <node title> to <node type>", \
    .msg_edit                       = "%s parser: failed to convert value (%s) in node %s to %s", \
    .detail                         = "The host application was able to find the specified node in the JSON content containing expected JSON value type (G_TYPE_INT64, G_TYPE_DOUBLE) but its conversion failed. For example, if negative integer is used but an unsigned integer is expected", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_008 Message_t{ \
    .id                             = "JPR_008", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: unable to find array in node <node title>", \
    .msg_edit                       = "%s parser: unable to find array in node %s", \
    .detail                         = "The host application was able to find the specified node in the JSON content, but the JSON node does not contain a JSON array (JSON_NODE_ARRAY) as expected", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_009 Message_t{ \
    .id                             = "JPR_009", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: unable to list members for node <node title>", \
    .msg_edit                       = "%s parser: unable to list members for node %s", \
    .detail                         = "The host application was not able to list the JSON node names contained in the JSON object (JSON_NODE_OBJECT) ", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_010 Message_t{ .id = "JPR_010", .state = "obsolete" }
#define MSG_JPR_011 Message_t{ \
    .id                             = "JPR_011", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: invalid node name <node title>", \
    .msg_edit                       = "%s parser: invalid node name %s", \
    .detail                         = "An unknow node name was found in the JSON content being parsed", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_012 Message_t{ \
    .id                             = "JPR_012", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: invalid node type (<node type found>) provided for node <node title>. Expected: <expected node type>.", \
    .msg_edit                       = "%s parser: invalid node type (%s) provided for node %s. Expected: %s", \
    .detail                         = "The host application was able to find the specified node in the JSON content but the JSON node type is not as expected: value (JSON_NODE_VALUE), array (JSON_NODE_ARRAY), or object (JSON_NODE_OBJECT) ", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_013 Message_t{ \
    .id                             = "JPR_013", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<content name> parser: unable to parse JSON content, invalid syntax (error format is <format>): <json-glib parser error>", \
    .msg_edit                       = "%s parser: unable to parse JSON content, invalid syntax (error format is %s):  %s", \
    .detail                         = "The host application is not able to parse the JSON content as it contains invalid JSON syntax", \
    .resolution                     = "For test JSON file, refer to UG for complete definition of supported content and/or try '-g' command line option (e.g. to identify the supported testcases or memory names for the targeted card) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_JPR_014 Message_t{ \
    .id                             = "JPR_014", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "<content name> parser: node <node title> not available", \
    .msg_edit                       = "%s parser: node %s not available", \
    .detail                         = "The specified node could not be found within the JSON file. This message is only displayed once per node type (power/temperature). Multiple missing node could be present", \
    .resolution                     = "Check that the node is defined correctly or if it's actually available in the JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}

#define MSG_MIO_001 Message_t{ \
    .id                             = "MIO_001", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using memory page size <page size> bytes", \
    .msg_edit                       = "Using memory page size %s bytes", \
    .detail                         = "The memory page size is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MIO_002 Message_t{ \
    .id                             = "MIO_002", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameter <total size parameter name> value (<total size parameter value> MB) is not a multiple of parameter <buffer size parameter name> value (<buffer size parameter value> MB) for memory type <type>", \
    .msg_edit                       = "%s value (%s MB) is not a multiple of %s value (%s MB) for memory type %s", \
    .detail                         = "The total size of the data transferred, set by DMA testcase parameters in test JSON file, must be a multiple of the buffer size, set in DMA test sequence", \
    .resolution                     = "Check total size and buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_003 Message_t{ \
    .id                             = "MIO_003", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to <write/read> data <from/to> device", \
    .msg_edit                       = "Failed to %s data %s device", \
    .detail                         = "The host application was unable to communicate with the card via PCIe BAR peek-poke", \
    .resolution                     = "Check communication with the card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_004 Message_t{ \
    .id                             = "MIO_004", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<parameter> (<size>) is not a multiple of <parameter> (<size>)", \
    .msg_edit                       = "%s (%s) is not a multiple of %s (%s)", \
    .detail                         = "The number of buffers cannot be computed", \
    .resolution                     = "Check total size and buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_005 Message_t{ \
    .id                             = "MIO_005", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "PCIe BAR selected not found in design metadata. Valid BARs are: <BAR indexes>.", \
    .msg_edit                       = "PCIe BAR selected not found in design metadata. Valid BARs are: %s.", \
    .detail                         = "SUpported BAR are defined in metadata", \
    .resolution                     = "Check PCIe BAR exists in design and is present in metadata", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_006 Message_t{ \
    .id                             = "MIO_006", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to allocate reference data host buffer", \
    .msg_edit                       = "", \
    .detail                         = "The host application was not able to allocate the memory necessary for the reference data (size equals buffer size) ", \
    .resolution                     = "Check memory usage", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_007 Message_t{ \
    .id                             = "MIO_007", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to allocate host buffer index <buffer index>", \
    .msg_edit                       = "Failed to allocate host buffer index %s", \
    .detail                         = "The host application was not able to allocate the memory necessary for one of the host buffers (size equals buffer size). Note that buffers with lower index have been allocated successfully", \
    .resolution                     = "Check memory usage", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_008 Message_t{ \
    .id                             = "MIO_008", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Failed to compute <read or write> BW", \
    .msg_edit                       = "Failed to compute %s BW. The DMA transfer duration was equal to 0", \
    .detail                         = "The host application was not able to compute the BW", \
    .resolution                     = "Check board status using xbutil examine", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_009 Message_t{ \
    .id                             = "MIO_009", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Data integrity test failed for buffer <buffer index>", \
    .msg_edit                       = "Data integrity test failed for buffer: %s", \
    .detail                         = "Data integrity was not maintained during the test. The data read by host application does not match the reference data", \
    .resolution                     = "Check board status using xbutil examine", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_010 Message_t{ .id = "MIO_010", .state = "obsolete" }
#define MSG_MIO_011 Message_t{ \
    .id                             = "MIO_011", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Data integrity and bandwidth test pass", \
    .msg_edit                       = "Data integrity and bandwidth test pass", \
    .detail                         = "The read data matches the reference data AND the BW is within the range defined in card definition JSON file" \
}
#define MSG_MIO_012 Message_t{ \
    .id                             = "MIO_012", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Data integrity and/or bandwidth test fail", \
    .msg_edit                       = "Data integrity and/or bandwidth test fail", \
    .detail                         = "The read data does not match the reference data OR the BW is not within the range defined in card definition JSON file.", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_013 Message_t{ \
    .id                             = "MIO_013", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Data integrity test pass", \
    .msg_edit                       = "Data integrity test pass", \
    .detail                         = "The read data matches the reference data. Bandwidth is not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MIO_014 Message_t{ \
    .id                             = "MIO_014", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Data integrity test fail", \
    .msg_edit                       = "Data integrity test fail", \
    .detail                         = "The read data does not match the reference data. Bandwidth is not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_015 Message_t{ \
    .id                             = "MIO_015", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Read data does not match write data", \
    .msg_edit                       = "", \
    .detail                         = "Data integrity errors have been detected", \
    .resolution                     = "Check the INFO message that follows to see which byte of the data was not transferred correctly", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_016 Message_t{ \
    .id                             = "MIO_016", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<line of BW test fail result>", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW is not within the range defined in card definition JSON file. The BW values are displayed in the INFO messages that follow", \
    .resolution                     = "Check board status using xbutil examine. Check memory page size. Check for other processes accessing the memory. Try to increase the test duration", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_017 Message_t{ \
    .id                             = "MIO_017", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<line of BW test pass result>", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW is within the range defined in card definition JSON file. The BW values are displayed in the INFO messages that follow" \
}
#define MSG_MIO_018 Message_t{ \
    .id                             = "MIO_018", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "BW values", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW values are reported by the host application" \
}
#define MSG_MIO_019 Message_t{ .id = "MIO_019", .state = "obsolete" }
#define MSG_MIO_020 Message_t{ .id = "MIO_020", .state = "obsolete" }
#define MSG_MIO_021 Message_t{ \
    .id                             = "MIO_021", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of data integrity error details>", \
    .msg_edit                       = "%s", \
    .detail                         = "When the data integrity test fails, the data index and values are reported" \
}
#define MSG_MIO_022 Message_t{ .id = "MIO_022", .state = "obsolete" }
#define MSG_MIO_023 Message_t{ .id = "MIO_023", .state = "obsolete" }
#define MSG_MIO_024 Message_t{ .id = "MIO_024", .state = "obsolete" }
#define MSG_MIO_025 Message_t{ .id = "MIO_025", .state = "obsolete" }
#define MSG_MIO_026 Message_t{ \
    .id                             = "MIO_026", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<count> write-read-check cycles performed during this test", \
    .msg_edit                       = "%s write-read-check cycles performed during this test", \
    .detail                         = "The quantity of write-read-check cycles (memory written and read back) during the test is reported", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MIO_027 Message_t{ \
    .id                             = "MIO_027", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Reloading reference data in buffer <buffer index> after data integrity check failed. This might take extra time and impact quantity of write-read-check cycles done in given test duration", \
    .msg_edit                       = "Reloading reference data in buffer %s after data integrity check failed. This might take extra time and impact quantity of write-read-check cycles done in given test duration", \
    .detail                         = "A host buffer need to be filled with reference data before being are written to the device during next write-read-check cycle. The time needed to complete this action depend on the buffer size", \
    .resolution                     = "If needed, increase test duration so more write-read-check cycles are performed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_028 Message_t{ \
    .id                             = "MIO_028", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Test <index> will be stopped on <error> error", \
    .msg_edit                       = "Test %s will be stopped on %s error", \
    .detail                         = "Tests are stopped on error as enabled in test JSON file", \
    .resolution                     = "Check the cause of the error. Update test JSON file if to disable stop on error if needed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_029 Message_t{ \
    .id                             = "MIO_029", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Bandwidth test pass", \
    .msg_edit                       = "Bandwidth test pass", \
    .detail                         = "Bandwidth is in expected range. Data integrity is not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MIO_030 Message_t{ \
    .id                             = "MIO_030", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Bandwidth test fail", \
    .msg_edit                       = "Bandwidth test fail", \
    .detail                         = "Bandwidth is not in expected range. Data integrity is not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MIO_031 Message_t{ \
    .id                             = "MIO_031", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Test pass", \
    .msg_edit                       = "Test pass", \
    .detail                         = "Bandwidth and data integrity are not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MIO_032 Message_t{ \
    .id                             = "MIO_032", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Test fail", \
    .msg_edit                       = "Test fail", \
    .detail                         = "Bandwidth and data integrity are not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}

#define MSG_DMA_001 Message_t{ \
    .id                             = "DMA_001", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using memory page size <page size> bytes", \
    .msg_edit                       = "Using memory page size %s bytes", \
    .detail                         = "The memory page size is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_DMA_002 Message_t{ \
    .id                             = "DMA_002", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameter <total size parameter name> value (<total size parameter value> MB) is not a multiple of parameter <buffer size parameter name> value (<buffer size parameter value> MB) for memory type <type>", \
    .msg_edit                       = "%s value (%s MB) is not a multiple of %s value (%s MB) for memory type %s", \
    .detail                         = "The total size of the data transferred, set by DMA testcase parameters in test JSON file, must be a multiple of the buffer size, set in DMA test sequence", \
    .resolution                     = "Check total size and buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_003 Message_t{ .id = "DMA_003", .state = "obsolete" }
#define MSG_DMA_004 Message_t{ \
    .id                             = "DMA_004", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to compute positive integer value for number of buffers <number of buffers>.", \
    .msg_edit                       = "Failed to compute positive integer value for number of buffers %s.", \
    .detail                         = "The number of buffers cannot be computed", \
    .resolution                     = "Check total size and buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_005 Message_t{ .id = "DMA_005", .state = "obsolete" }
#define MSG_DMA_006 Message_t{ \
    .id                             = "DMA_006", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to allocate reference data host buffer", \
    .msg_edit                       = "", \
    .detail                         = "The host application was not able to allocate the memory necessary for the reference data (size equals buffer size) ", \
    .resolution                     = "Check memory usage", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_007 Message_t{ \
    .id                             = "DMA_007", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to allocate host buffer index <buffer index>", \
    .msg_edit                       = "Failed to allocate host buffer index %s", \
    .detail                         = "The host application was not able to allocate the memory necessary for one of the host buffers (size equals buffer size). Note that buffers with lower index have been allocated successfully", \
    .resolution                     = "Check memory usage", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_008 Message_t{ \
    .id                             = "DMA_008", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Failed to compute <read or write> BW", \
    .msg_edit                       = "Failed to compute %s BW. The DMA transfer duration was equal to 0", \
    .detail                         = "The host application was not able to compute the BW", \
    .resolution                     = "Check board status using xbutil examine", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_009 Message_t{ \
    .id                             = "DMA_009", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Data integrity test failed for buffer <buffer index>", \
    .msg_edit                       = "%s - Data integrity test failed for buffer: %s", \
    .detail                         = "Data integrity was not maintained during the test. The data read by host application does not match the reference data", \
    .resolution                     = "Check board status using xbutil examine", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_010 Message_t{ .id = "DMA_010", .state = "obsolete" }
#define MSG_DMA_011 Message_t{ \
    .id                             = "DMA_011", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Data integrity and bandwidth test pass", \
    .msg_edit                       = "%s - Data integrity and bandwidth test pass", \
    .detail                         = "The read data matches the reference data AND the BW is within the range defined in card definition JSON file" \
}
#define MSG_DMA_012 Message_t{ \
    .id                             = "DMA_012", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Data integrity and/or bandwidth test fail", \
    .msg_edit                       = "%s - Data integrity and/or bandwidth test fail", \
    .detail                         = "The read data does not match the reference data OR the BW is not within the range defined in card definition JSON file.", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_013 Message_t{ \
    .id                             = "DMA_013", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Data integrity test pass", \
    .msg_edit                       = "%s - Data integrity test pass", \
    .detail                         = "The read data matches the reference data. Bandwidth is not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_DMA_014 Message_t{ \
    .id                             = "DMA_014", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Data integrity test fail", \
    .msg_edit                       = "%s - Data integrity test fail", \
    .detail                         = "The read data does not match the reference data. Bandwidth is not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_015 Message_t{ \
    .id                             = "DMA_015", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Read data does not match write data", \
    .msg_edit                       = "", \
    .detail                         = "Data integrity errors have been detected", \
    .resolution                     = "Check the INFO message that follows to see which byte of the data was not transferred correctly", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_016 Message_t{ \
    .id                             = "DMA_016", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<line of BW test fail result>", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW is not within the range defined in card definition JSON file. The BW values are displayed in the INFO messages that follow", \
    .resolution                     = "Check board status using xbutil examine. Check memory page size. Check for other processes accessing the memory. Try to increase the test duration", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_017 Message_t{ \
    .id                             = "DMA_017", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<line of BW test pass result>", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW is within the range defined in card definition JSON file. The BW values are displayed in the INFO messages that follow" \
}
#define MSG_DMA_018 Message_t{ \
    .id                             = "DMA_018", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "BW values", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW values are reported by the host application" \
}
#define MSG_DMA_019 Message_t{ \
    .id                             = "DMA_019", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Test <idx> is defined by memory type (<memory type>). Creating <quantity> tests, one per each memory tag of the memory type", \
    .msg_edit                       = "Test %s is defined by memory type (%s). Creating %s tests, one per each tag of the memory type", \
    .detail                         = "When the test is defined by memory type in the test JSON file, a test is created for each memory tag found for this type. The available tag for each memory type is defined in the card definition JSON file and displayed at xbtest start-up. Try: xbtest -c verify -d <device BDF>", \
    .resolution                     = "Define the memory tag you want to target if you want to target a specific memory tag, otherwise this warning can be safely ignored" \
}
#define MSG_DMA_020 Message_t{ .id = "DMA_020", .state = "obsolete" }
#define MSG_DMA_021 Message_t{ \
    .id                             = "DMA_021", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of data integrity error details>", \
    .msg_edit                       = "%s", \
    .detail                         = "When the data integrity test fails, the data index and values are reported" \
}
#define MSG_DMA_022 Message_t{ .id = "DMA_022", .state = "obsolete" }
#define MSG_DMA_023 Message_t{ \
    .id                             = "DMA_023", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameter <buffer size parameter name> value (<buffer size parameter value> MB) is <invalid> for memory type <type>", \
    .msg_edit                       = "Parameter %s value (%s MB) is %s for memory type %s", \
    .detail                         = "The value of the buffer size, quantity of data transferred in each buffer, is not set as expected in DMA test sequence", \
    .resolution                     = "Check buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_024 Message_t{ \
    .id                             = "DMA_024", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to find memory configuration for memory <type/tag> <type/tag name>", \
    .msg_edit                       = "Failed to find memory configuration for memory %s %s", \
    .detail                         = "No memory configuration corresponds to the provided tag or type", \
    .resolution                     = "Try command line option -g to check available memory tag or type", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_025 Message_t{ \
    .id                             = "DMA_025", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Due to usage of memory type, the test sequence has been extended", \
    .msg_edit                       = "", \
    .detail                         = "When the test sequence is using memory type, each tag of the selected type will be tested, resulting in a longer test sequence" \
}
#define MSG_DMA_026 Message_t{ \
    .id                             = "DMA_026", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<count> write-read-check cycles performed during this test", \
    .msg_edit                       = "%s write-read-check cycles performed during this test", \
    .detail                         = "The quantity of write-read-check cycles (memory written and read back) during the test is reported", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_DMA_027 Message_t{ \
    .id                             = "DMA_027", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "<memory tag> - Reloading reference data in buffer <buffer index> after data integrity check failed. This might take extra time and impact quantity of write-read-check cycles done in given test duration", \
    .msg_edit                       = "%s - Reloading reference data in buffer %s after data integrity check failed. This might take extra time and impact quantity of write-read-check cycles done in given test duration", \
    .detail                         = "A host buffer need to be filled with reference data before being are written to the device during next write-read-check cycle. The time needed to complete this action depend on the buffer size", \
    .resolution                     = "If needed, increase test duration so more write-read-check cycles are performed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_028 Message_t{ \
    .id                             = "DMA_028", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Test <index> will be stopped on <error> error", \
    .msg_edit                       = "Test %s will be stopped on %s error", \
    .detail                         = "Tests are stopped on error as enabled in test JSON file", \
    .resolution                     = "Check the cause of the error. Update test JSON file if to disable stop on error if needed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_029 Message_t{ \
    .id                             = "DMA_029", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Bandwidth test pass", \
    .msg_edit                       = "%s - Bandwidth test pass", \
    .detail                         = "Bandwidth is in expected range. Data integrity is not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_DMA_030 Message_t{ \
    .id                             = "DMA_030", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Bandwidth test fail", \
    .msg_edit                       = "%s - Bandwidth test fail", \
    .detail                         = "Bandwidth is not in expected range. Data integrity is not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_031 Message_t{ \
    .id                             = "DMA_031", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Test pass", \
    .msg_edit                       = "%s - Test pass", \
    .detail                         = "Bandwidth and data integrity are not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_DMA_032 Message_t{ \
    .id                             = "DMA_032", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Test fail", \
    .msg_edit                       = "%s - Test fail", \
    .detail                         = "Bandwidth and data integrity are not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DMA_033 Message_t{ \
    .id                             = "DMA_033", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using AMI, DMA disabled", \
    .msg_edit                       = "", \
    .detail                         = "AMI currently does not support use of DMA", \
    .resolution                     = "", \
    .display_classic_console        = true, \
}



#define MSG_P2P_001 Message_t{ \
    .id                             = "P2P_001", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using memory page size <page size> bytes", \
    .msg_edit                       = "Using memory page size %s bytes", \
    .detail                         = "The memory page size is displayed", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_P2P_002 Message_t{ \
    .id                             = "P2P_002", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameter <total size parameter name> value (<total size parameter value> MB) is not a multiple of parameter <buffer size parameter name> value (<buffer size parameter value> MB) for test <idx>", \
    .msg_edit                       = "%s value (%s MB) is not a multiple of %s value (%s MB) for test %s", \
    .detail                         = "The total size of the data transferred, set by P2P testcase parameters in test JSON file, must be a multiple of the buffer size, set in P2P test sequence", \
    .resolution                     = "Check total size and buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_003 Message_t{ .id = "P2P_003", .state = "obsolete" }
#define MSG_P2P_004 Message_t{ \
    .id                             = "P2P_004", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to compute positive integer value for number of buffers <number of buffers>.", \
    .msg_edit                       = "Failed to compute positive integer value for number of buffers %s.", \
    .detail                         = "The number of buffers cannot be computed", \
    .resolution                     = "Check total size and buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_005 Message_t{ .id = "P2P_005", .state = "obsolete" }
#define MSG_P2P_006 Message_t{ \
    .id                             = "P2P_006", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to allocate reference data host buffer", \
    .msg_edit                       = "", \
    .detail                         = "The host application was not able to allocate the memory necessary for the reference data (size equals buffer size) ", \
    .resolution                     = "Check memory usage", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_007 Message_t{ \
    .id                             = "P2P_007", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to allocate host buffer index <buffer index>", \
    .msg_edit                       = "Failed to allocate host buffer index %s", \
    .detail                         = "The host application was not able to allocate the memory necessary for one of the host buffers (size equals buffer size). Note that buffers with lower index have been allocated successfully", \
    .resolution                     = "Check memory usage", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_008 Message_t{ \
    .id                             = "P2P_008", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Failed to compute <read or write> BW", \
    .msg_edit                       = "Failed to compute %s BW. The P2P transfer duration was equal to 0", \
    .detail                         = "The host application was not able to compute the BW", \
    .resolution                     = "Check board status using xbutil examine", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_009 Message_t{ \
    .id                             = "P2P_009", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Data integrity test failed for buffer <buffer index>", \
    .msg_edit                       = "%s - Data integrity test failed for buffer: %s", \
    .detail                         = "Data integrity was not maintained during the test. The data read by host application does not match the reference data", \
    .resolution                     = "Check board status using xbutil examine", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_010 Message_t{ .id = "P2P_010", .state = "obsolete" }
#define MSG_P2P_011 Message_t{ \
    .id                             = "P2P_011", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Data integrity and bandwidth test pass", \
    .msg_edit                       = "%s - Data integrity and bandwidth test pass", \
    .detail                         = "The read data matches the reference data AND the BW is within the range defined in card definition JSON file" \
}
#define MSG_P2P_012 Message_t{ \
    .id                             = "P2P_012", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Data integrity and/or bandwidth fail", \
    .msg_edit                       = "%s - Data integrity and/or bandwidth test fail", \
    .detail                         = "The read data does not match the reference data OR the BW is not within the range defined in card definition JSON file.", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_013 Message_t{ \
    .id                             = "P2P_013", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Data integrity test pass", \
    .msg_edit                       = "%s - Data integrity test pass", \
    .detail                         = "The read data matches the reference data", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_P2P_014 Message_t{ \
    .id                             = "P2P_014", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Data integrity test fail", \
    .msg_edit                       = "%s - Data integrity test fail", \
    .detail                         = "The read data does not match the reference data", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_015 Message_t{ \
    .id                             = "P2P_015", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Read data does not match write data", \
    .msg_edit                       = "", \
    .detail                         = "Data integrity errors have been detected", \
    .resolution                     = "Check the INFO message that follows to see which byte of the data was not transferred correctly", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_016 Message_t{ \
    .id                             = "P2P_016", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<line of BW test fail result>", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW is not within the range defined in card definition JSON file. The BW values are displayed in the INFO messages that follow", \
    .resolution                     = "Check board status using xbutil examine. Check memory page size. Check for other processes accessing the memory. Try to increase the test duration", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_017 Message_t{ \
    .id                             = "P2P_017", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<line of BW test pass result>", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW is within the range defined in card definition JSON file. The BW values are displayed in the INFO messages that follow" \
}
#define MSG_P2P_018 Message_t{ \
    .id                             = "P2P_018", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "BW values", \
    .msg_edit                       = "%s", \
    .detail                         = "The BW values are reported by the host application" \
}
#define MSG_P2P_019 Message_t{ \
    .id                             = "P2P_019", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Test <idx> is defined by <definition>. Creating <quantity> tests, <description>", \
    .msg_edit                       = "Test %s is defined by %s. Creating %s tests, %s", \
    .detail                         = "When the test is defined by memory type in the test JSON file, a test is created for each memory tag found for this type. The available tag for each memory type is defined in the card definition JSON file and displayed at xbtest start-up. Try: xbtest -c verify -d <device BDF>", \
    .resolution                     = "Define the memory tag you want to target if you want to target a specific memory tag, otherwise this warning can be safely ignored" \
}
#define MSG_P2P_020 Message_t{ .id = "P2P_020", .state = "obsolete" }
#define MSG_P2P_021 Message_t{ \
    .id                             = "P2P_021", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<line of data integrity error details>", \
    .msg_edit                       = "%s", \
    .detail                         = "When the data integrity test fails, the data index and values are reported" \
}
#define MSG_P2P_022 Message_t{ .id = "P2P_022", .state = "obsolete" }
#define MSG_P2P_023 Message_t{ \
    .id                             = "P2P_023", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameter <buffer size parameter name> value (<buffer size parameter value> MB) is <invalid> for test <idx>", \
    .msg_edit                       = "Parameter %s value (%s MB) is %s for test %s", \
    .detail                         = "The value of the buffer size, quantity of data transferred in each buffer, is not set as expected in P2P test sequence", \
    .resolution                     = "Check buffer size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_024 Message_t{ \
    .id                             = "P2P_024", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to find memory configuration for memory <type/tag> <type/tag name>", \
    .msg_edit                       = "Failed to find memory configuration for memory %s %s", \
    .detail                         = "No memory configuration corresponds to the provided tag or type", \
    .resolution                     = "Try command line option -g to check available memory tag or type", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_025 Message_t{ \
    .id                             = "P2P_025", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Due to usage of <reason>, the test sequence has been extended", \
    .msg_edit                       = "Due to usage of %s, the test sequence has been extended", \
    .detail                         = "When the test sequence is using memory type, each tag of the selected type will be tested, resulting in a longer test sequence" \
}
#define MSG_P2P_026 Message_t{ \
    .id                             = "P2P_026", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<count> write-read-check cycles performed during this test", \
    .msg_edit                       = "%s write-read-check cycles performed during this test", \
    .detail                         = "The quantity of write-read-check cycles (memory written and read back) during the test is reported", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_P2P_027 Message_t{ \
    .id                             = "P2P_027", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "<memory tag> - Reloading reference data in buffer <buffer index> after data integrity check failed. This might take extra time and impact quantity of write-read-check cycles done in given test duration", \
    .msg_edit                       = "%s - Reloading reference data in buffer %s after data integrity check failed. This might take extra time and impact quantity of write-read-check cycles done in given test duration", \
    .detail                         = "A host buffer need to be filled with reference data before being are written to the device during next write-read-check cycle. The time needed to complete this action depend on the buffer size", \
    .resolution                     = "If needed, increase test duration so more write-read-check cycles are performed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_028 Message_t{ \
    .id                             = "P2P_028", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Test <index> will be stopped on <error> error", \
    .msg_edit                       = "Test %s will be stopped on %s error", \
    .detail                         = "Tests are stopped on error as enabled in test JSON file", \
    .resolution                     = "Check the cause of the error. Update test JSON file if to disable stop on error if needed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_029 Message_t{ \
    .id                             = "P2P_029", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to get FD of target OpenCL buffer <index>. OpenCL return code = <ret>", \
    .msg_edit                       = "Failed to get FD of target OpenCL buffer %s. OpenCL return code = %s", \
    .detail                         = "Unable to obtain the file descriptor of the OpenCL buffer", \
    .resolution                     = "Check the cause of the error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_030 Message_t{ \
    .id                             = "P2P_030", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to import OpenCL buffer <index> from target buffer FD", \
    .msg_edit                       = "Failed to import OpenCL buffer %s from target buffer FD", \
    .detail                         = "Unable to obtain the file descriptor of the OpenCL buffer", \
    .resolution                     = "Check the cause of the error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_031 Message_t{ \
    .id                             = "P2P_031", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to <open/close> NVMe path <path>. Error <error>", \
    .msg_edit                       = "Failed to %s NVMe path %s. Error: %s", \
    .detail                         = "Unable to open or close the NVMe path needed for p2p_nvme testcase", \
    .resolution                     = "Check the path exists and write/read permissions", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_032 Message_t{ \
    .id                             = "P2P_032", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to <write/read> NVMe path <path>. Error: <error>", \
    .msg_edit                       = "Failed to %s NVMe path %s. Error: %s", \
    .detail                         = "Unable to write or read the NVMe path in the p2p_nvme testcase", \
    .resolution                     = "Check the space left on disk, the disk configuration or the path write/read permissions", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_033 Message_t{ \
    .id                             = "P2P_033", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to get extension function: <function name>", \
    .msg_edit                       = "Failed to get extension function: %s", \
    .detail                         = "Unable to get extension function needed for the p2p_card testcase", \
    .resolution                     = "Check your card setup", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_034 Message_t{ \
    .id                             = "P2P_034", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameters <param> and <param> cannot be combined in test <test>", \
    .msg_edit                       = "Parameters %s and %s cannot be combined in test %s", \
    .detail                         = "In this test case, these parameters cannot be combined", \
    .resolution                     = "Set only one of these parameters in the same test of the test sequence", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_035 Message_t{ \
    .id                             = "P2P_035", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Card memory provided as <param> in test <test>. The NVMe device will be used as <param>", \
    .msg_edit                       = "Card memory provided as %s in test %s. The NVMe device will be used as %s", \
    .detail                         = "In this test case, these parameters cannot be combined", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_P2P_036 Message_t{ \
    .id                             = "P2P_036", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Check the soft resource limit for maximum number of open file descriptors. This limit may be smaller than the number of buffers (<num>) required for this test. See command: $ ulimit -Sn", \
    .msg_edit                       = "Check the soft resource limit for maximum number of open file descriptors. This limit may be smaller than the number of buffers (%s) required for this test. See command: $ ulimit -Sn", \
    .detail                         = "One FD is opened for each buffer when importing target OpenCL buffers to the source OpenCL context", \
    .resolution                     = "Increase the soft resource limit for maximum number of open file descriptors, or reduce the number of buffers by decreasing total size and/or increasing buffer size", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_037 Message_t{ \
    .id                             = "P2P_037", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Bandwidth test pass", \
    .msg_edit                       = "%s - Bandwidth test pass", \
    .detail                         = "Bandwidth is in expected range. Data integrity is not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_P2P_038 Message_t{ \
    .id                             = "P2P_038", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Bandwidth test fail", \
    .msg_edit                       = "%s - Bandwidth test fail", \
    .detail                         = "Bandwidth is not in expected range. Data integrity is not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_039 Message_t{ \
    .id                             = "P2P_039", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "<memory tag> - Test pass", \
    .msg_edit                       = "%s - Test pass", \
    .detail                         = "Bandwidth and data integrity are not checked", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_P2P_040 Message_t{ \
    .id                             = "P2P_040", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<memory tag> - Test fail", \
    .msg_edit                       = "%s - Test fail", \
    .detail                         = "Bandwidth and data integrity are not checked", \
    .resolution                     = "Check error messages printed before", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_P2P_041 Message_t{ \
    .id                             = "P2P_041", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Using AMI, P2P disabled", \
    .msg_edit                       = "", \
    .detail                         = "AMI currently does not support use of P2P", \
    .resolution                     = "", \
    .display_classic_console        = true, \
}



#define MSG_VER_001 Message_t{ \
    .id                             = "VER_001", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "For xbtest HW IP <IP name>, HW version (<HW version> matches SW version (<SW version>) ", \
    .msg_edit                       = "For xbtest HW IP %s, HW version (%s) matches SW version (%s) ", \
    .detail                         = "For each xbtest HW IP, HW and SW versions are checked to be compatible. The verify test can continue", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_VER_002 Message_t{ \
    .id                             = "VER_002", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "For xbtest HW IP <IP name>, HW version (<HW version> does not match SW version (<SW version>) ", \
    .msg_edit                       = "For xbtest HW IP %s, HW version (%s) does not match SW version (%s) ", \
    .detail                         = "HW and SW versions are not compatible for this xbtest HW IP. The verify test is interrupted", \
    .resolution                     = "Check versions of host application and HW design in use", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_003 Message_t{ \
    .id                             = "VER_003", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "For xbtest HW IP <IP name>, HW version (<HW version>) is obsolete and is not compatible with SW version (<SW version>) ", \
    .msg_edit                       = "For xbtest HW IP %s, HW version (%s) is obsolete and is not compatible with SW version (%s) ", \
    .detail                         = "HW and SW versions are not compatible for this xbtest HW IP. The verify test is interrupted", \
    .resolution                     = "Check versions of host application and HW design in use", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_004 Message_t{ \
    .id                             = "VER_004", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Build info of xbtest HW IP <IP name>. HW build: <HW build> (SW build: <SW build>) ", \
    .msg_edit                       = "Build info of xbtest HW IP %s. HW build: %s (SW build: %s) ", \
    .detail                         = "This message reports HW and SW build numbers when not matching", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_VER_005 Message_t{ \
    .id                             = "VER_005", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Build info of xbtest HW IP <IP name>. <BI parameter name> value read (<BI parameter value found>) does not equal expected value (<expected BI parameter value>) ", \
    .msg_edit                       = "Build info of xbtest HW IP %s. %s value read (%s) does not equal expected value (%s) ", \
    .detail                         = "This error message indicates that an information in the HW build info of a xbtest HW IP has not been set as expected by the host application", \
    .resolution                     = "Check versions of host application and HW design in use. Check configuration of xbtest HW IPs corresponds to configuration expected in SW. Check host application can communicate with the board", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_006 Message_t{ \
    .id                             = "VER_006", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "No build info check performed as no valid xbtest HW IP detected", \
    .msg_edit                       = "", \
    .detail                         = "No xbtest xbtest HW IP has been found in the HW design and thus, the verify test cannot run", \
    .resolution                     = "Check the HW design in use is correct", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_007 Message_t{ \
    .id                             = "VER_007", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Test pass for xbtest HW IP <IP name>", \
    .msg_edit                       = "Test pass for xbtest HW IP %s", \
    .detail                         = "Global PASS message indicated that the all operations in the verify task for the xbtest HW IP have been performed successfully", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_VER_008 Message_t{ \
    .id                             = "VER_008", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Test fail for xbtest HW IP <IP name>", \
    .msg_edit                       = "Test fail for xbtest HW IP %s", \
    .detail                         = "Global ERROR message indicated that the operations in the verify task for the xbtest HW IP have failed", \
    .resolution                     = "Check the error messages printed above", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_009 Message_t{ \
    .id                             = "VER_009", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "No test performed", \
    .msg_edit                       = "", \
    .detail                         = "This error means that a valid xbtest HW IP has been found in the HW design, but the verify task was not able to run any operation", \
    .resolution                     = "Check the board state. Note that this message is not displayed when host application has not been interrupted at the beginning of the verify task", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_010 Message_t{ \
    .id                             = "VER_010", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Build info of xbtest HW IP <IP name>. <BI parameter name> value read (<BI parameter value found>) does not equal expected value (<expected BI parameter value>) ", \
    .msg_edit                       = "Build info of xbtest HW IP %s. %s value read (%s) does not equal expected value (%s) ", \
    .detail                         = "This error message indicates that an information in the HW build info of a xbtest HW IP has not been set as expected by the host application", \
    .resolution                     = "Check versions of host application and HW design in use. Check configuration of xbtest HW IPs corresponds to configuration expected in SW. Check host application can communicate with the board", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_011 Message_t{ \
    .id                             = "VER_011", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Test skipped for xbtest HW IP <IP name>", \
    .msg_edit                       = "Test skipped for xbtest HW IP %s", \
    .detail                         = "Indication that verify task has been skipped for a xbtest HW IP", \
    .resolution                     = "Check previous message to identify why this xbtest HW IP is ignored", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_012 Message_t{ \
    .id                             = "VER_012", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Verify xbtest HW IP has not access to the DNA", \
    .msg_edit                       = "", \
    .detail                         = "The Verify xbtest HW IP is not containing the logic to read the DNA ", \
    .resolution                     = "Check HW design generation settings and HW design accessibility to the DNA information", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_013 Message_t{ \
    .id                             = "VER_013", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Read DNA timed out after <value> seconds", \
    .msg_edit                       = "Read DNA timed out after %s seconds", \
    .detail                         = "The DNA could be read within the specified time", \
    .resolution                     = "Check what could slow down access to the card or the HW design DNA access", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_VER_014 Message_t{ \
    .id                             = "VER_014", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "FPGA DNA: <value>", \
    .msg_edit                       = "FPGA DNA: 0x%s_%s_%s", \
    .detail                         = "DNA value read out of the FPGA" \
}
#define MSG_VER_015 Message_t{ \
    .id                             = "VER_015", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "HW Build version: <value>", \
    .msg_edit                       = "HW Build version: %s", \
    .detail                         = "Reference version of the HW sources used to build the HW design" \
}
#define MSG_ETH_001 Message_t{ \
    .id                             = "ETH_001", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Loading GT MAC xbtest HW IP configuration into PLRAM", \
    .msg_edit                       = "", \
    .detail                         = "This message is printed at the beginning of the GT MAC test" \
}
#define MSG_ETH_002 Message_t{ \
    .id                             = "ETH_002", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Checking GT xbtest HW IP status", \
    .msg_edit                       = "", \
    .detail                         = "When HW is active and the test_sequence mode is set to status, the host application checks the HW status registers of GT xbtest HW IP" \
}
#define MSG_ETH_003 Message_t{ .id = "ETH_003", .state = "obsolete" }
#define MSG_ETH_004 Message_t{ \
    .id                             = "ETH_004", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Unexpected non-zero value (<status value>) is read for status <status name>", \
    .msg_edit                       = "Unexpected non-zero value (%s) is read for status %s", \
    .detail                         = "The value of a HW status register of GT MAC xbtest HW IP (for the specified lane) has been read and its value is not as expected", \
    .resolution                     = "Check the definition of the MAC status to understand why this error occurs", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_005 Message_t{ \
    .id                             = "ETH_005", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Expected zero value (<status value>) is read for status <status name>", \
    .msg_edit                       = "Expected zero value (%s) is read for status %s", \
    .detail                         = "The value of a HW status register of GT MAC xbtest HW IP (for the specified lane) has been read and its value equals zero as expected", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ETH_006 Message_t{ \
    .id                             = "ETH_006", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Status <status name> = \t<status value>", \
    .msg_edit                       = "Status %s = \t%s", \
    .detail                         = "Reporting the value of a HW status register of GT MAC xbtest HW IP (for the specified lane) that has been read", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ETH_007 Message_t{ \
    .id                             = "ETH_007", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "No good packets received", \
    .msg_edit                       = "", \
    .detail                         = "The current Lane is not receiving valid data traffic", \
    .resolution                     = "Check the Lane status and/or the board setup to see if this Lane is receiving traffic", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_008 Message_t{ .id = "ETH_008", .state = "obsolete" }
#define MSG_ETH_009 Message_t{ .id = "ETH_009", .state = "obsolete" }
#define MSG_ETH_010 Message_t{ \
    .id                             = "ETH_010", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "GT xbtest HW IP status check pass", \
    .msg_edit                       = "", \
    .detail                         = "The HW status registers of GT xbtest HW IP are correct" \
}
#define MSG_ETH_011 Message_t{ \
    .id                             = "ETH_011", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "GT xbtest HW IP status check fail", \
    .msg_edit                       = "", \
    .detail                         = "The HW status registers of GT xbtest HW IP are invalid", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_012 Message_t{ \
    .id                             = "ETH_012", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Test passed for all lanes", \
    .msg_edit                       = "", \
    .detail                         = "Global message indicating that the HW status registers of GT xbtest HW IP are correct for all lanes", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ETH_013 Message_t{ \
    .id                             = "ETH_013", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Test failed for some lanes", \
    .msg_edit                       = "", \
    .detail                         = "Global message indicating that the HW status registers of GT xbtest HW IP are invalid for some lanes", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_014 Message_t{ \
    .id                             = "ETH_014", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Link down detected", \
    .msg_edit                       = "", \
    .detail                         = "The XXV ethernet core has detected a link down for the specified lane during the test. Link down should only occur when the GT is reset", \
    .resolution                     = "Check the connection to the GT. Is loopback module present (or used)? Check the analyser (is used), is the laser on?", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_015 Message_t{ \
    .id                             = "ETH_015", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Expected link down detected", \
    .msg_edit                       = "", \
    .detail                         = "The XXV ethernet core has detected an expected link down after a GT reset, this is normal behaviour as a GT reset has been issued by the GT MAC xbtest HW IP" \
}
#define MSG_ETH_016 Message_t{ \
    .id                             = "ETH_016", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "GT TX reset detected", \
    .msg_edit                       = "", \
    .detail                         = "The XXV ethernet core has detected a GT TX reset", \
    .resolution                     = "Check the connection to the GT, did you change lane rate?", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_017 Message_t{ \
    .id                             = "ETH_017", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Expected GT TX reset detected", \
    .msg_edit                       = "", \
    .detail                         = "The XXV ethernet core has detected an expected GT TX reset, this is normal behaviour as a GT reset has been issued by the GT MAC xbtest HW IP" \
}
#define MSG_ETH_018 Message_t{ \
    .id                             = "ETH_018", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "GT RX reset detected", \
    .msg_edit                       = "", \
    .detail                         = "The XXV ethernet core has detected a GT RX reset", \
    .resolution                     = "Check the connection to the GT. Is loopback module present (or used)? Check the analyser (is used), is the laser on?", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_019 Message_t{ \
    .id                             = "ETH_019", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Expected GT RX reset detected", \
    .msg_edit                       = "", \
    .detail                         = "The XXV ethernet core has detected an expected GT RX reset, this is normal behaviour as a GT reset has been issued by the GT MAC xbtest HW IP" \
}
#define MSG_ETH_020 Message_t{ .id = "ETH_020", .state = "obsolete" }
#define MSG_ETH_021 Message_t{ .id = "ETH_021", .state = "obsolete" }
#define MSG_ETH_022 Message_t{ \
    .id                             = "ETH_022", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Quantity of packet with valid addresses matches the total quantity of packet sent: <packets>", \
    .msg_edit                       = "Quantity of packet with valid addresses matches the total quantity of packet sent: %s", \
    .detail                         = "The quantity of packet with valid addresses matches the total quantity of packet sent" \
}
#define MSG_ETH_023 Message_t{ \
    .id                             = "ETH_023", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Quantity of bytes from packet with valid addresses matches the total quantity of byte sent: <bytes>", \
    .msg_edit                       = "Quantity of bytes from packet with valid addresses matches the total quantity of byte sent: %s", \
    .detail                         = "The quantity of bytes from packet with valid addresses matches the total quantity of byte sent" \
}
#define MSG_ETH_024 Message_t{ \
    .id                             = "ETH_024", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Quantity of packet with valid addresses does not match the total quantity of packet sent: <packets>, expected <packets>", \
    .msg_edit                       = "Quantity of packet with valid addresses does not match the total quantity of packet sent: %s , expected %s", \
    .detail                         = "The quantity of packet with valid addresses does not match the total quantity of packet sent", \
    .resolution                     = "Check the GT MAC configuration tx_mapping_<TX lane index> is set to the correct <RX lane index>", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_025 Message_t{ \
    .id                             = "ETH_025", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Quantity of bytes from packet with valid addresses does not match the total quantity of byte sent: <bytes>, expected. <bytes>", \
    .msg_edit                       = "Quantity of bytes from packet with valid addresses does not matches the total quantity of byte sent: %s , expected %s", \
    .detail                         = "The quantity of bytes from packet with valid addresses does not match the total quantity of byte sent", \
    .resolution                     = "Check the GT MAC configuration tx_mapping_<TX lane index> is set to the correct <RX lane index>", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_026 Message_t{ \
    .id                             = "ETH_026", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "MAC address contains 1 octet out of range [0x0,0xFF]. Address: <value>; Octet: <value>", \
    .msg_edit                       = "MAC address contains 1 octet out of range [0x0,0xFF]. Address: %s; Octet: %s", \
    .detail                         = "The MAC address contains an octet which is not correct", \
    .resolution                     = "Check the MAC address provided. It must be composed of 6 octets", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_027 Message_t{ \
    .id                             = "ETH_027", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "MAC address doesn't contain exactly 6 octets separated by colons (':'), Address: <value>", \
    .msg_edit                       = "MAC address doesn't contain exactly 6 octets separated by colons (':'), Address: %s", \
    .detail                         = "The MAC address is not valid. it must be composed of 6 octets separated by colons (':') ", \
    .resolution                     = "Check the MAC address provided. It must be composed of 6 octets separated by colons (':') ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_028 Message_t{ \
    .id                             = "ETH_028", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "MAC address octet conversion to integer failed. Address: <value>; octet: <value>", \
    .msg_edit                       = "MAC address octet conversion to integer failed. Address: %s; octet: %s", \
    .detail                         = "The MAC address is not valid. it contains an octet which cannot be converted", \
    .resolution                     = "Check the MAC address provided. It must be composed of hexadecimal octets", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_029 Message_t{ \
    .id                             = "ETH_029", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "A MAC address octet should be only composed of 2 hexadecimal characters. Address: <value>; octet: <value>", \
    .msg_edit                       = "A MAC address octet should be only composed of 2 hexadecimal characters. Address: %s; octet: %s", \
    .detail                         = "The MAC address octet is not valid, only 2 hexadecimal characters are expected", \
    .resolution                     = "Check the MAC address provided. It must be composed of hexadecimal octets", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_030 Message_t{ \
    .id                             = "ETH_030", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Board MAC address list:", \
    .msg_edit                       = "", \
    .detail                         = "List the MAC address extracted from Board EEPROM" \
}
#define MSG_ETH_031 Message_t{ \
    .id                             = "ETH_031", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<lane info>", \
    .msg_edit                       = "%s", \
    .detail                         = "Reporting source and destination MAC addresses and resulting lane state in a table" \
}
#define MSG_ETH_032 Message_t{ \
    .id                             = "ETH_032", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "MAC address <index> (<address>) is identical to MAC address <index> (<address>) ", \
    .msg_edit                       = "MAC address %s (0x%s) is identical to MAC address %s (0x%s) ", \
    .detail                         = "Two identical MAC addresses have been detected", \
    .resolution                     = "Make sure that the MAC address for each lane is defined uniquely", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_033 Message_t{ \
    .id                             = "ETH_033", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "There is not enough valid board MAC addresses to serve all lanes; <quantity> valid addresses for a total of <quantity> lanes. Some lanes may be disabled", \
    .msg_edit                       = "There is not enough valid board MAC addresses to serve all lanes; %s valid addresses for a total of %s lanes. Some lanes may be disabled", \
    .detail                         = "Each GT requires one MAC addresses for each of the four lanes. Not enough board MAC addresses are available for all lanes of all GTs. Some lanes may be disabled", \
    .resolution                     = "Make sure that there is one valid MAC address for each lane", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_034 Message_t{ \
    .id                             = "ETH_034", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Round robin assignment of source MAC addresses for all GTs & lanes from board MAC address list. If there are not enough valid MAC addresses, some lanes may be disabled", \
    .msg_edit                       = "", \
    .detail                         = "Display source MAC addresses for all lanes of the GT. A Round Robin distribution is used from the valid board MAC address list to each GT lane" \
}
#define MSG_ETH_035 Message_t{ \
    .id                             = "ETH_035", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Destination MAC address per lane (based on tx_mapping). If the destination MAC address is not valid, the lane will be disabled", \
    .msg_edit                       = "", \
    .detail                         = "List of the destination MAC addresses per lane, based on the tx_mapping available in the test JSON file" \
}
#define MSG_ETH_036 Message_t{ \
    .id                             = "ETH_036", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "-board_mac_addr_<index>: <address>", \
    .msg_edit                       = "\t-board_mac_addr_%s: %s", \
    .detail                         = "List of Board MAC addresses" \
}
#define MSG_ETH_037 Message_t{ \
    .id                             = "ETH_037", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "MAC address is longer than 6 octets: 0x<address>", \
    .msg_edit                       = "MAC address is longer than 6 octets: 0x%s", \
    .detail                         = "List of board MAC addresses", \
    .resolution                     = "Provide a MAC address smaller than 2^48", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_038 Message_t{ \
    .id                             = "ETH_038", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "No board MAC addresses available", \
    .msg_edit                       = "", \
    .detail                         = "No MAC address could have been extracted from the board", \
    .resolution                     = "Check that some MAC addresses are available, via the command 'xbmgmt examine -d <BDF>' or 'xbutil examine -d <BDF>' ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_039 Message_t{ \
    .id                             = "ETH_039", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The value (<parameter value>) for <packet_cfg> is out of range. The valid value/range for this parameter are: 'sweep', [<min value>,<max value>], [<min value>,<max value>] ", \
    .msg_edit                       = "The value (%s) for %s is out of range. The valid value/range for this parameter are: %s, [%s,%s], [%s,%s] ", \
    .detail                         = "The value supplied in the test JSON file for packet_cfg is out of range. The supported range is displayed along with the failure message", \
    .resolution                     = "Provide a value within the valid range", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_040 Message_t{ \
    .id                             = "ETH_040", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The GT's are not running at the request lane rate: 1 or more lanes are <rate>, while expected <rate> (<status>) ", \
    .msg_edit                       = "The GT's are not running at the request lane rate: 1 or more lanes are <%s>, while expected rate <%s> (%s) ", \
    .detail                         = "Internal GT's lane rate is not set correctly", \
    .resolution                     = "Contact Xilinx Support", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_041 Message_t{ \
    .id                             = "ETH_041", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Tx <byte/packet> sent (<quantity>) within range [<min>, <max>]", \
    .msg_edit                       = "Tx %s sent (%s) within range [%s, %s]", \
    .detail                         = "For the specified lane, the quantity of bytes or packets sent is within range. The range is computed based on the theoretical rate achieved for the used packet size. A tolerance of 20% is used.", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ETH_042 Message_t{ \
    .id                             = "ETH_042", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Tx <byte/packet> sent <quantity> out of range [<min>, <max>]", \
    .msg_edit                       = "Tx %s sent (%s) out of range [%s, %s]", \
    .detail                         = "For the specified lane, the quantity of byte/packet sent is out of range. The range is computed based on the theoretical rate achieved for the used packet size. A tolerance of 20% is used. This is not necessary an issue as by design part of xbtest GT MAC traffic controller/generator is shared by all lanes. Using small packet (typically < 128 Bytes) is more sensitive to other lanes configuration", \
    .resolution                     = "Reduce the quantity of lane using small packets", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_043 Message_t{ \
    .id                             = "ETH_043", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The duration of the 'run' test is too long <duration>, this may result in MAC counter saturation. For <rate>, the max duration is <max> seconds, please split your test sequence.", \
    .msg_edit                       = "The duration of the 'run' test is too long (%s), this may result in MAC counter saturation. For %s, the max duration is %s seconds, please split your test sequence.", \
    .detail                         = "The duration is too big for the selected rate", \
    .resolution                     = "Please split your test sequence into multiple 'clear', 'run', 'check' and use smaller duration for each 'run' ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_044 Message_t{ \
    .id                             = "ETH_044", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "The board MAC address (<addr>) is not valid", \
    .msg_edit                       = "The board MAC address (%s) is not valid", \
    .detail                         = "The MAC address stored on the board EEPROM is not correct. It won't be used and all lanes using it will be disabled", \
    .resolution                     = "Contact Xilinx Support for how to reprogram your card with valid MAC address", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_045 Message_t{ \
    .id                             = "ETH_045", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "The selected board address is out of the range of the valid board MAC addresses available: <index> > <available> ", \
    .msg_edit                       = "The selected board address is out of the range of the valid board MAC addresses available: %s (selected) > %s (available) ", \
    .detail                         = "The board MAC addresses are displayed via 'xbutil examine -d <BDF>'. They may not be all valid. Only valid board addresses can be selected", \
    .resolution                     = "Use a small board MAC address index", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_046 Message_t{ \
    .id                             = "ETH_046", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "All lanes are disabled, no test can be run", \
    .msg_edit                       = "All lanes are disabled, no test can be run", \
    .detail                         = "All lanes have been disabled, there is no need to run the testcase", \
    .resolution                     = "Check previous messages to understand why all lanes are disabled. Alternatively check that they are not all disabled in the test JSON file (in such case, please remove the testcase) ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_047 Message_t{ \
    .id                             = "ETH_047", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "The jumbo frame must use even packet size: <packet_size>", \
    .msg_edit                       = "The jumbo frame must use even packet size: %s", \
    .detail                         = "When using jumbo frame, the length of the packet must be even value", \
    .resolution                     = "Provide an even value", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_048 Message_t{ \
    .id                             = "ETH_048", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "The quantity of <byte/packet> sent is null", \
    .msg_edit                       = "The quantity of %s sent is null", \
    .detail                         = "Although the lane is not disabled, no traffic is sent. Check previous messages ", \
    .resolution                     = "Check if something is not blocking the lane. Check previous messages and GT connections ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_049 Message_t{ \
    .id                             = "ETH_049", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "The test sequence rate (<requested_rate>) is not supported by the HW design (which only supports <supported_rate>) ", \
    .msg_edit                       = "The test sequence rate (%s) is not supported by the HW design (which only supports %s) ", \
    .detail                         = "The test rate is not possible with the HW design selected", \
    .resolution                     = "Use supported rate: change test setting or use different HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_050 Message_t{ \
    .id                             = "ETH_050", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "The HW design contains an unsupported GT MAC xbtest HW IP rate", \
    .msg_edit                       = "The HW design contains an unsupported GT MAC xbtest HW IP rate", \
    .detail                         = "The HW design is not compatible with the SW", \
    .resolution                     = "Use SW compatible HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_051 Message_t{ \
    .id                             = "ETH_051", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Port to Port connection detected between GT port <index> and this GT port (<index>) ", \
    .msg_edit                       = "Port to Port connection detected between GT port %s and this GT port (<%s>) ", \
    .detail                         = "Traffic loop detected, make sure that listed ports are cross connected" \
}
#define MSG_ETH_052 Message_t{ \
    .id                             = "ETH_052", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Port to Port connection: Test sequence & conguration is coming for GTMAC xbtest HW IP <index>", \
    .msg_edit                       = "Port to Port connection: Test sequence & conguration is coming for GTMAC xbtest HW IP %s ", \
    .detail                         = "All configurations (global, lane mapping, test_sequence ...) are coming from the other GTMAC xbtest HW IP", \
    .resolution                     = "if this behaviour is not desired, remove the port to port connection setting" \
}
#define MSG_ETH_053 Message_t{ \
    .id                             = "ETH_053", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Test Sequence must de defined", \
    .msg_edit                       = "Test Sequence must de defined", \
    .detail                         = "In this mode, a test sequence must be defined", \
    .resolution                     = "Provide a test_sequence", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_054 Message_t{ \
    .id                             = "ETH_054", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Port to Port connection: Paired GTMAC not in sync: GTMAC[<index>] is at test <cnt>, while GTMAC[<index>] is at test <cnt>", \
    .msg_edit                       = "Port to Port connection: Paired GTMAC not in sync: GTMAC[%s] is at test %s, while GTMAC[%s] is at test %s", \
    .detail                         = "The paired GT test sequences are out of sync, check log file and the test sequence used", \
    .resolution                     = "Check both GTMAC test_sequence", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_055 Message_t{ \
    .id                             = "ETH_055", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Port to Port connection: Traffic counter from the pair GT are not available, cross check of Tx-Rx counter won't be possible", \
    .msg_edit                       = "Port to Port connection: Traffic counter from the pair GT are not available, cross check of Tx-Rx counter won't be possible", \
    .detail                         = "The paired GT didn't send its traffic counter, check its status in log file", \
    .resolution                     = "Check paired GTMAC error report in log file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_056 Message_t{ \
    .id                             = "ETH_056", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "The RS FEC is not supported by the HW design", \
    .msg_edit                       = "The RS FEC is not supported by the HW design", \
    .detail                         = "The RS FEC is not possible with the HW design selected", \
    .resolution                     = "Use supported test parameter: change test setting or use different HW design", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_057 Message_t{ \
    .id                             = "ETH_057", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: PRBS Error detected", \
    .msg_edit                       = "Lane[%s]: PRBS Error detected", \
    .detail                         = "A PRBS error has been detected", \
    .resolution                     = "Check connection and GT settings", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_058 Message_t{ \
    .id                             = "ETH_058", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: Null <Tx/Rx> PRBS seed detected", \
    .msg_edit                       = "Lane[%s]: Null %s PRBS seed detected", \
    .detail                         = "A Null PRBS Seed has been detected", \
    .resolution                     = "Check connection and GT settings", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_059 Message_t{ \
    .id                             = "ETH_059", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Lane[<index>]: GT xbtest HW IP status check pass", \
    .msg_edit                       = "Lane[%s]: GT xbtest HW IP status check pass", \
    .detail                         = "The HW status registers of GT xbtest HW IP are correct" \
}
#define MSG_ETH_060 Message_t{ \
    .id                             = "ETH_060", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: GT xbtest HW IP status check fail", \
    .msg_edit                       = "Lane[%s]: GT xbtest HW IP status check fail", \
    .detail                         = "The HW status registers of GT xbtest HW IP are invalid", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_061 Message_t{ \
    .id                             = "ETH_061", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: No PRBS Data <received/transmitted>", \
    .msg_edit                       = "Lane[%s]: No PRBS Data %s", \
    .detail                         = "No PRBS data packet received, only header ones", \
    .resolution                     = "Check your test sequence or your GT connections", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_062 Message_t{ \
    .id                             = "ETH_062", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Lane[<index>]: Quantity of PRBS data word <received/transmitted> : value", \
    .msg_edit                       = "Lane[%s]: Quantity of PRBS data word %s: %s", \
    .detail                         = "Report the quantity of data word received" \
}
#define MSG_ETH_063 Message_t{ \
    .id                             = "ETH_063", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: Quantity of incorrect data words received: <value>. Word error ratio (<ratio> %) above threshold (<thresh> %) ", \
    .msg_edit                       = "Lane[%s]: Quantity of incorrect data words received: %s. Word error ratio (%s %) above threshold (%s %) ", \
    .detail                         = "Reports the quantity of erroneous data word received", \
    .resolution                     = "Check link quality", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_064 Message_t{ \
    .id                             = "ETH_064", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "GT power good not set, GT reset is not allowed", \
    .msg_edit                       = "GT power good not set, GT reset is not allowed", \
    .detail                         = "GT Power good indicator. When this signal asserts High, the clock output from IO Buffer is ready after a delay", \
    .resolution                     = "Check FPGA power supply", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_065 Message_t{ \
    .id                             = "ETH_065", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "GT <path> reset not dones", \
    .msg_edit                       = "GT %s reset not done", \
    .detail                         = "GT reset is not finished", \
    .resolution                     = "Increase duration of the test step", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_066 Message_t{ \
    .id                             = "ETH_066", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "GT PLL has not locked", \
    .msg_edit                       = "", \
    .detail                         = "GT PLL did not go from unlocked to locked", \
    .resolution                     = "Check PLL", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_067 Message_t{ \
    .id                             = "ETH_067", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Unexpected GT PLL lock indicator: <rising/falling/low>", \
    .msg_edit                       = "Unexpected GT PLL lock indicator: %s", \
    .detail                         = "GT PLL lock indicator not in the expecetd state or falling/rising edge detected", \
    .resolution                     = "Check PLL", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_068 Message_t{ \
    .id                             = "ETH_068", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: Unexpected <Tx/Rx> <type> state: <rising/falling/low>", \
    .msg_edit                       = "Lane[ %s]: Unexpected  %s  %s state:  %s", \
    .detail                         = "GT reset is not as expected", \
    .resolution                     = "Check your reset procedure", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_069 Message_t{ \
    .id                             = "ETH_069", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "GT <path> reset not done after <watchdog> attempts, aborting all tests", \
    .msg_edit                       = "GT %s reset not done after %s attempts, aborting all tests", \
    .detail                         = "GT reset can't be completed with the time allocated", \
    .resolution                     = "Increase duration of the reset test step", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_070 Message_t{ \
    .id                             = "ETH_070", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Lane[<index>]: <Tx/Rx> data rate within range (<rate> +/- <tolerance>%): <rate> ", \
    .msg_edit                       = "Lane[%s]: %s data rate within range (%s Gbps +/- %s%): %s Gbps", \
    .detail                         = "Report the data rate measured", \
}
#define MSG_ETH_071 Message_t{ \
    .id                             = "ETH_071", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: <Tx/Rx> data rate outside range (<rate> +/- <tolerance>%): <rate>", \
    .msg_edit                       = "Lane[%s]: %s data rate outside range(%s Gbps +/- %s%): %s Gbps", \
    .detail                         = "Data rate incorrect", \
    .resolution                     = "Check reset procedure or GT connections", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_072 Message_t{ \
    .id                             = "ETH_072", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Un-expected <rising/falling> edge of power good", \
    .msg_edit                       = "Un-expected %s edge of power good", \
    .detail                         = "GT Power good indicator. Power good indicator should be stable", \
    .resolution                     = "Check power supply quality", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_073 Message_t{ \
    .id                             = "ETH_073", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Lane[<index>]: PRBS error detected", \
    .msg_edit                       = "Lane[%s]: PRBS error detected", \
    .detail                         = "A PRBS error has been detected", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_ETH_074 Message_t{ \
    .id                             = "ETH_074", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Lane[<index>]: Quantity of incorrect data words received: <value>. Word error ratio (<ratio> %) below threshold (<thresh> %) ", \
    .msg_edit                       = "Lane[%s]: Quantity of incorrect data words received: %s. Word error ratio (%s %) below threshold (%s %) ", \
    .detail                         = "Reports the quantity of erroneous data word received" \
}
#define MSG_ETH_075 Message_t{ \
    .id                             = "ETH_075", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "GT <path> reset request not detected", \
    .msg_edit                       = "GT %s reset request not detected", \
    .detail                         = "GT reset request has not been fully detected; check previous error", \
    .resolution                     = "Check reset FSM usage", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_076 Message_t{ \
    .id                             = "ETH_076", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "<signal> is not going low", \
    .msg_edit                       = "%s is not going low", \
    .detail                         = "As the reset is triggered, a falling edge should be detected on the signal", \
    .resolution                     = "Check reset FSM", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_077 Message_t{ \
    .id                             = "ETH_077", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<signal> is not going high", \
    .msg_edit                       = " %s is not going high", \
    .detail                         = "As the reset has been triggered and is now over, a rising edge should be detected on the signal", \
    .resolution                     = "Check reset FSM", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_078 Message_t{ \
    .id                             = "ETH_078", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "GT <path> reset done after <qty> reset", \
    .msg_edit                       = "GT %s reset done after %s reset", \
    .detail                         = "GT reset over" \
}
#define MSG_ETH_079 Message_t{ \
    .id                             = "ETH_079", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "GT <path> wizard reset not fully completed, request another reset", \
    .msg_edit                       = "GT %s wizard reset not fully completed, request another reset", \
    .detail                         = "The GTF wizard reset sequence ended, but not all done signals are in high state", \
    .resolution                     = "Restart the test", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_080 Message_t{ \
    .id                             = "ETH_080", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Lane[<index>]: <Done type> done not high", \
    .msg_edit                       = "Lane[%s]: %s done is not high", \
    .detail                         = "After a reset, all signal done should be high.", \
    .resolution                     = "This will automatically re-trigger the reset FSM", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_081 Message_t{ \
    .id                             = "ETH_081", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "GT <path> all lanes wizard & bufferbypass & channel reset + sync done stayed high", \
    .msg_edit                       = "GT %s all lanes wizard & bufferbypass & channel reset + sync done stayed high", \
    .detail                         = "GT reset& sync stays stable during the test" \
}
#define MSG_ETH_082 Message_t{ \
    .id                             = "ETH_082", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: GT power good not set, GT reset is not allowed", \
    .msg_edit                       = "Lane[%s]: GT power good not set, GT reset is not allowed", \
    .detail                         = "GT Power good indicator. When this signal asserts High, the clock output from IO Buffer is ready after a delay of ~250 us", \
    .resolution                     = "Check link quality", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_083 Message_t{ \
    .id                             = "ETH_083", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: un-expected <rising/falling> edge of power good", \
    .msg_edit                       = "Lane[%s]: un-expected %s edge of power good", \
    .detail                         = "GT Power good indicator. Power good shoudl be stable", \
    .resolution                     = "Check link quality", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_084 Message_t{ \
    .id                             = "ETH_084", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Unexpected <Tx/Rx> <type> state: <rising/falling/low>", \
    .msg_edit                       = "Unexpected  %s  %s state:  %s", \
    .detail                         = "GT reset is not as expected", \
    .resolution                     = "Check your reset procedure", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_085 Message_t{ \
    .id                             = "ETH_085", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "No lane enabled", \
    .detail                         = "No lane enabled for the GT", \
    .resolution                     = "Enable at least 1 lane", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_086 Message_t{ \
    .id                             = "ETH_086", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Lane[<index>]: Total quantity of incorrect bit received: <value>, equivalent to a BER of <ber>, is above threshold (<thresh> %) ", \
    .msg_edit                       = "Lane[%s]: Total quantity of incorrect bit received: %s, equivalent to a BER of %s, is above threshold %s ", \
    .detail                         = "Reports the quantity of erroneous bit received", \
    .resolution                     = "Check link quality", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_ETH_087 Message_t{ \
    .id                             = "ETH_087", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Lane[<index>]: Total quantity of incorrect bit received: <value>, equivalent to a BER of <ber>, is below threshold (<thresh> %) ", \
    .msg_edit                       = "Lane[%s]: Total quantity of incorrect bit received: %s, equivalent to a BER of %s, is below threshold %s ", \
    .detail                         = "Reports the quantity of erroneous bit received", \
}
#define MSG_ETH_088 Message_t{ \
    .id                             = "ETH_088", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Current HW 1 sec timestamp (<current timestamp>) does not equal expected (<expected timestamp>)", \
    .msg_edit                       = "Current HW 1 sec timestamp (%s) does not equal expected (%s)", \
    .detail                         = "The average BW and BER results might not be accurate", \
    .resolution                     = "If needed, increase the test duration to have more precise measurements. Check if xbtest has enough OS resource to run" \
}
#define MSG_ETH_089 Message_t{ \
    .id                             = "ETH_089", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Lane[<index>]: <value> bits <transmitted/received> over <value> seconds", \
    .msg_edit                       = "Lane[%s]: %s bits %s over %s seconds", \
    .detail                         = "Report the quantity of data word sent" \
}

#define MSG_PWR_001 Message_t{ \
    .id                             = "PWR_001", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "All <site type>'s are disabled, they will not be used to consume power", \
    .msg_edit                       = "All %s's are disabled, they will not be used to consume power", \
    .detail                         = "There are four types of macro used by the Power xbtest HW IP to consume power: Flip-flop, DSP, BRAM or URAM. By default, they are all used but can be disabled selectively. If a type is disabled, all instances will be inactive (across multiple SLRs if applicable). This will reduce the maximum power consumed by the xbtest HW IP", \
    .resolution                     = "Check xbtest documentation for how to enable/disable macro type.", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_PWR_002 Message_t{ .id = "PWR_002", .state = "obsolete" }
#define MSG_PWR_003 Message_t{ .id = "PWR_003", .state = "obsolete" }
#define MSG_PWR_004 Message_t{ .id = "PWR_004", .state = "obsolete" }
#define MSG_PWR_005 Message_t{ .id = "PWR_005", .state = "obsolete" }
#define MSG_PWR_006 Message_t{ .id = "PWR_006", .state = "obsolete" }
#define MSG_PWR_007 Message_t{ .id = "PWR_007", .state = "obsolete" }
#define MSG_PWR_008 Message_t{ .id = "PWR_008", .state = "obsolete" }
#define MSG_PWR_009 Message_t{ .id = "PWR_009", .state = "obsolete" }
#define MSG_PWR_010 Message_t{ .id = "PWR_010", .state = "obsolete" }
#define MSG_PWR_011 Message_t{ .id = "PWR_011", .state = "obsolete" }
#define MSG_PWR_012 Message_t{ \
    .id                             = "PWR_012", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Make <number of readings> unique sensor reading(s), wait at least 1 second in between each", \
    .msg_edit                       = "Make %s unique sensor reading(s), wait at least 1 second in between each", \
    .detail                         = "Displays number of sensor readings", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_PWR_013 Message_t{ \
    .id                             = "PWR_013", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Measured an average <measurement name> of <power> W with toggle rate of <toggle rate> %, during <time> sec, at average temperature of <temperature> C", \
    .msg_edit                       = "Measured an average %s of %s W with toggle rate of %s %, during %s sec, at average temperature of %s C", \
    .detail                         = "Displays result of measurement" \
}
#define MSG_PWR_014 Message_t{ .id = "PWR_014", .state = "obsolete" }
#define MSG_PWR_015 Message_t{ .id = "PWR_015", .state = "obsolete" }
#define MSG_PWR_016 Message_t{ .id = "PWR_016", .state = "obsolete" }
#define MSG_PWR_017 Message_t{ .id = "PWR_017", .state = "obsolete" }
#define MSG_PWR_018 Message_t{ .id = "PWR_018", .state = "obsolete" }
#define MSG_PWR_019 Message_t{ .id = "PWR_019", .state = "obsolete" }
#define MSG_PWR_020 Message_t{ .id = "PWR_020", .state = "obsolete" }
#define MSG_PWR_021 Message_t{ .id = "PWR_021", .state = "obsolete" }
#define MSG_PWR_022 Message_t{ .id = "PWR_022", .state = "obsolete" }
#define MSG_PWR_023 Message_t{ .id = "PWR_023", .state = "obsolete" }
#define MSG_PWR_024 Message_t{ .id = "PWR_024", .state = "obsolete" }
#define MSG_PWR_025 Message_t{ \
    .id                             = "PWR_025", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Start all Power xbtest HW IPs with a null toggle rate", \
    .msg_edit                       = "", \
    .detail                         = "Start-up of the Power xbtest HW IP", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_PWR_026 Message_t{ .id = "PWR_026", .state = "obsolete" }
#define MSG_PWR_027 Message_t{ .id = "PWR_027", .state = "obsolete" }
#define MSG_PWR_028 Message_t{ .id = "PWR_028", .state = "obsolete" }
#define MSG_PWR_029 Message_t{ .id = "PWR_029", .state = "obsolete" }
#define MSG_PWR_030 Message_t{ .id = "PWR_030", .state = "obsolete" }
#define MSG_PWR_031 Message_t{ .id = "PWR_031", .state = "obsolete" }
#define MSG_PWR_032 Message_t{ .id = "PWR_032", .state = "obsolete" }
#define MSG_PWR_033 Message_t{ .id = "PWR_033", .state = "obsolete" }
#define MSG_PWR_034 Message_t{ .id = "PWR_034", .state = "obsolete" }
#define MSG_PWR_035 Message_t{ .id = "PWR_035", .state = "obsolete" }
#define MSG_PWR_036 Message_t{ .id = "PWR_036", .state = "obsolete" }
#define MSG_PWR_037 Message_t{ .id = "PWR_037", .state = "obsolete" }
#define MSG_PWR_038 Message_t{ .id = "PWR_038", .state = "obsolete" }
#define MSG_PWR_039 Message_t{ .id = "PWR_039", .state = "obsolete" }
#define MSG_PWR_040 Message_t{ .id = "PWR_040", .state = "obsolete" }
#define MSG_PWR_041 Message_t{ .id = "PWR_041", .state = "obsolete" }
#define MSG_PWR_042 Message_t{ .id = "PWR_042", .state = "obsolete" }
#define MSG_PWR_043 Message_t{ \
    .id                             = "PWR_043", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Base power: <power> W (depends on other tests being launched) ", \
    .msg_edit                       = "Base power: %s W (depends on other tests being launched) ", \
    .detail                         = "Idle power prior to start the power test", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_PWR_044 Message_t{ .id = "PWR_044", .state = "obsolete" }
#define MSG_PWR_045 Message_t{ .id = "PWR_045", .state = "obsolete" }
#define MSG_PWR_046 Message_t{ .id = "PWR_046", .state = "obsolete" }
#define MSG_PWR_047 Message_t{ .id = "PWR_047", .state = "obsolete" }
#define MSG_PWR_048 Message_t{ .id = "PWR_048", .state = "obsolete" }
#define MSG_PWR_049 Message_t{ .id = "PWR_049", .state = "obsolete" }
#define MSG_PWR_050 Message_t{ .id = "PWR_050", .state = "obsolete" }
#define MSG_PWR_051 Message_t{ .id = "PWR_051", .state = "obsolete" }
#define MSG_PWR_052 Message_t{ .id = "PWR_052", .state = "obsolete" }
#define MSG_PWR_053 Message_t{ .id = "PWR_053", .state = "obsolete" }
#define MSG_PWR_054 Message_t{ \
    .id                             = "PWR_054", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Stop Power xbtest HW IPs <immediately/gradually>", \
    .msg_edit                       = "Stop Power xbtest HW IPs %s", \
    .detail                         = "The toggle rate of each the Power xbtest HW IPs are decrease gradually, to avoid current and voltage rapid variation, if disable_toggle_ramp parameter is not set", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_PWR_055 Message_t{ .id = "PWR_055", .state = "obsolete" }
#define MSG_PWR_056 Message_t{ .id = "PWR_056", .state = "obsolete" }
#define MSG_PWR_057 Message_t{ .id = "PWR_057", .state = "obsolete" }
#define MSG_PWR_058 Message_t{ .id = "PWR_058", .state = "obsolete" }
#define MSG_PWR_059 Message_t{ \
    .id                             = "PWR_059", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "No new sensor reading was available in the last <time> seconds", \
    .msg_edit                       = "No new sensor reading was available in the last %s seconds", \
    .detail                         = "The host application tries to get new sensor readings regularly. But it is taking too long", \
    .resolution                     = "Check sensor reading duration", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_PWR_060 Message_t{ \
    .id                             = "PWR_060", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Ramp to <percent> %: <time> second; Temperature: <temp> C, Power: <power> W, toggle rate: <percent> % ", \
    .msg_edit                       = "Ramp to %s %: %s second, Temperature: %s C, Power: %s W, Toggle Rate: %s % ", \
    .detail                         = "To avoid rush current in power regulators or/and other board components, the toggle rate steps are limited. This message describes the current update rate and gives some measurements throughout the ramp" \
}
#define MSG_PWR_061 Message_t{ \
    .id                             = "PWR_061", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Power or temperature was not available in the last <time> consecutive new sensor reading. Stop trying to measure average", \
    .msg_edit                       = "Power or temperature was not available in the last %s consecutive new sensor reading. Stop trying to measure average", \
    .detail                         = "The host application requires card power and temparature to compute average", \
    .resolution                     = "Check sensors valid available on card", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_001 Message_t{ \
    .id                             = "MGT_001", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Start 333ms tick function", \
    .msg_edit                       = "", \
    .detail                         = "The SW tick timer is working in the 333ms higher resolution because some memory tests are defined", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MGT_002 Message_t{ \
    .id                             = "MGT_002", \
    .state                          = "used", \
    .log_level                      = LOG_STATUS, \
    .msg                            = "Start 1s tick function", \
    .msg_edit                       = "", \
    .detail                         = "The SW tick timer is working at the slower rate of 1second" \
}
#define MSG_MGT_003 Message_t{ \
    .id                             = "MGT_003", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Timing misalignment: 333 ms timed out (350 ms) ", \
    .msg_edit                       = "", \
    .detail                         = "The SW 333 ms tick timer is not generated at the correct rate", \
    .resolution                     = "Check if your OS or processors are not overloaded", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_004 Message_t{ \
    .id                             = "MGT_004", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Timing mis alignment: 1 s timed out (1.1 s) ", \
    .msg_edit                       = "", \
    .detail                         = "The SW 1 s tick timer is not generated at the correct rate", \
    .resolution                     = "Check if your OS or processors are not overloaded", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_005 Message_t{ \
    .id                             = "MGT_005", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "<source type> source <display name>: operational value (<instant value>) <greater/lower> than defined warning limit (<limit>) ", \
    .msg_edit                       = "%s source %s: operational value (%s) %s than defined warning limit (%s) ", \
    .detail                         = "The listed sensor reports a value greater or lower than its limits", \
    .resolution                     = "If the temperature is greater than the limit, increase the cooling of the board. If the temperature is lower than the limit, are you monitoring existing sensors? Check sensor availability via 'xbutil examine'. Or review limit set in test JSON file. See User Guide for more info", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_006 Message_t{ .id = "MGT_006", .state = "obsolete" }
#define MSG_MGT_007 Message_t{ .id = "MGT_007", .state = "obsolete" }
#define MSG_MGT_008 Message_t{ \
    .id                             = "MGT_008", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Sensor reading duration summary:", \
    .msg_edit                       = "", \
    .detail                         = "These messages display the duration it took xbtest to gather, throughout the entire test, all necessary board sensors (current, voltage, power, temperature ...). Durations are grouped by second and their respective quantities are listed. Sensor reading duration may be affected in multi-card systems when multiple xbtest are running in parallel." \
}
#define MSG_MGT_009 Message_t{ \
    .id                             = "MGT_009", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Displays quantity per second", \
    .msg_edit                       = "%s", \
    .detail                         = "These messages display the duration it took xbtest to gather, throughout the entire test, all necessary board sensors (current, voltage, power, temperature ...). Durations are grouped by second and their respective quantities are listed. Sensor reading duration may be affected in multi-card systems when multiple xbtest are running in parallel." \
}
#define MSG_MGT_010 Message_t{ \
    .id                             = "MGT_010", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "It took more than <duration> seconds to read the sensors", \
    .msg_edit                       = "It took more than %s seconds to read the sensors", \
    .detail                         = "When displayed, a delay in reading a board sensor has occurred. This could adversely affect xbtest: some HW watchdog could have triggered and stopped xbtest HW IPs which could impact test results", \
    .resolution                     = "Is your system (OS/processor) overloaded? Reduced the quantity of cards tested simultaneously.", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_011 Message_t{ \
    .id                             = "MGT_011", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Set watchdog duration to default <duration> seconds", \
    .msg_edit                       = "Set watchdog duration to default %s seconds", \
    .detail                         = "Define the duration of the HW watchdog duration present into each xbtest HW IP" \
}
#define MSG_MGT_012 Message_t{ \
    .id                             = "MGT_012", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "All HW watchdogs have been disabled", \
    .msg_edit                       = "All HW watchdogs have been disabled", \
    .detail                         = "The HW watchdog present in every xbtest HW IP has been disabled", \
    .resolution                     = "In case of SW termination, the HW will be kept running with the last configuration, which will prevent any new test until a reset of the board via 'xbutil reset' ", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_013 Message_t{ \
    .id                             = "MGT_013", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Watchdog duration has been overwritten to <duration> seconds", \
    .msg_edit                       = "Watchdog duration has been overwritten to %s seconds", \
    .detail                         = "The default watchdog duration has been overwritten, this has no impact on the ongoing test", \
    .resolution                     = "In case of SW termination, the HW will be kept running with the last configuration, which will prevent any new test until the watchdog triggers", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_014 Message_t{ \
    .id                             = "MGT_014", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Watchdog duration is not correct <value>, supported values are <set_of_values>", \
    .msg_edit                       = "Watchdog duration is not correct %s, supported values are %s", \
    .detail                         = "The watchdog configuration is wrong", \
    .resolution                     = "Provide a valid configuration, the supported values are displayed in the error message", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_015 Message_t{ .id = "MGT_015", .state = "obsolete" }
#define MSG_MGT_016 Message_t{ \
    .id                             = "MGT_016", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "<source type> source <display name>: operational value (<instant value>) <greater/lower> than defined error limit (<limit>) ", \
    .msg_edit                       = "%s source %s: operational value (%s) %s than defined error limit (%s) ", \
    .detail                         = "The listed sensor reports a value greater or lower than its limits", \
    .resolution                     = "If the temperature is greater than the limit, increase the cooling of the board. If the temperature is lower than the limit, are you monitoring existing sensors? Check sensor availability via xbutil query. Or review limit set in test JSON file. See User Guide for more info", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_017 Message_t{ .id = "MGT_017", .state = "obsolete" }
#define MSG_MGT_018 Message_t{ .id = "MGT_018", .state = "obsolete" }
#define MSG_MGT_019 Message_t{ \
    .id                             = "MGT_019", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "<source type> source <display name>: operational value (<instant value>) <greater/lower> than defined abort limit (<limit>) ", \
    .msg_edit                       = "%s source %s: operational value (%s) %s than defined abort limit (%s) ", \
    .detail                         = "The listed sensor reports a value greater or lower than its limits", \
    .resolution                     = "If the temperature is greater than the limit, increase the cooling of the board. If the temperature is lower than the limit, are you monitoring existing sensors? Check sensor availability via xbutil query. Or review limit set in test JSON file. See User Guide for more info", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_020 Message_t{ .id = "MGT_020", .state = "obsolete" }
#define MSG_MGT_021 Message_t{ .id = "MGT_021", .state = "obsolete" }
#define MSG_MGT_022 Message_t{ \
    .id                             = "MGT_022", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "<source type> source <source id>: source was not found in device info", \
    .msg_edit                       = "%s source %s: source was not found in device info", \
    .detail                         = "The listed sensor was not found in device info query", \
    .resolution                     = "Check the info reported by device info query", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_023 Message_t{ \
    .id                             = "MGT_023", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Percentage of failing device info queries (<percent> %) greater than defined error threshold (<threshold> %): <qty fail> failed over <qty query> queries", \
    .msg_edit                       = "Percentage of failing device info queries (%s %) greater than defined error threshold (%s %): %s failed over %s queries", \
    .detail                         = "The quantity of failing device info queries is too high", \
    .resolution                     = "Check the info reported by device info query", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MGT_024 Message_t{ \
    .id                             = "MGT_024", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Percentage of failing device info queries (<percent> %) greater than defined warning threshold (<threshold> %): <qty fail> failed over <qty query> queries", \
    .msg_edit                       = "Percentage of failing device info queries (%s %) greater than defined warning threshold (%s %): %s failed over %s queries", \
    .detail                         = "The quantity of failing device info queries is too high", \
    .resolution                     = "Check the info reported by device info query", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_001 Message_t{ \
    .id                             = "MEM_001", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Unable to run memory test because <error>", \
    .msg_edit                       = "Unable to run memory test because %s", \
    .detail                         = "This could be because the memory type or the number of channels has not been detected correctly", \
    .resolution                     = "Check the HW design configuration or contact Xilinx Support", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_002 Message_t{ .id = "MEM_002", .state = "obsolete" }
#define MSG_MEM_003 Message_t{ .id = "MEM_003", .state = "obsolete" }
#define MSG_MEM_004 Message_t{ \
    .id                             = "MEM_004", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Provided <block size name> (<block size value> MB) is not a multiple of <burst size name> (<burst size value> Bytes) ", \
    .msg_edit                       = "Provided %s (%s MB) is not a multiple of %s (%s Bytes) ", \
    .detail                         = "The block size must be a multiple of the burst size as partial bursts are not supported", \
    .resolution                     = "Check documentation for the definition of block size and burst size", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_005 Message_t{ \
    .id                             = "MEM_005", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "Saving measurement not supported for this type of test: <test type>.", \
    .msg_edit                       = "Saving measurement not supported for this type of test:  %s", \
    .detail                         = "Bandwidth measurement & results cannot be stored in CSV file for the test type used", \
    .resolution                     = "Value can be stored in CSV file for the following test type: alternate_wr_rd, only_rd, only_wr, simultaneous_wr_rd" \
}
#define MSG_MEM_006 Message_t{ .id = "MEM_006", .state = "obsolete" }
#define MSG_MEM_007 Message_t{ .id = "MEM_007", .state = "obsolete" }
#define MSG_MEM_008 Message_t{ .id = "MEM_008", .state = "obsolete" }
#define MSG_MEM_009 Message_t{ .id = "MEM_009", .state = "obsolete" }
#define MSG_MEM_010 Message_t{ .id = "MEM_010", .state = "obsolete" }
#define MSG_MEM_011 Message_t{ .id = "MEM_011", .state = "obsolete" }
#define MSG_MEM_012 Message_t{ .id = "MEM_012", .state = "obsolete" }
#define MSG_MEM_013 Message_t{ .id = "MEM_013", .state = "obsolete" }
#define MSG_MEM_014 Message_t{ .id = "MEM_014", .state = "obsolete" }
#define MSG_MEM_015 Message_t{ .id = "MEM_015", .state = "obsolete" }
#define MSG_MEM_016 Message_t{ .id = "MEM_016", .state = "obsolete" }
#define MSG_MEM_017 Message_t{ .id = "MEM_017", .state = "obsolete" }
#define MSG_MEM_018 Message_t{ .id = "MEM_018", .state = "obsolete" }
#define MSG_MEM_019 Message_t{ \
    .id                             = "MEM_019", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "1 sec HW toggle not detected <number of times> times consecutively", \
    .msg_edit                       = "1 sec HW toggle not detected %s times consecutively", \
    .detail                         = "The host application waits for a status bit to toggle once per second to get the measurement from the xbtest HW IPs. The host application has not detected a toggle of the status bit more than 5 times consecutively", \
    .resolution                     = "Check the communication between the board and the host", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_020 Message_t{ .id = "MEM_020", .state = "obsolete" }
#define MSG_MEM_021 Message_t{ .id = "MEM_021", .state = "obsolete" }
#define MSG_MEM_022 Message_t{ \
    .id                             = "MEM_022", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Data integrity not checked as this test is write only or data integrity check is disabled", \
    .msg_edit                       = "", \
    .detail                         = "The host application does not check the data integrity for this test as no data is read from the memory" \
}
#define MSG_MEM_023 Message_t{ \
    .id                             = "MEM_023", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Data integrity test fail <optional channel info>", \
    .msg_edit                       = "Data integrity test fail %s", \
    .detail                         = "Global message reporting that the data integrity was not maintained during the test. An error has been detected in the read data", \
    .resolution                     = "Check previous error for details about which section of the test failed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_024 Message_t{ \
    .id                             = "MEM_024", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Data integrity test pass <optional channel info>", \
    .msg_edit                       = "Data integrity test pass %s", \
    .detail                         = "Global message reporting that the data integrity was maintained during the test (read data matches write data). This is applicable only if the test includes reading the memory." \
}
#define MSG_MEM_025 Message_t{ \
    .id                             = "MEM_025", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Bandwidth and/or latency test fail <optional channel info>", \
    .msg_edit                       = "Bandwidth and/or latency test fail %s", \
    .detail                         = "Global message reporting that the bandwidth measured was not as expected", \
    .resolution                     = "Try to increase the duration of the test to have a more accurate value or check if clock throttling was activated", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_026 Message_t{ \
    .id                             = "MEM_026", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Bandwidth and latency test pass <optional channel info>", \
    .msg_edit                       = "Bandwidth and latency test pass %s", \
    .detail                         = "Global message reporting that the bandwidth measured was as expected" \
}
#define MSG_MEM_027 Message_t{ .id = "MEM_027", .state = "obsolete" }
#define MSG_MEM_028 Message_t{ .id = "MEM_028", .state = "obsolete" }
#define MSG_MEM_029 Message_t{ .id = "MEM_029", .state = "obsolete" }
#define MSG_MEM_030 Message_t{ .id = "MEM_030", .state = "obsolete" }
#define MSG_MEM_031 Message_t{ \
    .id                             = "MEM_031", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Memory testcase disabled using a parameter of test JSON file <optional channel info>", \
    .msg_edit                       = "Memory testcase disabled using a parameter of test JSON file %s", \
    .detail                         = "Some or all channels are disabled. If all channels are disabled, the test will stop without error" \
}
#define MSG_MEM_032 Message_t{ .id = "MEM_032", .state = "obsolete" }
#define MSG_MEM_033 Message_t{ .id = "MEM_033", .state = "obsolete" }
#define MSG_MEM_034 Message_t{ \
    .id                             = "MEM_034", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Unable to compute valid (uint) maximum total number of transfers (<computed max>) because of the values of memory size per port (<mem size> bytes) and AXI data size (<AXI data size> bytes) ", \
    .msg_edit                       = "Unable to compute valid (uint) maximum total number of transfers (%s) because of the values of memory size per port (%s bytes) and AXI data size (%s bytes) ", \
    .detail                         = "The Memory xbtest HW IP configuration is not supported because either the memory size is not a multiple of the AXI data size or the memory size is too large", \
    .resolution                     = "Try to reduce the memory size per port or increase AXI data size", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_035 Message_t{ \
    .id                             = "MEM_035", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Selected write and read memory areas are not valid. <error>: write memory area = [<start MB>, <end MB>], read memory area = [<start MB>, <end MB>]", \
    .msg_edit                       = "Selected write and read memory areas are not valid. %s: write memory area = [%s, %s], read memory area = [%s, %s]", \
    .detail                         = "In alternate mode, the memory area written must be the same as the memory are read. For simultaneous mode, the memory area written cannot overlap the memory area read", \
    .resolution                     = "Check documentation for start address and block size definition", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_036 Message_t{ \
    .id                             = "MEM_036", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "Writing memory before the <test mode> test starts <optional channel info>", \
    .msg_edit                       = "Writing memory before the %s test starts %s", \
    .detail                         = "The xbtest HW IP starts writing the memory with a PRBS for the data integrity to be checked during next test. Only the memory area read during next test is written", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MEM_037 Message_t{ \
    .id                             = "MEM_037", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Memory successfully written before the test starts", \
    .msg_edit                       = "Memory successfully written before the test starts", \
    .detail                         = "The xbtest HW IP was able to write the target memory", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MEM_038 Message_t{ \
    .id                             = "MEM_038", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Error detected writing memory before the test starts", \
    .msg_edit                       = "Error detected writing memory before the test starts", \
    .detail                         = "The xbtest HW IP was not able to write the target memory", \
    .resolution                     = "Check previous error for details about which section of the test failed", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_039 Message_t{ \
    .id                             = "MEM_039", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Provided <parameter name> (<parameter value> <unit>) is not a multiple of AXI data size (<AXI data size value> Bytes) ", \
    .msg_edit                       = "Provided %s (%s %s) is not a multiple of AXI data size (%s Bytes) ", \
    .detail                         = "The burst/block size must be a multiple of the AXI data size", \
    .resolution                     = "Check documentation for the definition of block size and burst size", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_040 Message_t{ \
    .id                             = "MEM_040", \
    .state                          = "used", \
    .log_level                      = LOG_INFO, \
    .msg                            = "<data integrity, bandwidth and latency results>", \
    .msg_edit                       = "%s", \
    .detail                         = "At the end of each test, the data integrity, the average write/read bandwidth and latency results are displayed for each channel. Measurements are marked with *** when outside the expected range" \
}
#define MSG_MEM_041 Message_t{ \
    .id                             = "MEM_041", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Test setup phase failed", \
    .msg_edit                       = "", \
    .detail                         = "An error occurred during the setup phase. The test itself will not be run", \
    .resolution                     = "Check the preceding error message printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_042 Message_t{ \
    .id                             = "MEM_042", \
    .state                          = "used", \
    .log_level                      = LOG_PASS, \
    .msg                            = "Test setup phase passed", \
    .msg_edit                       = "", \
    .detail                         = "The test itself will be run now", \
    .resolution                     = "", \
    .display_classic_console        = false \
}
#define MSG_MEM_043 Message_t{ .id = "MEM_043", .state = "obsolete" }
#define MSG_MEM_044 Message_t{ \
    .id                             = "MEM_044", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Test configuration not updated in the xbtest HW IP after <timeout> seconds. Waiting", \
    .msg_edit                       = "Test configuration not updated in the xbtest HW IP after %s seconds. Waiting", \
    .detail                         = "Although a new test configuration has been sent to the xbtest HW IP, the host application is still waiting for the xbtest HW IP configuration to be updated. Some of the channels are still running the previous configuration", \
    .resolution                     = "Is the bandwidth of the memory under test unexpectedly low? Switching between test configuration is only permitted at the end of the read/write cycle (cycle = entire range of the memory). Isolate the sequence that creates this lock-up condition and then contact support for further assistance" \
}
#define MSG_MEM_045 Message_t{ \
    .id                             = "MEM_045", \
    .state                          = "used", \
    .log_level                      = LOG_ERROR, \
    .msg                            = "Timeout! Test configuration not updated in the xbtest HW IP after %s seconds", \
    .msg_edit                       = "Timeout! Test configuration not updated in the xbtest HW IP after %s seconds", \
    .detail                         = "Although a new test configuration has been sent to the xbtest HW IP, some of the channels are still running the previous configuration", \
    .resolution                     = "Is the bandwidth of the memory under test unexpectedly low? Switching between test configuration is only permitted at the end of the read/write cycle (cycle = entire range of the memory). Isolate the sequence that creates this lock-up condition and then contact support for further assistance", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_046 Message_t{ \
    .id                             = "MEM_046", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "All memories have been disabled, please make sure that at least one memory is tested", \
    .msg_edit                       = "", \
    .detail                         = "All memories have been disabled, no test can be performed", \
    .resolution                     = "Test at least 1 memory or remove the memory testcase from the test JSON file", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_047 Message_t{ \
    .id                             = "MEM_047", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "The host application was not able to read xbtest HW IP measurements every second. <quantity> measurements were done during <duration> seconds of test", \
    .msg_edit                       = "The host application was not able to read xbtest HW IP measurements every second. %s measurements were done during %s seconds of test", \
    .detail                         = "The average BW and latency results might not be accurate", \
    .resolution                     = "If needed, increase the test duration to have more precise measurements. Check if xbtest has enough OS resource to run", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_048 Message_t{ \
    .id                             = "MEM_048", \
    .state                          = "used", \
    .log_level                      = LOG_CRIT_WARN, \
    .msg                            = "Current HW 1 sec timestamp (<current timestamp>) does not equal expected (<expected timestamp>) <optional channel info>", \
    .msg_edit                       = "Current HW 1 sec timestamp (%s) does not equal expected (%s) %s", \
    .detail                         = "The average BW and latency results might not be accurate", \
    .resolution                     = "If needed, increase the test duration to have more precise measurements. Check if xbtest has enough OS resource to run" \
}
#define MSG_MEM_049 Message_t{ \
    .id                             = "MEM_049", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to read <IP name> status. AXI pointers mismatch: Single register read (0x<value>) vs register dump read (0x<value>) <optional channel info>", \
    .msg_edit                       = "Failed to read <IP name> status. AXI pointers mismatch: Single register read (0x%s) vs register dump read (0x%s) %s", \
    .detail                         = "The host application was not able to access the Memory xbtest HW IP status registers. Verify task failed.", \
    .resolution                     = "Contact Xilinx Support", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_050 Message_t{ .id = "MEM_050", .state = "obsolete" }
#define MSG_MEM_051 Message_t{ \
    .id                             = "MEM_051", \
    .state                          = "used", \
    .log_level                      = LOG_WARN, \
    .msg                            = "A power testcase was potentially running in parallel to the memory testcase, which might have activated clock throttling if feature applicable, and impacting the memory testcase", \
    .msg_edit                       = "", \
    .detail                         = "The memory test is not running in optimal conditions", \
    .resolution                     = "Check if clock throttling was activated", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_052 Message_t{ \
    .id                             = "MEM_052", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid value (<value>) provided for parameter <param> <optional channel info>. Expected value is string 32-bit hexadecimal value starting with 0x and with maximum 8 hexadecimal char", \
    .msg_edit                       = "Invalid value (%s) provided for parameter %s %s. Expected value is string 32-bit hexadecimal value starting with 0x and with maximum 8 hexadecimal char", \
    .detail                         = "The value provided does not match the expected format for this parameter", \
    .resolution                     = "Provide value with the expected format", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_053 Message_t{ \
    .id                             = "MEM_053", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "At least <num> patterns must be provided for <control> pattern <optional channel info>", \
    .msg_edit                       = "At least %s patterns must be provided for %s pattern %s", \
    .detail                         = "The value provided does not match the expected format for this parameter", \
    .resolution                     = "Provide value with the expected format", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_MEM_054 Message_t{ \
    .id                             = "MEM_054", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Parameters <write/read> Memory xbtest HW IP rate and requested BW are incompatible", \
    .msg_edit                       = "Parameters %s Memory xbtest HW IP rate and requested BW are incompatible", \
    .detail                         = "Requested bandwidth is used to compute the Memory xbtest HW IP rate. Both parameters cannot be used simultaneously", \
    .resolution                     = "Only use Memory xbtest HW IP rate or requested bandwidth, but not both", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true, \
    .num_display_dynamic_console    = 1 \
}
#define MSG_DEBUG_EXCEPTION Message_t{ \
    .id                             = "DBG_000", \
    .state                          = "used", \
    .log_level                      = LOG_DEBUG_EXCEPTION, \
    .msg                            = "<content>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message reports debug information regarding errors or unexpected behaviours" \
}
#define MSG_DEBUG_TESTCASE Message_t{ \
    .id                             = "DBG_001", \
    .state                          = "used", \
    .log_level                      = LOG_DEBUG_TESTCASE, \
    .msg                            = "<content>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message reports debug information for test cases and tasks" \
}
#define MSG_DEBUG_SETUP Message_t{ \
    .id                             = "DBG_002", \
    .state                          = "used", \
    .log_level                      = LOG_DEBUG_SETUP, \
    .msg                            = "<content>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message reports debug information for host application setup phase" \
}
#define MSG_DEBUG_OPENCL Message_t{ \
    .id                             = "DBG_003", \
    .state                          = "used", \
    .log_level                      = LOG_DEBUG_OPENCL, \
    .msg                            = "<content>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message reports debug information for device access related calls" \
}
#define MSG_DEBUG_PARSING Message_t{ \
    .id                             = "DBG_004", \
    .state                          = "used", \
    .log_level                      = LOG_DEBUG_PARSING, \
    .msg                            = "<content>", \
    .msg_edit                       = "%s", \
    .detail                         = "This message reports debug information for parsing steps like JSON, command line, etc" \
}

// The following messages (id="MSG_XXX") are not defined in MESSAGES_JSON as they are printed when an error occurs in the message parser
#define MSG_DEF_001 Message_t{ \
    .id                             = "MSG_001", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Failed to get messages definition", \
    .msg_edit                       = "", \
    .detail                         = "The host application was not able to parse the message definition", \
    .resolution                     = "Check error messages printed before this error", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_DEF_002 Message_t{ .id = "MSG_002", .state = "obsolete" }
#define MSG_DEF_003 Message_t{ .id = "MSG_003", .state = "obsolete" }
#define MSG_DEF_004 Message_t{ \
    .id                             = "MSG_004", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Value not defined for field: <field name>", \
    .msg_edit                       = "Value not defined for field: %s", \
    .detail                         = "A non-empty value is required for fields: id, severity, msg, detail, state", \
    .resolution                     = "Add missing value of the field", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_DEF_005 Message_t{ \
    .id                             = "MSG_005", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid message definition. Message ID already defined: <id>", \
    .msg_edit                       = "Invalid message definition. Message ID already defined: %s", \
    .detail                         = "The member id in the message definition must be unique", \
    .resolution                     = "Find where the id has been set more than once and define new id", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_DEF_006 Message_t{ \
    .id                             = "MSG_006", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid message log_level: <log_level>", \
    .msg_edit                       = "Invalid message log_level: %s", \
    .detail                         = "The value of member log_level is invalid. The allowed log_level are described in the message information specification. No other log_level allowed.", \
    .resolution                     = "Find where the invalid log_level has been defined and correct its value", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_DEF_007 Message_t{ \
    .id                             = "MSG_007", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Invalid message state: <state>", \
    .msg_edit                       = "Invalid message state: %s", \
    .detail                         = "The value of member state is invalid. The field state must be defined in lower case. The allowed state are described in the message information specification. No other state allowed.", \
    .resolution                     = "Find where the invalid state has been defined and correct its value", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_DEF_008 Message_t{ \
    .id                             = "MSG_008", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "Found unexpected resolution for message with severity <severity> and ID <id>", \
    .msg_edit                       = "Found unexpected resolution for message with severity %s and ID %s", \
    .detail                         = "The value of member resolution should remain empty for messages of severity INFO, STATUS and PASS", \
    .resolution                     = "Remove resolution of message", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_DEF_009 Message_t{ \
    .id                             = "MSG_009", \
    .state                          = "used", \
    .log_level                      = LOG_FAILURE, \
    .msg                            = "No resolution found for message with severity <severity> and ID <id>", \
    .msg_edit                       = "No resolution found for message with severity %s and ID %s", \
    .detail                         = "The value of member resolution should not be empty for messages of severity WARNING, CRIT_WARN, ERROR, FAILURE", \
    .resolution                     = "Add resolution to message", \
    .display_classic_console        = true, \
    .display_dynamic_console        = true \
}
#define MSG_DEF_010 Message_t{ .id = "MSG_010", .state = "obsolete" }
#define MSG_DEF_011 Message_t{ .id = "MSG_011", .state = "obsolete" }
#define MSG_DEF_012 Message_t{ .id = "MSG_012", .state = "obsolete" }
#define MSG_DEF_013 Message_t{ .id = "MSG_013", .state = "obsolete" }

} // namespace

#endif /* _MESSAGES_H */
