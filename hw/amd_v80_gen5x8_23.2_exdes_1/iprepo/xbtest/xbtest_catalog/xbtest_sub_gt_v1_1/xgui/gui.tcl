# Copyright (C) 2022 Xilinx, Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Definitional proc to organize widgets for parameters.

proc init_gui { IPINST } {

    ipgui::add_param $IPINST -name "Component_Name"

    #---> Adding Page ----------------------------------------------------------------------------------------------------

    set Overview [ipgui::add_page $IPINST -name "Overview"]

    ipgui::add_static_text $IPINST -name paragraph_1 -parent $Overview -text "xbtest XXV & GT Subsystem"

    ipgui::add_param $IPINST -name "C_BOARD" -parent ${Overview}
    ipgui::add_param $IPINST -name "GT_GROUP_SELECT" -parent ${Overview}
    ipgui::add_param $IPINST -name "ENABLE_RSFEC" -parent ${Overview}
    ipgui::add_param $IPINST -name "GT_TYPE" -parent ${Overview}

}

# general parameters ###################################################################################################

proc update_PARAM_VALUE.GT_GROUP_SELECT { PARAM_VALUE.GT_GROUP_SELECT } {
    # Procedure called to update GT_GROUP_SELECT when any of the dependent parameters in the arguments change
}
proc validate_PARAM_VALUE.GT_GROUP_SELECT { PARAM_VALUE.GT_GROUP_SELECT } {
    # Procedure called to validate GT_GROUP_SELECT
    return true
}
proc update_MODELPARAM_VALUE.GT_GROUP_SELECT { MODELPARAM_VALUE.GT_GROUP_SELECT PARAM_VALUE.GT_GROUP_SELECT } {
     # Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
     set_property value [get_property value ${PARAM_VALUE.GT_GROUP_SELECT}] ${MODELPARAM_VALUE.GT_GROUP_SELECT}
}

proc update_PARAM_VALUE.C_BOARD { PARAM_VALUE.C_BOARD } {
    # Procedure called to update C_BOARD when any of the dependent parameters in the arguments change
}
proc validate_PARAM_VALUE.C_BOARD { PARAM_VALUE.C_BOARD } {
    # Procedure called to validate C_BOARD
    return true
}
proc update_MODELPARAM_VALUE.C_BOARD { MODELPARAM_VALUE.C_BOARD PARAM_VALUE.C_BOARD } {
     # Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
     set_property value [get_property value ${PARAM_VALUE.C_BOARD}] ${MODELPARAM_VALUE.C_BOARD}
}

proc update_PARAM_VALUE.GT_TYPE { PARAM_VALUE.GT_TYPE } {
    # Procedure called to update GT_TYPE when any of the dependent parameters in the arguments change
}
proc validate_PARAM_VALUE.GT_TYPE { PARAM_VALUE.GT_TYPE } {
    # Procedure called to validate GT_TYPE
    return true
}
proc update_MODELPARAM_VALUE.GT_TYPE { MODELPARAM_VALUE.GT_TYPE PARAM_VALUE.GT_TYPE } {
     # Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
     set_property value [get_property value ${PARAM_VALUE.GT_TYPE}] ${MODELPARAM_VALUE.GT_TYPE}
}
