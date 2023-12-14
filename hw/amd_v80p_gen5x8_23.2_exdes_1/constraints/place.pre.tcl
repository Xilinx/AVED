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

# Connect the DMA reset detection signal to the PMC Interrupt input to allow a full PDI reload to be triggered on PCIe hot reset
set PS9_IRQ_pin [get_pins -of [get_cells -hierarchical PS9_inst -filter { PARENT =~ "top_i/cips*"}] -filter { REF_PIN_NAME =~ "PMCPLIRQ[4]"}]
if {[llength ${PS9_IRQ_pin}] == 1} {
    disconnect_net -objects ${PS9_IRQ_pin}
    connect_net -hierarchical -net [get_nets -of [get_pins top_i/clock_reset/pcie_mgmt_pdi_reset/and_0/Res]] -objects ${PS9_IRQ_pin}
} else {
    puts "Unable to get PMCPLIRQ pin for Force Reset rewiring."
}

