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

# common.tcl - Contains a common set of procs that can be used in other script tcl

set C_KRNL_MODE_VERIFY      5
set C_KRNL_MODE_POWER       0
set C_KRNL_MODE_MEMORY      1
set C_KRNL_MODE_GT_MAC      4
set C_KRNL_MODE_GT_LPBK     3
set C_KRNL_MODE_GT_PRBS     6
set C_KRNL_MODE_GTF_PRBS    7
set C_KRNL_MODE_GTM_PRBS    8
set C_KRNL_MODE_GTYP_PRBS   9

proc dict_exist { mydict keys } {
    return [dict exist $mydict {*}$keys]
}

proc dict_get_quiet { mydict keys } {
    if {[dict_exist $mydict $keys]} {
        return [dict get $mydict {*}$keys]
    }
    return {}
}

proc dict_set { mydict_ref keys val } {
    upvar 1 $mydict_ref mydict; # Dictionary passed as ref.
    dict set mydict {*}$keys $val
}

proc dict_unset { mydict_ref keys } {
    upvar 1 $mydict_ref mydict; # Dictionary passed as ref.
    dict unset mydict {*}$keys
}

proc move_file { from to } {
    if {[file exist $from]} {
        file mkdir [file dirname $to]
        file copy $from $to
    }
}

proc write_file { filename data } {
    set fp [open $filename w]; puts $fp $data; close $fp
}
proc read_file { filename } {
    set fp [open $filename r]; set data [read $fp]; close $fp
    return $data
}

proc log_config { config_ref excludes filename } {
    upvar 1 $config_ref config
    set DATA {}
    dict for {key value} $config  {
        if {[lsearch -exact $excludes $key] == -1} {
            lappend DATA "$key=$value"
        }
    }
    if {![file exist [file dirname $filename]]} {
        file mkdir [file dirname $filename]
    }
    write_file $filename [join $DATA "\n"]
    log_message $config "XBTEST_WIZARD-16" [list $filename]; # report log file name
}

# Similar to readlink -f
proc unlink { link_file } {
  set ret_unlink $link_file
  while {[file type $ret_unlink] == "link"} {
    set ret_unlink [file readlink $ret_unlink]
  }
  return $ret_unlink
}

# Similar to chmod -R
proc chmod_recursive { dir attrib } {
    foreach filename [glob -directory $dir *] {
        file attribute $filename -permissions $attrib
        if {[file isdirectory $filename]} {
            chmod_recursive $filename $attrib
        }
    }
}

proc json2dict { txt } {
    package require json

    # Remove space and new lines before processing with ::json::json2dict
    set data    {}
    set trim    true

    foreach c [split $txt ""] {
        if {[string match {\"} $c]} {
            set trim [expr ! $trim]
        }
        if {(![string match { } $c] && ![string match "\n" $c] && ![string match "\t" $c]) || !$trim} {
            append data $c
        }
    }
    return [::json::json2dict $data]
}

proc string2json {str} {
    return "\"$str\""
}
proc list2json {listVal} {
    return "\[[join $listVal ,]\]"
}
proc strlist2json {strListVal} {
    set tmp {}
    foreach val $strListVal {
        lappend tmp [string2json $val]
    }
    return [list2json $tmp]
}

proc log_message_bash { severity id msg } {
    return "echo \"$severity: \[$id\] $msg\""
}
proc check_file_bash { filename } {
    set     cmd {}
    lappend cmd "if \[ ! -f $filename \]; then"
    lappend cmd "\t[log_message_bash "ERROR" "XBTEST_WIZARD-14" "xbtest xclbin generation failed. $filename not found. Check error messages above"]"
    lappend cmd "\texit 1"
    lappend cmd "fi"
    return $cmd
}
proc remove_temp_file_bash { filename } {
    set     cmd {}
    lappend cmd [log_message_bash {INFO} {XBTEST_WIZARD-27} "Removing temporary file: $filename"]
    lappend cmd "rm $filename"
    return $cmd
}
proc check_ret_code_bash { cmd_name } {
    set     cmd {}
    lappend cmd "ret=$?"
    lappend cmd "if \[ \$ret != 0 \]; then"
    lappend cmd "\t[log_message_bash "ERROR" "XBTEST_WIZARD-14" "xbtest xclbin generation failed. $cmd_name failed with return code \$ret. Check error messages above"]"
    lappend cmd "\texit 1"
    lappend cmd "fi"
    return $cmd
}
proc check_dir_bash { dirname not_empty action } {
    set     cmd {}
    lappend cmd "if \[ ! -d $dirname \]; then"
    lappend cmd "\t[log_message_bash "ERROR" "XBTEST_WIZARD-14" "Unable to $action. Directory $dirname not found."]"
    lappend cmd "\texit 1"
    lappend cmd "fi"
    if {$not_empty} {
    lappend cmd "if \[ ! \"\$(ls -A $dirname)\" \]; then"
    lappend cmd "\t[log_message_bash "ERROR" "XBTEST_WIZARD-14" "Unable to $action. Directory $dirname is empty."]"
    lappend cmd "\texit 1"
    lappend cmd "fi"
    }
    return $cmd
}
proc check_env_var_bash { varname action } {
    set     cmd {}
    lappend cmd "if \[ -z \${$varname+x} \]; then"
    lappend cmd "\t[log_message_bash "ERROR" "XBTEST_WIZARD-14" "Unable to $action. Variable $varname is not set."]"
    lappend cmd "\texit 1"
    lappend cmd "fi"
    return $cmd
}
proc set_default_env_var_bash { varname default_value } {
    set     cmd {}
    lappend cmd "if \[ -z \${$varname+x} \]; then"
    lappend cmd "\texport $varname=$default_value"
    lappend cmd "fi"
    return $cmd
}
proc get_bash_delimiter { } {
    set     cmd {}
    lappend cmd ""
    lappend cmd "#############################################################################################################"
    lappend cmd ""
    return $cmd
}