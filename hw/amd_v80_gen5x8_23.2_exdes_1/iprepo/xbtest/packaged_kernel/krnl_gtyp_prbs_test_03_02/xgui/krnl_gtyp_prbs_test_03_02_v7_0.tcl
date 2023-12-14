# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  set Component_Name [ipgui::add_param $IPINST -name "Component_Name"]
  set_property tooltip {The example project will be called <Component Name>_ex, otherwise this value is unused.} ${Component_Name}
  ipgui::add_static_text $IPINST -name "Static_Text" -text {Customize wizard via TCL console before openning example design. See xbtest platform developer guide}

}

proc update_PARAM_VALUE.SLR_ASSIGNMENTS { PARAM_VALUE.SLR_ASSIGNMENTS } {
	# Procedure called to update SLR_ASSIGNMENTS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.SLR_ASSIGNMENTS { PARAM_VALUE.SLR_ASSIGNMENTS } {
	# Procedure called to validate SLR_ASSIGNMENTS
	return true
}

proc update_PARAM_VALUE.xpfm { PARAM_VALUE.xpfm } {
	# Procedure called to update xpfm when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.xpfm { PARAM_VALUE.xpfm } {
	# Procedure called to validate xpfm
	return true
}

proc update_PARAM_VALUE.wizard_config_json { PARAM_VALUE.wizard_config_json } {
	# Procedure called to update wizard_config_json when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.wizard_config_json { PARAM_VALUE.wizard_config_json } {
	# Procedure called to validate wizard_config_json
	return true
}

proc update_PARAM_VALUE.wizard_config_name { PARAM_VALUE.wizard_config_name } {
	# Procedure called to update wizard_config_name when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.wizard_config_name { PARAM_VALUE.wizard_config_name } {
	# Procedure called to validate wizard_config_name
	return true
}

proc update_PARAM_VALUE.C_BUILD_VERSION { PARAM_VALUE.C_BUILD_VERSION } {
	# Procedure called to update C_BUILD_VERSION when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_BUILD_VERSION { PARAM_VALUE.C_BUILD_VERSION } {
	# Procedure called to validate C_BUILD_VERSION
	return true
}

proc update_PARAM_VALUE.C_INTERNAL_RELEASE { PARAM_VALUE.C_INTERNAL_RELEASE } {
	# Procedure called to update C_INTERNAL_RELEASE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_INTERNAL_RELEASE { PARAM_VALUE.C_INTERNAL_RELEASE } {
	# Procedure called to validate C_INTERNAL_RELEASE
	return true
}

proc update_PARAM_VALUE.C_INIT { PARAM_VALUE.C_INIT } {
	# Procedure called to update C_INIT when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_INIT { PARAM_VALUE.C_INIT } {
	# Procedure called to validate C_INIT
	return true
}

proc update_PARAM_VALUE.MESSAGES_DICT { PARAM_VALUE.MESSAGES_DICT } {
	# Procedure called to update MESSAGES_DICT when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.MESSAGES_DICT { PARAM_VALUE.MESSAGES_DICT } {
	# Procedure called to validate MESSAGES_DICT
	return true
}


proc update_MODELPARAM_VALUE.C_BUILD_VERSION { MODELPARAM_VALUE.C_BUILD_VERSION PARAM_VALUE.C_BUILD_VERSION } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.C_BUILD_VERSION}] ${MODELPARAM_VALUE.C_BUILD_VERSION}
}

proc update_MODELPARAM_VALUE.C_GT_INDEX { MODELPARAM_VALUE.C_GT_INDEX } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_INDEX". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_INDEX}
}

proc update_MODELPARAM_VALUE.C_MEM_KRNL_INST { MODELPARAM_VALUE.C_MEM_KRNL_INST } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_MEM_KRNL_INST". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_MEM_KRNL_INST}
}

proc update_MODELPARAM_VALUE.C_NUM_USED_M_AXI { MODELPARAM_VALUE.C_NUM_USED_M_AXI } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_NUM_USED_M_AXI". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_NUM_USED_M_AXI}
}

proc update_MODELPARAM_VALUE.C_MEM_TYPE { MODELPARAM_VALUE.C_MEM_TYPE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_MEM_TYPE". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_MEM_TYPE}
}

proc update_MODELPARAM_VALUE.C_USE_AXI_ID { MODELPARAM_VALUE.C_USE_AXI_ID } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_USE_AXI_ID". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_USE_AXI_ID}
}

proc update_MODELPARAM_VALUE.C_USE_AIE { MODELPARAM_VALUE.C_USE_AIE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_USE_AIE". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_USE_AIE}
}

proc update_MODELPARAM_VALUE.C_AXIS_AIE_DATA_WIDTH { MODELPARAM_VALUE.C_AXIS_AIE_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_AXIS_AIE_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 128 ${MODELPARAM_VALUE.C_AXIS_AIE_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_THROTTLE_MODE { MODELPARAM_VALUE.C_THROTTLE_MODE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_THROTTLE_MODE". Setting updated value from the model parameter.
set_property value 1 ${MODELPARAM_VALUE.C_THROTTLE_MODE}
}

proc update_MODELPARAM_VALUE.C_GT_RATE { MODELPARAM_VALUE.C_GT_RATE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE}
}

proc update_MODELPARAM_VALUE.C_GT_TYPE { MODELPARAM_VALUE.C_GT_TYPE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_TYPE". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_TYPE}
}

proc update_MODELPARAM_VALUE.C_GT_MAC_IP_SEL { MODELPARAM_VALUE.C_GT_MAC_IP_SEL } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_MAC_IP_SEL". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_MAC_IP_SEL}
}

proc update_MODELPARAM_VALUE.C_GT_MAC_ENABLE_RSFEC { MODELPARAM_VALUE.C_GT_MAC_ENABLE_RSFEC } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_MAC_ENABLE_RSFEC". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_MAC_ENABLE_RSFEC}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL { MODELPARAM_VALUE.C_GT_IP_SEL } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_00 { MODELPARAM_VALUE.C_GT_IP_SEL_00 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_00". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_00}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_01 { MODELPARAM_VALUE.C_GT_IP_SEL_01 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_01". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_01}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_02 { MODELPARAM_VALUE.C_GT_IP_SEL_02 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_02". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_02}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_03 { MODELPARAM_VALUE.C_GT_IP_SEL_03 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_03". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_03}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_04 { MODELPARAM_VALUE.C_GT_IP_SEL_04 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_04". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_04}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_08 { MODELPARAM_VALUE.C_GT_IP_SEL_08 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_08". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_08}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_09 { MODELPARAM_VALUE.C_GT_IP_SEL_09 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_09". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_09}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_10 { MODELPARAM_VALUE.C_GT_IP_SEL_10 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_10". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_10}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_11 { MODELPARAM_VALUE.C_GT_IP_SEL_11 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_11". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_11}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_12 { MODELPARAM_VALUE.C_GT_IP_SEL_12 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_12". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_12}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_13 { MODELPARAM_VALUE.C_GT_IP_SEL_13 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_13". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_13}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_14 { MODELPARAM_VALUE.C_GT_IP_SEL_14 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_14". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_14}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_15 { MODELPARAM_VALUE.C_GT_IP_SEL_15 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_15". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_15}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_16 { MODELPARAM_VALUE.C_GT_IP_SEL_16 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_16". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_16}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_17 { MODELPARAM_VALUE.C_GT_IP_SEL_17 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_17". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_17}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_18 { MODELPARAM_VALUE.C_GT_IP_SEL_18 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_18". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_18}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_19 { MODELPARAM_VALUE.C_GT_IP_SEL_19 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_19". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_19}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_20 { MODELPARAM_VALUE.C_GT_IP_SEL_20 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_20". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_20}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_21 { MODELPARAM_VALUE.C_GT_IP_SEL_21 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_21". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_21}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_22 { MODELPARAM_VALUE.C_GT_IP_SEL_22 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_22". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_22}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_23 { MODELPARAM_VALUE.C_GT_IP_SEL_23 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_23". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_23}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_24 { MODELPARAM_VALUE.C_GT_IP_SEL_24 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_24". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_24}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_25 { MODELPARAM_VALUE.C_GT_IP_SEL_25 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_25". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_25}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_26 { MODELPARAM_VALUE.C_GT_IP_SEL_26 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_26". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_26}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_27 { MODELPARAM_VALUE.C_GT_IP_SEL_27 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_27". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_27}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_28 { MODELPARAM_VALUE.C_GT_IP_SEL_28 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_28". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_28}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_29 { MODELPARAM_VALUE.C_GT_IP_SEL_29 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_29". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_29}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_30 { MODELPARAM_VALUE.C_GT_IP_SEL_30 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_30". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_30}
}

proc update_MODELPARAM_VALUE.C_GT_IP_SEL_31 { MODELPARAM_VALUE.C_GT_IP_SEL_31 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_IP_SEL_31". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_IP_SEL_31}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_00 { MODELPARAM_VALUE.C_GT_RATE_00 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_00". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_00}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_01 { MODELPARAM_VALUE.C_GT_RATE_01 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_01". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_01}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_02 { MODELPARAM_VALUE.C_GT_RATE_02 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_02". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_02}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_03 { MODELPARAM_VALUE.C_GT_RATE_03 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_03". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_03}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_04 { MODELPARAM_VALUE.C_GT_RATE_04 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_04". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_04}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_08 { MODELPARAM_VALUE.C_GT_RATE_08 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_08". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_08}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_09 { MODELPARAM_VALUE.C_GT_RATE_09 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_09". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_09}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_10 { MODELPARAM_VALUE.C_GT_RATE_10 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_10". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_10}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_11 { MODELPARAM_VALUE.C_GT_RATE_11 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_11". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_11}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_12 { MODELPARAM_VALUE.C_GT_RATE_12 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_12". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_12}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_13 { MODELPARAM_VALUE.C_GT_RATE_13 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_13". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_13}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_14 { MODELPARAM_VALUE.C_GT_RATE_14 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_14". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_14}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_15 { MODELPARAM_VALUE.C_GT_RATE_15 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_15". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_15}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_16 { MODELPARAM_VALUE.C_GT_RATE_16 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_16". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_16}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_17 { MODELPARAM_VALUE.C_GT_RATE_17 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_17". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_17}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_18 { MODELPARAM_VALUE.C_GT_RATE_18 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_18". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_18}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_19 { MODELPARAM_VALUE.C_GT_RATE_19 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_19". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_19}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_20 { MODELPARAM_VALUE.C_GT_RATE_20 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_20". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_20}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_21 { MODELPARAM_VALUE.C_GT_RATE_21 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_21". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_21}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_22 { MODELPARAM_VALUE.C_GT_RATE_22 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_22". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_22}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_23 { MODELPARAM_VALUE.C_GT_RATE_23 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_23". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_23}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_24 { MODELPARAM_VALUE.C_GT_RATE_24 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_24". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_24}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_25 { MODELPARAM_VALUE.C_GT_RATE_25 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_25". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_25}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_26 { MODELPARAM_VALUE.C_GT_RATE_26 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_26". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_26}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_27 { MODELPARAM_VALUE.C_GT_RATE_27 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_27". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_27}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_28 { MODELPARAM_VALUE.C_GT_RATE_28 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_28". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_28}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_29 { MODELPARAM_VALUE.C_GT_RATE_29 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_29". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_29}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_30 { MODELPARAM_VALUE.C_GT_RATE_30 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_30". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_30}
}

proc update_MODELPARAM_VALUE.C_GT_RATE_31 { MODELPARAM_VALUE.C_GT_RATE_31 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_GT_RATE_31". Setting updated value from the model parameter.
set_property value 0 ${MODELPARAM_VALUE.C_GT_RATE_31}
}

proc update_MODELPARAM_VALUE.C_DNA_READ { MODELPARAM_VALUE.C_DNA_READ } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_DNA_READ". Setting updated value from the model parameter.
set_property value 1 ${MODELPARAM_VALUE.C_DNA_READ}
}

proc update_MODELPARAM_VALUE.C_S_AXI_CONTROL_ADDR_WIDTH { MODELPARAM_VALUE.C_S_AXI_CONTROL_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_S_AXI_CONTROL_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 12 ${MODELPARAM_VALUE.C_S_AXI_CONTROL_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_S_AXI_CONTROL_DATA_WIDTH { MODELPARAM_VALUE.C_S_AXI_CONTROL_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_S_AXI_CONTROL_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 32 ${MODELPARAM_VALUE.C_S_AXI_CONTROL_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M01_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M01_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M01_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M01_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M02_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M02_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M02_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M02_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M03_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M03_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M03_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M03_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M04_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M04_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M04_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M04_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M05_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M05_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M05_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M05_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M06_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M06_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M06_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M06_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M07_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M07_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M07_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M07_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M08_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M08_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M08_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M08_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M09_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M09_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M09_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M09_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M10_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M10_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M10_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M10_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M11_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M11_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M11_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M11_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M12_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M12_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M12_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M12_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M13_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M13_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M13_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M13_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M14_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M14_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M14_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M14_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M15_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M15_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M15_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M15_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M16_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M16_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M16_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M16_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M17_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M17_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M17_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M17_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M18_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M18_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M18_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M18_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M19_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M19_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M19_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M19_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M20_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M20_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M20_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M20_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M21_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M21_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M21_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M21_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M22_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M22_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M22_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M22_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M23_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M23_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M23_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M23_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M24_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M24_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M24_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M24_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M25_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M25_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M25_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M25_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M26_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M26_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M26_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M26_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M27_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M27_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M27_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M27_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M28_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M28_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M28_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M28_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M29_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M29_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M29_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M29_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M30_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M30_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M30_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M30_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M31_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M31_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M31_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M31_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M32_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M32_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M32_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M32_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M33_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M33_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M33_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M33_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M34_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M34_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M34_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M34_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M35_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M35_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M35_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M35_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M36_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M36_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M36_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M36_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M37_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M37_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M37_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M37_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M38_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M38_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M38_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M38_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M39_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M39_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M39_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M39_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M40_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M40_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M40_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M40_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M41_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M41_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M41_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M41_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M42_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M42_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M42_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M42_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M43_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M43_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M43_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M43_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M44_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M44_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M44_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M44_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M45_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M45_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M45_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M45_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M46_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M46_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M46_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M46_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M47_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M47_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M47_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M47_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M48_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M48_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M48_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M48_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M49_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M49_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M49_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M49_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M50_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M50_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M50_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M50_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M51_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M51_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M51_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M51_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M52_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M52_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M52_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M52_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M53_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M53_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M53_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M53_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M54_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M54_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M54_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M54_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M55_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M55_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M55_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M55_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M56_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M56_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M56_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M56_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M57_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M57_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M57_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M57_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M58_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M58_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M58_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M58_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M59_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M59_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M59_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M59_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M60_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M60_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M60_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M60_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M61_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M61_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M61_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M61_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M62_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M62_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M62_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M62_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M63_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M63_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M63_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M63_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M64_AXI_THREAD_ID_WIDTH { MODELPARAM_VALUE.C_M64_AXI_THREAD_ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M64_AXI_THREAD_ID_WIDTH". Setting updated value from the model parameter.
set_property value 2 ${MODELPARAM_VALUE.C_M64_AXI_THREAD_ID_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M00_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M00_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M00_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M00_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M01_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M01_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M01_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M01_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M02_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M02_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M02_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M02_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M03_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M03_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M03_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M03_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M04_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M04_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M04_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M04_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M05_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M05_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M05_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M05_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M06_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M06_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M06_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M06_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M07_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M07_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M07_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M07_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M08_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M08_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M08_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M08_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M09_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M09_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M09_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M09_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M10_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M10_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M10_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M10_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M11_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M11_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M11_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M11_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M12_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M12_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M12_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M12_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M13_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M13_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M13_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M13_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M14_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M14_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M14_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M14_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M15_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M15_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M15_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M15_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M16_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M16_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M16_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M16_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M17_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M17_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M17_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M17_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M18_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M18_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M18_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M18_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M19_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M19_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M19_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M19_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M20_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M20_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M20_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M20_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M21_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M21_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M21_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M21_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M22_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M22_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M22_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M22_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M23_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M23_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M23_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M23_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M24_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M24_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M24_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M24_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M25_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M25_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M25_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M25_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M26_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M26_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M26_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M26_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M27_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M27_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M27_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M27_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M28_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M28_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M28_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M28_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M29_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M29_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M29_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M29_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M30_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M30_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M30_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M30_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M31_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M31_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M31_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M31_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M32_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M32_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M32_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M32_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M33_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M33_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M33_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M33_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M34_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M34_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M34_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M34_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M35_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M35_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M35_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M35_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M36_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M36_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M36_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M36_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M37_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M37_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M37_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M37_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M38_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M38_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M38_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M38_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M39_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M39_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M39_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M39_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M40_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M40_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M40_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M40_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M41_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M41_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M41_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M41_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M42_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M42_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M42_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M42_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M43_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M43_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M43_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M43_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M44_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M44_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M44_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M44_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M45_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M45_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M45_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M45_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M46_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M46_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M46_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M46_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M47_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M47_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M47_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M47_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M48_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M48_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M48_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M48_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M49_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M49_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M49_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M49_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M50_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M50_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M50_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M50_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M51_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M51_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M51_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M51_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M52_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M52_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M52_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M52_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M53_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M53_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M53_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M53_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M54_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M54_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M54_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M54_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M55_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M55_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M55_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M55_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M56_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M56_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M56_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M56_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M57_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M57_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M57_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M57_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M58_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M58_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M58_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M58_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M59_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M59_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M59_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M59_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M60_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M60_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M60_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M60_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M61_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M61_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M61_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M61_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M62_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M62_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M62_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M62_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M63_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M63_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M63_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M63_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M64_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_M64_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M64_AXI_ADDR_WIDTH". Setting updated value from the model parameter.
set_property value 64 ${MODELPARAM_VALUE.C_M64_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M00_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M00_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M00_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 32 ${MODELPARAM_VALUE.C_M00_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M01_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M01_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M01_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M01_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M02_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M02_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M02_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M02_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M03_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M03_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M03_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M03_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M04_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M04_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M04_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M04_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M05_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M05_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M05_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M05_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M06_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M06_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M06_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M06_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M07_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M07_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M07_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M07_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M08_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M08_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M08_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M08_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M09_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M09_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M09_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M09_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M10_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M10_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M10_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M10_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M11_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M11_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M11_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M11_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M12_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M12_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M12_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M12_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M13_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M13_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M13_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M13_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M14_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M14_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M14_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M14_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M15_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M15_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M15_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M15_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M16_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M16_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M16_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M16_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M17_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M17_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M17_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M17_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M18_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M18_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M18_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M18_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M19_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M19_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M19_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M19_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M20_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M20_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M20_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M20_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M21_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M21_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M21_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M21_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M22_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M22_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M22_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M22_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M23_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M23_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M23_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M23_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M24_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M24_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M24_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M24_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M25_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M25_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M25_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M25_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M26_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M26_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M26_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M26_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M27_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M27_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M27_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M27_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M28_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M28_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M28_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M28_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M29_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M29_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M29_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M29_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M30_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M30_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M30_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M30_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M31_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M31_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M31_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M31_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M32_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M32_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M32_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M32_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M33_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M33_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M33_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M33_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M34_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M34_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M34_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M34_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M35_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M35_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M35_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M35_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M36_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M36_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M36_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M36_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M37_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M37_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M37_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M37_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M38_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M38_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M38_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M38_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M39_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M39_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M39_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M39_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M40_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M40_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M40_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M40_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M41_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M41_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M41_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M41_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M42_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M42_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M42_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M42_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M43_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M43_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M43_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M43_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M44_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M44_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M44_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M44_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M45_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M45_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M45_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M45_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M46_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M46_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M46_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M46_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M47_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M47_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M47_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M47_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M48_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M48_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M48_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M48_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M49_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M49_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M49_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M49_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M50_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M50_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M50_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M50_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M51_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M51_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M51_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M51_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M52_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M52_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M52_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M52_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M53_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M53_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M53_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M53_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M54_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M54_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M54_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M54_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M55_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M55_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M55_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M55_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M56_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M56_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M56_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M56_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M57_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M57_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M57_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M57_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M58_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M58_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M58_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M58_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M59_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M59_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M59_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M59_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M60_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M60_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M60_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M60_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M61_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M61_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M61_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M61_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M62_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M62_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M62_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M62_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M63_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M63_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M63_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M63_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_M64_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_M64_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "C_M64_AXI_DATA_WIDTH". Setting updated value from the model parameter.
set_property value 512 ${MODELPARAM_VALUE.C_M64_AXI_DATA_WIDTH}
}

