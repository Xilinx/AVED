#!/usr/bin/env bash
# (c) Copyright 2024, Advanced Micro Devices, Inc.
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
set -Eeuo pipefail

# Init
DESIGN="amd_v80_gen5x8_23.2_exdes_2"
HW_DIR=$(realpath ./)
FW_DIR=$(realpath ./../../fw/AMC)
XSA=${XSA:-$(realpath ${HW_DIR})/build/${DESIGN}.xsa}

# Step HW
pushd ${HW_DIR}
  mkdir -p ./build
  vivado -source src/create_design.tcl -source src/build_design.tcl -mode batch -nojournal -log ./build/vivado.log
popd

# Step FW
pushd ${FW_DIR}
  ./scripts/build.sh -os freertos10_xilinx -profile v80 -xsa $XSA
  cp -a ${FW_DIR}/build/amc.elf ${HW_DIR}/build
  # Takes in fpt.json and produces fpt.bin
popd


# Step FPT
pushd ${FW_DIR}/build
  ../scripts/gen_fpt.py -f ../scripts/fpt.json
  cp -a ${FW_DIR}/build/fpt.bin ${HW_DIR}/build
popd

# Step PDI combine
# Generate PDI w/ bootgen
pushd ${HW_DIR}
  bootgen -arch versal -image ${HW_DIR}/fpt/pdi_combine.bif -w -o ${HW_DIR}/build/${DESIGN}_nofpt.pdi
popd

# final pdi generation
${HW_DIR}/fpt/fpt_pdi_gen.py --fpt ${HW_DIR}/build/fpt.bin --pdi ${HW_DIR}/build/${DESIGN}_nofpt.pdi --output ${DESIGN}.pdi
