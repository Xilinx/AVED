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

# log_message "XBTEST_WIZARD-2" {} "log_test"

# Note about severities:
#   *  ERROR - An ERROR condition implies an issue has been encountered which
#      will render design results unusable and cannot be resolved without user
#      intervention.
#
#   *  {CRITICAL WARNING} - A CRITICAL WARNING message indicates that certain
#      input/constraints will either not be applied or are outside the best
#      practices for a FPGA family. User action is strongly recommended.
#
#      Note: Since this is a two word value, it must be enclosed in "" or {}.
#
#   *  WARNING - A WARNING message indicates that design results may be
#      sub-optimal because constraints or specifications may not be applied as
#      intended. User action may be taken or may be reserved.
#
#   *  INFO - An INFO message is the same as a STATUS message, but includes a
#      severity and message ID tag. An INFO message includes a message ID to
#      allow further investigation through answer records if needed.
#
#   *  STATUS - A STATUS message communicates general status of the process
#      and feedback to the user regarding design processing. A STATUS message
#      does not include a message ID.

# Note: Raptor2 fails when CRITICAL WARNING or ERROR messages are displayed during the example design generation

proc load_messages_json { {mode "ip"} } {
    package require json

    set MESSAGE_FIELDS      {id severity msg msg_edit detail resolution state}
    set MESSAGE_FIELDS_REQ  {id severity msg detail state}
    set MESSAGE_SEVERITIES  {STATUS INFO WARNING {CRITICAL WARNING} ERROR}
    set MESSAGE_STATES      {used reserved obsolete}

    set messages_txt ""
    if {$mode == "ip"} {
        # Mode ip
        set filename [file join data messages.json]
        while {[gets_ipfile $filename line] >= 0} {
            append messages_txt $line
        }
        close_ipfile $filename
    } else {
        # Mode debug
        set filename $mode
        set fp [open $filename r]
        while { [gets $fp line] >= 0 } {
            append messages_txt $line
        }
        close $fp
    }
    if {$messages_txt == ""} {
        common::send_msg_id {XBTEST_MESSAGES-7} {ERROR} "load_messages_json: No messages loaded"
    }

    set messages_json [::json::json2dict $messages_txt]
    set messages_dict [dict create]
    foreach msg_def $messages_json {
        # Check all keys are known
        foreach key [dict keys $msg_def] {
            if {[lsearch -exact $MESSAGE_FIELDS $key] == -1} {
                common::send_msg_id {XBTEST_MESSAGES-1} {ERROR} "load_messages_json: Unknown message key $key."
            }
        }
        foreach key $MESSAGE_FIELDS {
            # Check all keys are known
            if {[dict exists $msg_def $key] == 0} {
                common::send_msg_id {XBTEST_MESSAGES-2} {ERROR} "load_messages_json: Message key $key does not exist."
            }
            # Check required keys values
            if {(([dict_get_quiet $msg_def $key] == {}) && ([lsearch -exact $MESSAGE_FIELDS_REQ $key] != -1))} {
                common::send_msg_id {XBTEST_MESSAGES-3} {ERROR} "load_messages_json: Value is not set for message key $key."
            }
        }
        set id          [dict_get_quiet $msg_def id]
        set severity    [dict_get_quiet $msg_def severity]
        set msg         [dict_get_quiet $msg_def msg]
        set msg_edit    [dict_get_quiet $msg_def msg_edit]
        set detail      [dict_get_quiet $msg_def detail]
        set resolution  [dict_get_quiet $msg_def resolution]
        set state       [dict_get_quiet $msg_def state]
        # Check message ID not already defined
        if {[dict exists $messages_dict $id] == 1} {
            common::send_msg_id {XBTEST_MESSAGES-4} {ERROR} "load_messages_json: ID $id already defined"
        }
        # Check that severity is defined
        if {[lsearch -exact $MESSAGE_SEVERITIES $severity] == -1} {
            common::send_msg_id {XBTEST_MESSAGES-5} {ERROR} "load_messages_json: Unknown message severity $severity."
        }
        # Check that state is defined
        if {[lsearch -exact $MESSAGE_STATES $state] == -1} {
            common::send_msg_id {XBTEST_MESSAGES-6} {ERROR} "load_messages_json: Unknown message state $state."
        }
        # No resolution for INFO and PASS messages
        if {(($severity == {STATUS}) || ($severity == {INFO})) && ($resolution != "")} {
            common::send_msg_id {XBTEST_MESSAGES-7} {ERROR} "load_messages_json: Unexpected resolution set for message of severity $severity."
        } elseif {(($severity != {STATUS}) && ($severity != {INFO})) && ($resolution == "")} {
            common::send_msg_id {XBTEST_MESSAGES-8} {ERROR} "load_messages_json: Missing resolution for message of severity $severity."
        }
        dict set messages_dict $id $msg_def
    }
    common::send_msg_id {XBTEST_MESSAGES-7} {INFO} "load_messages_json: Messages successfully loaded"
    return $messages_dict
}

proc log_message { config id {arg_in {}} } {
    set MESSAGES_DICT [dict get $config MESSAGES_DICT]

    # Check message ID not already defined
    if {[dict exists $MESSAGES_DICT $id] == 0} {
        common::send_msg_id {XBTEST_MESSAGES-9} {ERROR} "log_message: No message definition exists for message ID: $id"
    }
    set msg_def     [dict_get_quiet $MESSAGES_DICT $id]
    set severity    [dict_get_quiet $msg_def severity]
    set msg         [dict_get_quiet $msg_def msg]
    set msg_edit    [dict_get_quiet $msg_def msg_edit]

    if {$msg_edit == ""} {
        set msg_out $msg
        set num_arg_in  [llength $arg_in]
        if {$num_arg_in > 0} {
            append msg_out "; <others> "
            for {set i 0} {$i < $num_arg_in} {incr i} {
                append msg_out [lindex $arg_in $i]
                if  {$i < $num_arg_in - 1} {
                    append msg_out ", "
                }
            }
        }
    } else {
        set num_arg_msg [llength [regexp -all -inline "%s" $msg_edit]]
        set num_arg_in  [llength $arg_in]
        if {$num_arg_msg > $num_arg_in} {
            set arg $arg_in
            for {set i $num_arg_in} {$i < $num_arg_msg} {incr i} {
                lappend arg "<NULL>"
            }
        } else {
            set arg $arg_in
        }
        set format_cmd "\[format \"$msg_edit\""
        for {set i 0} {$i < [llength $arg]} {incr i} {
            if  {$i < [llength $arg]} {
                append format_cmd " "
            }
            append format_cmd "\"[lindex $arg $i]\""
        }
        append format_cmd "\]"
        set msg_out [subst $format_cmd]

        if {$num_arg_msg < $num_arg_in} {
            append msg_out "; <others> "
            for {set i $num_arg_msg} {$i < $num_arg_in} {incr i} {
                append msg_out [lindex $arg_in $i]
                if  {$i < $num_arg_in - 1} {
                    append msg_out ", "
                }
            }
        }
    }
    common::send_msg_id $id $severity $msg_out
    return "$severity: \[$id\] $msg_out"
}
