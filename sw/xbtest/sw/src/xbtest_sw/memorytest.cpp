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

#include "memorytest.h"

namespace xbtest
{

MemoryTest::MemoryTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
    const Memory_Type_Parameters_t & test_parameters, const bool & power_tc_exists, const uint & memory_idx, const uint & cu_idx
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_devicemgt         = device_mgt;
    this->m_test_parameters   = test_parameters;
    this->m_power_tc_exists   = power_tc_exists;
    this->m_memory_idx        = memory_idx;
    this->m_cu_idx            = cu_idx;

    this->m_memory_settings   = this->m_xbtest_sw_config->GetMemorySettings(memory_idx);
    this->m_cu_name           = device->GetMemoryCuName(this->m_memory_settings.type, this->m_memory_settings.name, cu_idx);

    this->m_log_header    = LOG_HEADER_MEMORY;
    this->m_log_header_2  = this->m_memory_settings.name;
    if ((this->m_memory_settings.type == SINGLE_CHANNEL) && (this->m_xbtest_sw_config->GetMemoryNumCU(this->m_memory_settings.name) > 1))
    {
        this->m_log_header_3 = GetCUConnectionVppSptag(0);
    }

    this->m_queue_testcase = MEMORY_MEMBER.name;
    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        if ((this->m_memory_settings.type == SINGLE_CHANNEL) && (this->m_xbtest_sw_config->GetMemoryNumCU(this->m_memory_settings.name) > 1))
        {
            this->m_queue_thread = GetCUConnectionVppSptag(0);
        }
        else
        {
            this->m_queue_thread = this->m_memory_settings.name;
        }
    }
}

MemoryTest::~MemoryTest() = default;

bool MemoryTest::PreSetup()
{
    auto global_settings_failure = RET_SUCCESS;
    this->m_state = TestState::TS_PRE_SETUP;

    if (global_settings_failure == RET_SUCCESS)
    {
        if (this->m_memory_settings.name.empty())
        {
            global_settings_failure = RET_FAILURE;
            LogMessage(MSG_MEM_001, {"memory name is not set in input memory settings"}); // Error, memory name in settings provided is empty, something went wrong
        }
    }
    if (global_settings_failure == RET_SUCCESS)
    {
        if ((this->m_memory_settings.type != SINGLE_CHANNEL) && (this->m_memory_settings.type != MULTI_CHANNEL))
        {
            global_settings_failure = RET_FAILURE;
            LogMessage(MSG_MEM_001, {"memory type (" + this->m_memory_settings.type + ") is unknown"}); // Error, only SINGLE_CHANNEL or MULTI_CHANNEL are supported, something went wrong
        }
    }
    if (global_settings_failure == RET_SUCCESS)
    {
        LogMessage(MSG_CMN_004);
    }
    if (global_settings_failure == RET_SUCCESS)
    {
        if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
        {
            this->m_console_mgmt->AllocateTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
        }
    }
    if (global_settings_failure == RET_SUCCESS)
    {
        LogMessage(MSG_CMN_004);
        global_settings_failure |= GetTestSequence();
    }
    if (global_settings_failure == RET_SUCCESS)
    {
        this->m_num_channels = this->m_memory_settings.num_channels;
        if (this->m_num_channels <= 0)
        {
            global_settings_failure = RET_FAILURE;
            LogMessage(MSG_MEM_001, {"number of channels (" + std::to_string(this->m_num_channels) + ") is invalid"}); // Failed to get number of channels, something went wrong
        }
    }
    if (global_settings_failure == RET_SUCCESS)
    {
        this->m_buffer_size = MEMORY_CHAN_BUF_SIZE * (uint64_t)(this->m_num_channels);

        this->m_cu_rate           = this->m_memory_settings.cu_rate;
        this->m_cu_outstanding    = this->m_memory_settings.cu_outstanding;
        this->m_cu_burst_size     = this->m_memory_settings.cu_burst_size;

        // Set the boundaries of the test parameters depending on the test type
        //  and based on the AXI4 data width
        this->m_AXI_num_data_bytes = this->m_memory_settings.axi_data_size/8;
        LogMessage(MSG_DEBUG_TESTCASE, {"xbtest HW IP is connected to the memory using an AXI4 data width of " + std::to_string(this->m_AXI_num_data_bytes) + " Bytes"});
        // See if the AXI AW/RID are used in HW in this xbtest HW IP
        this->m_num_axi_thread = this->m_memory_settings.num_axi_thread;

        // Memory size does not increase if channels are disabled
        // this->m_num_channels should be 1 in for single channel
        this->m_TC_Cfg.chan_config_global.mem_size_bytes = (uint64_t)(this->m_memory_settings.cu_size_mb) * (uint64_t)(1024) * (uint64_t)(1024); // total memory size in bytes
        this->m_TC_Cfg.chan_config_global.mem_size_bytes /= (uint64_t)(this->m_num_channels); // memory size per port in bytes

        // Rate
        this->m_min.rate = MIN_MEM_RATE;
        this->m_max.rate = MAX_MEM_RATE;

        // Bandwidth
        this->m_min.bandwidth = MIN_MEM_BANDWIDTH;
        this->m_max.bandwidth = (uint)((double)(this->m_AXI_num_data_bytes) * (double)(this->m_xbtest_sw_config->GetClockFrequency(0)) * (double)(1000) * (double)(1000) / (double)(1024) / (double)(1024));

        // Start address
        this->m_min.start_addr = MIN_START_ADDR;
        this->m_max.start_addr = this->m_TC_Cfg.chan_config_global.mem_size_bytes - this->m_min.block_size;

        // check overflow of max number of transfers
        this->m_min.burst_xfer    = MIN_BURST_XFER;
        if (this->m_AXI_num_data_bytes == 64) // 64 bytes (512 bits...)
        {
            this->m_max.burst_xfer = MAX_BURST_XFER_512b;
        }
        else
        {
            this->m_max.burst_xfer = MAX_BURST_XFER_BELOW_512b;
        }
        this->m_min.burst_size = this->m_min.burst_xfer * this->m_AXI_num_data_bytes;
        this->m_max.burst_size = this->m_max.burst_xfer * this->m_AXI_num_data_bytes;

        this->m_min.block_size = MIN_BLOCK_SIZE; // and block_size must be multiple of this->m_min.burst_size; HW does not support block_size < 2*burst_size
        this->m_max.block_size = (uint)( this->m_TC_Cfg.chan_config_global.mem_size_bytes / (uint64_t)(1024) / (uint64_t)(1024) );

        // total_xfer must be multiple of burst_xfer > 2*burst_xfer
        this->m_min.total_xfer = (uint)( (uint64_t)(this->m_min.block_size) * (uint64_t)(1024) * (uint64_t)(1024) / (uint64_t)(this->m_AXI_num_data_bytes) ); // will be set to this->m_min.burst_xfer or test_it_cfg.burst_size
        this->m_max.total_xfer = (uint)( (uint64_t)(this->m_max.block_size) * (uint64_t)(1024) * (uint64_t)(1024) / (uint64_t)(this->m_AXI_num_data_bytes) ); // will be set depending on start address

        double max_total_xfer_f = (double)(this->m_TC_Cfg.chan_config_global.mem_size_bytes)/(double)(this->m_AXI_num_data_bytes);
        if ((double)(this->m_max.total_xfer) != max_total_xfer_f)
        {
            LogMessage(MSG_MEM_034, {std::to_string(max_total_xfer_f), std::to_string(this->m_TC_Cfg.chan_config_global.mem_size_bytes), std::to_string(this->m_AXI_num_data_bytes)});
            global_settings_failure = RET_FAILURE;
        }

        // Nominal burst size
        if (!(this->m_cu_burst_size.alt_wr_rd.read.exists))
        {
            this->m_cu_burst_size.alt_wr_rd.read.nominal = this->m_max.burst_size;
        }
        if (!(this->m_cu_burst_size.alt_wr_rd.write.exists))
        {
            this->m_cu_burst_size.alt_wr_rd.write.nominal = this->m_max.burst_size;
        }
        if (!(this->m_cu_burst_size.only_rd.read.exists))
        {
            this->m_cu_burst_size.only_rd.read.nominal = this->m_max.burst_size;
        }
        if (!(this->m_cu_burst_size.only_wr.write.exists))
        {
            this->m_cu_burst_size.only_wr.write.nominal = this->m_max.burst_size;
        }
        if (!(this->m_cu_burst_size.simul_wr_rd.read.exists))
        {
            this->m_cu_burst_size.simul_wr_rd.read.nominal = this->m_max.burst_size;
        }
        if (!(this->m_cu_burst_size.simul_wr_rd.write.exists))
        {
            this->m_cu_burst_size.simul_wr_rd.write.nominal = this->m_max.burst_size;
        }
        LogMessage(MSG_ITF_058, {"Nominal burst size (Bytes) for each xbtest HW IP channel in memory test: "
                                + ALTERNATE_WR_RD + " mode = "
                                + "{write: " + std::to_string(this->m_cu_burst_size.alt_wr_rd.write.nominal) + " / "
                                +   "read: " + std::to_string(this->m_cu_burst_size.alt_wr_rd.read.nominal) + "}, "
                                + ONLY_WR + " mode = "
                                + "{write: " + std::to_string(this->m_cu_burst_size.only_wr.write.nominal) + "}, "
                                + ONLY_RD + " mode = "
                                +  "{read: " + std::to_string(this->m_cu_burst_size.only_rd.read.nominal) + "}, "
                                + SIMULTANEOUS_WR_RD + " mode = "
                                + "{write: " + std::to_string(this->m_cu_burst_size.simul_wr_rd.write.nominal) + " / "
                                +   "read: " + std::to_string(this->m_cu_burst_size.simul_wr_rd.read.nominal) + "}"});

        LogMessage(MSG_DEBUG_TESTCASE, {"Parameter boundaries: rate = ["                          + std::to_string(this->m_min.rate)         + " ; " + std::to_string(this->m_max.rate)        + "], "
                                                          + "bandwidth = ["                     + std::to_string(this->m_min.bandwidth)    + " ; " + std::to_string(this->m_max.bandwidth)   + "], "
                                                          + "start address offset = ["          + std::to_string(this->m_min.start_addr)   + " ; " + std::to_string(this->m_max.start_addr)  + "], "
                                                          + "burst size = ["                    + std::to_string(this->m_min.burst_size)   + " ; " + std::to_string(this->m_max.burst_size)  + "], "
                                                          + "block size = ["                    + std::to_string(this->m_min.block_size)   + " ; " + std::to_string(this->m_max.block_size)  + "], "
                                                          + "number of transfers per burst = [" + std::to_string(this->m_min.burst_xfer)   + " ; " + std::to_string(this->m_max.burst_xfer)  + "], "
                                                          + "total number of transfers = ["     + std::to_string(this->m_min.total_xfer)   + " ; " + std::to_string(this->m_max.total_xfer)  + "]"});
    }

    // Test parameters
    // Init
    // Json_Parameters_t init_param;
    // init_param.clear();
    // MemoryCoreCfg_t init_cfg;
    this->m_mem_chan_config.clear();
    this->m_mem_chan_config_pattern.clear();
    this->m_TC_Cfg.chan_config.clear();

    // for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    // {
        // this->m_mem_chan_config[ch_idx] = init_param;
        // this->m_TC_Cfg.chan_config[ch_idx] = init_cfg;
    // }

    // Select channel parameters
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        if (this->m_memory_settings.type == SINGLE_CHANNEL)
        {
            auto it = this->m_test_parameters.memory_tag_config.find(GetCUConnectionVppSptag(ch_idx));
            if (it != this->m_test_parameters.memory_tag_config.end())
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Using memory specific parameters provided in " + TEST_JSON + " " + GetMessageTagChan(ch_idx)});
                this->m_mem_chan_config[ch_idx] = it->second;
            }

            it = this->m_test_parameters.memory_tag_config_pattern.find(GetCUConnectionVppSptag(ch_idx));
            if (it != this->m_test_parameters.memory_tag_config_pattern.end())
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Using memory specific pattern parameters provided in " + TEST_JSON + " " + GetMessageTagChan(ch_idx)});
                this->m_mem_chan_config_pattern[ch_idx] = it->second;
            }
        }
        else
        {
            auto it = this->m_test_parameters.memory_chan_config.find(ch_idx);
            if (it != this->m_test_parameters.memory_chan_config.end())
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Using memory specific parameters provided in " + TEST_JSON + " " + GetMessageTagChan(ch_idx)});
                this->m_mem_chan_config[ch_idx] = it->second;
            }
            it = this->m_test_parameters.memory_chan_config_pattern.find(ch_idx);
            if (it != this->m_test_parameters.memory_chan_config_pattern.end())
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Using memory specific pattern parameters provided in " + TEST_JSON + " " + GetMessageTagChan(ch_idx)});
                this->m_mem_chan_config_pattern[ch_idx] = it->second;
            }
        }
    }

    // Disable memory
    global_settings_failure |= GetJsonParamBool(DISABLE_MEMORY_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.chan_config_global.disable, false);
    auto all_channels_disabled = this->m_TC_Cfg.chan_config_global.disable;
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        global_settings_failure |= GetJsonParamBool(DISABLE_MEMORY_MEMBER, " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config[ch_idx], this->m_TC_Cfg.chan_config[ch_idx].disable, this->m_TC_Cfg.chan_config_global.disable);

        all_channels_disabled &= this->m_TC_Cfg.chan_config[ch_idx].disable;
    }
    // For multi-channel, all channels disabled is considered as a failure
    if ((all_channels_disabled) && (this->m_memory_settings.type == MULTI_CHANNEL))
    {
        LogMessage(MSG_MEM_046);
        global_settings_failure = RET_FAILURE;
    }

    // Select channels to test
    this->m_enabled_ch_indexes.clear();
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        if (this->m_TC_Cfg.chan_config[ch_idx].disable)
        {
            LogMessage(MSG_MEM_031, {GetMessageChan(ch_idx)});
        }
        else
        {
            this->m_enabled_ch_indexes.emplace_back(ch_idx);
        }
    }

    // Control pattern
    global_settings_failure |= GetJsonParamStr(PATTERN_CTRL_MEMBER, this->m_test_parameters.global_config_pattern, SUPPORTED_PATTERN_CTRL, this->m_TC_Cfg.chan_config_global.pattern_ctrl, PATTERN_CTRL_PRBS);
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        global_settings_failure |= GetJsonParamStr(PATTERN_CTRL_MEMBER, " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config_pattern[ch_idx], SUPPORTED_PATTERN_CTRL, this->m_TC_Cfg.chan_config[ch_idx].pattern_ctrl, this->m_TC_Cfg.chan_config_global.pattern_ctrl);
    }

    // List pattern
    std::vector<std::string> pattern_list_str_default = {"0xCAFECAFE", "0xF00DF00D"};

    global_settings_failure |= GetJsonParamArrayStr(PATTERN_LIST_MEMBER, "(global)", this->m_test_parameters.global_config_pattern, this->m_TC_Cfg.chan_config_global.pattern_list_str, pattern_list_str_default);
    this->m_TC_Cfg.chan_config_global.pattern_list.clear();
    for (const auto & pattern_str : this->m_TC_Cfg.chan_config_global.pattern_list_str)
    {
        uint pattern;
        if (StrHexToNum(pattern_str, pattern, 8) == RET_FAILURE)
        {
            LogMessage(MSG_MEM_052, {pattern_str, PATTERN_LIST_MEMBER.name, "(global)"});
            global_settings_failure = RET_FAILURE;
        }
        this->m_TC_Cfg.chan_config_global.pattern_list.emplace_back(pattern);
    }
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        global_settings_failure |= GetJsonParamArrayStr(PATTERN_LIST_MEMBER, " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config_pattern[ch_idx], this->m_TC_Cfg.chan_config[ch_idx].pattern_list_str, this->m_TC_Cfg.chan_config_global.pattern_list_str);
        this->m_TC_Cfg.chan_config[ch_idx].pattern_list.clear();
        for (const auto & pattern_str : this->m_TC_Cfg.chan_config[ch_idx].pattern_list_str)
        {
            uint pattern;
            if (StrHexToNum(pattern_str, pattern, 8) == RET_FAILURE)
            {
                LogMessage(MSG_MEM_052, {pattern_str, PATTERN_LIST_MEMBER.name, GetMessageTagChan(ch_idx)});
                global_settings_failure = RET_FAILURE;
            }
            this->m_TC_Cfg.chan_config[ch_idx].pattern_list.emplace_back(pattern);
        }
        if (StrMatchNoCase(this->m_TC_Cfg.chan_config[ch_idx].pattern_ctrl, PATTERN_CTRL_ALTERNATE))
        {
            if (this->m_TC_Cfg.chan_config[ch_idx].pattern_list.size() < 2)
            {
                LogMessage(MSG_MEM_053, {"2", PATTERN_CTRL_ALTERNATE, GetMessageTagChan(ch_idx)});
                global_settings_failure = RET_FAILURE;
            }
        }
        else if (StrMatchNoCase(this->m_TC_Cfg.chan_config[ch_idx].pattern_ctrl, PATTERN_CTRL_FIXED))
        {
            if (this->m_TC_Cfg.chan_config[ch_idx].pattern_list.empty())
            {
                LogMessage(MSG_MEM_053, {"1", PATTERN_CTRL_FIXED, GetMessageTagChan(ch_idx)});
                global_settings_failure = RET_FAILURE;
            }
        }
    }


    // Single id
    if (this->m_num_axi_thread > 1)
    {
        global_settings_failure |= GetJsonParamBool(SINGLE_AXI_THREAD_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.chan_config_global.single_axi_thread, false);
        for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
        {
            global_settings_failure |= GetJsonParamBool(SINGLE_AXI_THREAD_MEMBER, " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config[ch_idx], this->m_TC_Cfg.chan_config[ch_idx].single_axi_thread, this->m_TC_Cfg.chan_config_global.single_axi_thread);
        }
    }
    else
    {
        this->m_TC_Cfg.chan_config_global.single_axi_thread = true;
        for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
        {
            this->m_TC_Cfg.chan_config[ch_idx].single_axi_thread = true;
        }
    }

    // xbtest HW IP bandwidth
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_WR_BW_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.alt_wr_rd.write.low,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.write.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_WR_BW_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.alt_wr_rd.write.high,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.write.high);
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_RD_BW_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.alt_wr_rd.read.low,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.read.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_RD_BW_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.alt_wr_rd.read.high,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.read.high);

    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_WR_BW_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.only_wr.write.low,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.only_wr.write.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_WR_BW_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.only_wr.write.high,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.only_wr.write.high);
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_RD_BW_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.only_rd.read.low,       MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.only_rd.read.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_RD_BW_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.only_rd.read.high,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.only_rd.read.high);

    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_WR_BW_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.simul_wr_rd.write.low,  MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.write.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_WR_BW_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.simul_wr_rd.write.high, MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.write.high);
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_RD_BW_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.simul_wr_rd.read.low,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.read.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_RD_BW_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_bw.simul_wr_rd.read.high,  MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.read.high);

    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        std::string name_cmplt = " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_WR_BW_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.write.low,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.write.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_WR_BW_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.write.high,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.write.high);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_RD_BW_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.read.low,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.read.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_RD_BW_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.alt_wr_rd.read.high,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.read.high);

        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_WR_BW_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.only_wr.write.low,       MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_wr.write.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_WR_BW_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.only_wr.write.high,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_wr.write.high);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_RD_BW_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.only_rd.read.low,        MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_rd.read.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_RD_BW_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.only_rd.read.high,       MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_rd.read.high);

        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_WR_BW_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.write.low,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.write.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_WR_BW_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.write.high,  MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.write.high);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_RD_BW_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.read.low,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.read.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_RD_BW_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_bw.simul_wr_rd.read.high,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.read.high);
    }

    // Check of low vs high BW thresholds
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        Json_Val_Def_t json_val_def;

        json_val_def = LO_THRESH_ALT_WR_BW_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.write.low,   HI_THRESH_ALT_WR_BW_MEMBER,     this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.write.high);

        json_val_def = LO_THRESH_ALT_RD_BW_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.read.low,    HI_THRESH_ALT_RD_BW_MEMBER,     this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.read.high);

        json_val_def = LO_THRESH_ONLY_WR_BW_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_wr.write.low,      HI_THRESH_ONLY_WR_BW_MEMBER,    this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_wr.write.high);

        json_val_def = LO_THRESH_ONLY_RD_BW_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def, this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_rd.read.low,       HI_THRESH_ONLY_RD_BW_MEMBER,    this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_rd.read.high);

        json_val_def = LO_THRESH_SIMUL_WR_BW_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.write.low, HI_THRESH_SIMUL_WR_BW_MEMBER,   this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.write.high);

        json_val_def = LO_THRESH_SIMUL_RD_BW_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.read.low,  HI_THRESH_SIMUL_RD_BW_MEMBER,   this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.read.high);
    }

    // Disable pre-write
    global_settings_failure |= GetJsonParamBool(DISABLE_PREWRITE_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.chan_config_global.disable_prewrite, false);
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        global_settings_failure |= GetJsonParamBool(DISABLE_PREWRITE_MEMBER, " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config[ch_idx], this->m_TC_Cfg.chan_config[ch_idx].disable_prewrite, this->m_TC_Cfg.chan_config_global.disable_prewrite);
    }

    // xbtest HW IP latency
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_WR_LAT_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.alt_wr_rd.write.low,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.write.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_WR_LAT_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.alt_wr_rd.write.high,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.write.high);
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_RD_LAT_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.alt_wr_rd.read.low,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.read.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_RD_LAT_MEMBER,    this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.alt_wr_rd.read.high,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.read.high);

    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_WR_LAT_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.only_wr.write.low,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.only_wr.write.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_WR_LAT_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.only_wr.write.high,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.only_wr.write.high);
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_RD_LAT_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.only_rd.read.low,       MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.only_rd.read.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_RD_LAT_MEMBER,   this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.only_rd.read.high,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.only_rd.read.high);

    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_WR_LAT_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.simul_wr_rd.write.low,  MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.write.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_WR_LAT_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.simul_wr_rd.write.high, MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.write.high);
    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_RD_LAT_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.simul_wr_rd.read.low,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.read.low);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_RD_LAT_MEMBER,  this->m_test_parameters.global_config, MIN_LO_THRESH, this->m_memory_settings.cu_latency.simul_wr_rd.read.high,  MAX_UINT_VAL, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.read.high);

    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        auto name_cmplt = " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_WR_LAT_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.write.low,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.write.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_WR_LAT_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.write.high,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.write.high);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ALT_RD_LAT_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.read.low,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.read.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ALT_RD_LAT_MEMBER,   name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.alt_wr_rd.read.high,     MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.read.high);

        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_WR_LAT_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.only_wr.write.low,       MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_wr.write.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_WR_LAT_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.only_wr.write.high,      MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_wr.write.high);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_ONLY_RD_LAT_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.only_rd.read.low,        MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_rd.read.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_ONLY_RD_LAT_MEMBER,  name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.only_rd.read.high,       MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_rd.read.high);

        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_WR_LAT_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.write.low,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.write.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_WR_LAT_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.write.high,  MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.write.high);
        global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_SIMUL_RD_LAT_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.read.low,    MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.read.low);
        global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_SIMUL_RD_LAT_MEMBER, name_cmplt, this->m_mem_chan_config[ch_idx], MIN_LO_THRESH, this->m_TC_Cfg.chan_config_global.cu_latency.simul_wr_rd.read.high,   MAX_UINT_VAL, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.read.high);
    }

    // Check of low vs high latency thresholds
    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        Json_Val_Def_t json_val_def;

        json_val_def = LO_THRESH_ALT_WR_LAT_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.write.low,      HI_THRESH_ALT_WR_LAT_MEMBER,    this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.write.high);

        json_val_def = LO_THRESH_ALT_RD_LAT_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.read.low,       HI_THRESH_ALT_RD_LAT_MEMBER,    this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.read.high);

        json_val_def = LO_THRESH_ONLY_WR_LAT_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_wr.write.low,         HI_THRESH_ONLY_WR_LAT_MEMBER,   this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_wr.write.high);

        json_val_def = LO_THRESH_ONLY_RD_LAT_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def, this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_rd.read.low,          HI_THRESH_ONLY_RD_LAT_MEMBER,   this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_rd.read.high);

        json_val_def = LO_THRESH_SIMUL_WR_LAT_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.write.low,    HI_THRESH_SIMUL_WR_LAT_MEMBER,  this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.write.high);

        json_val_def = LO_THRESH_SIMUL_RD_LAT_MEMBER; json_val_def.name += " " + GetMessageTagChan(ch_idx);
        global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def,  this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.read.low,     HI_THRESH_SIMUL_RD_LAT_MEMBER,  this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.read.high);
    }

    // check_bw/check_latency
    auto DEFAULT_CHECK_BW               = true;
    auto DEFAULT_CHECK_LATENCY          = true;
    auto DEFAULT_CHECK_DATA_INTEGRITY   = true;
    if (this->m_memory_settings.target == HOST)
    {
        DEFAULT_CHECK_BW      = false;
        DEFAULT_CHECK_LATENCY = false;
    }

    this->m_TC_Cfg.chan_config_global.check_bw_exists               = false;
    this->m_TC_Cfg.chan_config_global.check_latency_exists          = false;
    this->m_TC_Cfg.chan_config_global.check_data_integrity_exists   = false;

    auto it = FindJsonParam(this->m_test_parameters.global_config, CHECK_BW_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_TC_Cfg.chan_config_global.check_bw_exists = true;
    }
    it = FindJsonParam(this->m_test_parameters.global_config, CHECK_LATENCY_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_TC_Cfg.chan_config_global.check_latency_exists = true;
    }
    it = FindJsonParam(this->m_test_parameters.global_config, CHECK_DATA_INTEGRITY_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_TC_Cfg.chan_config_global.check_data_integrity_exists = true;
    }

    global_settings_failure |= GetJsonParamBool(CHECK_BW_MEMBER,                this->m_test_parameters.global_config, this->m_TC_Cfg.chan_config_global.check_bw,              DEFAULT_CHECK_BW);
    global_settings_failure |= GetJsonParamBool(CHECK_LATENCY_MEMBER,           this->m_test_parameters.global_config, this->m_TC_Cfg.chan_config_global.check_latency,         DEFAULT_CHECK_LATENCY);
    global_settings_failure |= GetJsonParamBool(CHECK_DATA_INTEGRITY_MEMBER,    this->m_test_parameters.global_config, this->m_TC_Cfg.chan_config_global.check_data_integrity,  DEFAULT_CHECK_DATA_INTEGRITY);

    for (int ch_idx = 0; ch_idx < this->m_num_channels; ch_idx++)
    {
        this->m_TC_Cfg.chan_config[ch_idx].check_bw_exists              = this->m_TC_Cfg.chan_config_global.check_bw_exists;
        this->m_TC_Cfg.chan_config[ch_idx].check_latency_exists         = this->m_TC_Cfg.chan_config_global.check_latency_exists;
        this->m_TC_Cfg.chan_config[ch_idx].check_data_integrity_exists  = this->m_TC_Cfg.chan_config_global.check_data_integrity_exists;

        auto it = FindJsonParam(this->m_mem_chan_config[ch_idx], CHECK_BW_MEMBER);
        if (it != this->m_mem_chan_config[ch_idx].end())
        {
            this->m_TC_Cfg.chan_config[ch_idx].check_bw_exists = true;
        }
        it = FindJsonParam(this->m_mem_chan_config[ch_idx], CHECK_LATENCY_MEMBER);
        if (it != this->m_mem_chan_config[ch_idx].end())
        {
            this->m_TC_Cfg.chan_config[ch_idx].check_latency_exists = true;
        }
        it = FindJsonParam(this->m_mem_chan_config[ch_idx], CHECK_DATA_INTEGRITY_MEMBER);
        if (it != this->m_mem_chan_config[ch_idx].end())
        {
            this->m_TC_Cfg.chan_config[ch_idx].check_data_integrity_exists = true;
        }

        global_settings_failure |= GetJsonParamBool(CHECK_BW_MEMBER,                " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config[ch_idx], this->m_TC_Cfg.chan_config[ch_idx].check_bw,              this->m_TC_Cfg.chan_config_global.check_bw);
        global_settings_failure |= GetJsonParamBool(CHECK_LATENCY_MEMBER,           " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config[ch_idx], this->m_TC_Cfg.chan_config[ch_idx].check_latency,         this->m_TC_Cfg.chan_config_global.check_latency);
        global_settings_failure |= GetJsonParamBool(CHECK_DATA_INTEGRITY_MEMBER,    " " + GetMessageTagChan(ch_idx), this->m_mem_chan_config[ch_idx], this->m_TC_Cfg.chan_config[ch_idx].check_data_integrity,  this->m_TC_Cfg.chan_config_global.check_data_integrity);
    }

    // measurement output csv files
    //      - "*_detail.csv": 1 line per second for all test with test idx, mode, BW, burst latency, including live
    //      - "*_result.csv": 1 line per test for all test with test idx, parameters, BW, burst latency results
    //      - "*_power.csv":  1 line per second for all test with test idx, mode, delays configuration and power and temperatures as xbtest_pfm_def

    // first_line_detail we always display current average followed by live measure...
    std::vector<std::string> first_line_detail;
    first_line_detail.emplace_back("Global time (s)");
    first_line_detail.emplace_back("Test");
    first_line_detail.emplace_back(TEST_MODE);
    first_line_detail.emplace_back("Measurement ID");
    first_line_detail.emplace_back(DATA_INTEG_LIV);
    first_line_detail.emplace_back(DATA_INTEG);
    first_line_detail.emplace_back(WR_RD_LIV_BW + " (MBps)");
    first_line_detail.emplace_back(WR_RD_AVG_BW + " (MBps)");

    first_line_detail.emplace_back(WR_LIV_BW + " (MBps)");
    first_line_detail.emplace_back(WR_AVG_BW + " (MBps)");

    first_line_detail.emplace_back(WR_LIV_BURST_INST);
    first_line_detail.emplace_back(WR_BURST_INST);

    first_line_detail.emplace_back(WR_LIV_BURST_LATENCY_AVG    + " (ns)");
    first_line_detail.emplace_back(WR_BURST_LATENCY_AVG        + " (ns)");

    first_line_detail.emplace_back(RD_LIV_BW + " (MBps)");
    first_line_detail.emplace_back(RD_AVG_BW + " (MBps)");

    first_line_detail.emplace_back(RD_LIV_BURST_INST);
    first_line_detail.emplace_back(RD_BURST_INST);

    first_line_detail.emplace_back(RD_LIV_BURST_LATENCY_AVG    + " (ns)");
    first_line_detail.emplace_back(RD_BURST_LATENCY_AVG        + " (ns)");

    first_line_detail.emplace_back("Timestamp");

    if (!(this->m_xbtest_sw_config->GetCommandLineLogDisable()))
    {
        this->m_use_outputfile = true;
        this->m_outputfile_name = "memory";

        std::vector<std::string> first_line_result;
        first_line_result.emplace_back("Test");
        first_line_result.emplace_back(DURATION                + " (s)");
        first_line_result.emplace_back(TEST_MODE);
        first_line_result.emplace_back(DATA_INTEG);
        first_line_result.emplace_back(WR_RD_AVG_BW            + " (MBps)");

        first_line_result.emplace_back(WR_MEM_RATE             + " (%)");
        first_line_result.emplace_back(WR_MEM_BANDWIDTH        + " (MBps)");
        first_line_result.emplace_back(WR_START_ADDR           + " (MB)");
        first_line_result.emplace_back(WR_END_ADDR             + " (MB)");
        first_line_result.emplace_back(WR_BURST_SIZE           + " (Bytes)");
        first_line_result.emplace_back(WR_BLOCK_SIZE           + " (MB)");
        first_line_result.emplace_back(WR_OUTSTANDING);
        first_line_result.emplace_back(WR_DATA_SIZE            + " (Bytes)");
        first_line_result.emplace_back(WR_BURST_XFER);
        first_line_result.emplace_back(WR_NUM_BURST);
        first_line_result.emplace_back(WR_TOTAL_XFER);
        first_line_result.emplace_back(WR_AVG_BW               + " (MBps)");
        first_line_result.emplace_back(WR_BURST_INST);
        first_line_result.emplace_back(WR_BURST_LATENCY_AVG    + " (ns)");

        first_line_result.emplace_back(RD_MEM_RATE             + " (%)");
        first_line_result.emplace_back(RD_MEM_BANDWIDTH        + " (MBps)");
        first_line_result.emplace_back(RD_START_ADDR           + " (MB)");
        first_line_result.emplace_back(RD_END_ADDR             + " (MB)");
        first_line_result.emplace_back(RD_BURST_SIZE           + " (Bytes)");
        first_line_result.emplace_back(RD_BLOCK_SIZE           + " (MB)");
        first_line_result.emplace_back(RD_OUTSTANDING);
        first_line_result.emplace_back(RD_DATA_SIZE            + " (Bytes)");
        first_line_result.emplace_back(RD_BURST_XFER);
        first_line_result.emplace_back(RD_NUM_BURST);
        first_line_result.emplace_back(RD_TOTAL_XFER);
        first_line_result.emplace_back(RD_AVG_BW               + " (MBps)");
        first_line_result.emplace_back(RD_BURST_INST);
        first_line_result.emplace_back(RD_BURST_LATENCY_AVG    + " (ns)");

        std::vector<std::string> first_line_mc_summary;
        if (this->m_memory_settings.type == MULTI_CHANNEL)
        {
            first_line_mc_summary.emplace_back("Test");
            first_line_mc_summary.emplace_back(DURATION        + " (s)");
            first_line_mc_summary.emplace_back(TEST_MODE);

            first_line_mc_summary.emplace_back(WR_MEM_RATE      + " (%)");
            first_line_mc_summary.emplace_back(WR_MEM_BANDWIDTH + " (MBps)");
            first_line_mc_summary.emplace_back(WR_START_ADDR    + " (MB)");
            first_line_mc_summary.emplace_back(WR_BURST_SIZE    + " (Bytes)");
            first_line_mc_summary.emplace_back(WR_BLOCK_SIZE    + " (MB)");
            first_line_mc_summary.emplace_back(WR_OUTSTANDING);
            first_line_mc_summary.emplace_back(RD_MEM_RATE      + " (%)");
            first_line_mc_summary.emplace_back(RD_MEM_BANDWIDTH + " (MBps)");
            first_line_mc_summary.emplace_back(RD_START_ADDR    + " (MB)");
            first_line_mc_summary.emplace_back(RD_BURST_SIZE    + " (Bytes)");
            first_line_mc_summary.emplace_back(RD_BLOCK_SIZE    + " (MB)");
            first_line_mc_summary.emplace_back(RD_OUTSTANDING);

            first_line_mc_summary.emplace_back("Combined " + WR_AVG_BW             + " (MBps)");
            first_line_mc_summary.emplace_back("Combined " + RD_AVG_BW             + " (MBps)");
            first_line_mc_summary.emplace_back("Combined " + WR_BURST_LATENCY_AVG  + " (ns)");
            first_line_mc_summary.emplace_back("Combined " + RD_BURST_LATENCY_AVG  + " (ns)");
            first_line_mc_summary.emplace_back("Combined " + DATA_INTEG);
            first_line_mc_summary.emplace_back("Combined " + WR_RD_AVG_BW          + " (MBps)");

            first_line_mc_summary.emplace_back("");
            for (const auto & ch_idx : this->m_enabled_ch_indexes)
            {
                first_line_mc_summary.emplace_back("Ch" + std::to_string(ch_idx) + " " + WR_AVG_BW + " (MBps)");
            }
            first_line_mc_summary.emplace_back("");
            for (const auto & ch_idx : this->m_enabled_ch_indexes)
            {
                first_line_mc_summary.emplace_back("Ch" + std::to_string(ch_idx) + " " + RD_AVG_BW + " (MBps)");
            }
            first_line_mc_summary.emplace_back("");
            for (const auto & ch_idx : this->m_enabled_ch_indexes)
            {
                first_line_mc_summary.emplace_back("Ch" + std::to_string(ch_idx) + " " + WR_BURST_LATENCY_AVG + " (ns)");
            }
            first_line_mc_summary.emplace_back("");
            for (const auto & ch_idx : this->m_enabled_ch_indexes)
            {
                first_line_mc_summary.emplace_back("Ch" + std::to_string(ch_idx) + " " + RD_BURST_LATENCY_AVG + " (ns)");
            }
        }

        for (const auto & ch_idx : this->m_enabled_ch_indexes)
        {
            std::ofstream of_detail;
            std::ofstream of_result;
            this->m_outputfile_detail[ch_idx] = std::move(of_detail);
            this->m_outputfile_result[ch_idx] = std::move(of_result);
            auto filename = this->m_outputfile_name + "_" + FormatName(GetCUConnectionVppSptag(ch_idx), "_") + "_";
            if (this->m_memory_settings.type == MULTI_CHANNEL)
            {
                filename += "ch_" + std::to_string(ch_idx) + "_";
            }
            global_settings_failure |= OpenOutputFile(filename + "detail.csv", this->m_outputfile_detail[ch_idx]);
            global_settings_failure |= OpenOutputFile(filename + "result.csv", this->m_outputfile_result[ch_idx]);

            std::ofstream RT_of_detail;
            this->m_RT_outputfile_detail[ch_idx] = std::move(RT_of_detail);
            this->m_RT_outputfile_detail_name[ch_idx] = filename + "detail.csv";
            global_settings_failure |= OpenRTOutputFile(this->m_RT_outputfile_detail_name[ch_idx], this->m_RT_outputfile_detail[ch_idx]);

            if (global_settings_failure == RET_SUCCESS)
            {
                // write the first line of the file containing the description of each column
                this->m_outputfile_detail[ch_idx] << StrVectToStr(first_line_detail, ",") << "\n";
                this->m_outputfile_detail[ch_idx].flush();

                this->m_outputfile_result[ch_idx] << StrVectToStr(first_line_result, ",") << "\n";
                this->m_outputfile_result[ch_idx].flush();

                this->m_RT_outputfile_detail_head[ch_idx] =  StrVectToStr(first_line_detail, ",");
                this->m_RT_outputfile_detail[ch_idx].close();
            }
        }

        if (this->m_memory_settings.type == MULTI_CHANNEL)
        {
            auto filename = this->m_outputfile_name + "_" + this->m_memory_settings.name + "_";
            global_settings_failure |= OpenOutputFile(filename + "combined_detail.csv", this->m_outputfile_combined_detail);
            global_settings_failure |= OpenOutputFile(filename + "combined_result.csv", this->m_outputfile_combined_result);
            global_settings_failure |= OpenOutputFile(filename + "mc_summary.csv",      this->m_outputfile_mc_summary);

            this->m_RT_outputfile_combined_detail_name = filename + "combined_detail.csv";
            global_settings_failure |= OpenRTOutputFile(this->m_RT_outputfile_combined_detail_name, this->m_RT_outputfile_combined_detail);
            if (global_settings_failure == RET_SUCCESS)
            {
                // write the first line of the file containing the description of each column
                this->m_outputfile_combined_detail << StrVectToStr(first_line_detail, ",") << "\n";
                this->m_outputfile_combined_detail.flush();

                this->m_outputfile_combined_result << StrVectToStr(first_line_result, ",") << "\n";
                this->m_outputfile_combined_result.flush();

                this->m_outputfile_mc_summary << StrVectToStr(first_line_mc_summary, ",") << "\n";
                this->m_outputfile_mc_summary.flush();

                this->m_RT_outputfile_combined_detail_head = StrVectToStr(first_line_detail, ",");
                this->m_RT_outputfile_combined_detail.close();
            }
        }
    }

    // Print configuration
    if (this->m_memory_settings.type == MULTI_CHANNEL)
    {
        // For multi-channel, always print the global configuration
        LogMessage(MSG_CMN_021, {"Test configuration:"});
        PrintTestcaseConfig(this->m_TC_Cfg.chan_config_global);
        for (const auto & cfg : this->m_mem_chan_config) // print channel config only if it is present in test JSON
        {
            auto ch_idx = cfg.first;
            LogMessage(MSG_CMN_021, {"Memory specific configuration " + GetMessageChan(ch_idx) + ":"});
            PrintTestcaseConfig(this->m_TC_Cfg.chan_config[ch_idx]);
        }
    }
    else
    {
        // For single-channel, only print the channel specific configuration of channel 0
        LogMessage(MSG_CMN_021, {"Test configuration:"});
        PrintTestcaseConfig(this->m_TC_Cfg.chan_config[0]);
    }
    if (global_settings_failure == RET_SUCCESS)
    {
        global_settings_failure |= ParseTestSequenceSettings(this->m_global_test_sequence, this->m_global_test_it_list);
    }
    // The test sequence override for the channel / tag is wrong, do not check for the next channel
    for (int ch_idx=0; ch_idx<this->m_num_channels && (global_settings_failure == RET_SUCCESS); ch_idx++)
    {
        auto it = FindJsonParam(this->m_mem_chan_config[ch_idx], TEST_SEQUENCE_MEMBER);
        if (it == this->m_mem_chan_config[ch_idx].end()) // Test sequence channel/tag overwrite
        {
            // Use global test sequence by default
            auto chan_test_it_list              = this->m_global_test_it_list;
            this->m_chan_test_it_list[ch_idx]   = this->m_global_test_it_list;
            this->m_chan_seq_override[ch_idx]   = false;
        }
        else
        {
            this->m_seq_override = true;
            LogMessage(MSG_CMN_043, {TEST_SEQUENCE_MEMBER.name + " " + GetMessageTagChan(ch_idx)});
            // Parse the test sequence
            auto chan_test_sequence = TestcaseParamCast<std::vector<Memory_Test_Sequence_Parameters_t>>(it->second);
            std::list<TestItConfig_t> chan_test_it_list;
            global_settings_failure |= ParseTestSequenceSettings(chan_test_sequence, chan_test_it_list);
            this->m_chan_test_it_list[ch_idx] = chan_test_it_list;
            this->m_chan_seq_override[ch_idx] = true;
            // Check the number of test in the new test sequence is the same as the global one
            if (global_settings_failure == RET_SUCCESS)
            {
                if (this->m_chan_test_it_list[ch_idx].size() != this->m_global_test_it_list.size())
                {
                    LogMessage(MSG_CMN_044, {std::to_string(this->m_chan_test_it_list[ch_idx].size()), GetMessageTagChan(ch_idx), std::to_string(this->m_global_test_it_list.size())});
                    global_settings_failure = RET_FAILURE;
                }
            }
            if (global_settings_failure == RET_SUCCESS)
            {
                auto it_chan_list   = this->m_chan_test_it_list[ch_idx].begin();
                auto it_global_list = this->m_global_test_it_list.begin();
                // Check all test in the new test sequence have same duration as the global test sequence
                for (uint test_idx = 1; test_idx <= this->m_global_test_it_list.size(); test_idx++)
                {
                    if (it_global_list->duration != it_chan_list->duration)
                    {
                        LogMessage(MSG_CMN_045, {DURATION_TEST_SEQ_MEMBER.name, GetMessageTagChan(ch_idx), std::to_string(it_global_list->duration), std::to_string(test_idx), std::to_string(it_chan_list->duration)});
                        global_settings_failure = RET_FAILURE;
                    }
                    it_chan_list++;
                    it_global_list++;
                }
            }
        }
    }
    if (this->m_use_outputfile)
    {
        auto sensor = this->m_xbtest_sw_config->GetSensorUsed();

        std::vector<std::string> first_line_power;
        first_line_power.emplace_back("Global time (s)");
        first_line_power.emplace_back("Test");
        first_line_power.emplace_back(TEST_MODE);
        first_line_power.emplace_back("Measurement ID");
        first_line_power.emplace_back(WR_MEM_RATE + " (%)");
        first_line_power.emplace_back(RD_MEM_RATE + " (%)");
        first_line_power.emplace_back("Measurement valid");
        for (const auto & source : sensor.fan_sources)
        {
            first_line_power.emplace_back(source.display_name_value);
            first_line_power.emplace_back(source.display_name_status);
        }
        for (const auto & source : sensor.thermal_sources)
        {
            first_line_power.emplace_back(source.display_name_value);
            first_line_power.emplace_back(source.display_name_status);
        }
        for (const auto & source : sensor.power_consumption_sources)
        {
            first_line_power.emplace_back(source.display_name_value);
            first_line_power.emplace_back(source.display_name_status);
        }
        for (const auto & source : sensor.power_rail_sources)
        {
            first_line_power.emplace_back(source.display_name_current_value);
            first_line_power.emplace_back(source.display_name_current_status);
            first_line_power.emplace_back(source.display_name_voltage_value);
            first_line_power.emplace_back(source.display_name_voltage_status);
            first_line_power.emplace_back(source.display_name_power);
        }
        auto filename = this->m_outputfile_name + "_" + this->m_memory_settings.name + "_";
        if (this->m_memory_settings.type == SINGLE_CHANNEL)
        {
            filename += FormatName(GetCUConnectionVppSptag(0), "_") + "_";
        }
        filename += "power.csv";
        global_settings_failure |= OpenOutputFile(filename, this->m_outputfile_power);
        if (global_settings_failure == RET_SUCCESS)
        {
            // write the first line of the file containing the description of each column
            this->m_outputfile_power << StrVectToStr(first_line_power, ",") << "\n";
            this->m_outputfile_power.flush();
        }
    }

    if (global_settings_failure == RET_FAILURE)
    {
        Abort();
    }
    return global_settings_failure;
}

void MemoryTest::Run()
{
    this->m_state   = TestState::TS_RUNNING;
    this->m_result  = TestResult::TR_FAILED;
    auto test_state = TEST_STATE_FAILURE;

    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_008, {GetCurrentTime()});
        test_state = RunTest();
    }

    if ((test_state < TEST_STATE_PASS) || (this->m_abort)) // any negatif state is an abort code
    {
        LogMessage(MSG_CMN_009, {GetCurrentTime()});
        this->m_result = TestResult::TR_ABORTED;
    }
    else if (test_state > TEST_STATE_PASS) // any positive state is an error code
    {
        LogMessage(MSG_CMN_010, {GetCurrentTime()});
        this->m_result = TestResult::TR_FAILED;
    }
    else
    {
        LogMessage(MSG_CMN_011, {GetCurrentTime()});
        this->m_result = TestResult::TR_PASSED;
    }

}

void MemoryTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void MemoryTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

void MemoryTest::PrintTestcaseConfig ( const MemoryCoreCfg_t & chan_config )
{
    LogMessage(MSG_CMN_021, {"\t- " + DISABLE_MEMORY_MEMBER.name      + " : " + BoolToStr(chan_config.disable)});
    LogMessage(MSG_CMN_021, {"\t- " + DISABLE_PREWRITE_MEMBER.name    + " : " + BoolToStr(chan_config.disable_prewrite)});
    LogMessage(MSG_CMN_021, {"\t- " + PATTERN_CTRL_MEMBER.name      + " : " + chan_config.pattern_ctrl});
    LogMessage(MSG_CMN_021, {"\t- " + PATTERN_LIST_MEMBER.name      + " :"});
    for (const auto & pattern : chan_config.pattern_list)
    {
        LogMessage(MSG_CMN_021, {"\t\t* 0x" + NumToStrHex(pattern, 8)});
    }
    if (this->m_num_axi_thread > 1)
    {
        LogMessage(MSG_CMN_021, {"\t- " + SINGLE_AXI_THREAD_MEMBER.name    + " : " + BoolToStr(chan_config.single_axi_thread)});
    }
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ALT_WR_BW_MEMBER.name   + " : " + std::to_string(chan_config.cu_bw.alt_wr_rd.write.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ALT_WR_BW_MEMBER.name   + " : " + std::to_string(chan_config.cu_bw.alt_wr_rd.write.high)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ALT_RD_BW_MEMBER.name   + " : " + std::to_string(chan_config.cu_bw.alt_wr_rd.read.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ALT_RD_BW_MEMBER.name   + " : " + std::to_string(chan_config.cu_bw.alt_wr_rd.read.high)});

    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ONLY_WR_BW_MEMBER.name  + " : " + std::to_string(chan_config.cu_bw.only_wr.write.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ONLY_WR_BW_MEMBER.name  + " : " + std::to_string(chan_config.cu_bw.only_wr.write.high)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ONLY_RD_BW_MEMBER.name  + " : " + std::to_string(chan_config.cu_bw.only_rd.read.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ONLY_RD_BW_MEMBER.name  + " : " + std::to_string(chan_config.cu_bw.only_rd.read.high)});

    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_SIMUL_WR_BW_MEMBER.name + " : " + std::to_string(chan_config.cu_bw.simul_wr_rd.write.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_SIMUL_WR_BW_MEMBER.name + " : " + std::to_string(chan_config.cu_bw.simul_wr_rd.write.high)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_SIMUL_RD_BW_MEMBER.name + " : " + std::to_string(chan_config.cu_bw.simul_wr_rd.read.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_SIMUL_RD_BW_MEMBER.name + " : " + std::to_string(chan_config.cu_bw.simul_wr_rd.read.high)});

    LogMessage(MSG_CMN_021, {"\t- " + CHECK_BW_MEMBER.name              + " : " + BoolToStr(chan_config.check_bw)});

    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ALT_WR_LAT_MEMBER.name   + " : " + std::to_string(chan_config.cu_latency.alt_wr_rd.write.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ALT_WR_LAT_MEMBER.name   + " : " + std::to_string(chan_config.cu_latency.alt_wr_rd.write.high)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ALT_RD_LAT_MEMBER.name   + " : " + std::to_string(chan_config.cu_latency.alt_wr_rd.read.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ALT_RD_LAT_MEMBER.name   + " : " + std::to_string(chan_config.cu_latency.alt_wr_rd.read.high)});

    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ONLY_WR_LAT_MEMBER.name  + " : " + std::to_string(chan_config.cu_latency.only_wr.write.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ONLY_WR_LAT_MEMBER.name  + " : " + std::to_string(chan_config.cu_latency.only_wr.write.high)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_ONLY_RD_LAT_MEMBER.name  + " : " + std::to_string(chan_config.cu_latency.only_rd.read.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_ONLY_RD_LAT_MEMBER.name  + " : " + std::to_string(chan_config.cu_latency.only_rd.read.high)});

    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_SIMUL_WR_LAT_MEMBER.name + " : " + std::to_string(chan_config.cu_latency.simul_wr_rd.write.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_SIMUL_WR_LAT_MEMBER.name + " : " + std::to_string(chan_config.cu_latency.simul_wr_rd.write.high)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_SIMUL_RD_LAT_MEMBER.name + " : " + std::to_string(chan_config.cu_latency.simul_wr_rd.read.low)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_SIMUL_RD_LAT_MEMBER.name + " : " + std::to_string(chan_config.cu_latency.simul_wr_rd.read.high)});

    LogMessage(MSG_CMN_021, {"\t- " + CHECK_LATENCY_MEMBER.name          + " : " + BoolToStr(chan_config.check_latency)});
    LogMessage(MSG_CMN_021, {"\t- " + CHECK_DATA_INTEGRITY_MEMBER.name   + " : " + BoolToStr(chan_config.check_data_integrity)});
}

std::string MemoryTest::GetCUConnectionVppSptag( const int & ch_idx )
{
    return this->m_xbtest_sw_config->GetCUConnectionVppSptag(this->m_cu_name, ch_idx);
}

uint64_t MemoryTest::GetAxiBaseAddress( const int & ch_idx )
{
    return this->m_xbtest_sw_config->GetCuConnectionActualBaseAddress(this->m_cu_name, ch_idx);
}

bool MemoryTest::ReadMemoryCu( const int & ch_idx, const uint & address, uint & read_data )
{
    return this->m_device->ReadMemoryCu(this->m_memory_settings.type, this->m_memory_settings.name, this->m_cu_idx, ch_idx, address, read_data);
}

bool MemoryTest::WriteMemoryCu( const int & ch_idx, const uint & address, const uint & value )
{
    return this->m_device->WriteMemoryCu(this->m_memory_settings.type, this->m_memory_settings.name, this->m_cu_idx, ch_idx, address, value);
}

void MemoryTest::ReadMemoryCuStatus()
{
    uint32_t rd_buffer[this->m_buffer_size/(uint64_t)(sizeof(uint32_t))];
    this->m_device->ReadMemoryCuStatus(this->m_memory_settings.type, this->m_memory_settings.name, this->m_cu_idx, &rd_buffer[0], this->m_buffer_size);

    /*
    LogMessage(MSG_DEBUG_TESTCASE,  {"ReadMemoryCuStatus:"});
    for (uint64_t i = 0; i < MEMORY_CHAN_BUF_SIZE * (uint64_t)(this->m_num_channels) / (uint64_t)(sizeof(uint32_t)); i++)
    {
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t- rd_buffer[" + std::to_string(i) + "] : 0x" + NumToStrHex<uint32_t>(rd_buffer[i])});
    }
    */
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        // offset per channel is in the status buffer
        auto base = MEMORY_CHAN_BUF_SIZE * (uint64_t)(ch_idx)/(uint64_t)(sizeof(uint32_t));

        this->m_chan_status[ch_idx].configuration                =             rd_buffer[base+ 0];                    // REG_STAT_CONFIGURATION
        this->m_chan_status[ch_idx].term_err_cnt                 =            (rd_buffer[base+ 1] >> 0) & 0x000000FF; // REG_STAT_ERROR_COUNT
        this->m_chan_status[ch_idx].axi_addr_ptr                 =  (uint64_t)(rd_buffer[base+ 2]);                   // REG_STAT_AXI_ADDR_PTR_LSB
        this->m_chan_status[ch_idx].axi_addr_ptr                |= ((uint64_t)(rd_buffer[base+ 3]) << 32);            // REG_STAT_AXI_ADDR_PTR_MSB
        this->m_chan_status[ch_idx].wr_burst_latency.reg_total   =  (uint64_t)(rd_buffer[base+ 4]);                   // REG_STAT_WR_BURST_LATENCY_TOTAL_LSB
        this->m_chan_status[ch_idx].wr_burst_latency.reg_total  |= ((uint64_t)(rd_buffer[base+ 5]) << 32);            // REG_STAT_WR_BURST_LATENCY_TOTAL_MSB
        this->m_chan_status[ch_idx].wr_burst_latency.reg_inst    =             rd_buffer[base+ 6];                    // REG_STAT_WR_BURST_LATENCY_INST
        this->m_chan_status[ch_idx].rd_burst_latency.reg_total   =  (uint64_t)(rd_buffer[base+ 7]);                   // REG_STAT_RD_BURST_LATENCY_TOTAL_LSB
        this->m_chan_status[ch_idx].rd_burst_latency.reg_total  |= ((uint64_t)(rd_buffer[base+ 8]) << 32);            // REG_STAT_RD_BURST_LATENCY_TOTAL_MSB
        this->m_chan_status[ch_idx].rd_burst_latency.reg_inst    =             rd_buffer[base+ 9];                    // REG_STAT_RD_BURST_LATENCY_INST
        this->m_chan_status[ch_idx].timestamp                    =             rd_buffer[base+10];                    // REG_STAT_TIMESTAMP

        this->m_chan_status[ch_idx].cfg_updated                  = (this->m_chan_status[ch_idx].configuration >>  0) & 0x00000001;
        this->m_chan_status[ch_idx].term_err                     = (this->m_chan_status[ch_idx].configuration >>  1) & 0x00000001;
        this->m_chan_status[ch_idx].gen_seed_err                 = (this->m_chan_status[ch_idx].configuration >>  2) & 0x00000001;
        this->m_chan_status[ch_idx].term_seed_err                = (this->m_chan_status[ch_idx].configuration >>  3) & 0x00000001;
        this->m_chan_status[ch_idx].toggle_1_sec                 = (this->m_chan_status[ch_idx].configuration >> 15) & 0x00000001;
        this->m_chan_status[ch_idx].timestamp_1_sec              = (this->m_chan_status[ch_idx].configuration >> 16) & 0x0000FFFF;

        /*
        if (ch_idx == 1)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"ReadMemoryCuStatus: " + GetMessageChan(ch_idx)});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- configuration               " + std::to_string(this->m_chan_status[ch_idx].configuration              )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- term_err_cnt                " + std::to_string(this->m_chan_status[ch_idx].term_err_cnt               )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- axi_addr_ptr                " + std::to_string(this->m_chan_status[ch_idx].axi_addr_ptr               )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- wr_burst_latency.reg_total  " + std::to_string(this->m_chan_status[ch_idx].wr_burst_latency.reg_total )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- wr_burst_latency.reg_inst   " + std::to_string(this->m_chan_status[ch_idx].wr_burst_latency.reg_inst  )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- rd_burst_latency.reg_total  " + std::to_string(this->m_chan_status[ch_idx].rd_burst_latency.reg_total )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- rd_burst_latency.reg_inst   " + std::to_string(this->m_chan_status[ch_idx].rd_burst_latency.reg_inst  )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- timestamp                   " + std::to_string(this->m_chan_status[ch_idx].timestamp                  )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- cfg_updated                 " + std::to_string(this->m_chan_status[ch_idx].cfg_updated                )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- term_err                    " + std::to_string(this->m_chan_status[ch_idx].term_err                   )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- gen_seed_err                " + std::to_string(this->m_chan_status[ch_idx].gen_seed_err               )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- term_seed_err               " + std::to_string(this->m_chan_status[ch_idx].term_seed_err              )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- toggle_1_sec                " + std::to_string(this->m_chan_status[ch_idx].toggle_1_sec               )});
            LogMessage(MSG_DEBUG_TESTCASE, {"\t- timestamp_1_sec             " + std::to_string(this->m_chan_status[ch_idx].timestamp_1_sec            )});
        }
        */
    }
    /*
    LogMessage(MSG_DEBUG_TESTCASE, {"Instantaneous measurement received for all running channel"});
    std::vector<std::string> results_header;
    uint col_size_1 = 16;
    uint col_size = 10;
    results_header.emplace_back(pad(           "Channel", ' ', col_size_1, PAD_ON_RIGHT));

    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        results_header.emplace_back(pad(std::to_string(ch_idx), ' ', col_size, PAD_ON_RIGHT));
    }
    LogMessage(MSG_DEBUG_TESTCASE, {StrVectToStr(results_header, " | ")});

    std::vector<std::string> results_wr;
    results_wr.emplace_back(pad(           "Write [Qty/s]", ' ', col_size_1, PAD_ON_RIGHT));
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        results_wr.emplace_back(pad(std::to_string(this->m_chan_status[ch_idx].wr_burst_latency.reg_inst), ' ', col_size, PAD_ON_RIGHT));
    }
    LogMessage(MSG_DEBUG_TESTCASE, {StrVectToStr(results_wr, " | ")});

    std::vector<std::string> results_rd;
    results_rd.emplace_back(pad(           "Read [Qty/s]", ' ', col_size_1, PAD_ON_RIGHT));
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        results_rd.emplace_back(pad(std::to_string(this->m_chan_status[ch_idx].rd_burst_latency.reg_inst), ' ', col_size, PAD_ON_RIGHT));
    }
    LogMessage(MSG_DEBUG_TESTCASE, {StrVectToStr(results_rd, " | ")});
    */

}

void MemoryTest::ComputeStatBurst()
{
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        GetLiveStatBurst(this->m_chan_status[ch_idx].wr_burst_latency, this->m_chan_test_it[ch_idx].write.burst_size);
        GetLiveStatBurst(this->m_chan_status[ch_idx].rd_burst_latency, this->m_chan_test_it[ch_idx].read.burst_size);

        AccumulateBurstStat(this->m_chan_status[ch_idx].wr_burst_latency, this->m_chan_test_it[ch_idx].write.burst_size);
        AccumulateBurstStat(this->m_chan_status[ch_idx].rd_burst_latency, this->m_chan_test_it[ch_idx].read.burst_size);
    }
}

double MemoryTest::StatBurstValToNs( const double & val )
{
    return val * (double)(1000) / (double)(this->m_xbtest_sw_config->GetClockFrequency(0));
}

double MemoryTest::ComputeAverageMbps( const uint & burst_size, const double & average_ns )
{
    double average_mbps = 0.0; // Set to 0 if cannot compute
    if (average_ns > 0.0)
    {
        average_mbps = (double)(burst_size) * BPNS_TO_MBPS / average_ns; // Used to have burst time in MBps
    }
    return average_mbps;
}

double MemoryTest::ComputeBwMbps( const uint & burst_size, const double & inst )
{
    return (double)(burst_size) * inst / (double)(1024) / (double)(1024);;
}

void MemoryTest::GetLiveStatBurst( Stat_Burst_t & stat_burst, const uint & burst_size )
{
    stat_burst.live_inst       =                  (double)(stat_burst.reg_inst);
    stat_burst.live_total_ns   = StatBurstValToNs((double)(stat_burst.reg_total));
    stat_burst.live_average_ns = 0.0; // Set to 0 if cannot compute
    if (stat_burst.live_inst > 0.0)
    {
        stat_burst.live_average_ns  = stat_burst.live_total_ns / stat_burst.live_inst; // ns
    }

    stat_burst.live_average_mbps   = ComputeAverageMbps(burst_size, stat_burst.live_average_ns);
    stat_burst.live_bw_mbps        = ComputeBwMbps(burst_size, stat_burst.live_inst);
}

void MemoryTest::AccumulateBurstStat( Stat_Burst_t & stat_burst, const uint & burst_size )
{
    if (this->m_hw_sec_cnt == 1)
    {
        stat_burst.acc_inst = stat_burst.live_inst;
        stat_burst.acc_ns   = stat_burst.live_average_ns;
    }
    else
    {
        stat_burst.acc_inst += stat_burst.live_inst;
        stat_burst.acc_ns   += stat_burst.live_average_ns;
    }
    stat_burst.average_inst    = stat_burst.acc_inst  / (double)(this->m_hw_sec_cnt);
    stat_burst.average_ns      = stat_burst.acc_ns    / (double)(this->m_hw_sec_cnt);
    stat_burst.average_mbps    = ComputeAverageMbps(burst_size, stat_burst.average_ns);
    stat_burst.average_bw_mbps = ComputeBwMbps     (burst_size, stat_burst.average_inst);
}

void MemoryTest::WaitHw1SecToggle()
{
    uint watchdog = 4; // Ensure 1 sec is detected in 4 * 333 ms = 1.33 sec
    while ((watchdog > 0) && (!(this->m_abort)))
    {
        this->m_tog_1sec = GetHw1secToggle(this->m_tog_ch_idx);
        if (this->m_tog_1sec != this->m_tog_1sec_last)
        {
            this->m_tog_1sec_last = this->m_tog_1sec;
            break;
        }
        this->m_timer->WaitFor333msTick();
        watchdog--;
    }

    this->m_timer->WaitFor333msTick(); // Wait 333 ms between HW sec toggle detected and reading status
    ReadMemoryCuStatus();
    this->m_hw_sec_cnt++;

    if (watchdog == 0)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"1 sec HW toggle not detected"});
        this->m_toggle_error_cnt++;
        if (this->m_toggle_error_cnt >= 5)
        {
            LogMessage(MSG_MEM_019, {std::to_string(this->m_toggle_error_cnt)});
        }
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"1 sec toggle detected after " + std::to_string(4-watchdog) + " wait of 333ms"});
        this->m_toggle_error_cnt = 0;
    }

    // reset HW watchdog
    ResetWatchdog();

    CheckTimestamp1Sec();
}

void MemoryTest::CheckTimestamp1Sec()
{
    // Under normal operations, the HW 1sec timestamp shall increment by one between two consecutive measurements (read by host every 1 sec)
    // Do not check the HW 1sec timestamp is the same for all channels
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        // LogMessage(MSG_DEBUG_TESTCASE, {"Ch(" + std::to_string(ch_idx) + "): toggle_1_sec: " + std::to_string(this->m_chan_status[ch_idx].toggle_1_sec) + " / timestamp_1_sec: " + std::to_string(this->m_chan_status[ch_idx].timestamp_1_sec)});
        if (this->m_hw_sec_cnt > 1)
        {
            if (this->m_chan_status[ch_idx].timestamp_1_sec != this->m_timestamp_1_sec_expected[ch_idx])
            {
                if (this->m_mem_048_display)
                {
                    LogMessage(MSG_MEM_048, {std::to_string(this->m_chan_status[ch_idx].timestamp_1_sec), std::to_string(this->m_timestamp_1_sec_expected[ch_idx]), GetMessageChan(ch_idx)});
                    this->m_mem_048_display = false;
                }
            }
        }
        this->m_timestamp_1_sec_expected[ch_idx] = this->m_chan_status[ch_idx].timestamp_1_sec + 1;
    }
}

void MemoryTest::PrintRegStatBurst( const std::string & stat_name, const Stat_Burst_t & stat_burst )
{
    LogMessage(MSG_DEBUG_TESTCASE, {stat_name + ".reg_inst            : 0x" + NumToStrHex<uint>(    stat_burst.reg_inst )});
    LogMessage(MSG_DEBUG_TESTCASE, {stat_name + ".reg_total           : 0x" + NumToStrHex<uint64_t>(stat_burst.reg_total)});
    LogMessage(MSG_DEBUG_TESTCASE, {stat_name + ".live_inst           : " + Float_to_String<double>(stat_burst.live_inst,          3) + " ns"});
    LogMessage(MSG_DEBUG_TESTCASE, {stat_name + ".live_total_ns       : " + Float_to_String<double>(stat_burst.live_total_ns,      3) + " ns"});
    LogMessage(MSG_DEBUG_TESTCASE, {stat_name + ".live_average_ns     : " + Float_to_String<double>(stat_burst.live_average_ns,    3) + " ns"});
    LogMessage(MSG_DEBUG_TESTCASE, {stat_name + ".live_average_mbps   : " + Float_to_String<double>(stat_burst.live_average_mbps,  3) + " MBps"});
    LogMessage(MSG_DEBUG_TESTCASE, {stat_name + ".live_bw_mbps        : " + Float_to_String<double>(stat_burst.live_bw_mbps,       3) + " MBps"});
}

void MemoryTest::PrintStatus()
{
    LogMessage(MSG_DEBUG_TESTCASE, {"xbtest HW IP configuration at this->m_hw_sec_cnt = " + std::to_string(this->m_hw_sec_cnt) + ":"});
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t- xbtest HW IP configuration " + GetMessageChan(ch_idx) + ":"});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* configuration         : 0x" + NumToStrHex<uint>(    this->m_chan_status[ch_idx].configuration     )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* axi_addr_ptr          : 0x" + NumToStrHex<uint64_t>(this->m_chan_status[ch_idx].axi_addr_ptr      )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* timestamp             : 0x" + NumToStrHex<uint>(    this->m_chan_status[ch_idx].timestamp         )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* toggle_1_sec          : "   + std::to_string(       this->m_chan_status[ch_idx].toggle_1_sec      )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* timestamp_1_sec       : "   + std::to_string(       this->m_chan_status[ch_idx].timestamp_1_sec   )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* cfg_updated           : "   + std::to_string(       this->m_chan_status[ch_idx].cfg_updated       )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* term_err              : "   + std::to_string(       this->m_chan_status[ch_idx].term_err          )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* term_err_cnt          : "   + std::to_string(       this->m_chan_status[ch_idx].term_err_cnt      )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* gen_seed_err          : "   + std::to_string(       this->m_chan_status[ch_idx].gen_seed_err      )});
        LogMessage(MSG_DEBUG_TESTCASE,  {"\t\t* term_seed_err         : "   + std::to_string(       this->m_chan_status[ch_idx].term_seed_err     )});
        PrintRegStatBurst(             "\t\t* wr_burst_latency",                                  this->m_chan_status[ch_idx].wr_burst_latency    );
        PrintRegStatBurst(             "\t\t* rd_burst_latency",                                  this->m_chan_status[ch_idx].rd_burst_latency    );
    }
}

void MemoryTest::ClearError( const int & ch_idx )
{
    uint read_data;
    ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data | MEM_CTRL_CLEAR_ERR);
}

void MemoryTest::SetRateReg( const int & ch_idx, const double & value_percent, const uint & burst_xfer, const uint & rate_ctrl_en, const uint & rate_reg_addr )
{
    uint read_data;
    ReadMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
    uint rate_reg_val = 0;
    if (value_percent == 100.0)
    {
        read_data &= ~rate_ctrl_en;
        rate_reg_val = 8191; // Not used now but leave this so it works with previous build (13 bits at 1 => synth'high=1 nearly all time for value_percent == 100)
    }
    else
    {
        read_data |= rate_ctrl_en;
        // For burst_xfer = 64 beats
        // value_percent = 50 => reg = 64
        // value_percent = 99 => reg = 127
        // and extend reg depending on burst size
        //  - e.g when reg = 128, then 1 req every 64 clock cycles
        double reg_f = (value_percent*(double)(127)/(double)(99)) * ((double)(64)/(double)(burst_xfer));
        rate_reg_val = (uint)reg_f;
        if (reg_f - rate_reg_val > 0.5)
        {
            rate_reg_val++; // rounds
        }
        if (rate_reg_val == 0)
        {
            rate_reg_val = 1;
        }
    }
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
    WriteMemoryCu(ch_idx, rate_reg_addr, rate_reg_val);
}

void MemoryTest::SetWrRate( const int & ch_idx, const double & value_percent, const uint & burst_xfer )
{
    SetRateReg(ch_idx, value_percent, burst_xfer, WR_MEM_RATE_EN, REG_CTRL_WR_MEM_RATE);
}

void MemoryTest::SetRdRate( const int & ch_idx, const double & value_percent, const uint & burst_xfer )
{
    SetRateReg(ch_idx, value_percent, burst_xfer, RD_MEM_RATE_EN, REG_CTRL_RD_MEM_RATE);
}

void MemoryTest::EnablePattern( const int & ch_idx )
{
    uint read_data;
    ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data | PATTERN_EN);
}

void MemoryTest::DisablePattern( const int & ch_idx )
{
    uint read_data;
    ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data & ~PATTERN_EN);
}

void MemoryTest::SetPatternA( const int & ch_idx, const uint & pattern )
{
    WriteMemoryCu(ch_idx, REG_CTRL_PATTERN_A, pattern);
}

void MemoryTest::SetPatternB( const int & ch_idx, const uint & pattern )
{
    WriteMemoryCu(ch_idx, REG_CTRL_PATTERN_B, pattern);
}

void MemoryTest::SetAxiMultiThread( const int & ch_idx )
{
    uint read_data;
    ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data | MULTI_THREAD_EN);
}

void MemoryTest::SetAxiSingleThread( const int & ch_idx )
{
    uint read_data;
    ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data & ~MULTI_THREAD_EN);
}

void MemoryTest::SetOutstandingWrites( const int & ch_idx, const uint & value )
{
    uint read_data;

    if (value == 0)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Maximum quantity of outstanding writes is not limited " + GetMessageChan(ch_idx)});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Maximum quantity of outstanding writes is limited to " + std::to_string(value) + " " + GetMessageChan(ch_idx)});
    }
    // Set value
    uint reg = 0; // Set to 0 when disabled
    if (value > 0)
    {
        reg = ((value-2) << 16); // MSB = 1 <=> value = 1
    }
    ReadMemoryCu(ch_idx, REG_CTRL_OUTSTANDING, read_data);
    read_data = (read_data & ~OUTSTANDING_WR_MASK) + (reg & OUTSTANDING_WR_MASK);
    WriteMemoryCu(ch_idx, REG_CTRL_OUTSTANDING, read_data);

    // Set enable bit
    ReadMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
    if (value > 0)
    {
        read_data |= OUTSTANDING_WR_EN;
    }
    else
    {
        read_data &= ~OUTSTANDING_WR_EN;
    }
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
}

void MemoryTest::SetOutstandingReads( const int & ch_idx, const uint & value )
{
    uint read_data;

    if (value == 0)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Maximum quantity of outstanding reads is not limited " + GetMessageChan(ch_idx)});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Maximum quantity of outstanding reads is limited to " + std::to_string(value) + " " + GetMessageChan(ch_idx)});
    }
    // Set value
    uint reg = 0; // Set to 0 when disabled
    if (value > 0)
    {
        reg = ((value-2) << 0); // MSB = 1 <=> value = 1
    }
    ReadMemoryCu(ch_idx, REG_CTRL_OUTSTANDING, read_data);
    read_data = (read_data & ~OUTSTANDING_RD_MASK) + (reg & OUTSTANDING_RD_MASK);
    WriteMemoryCu(ch_idx, REG_CTRL_OUTSTANDING, read_data);

    // Set enable bit
    ReadMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
    if (value > 0)
    {
        read_data |= OUTSTANDING_RD_EN;
    }
    else
    {
        read_data &= ~OUTSTANDING_RD_EN;
    }
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
}

void MemoryTest::SetMode( const int & ch_idx, const uint & value )
{
    uint read_data;
    // LogMessage(MSG_DEBUG_TESTCASE, {"Setting mode " + GetMessageChan(ch_idx) + " to " + TestModeEnumToString(value)});
    ReadMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
    read_data = (((value <<  4) & MEM_TEST_MODE_MASK) | (read_data & ~MEM_TEST_MODE_MASK));
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data);
}

bool MemoryTest::StartCU()
{
    return RET_SUCCESS;
}

bool MemoryTest::EnableWatchdogClkThrotDetection()
{
    return RET_SUCCESS;
}

bool MemoryTest::StopCU()
{
    return RET_SUCCESS;
}

bool MemoryTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_SUCCESS;
}

bool MemoryTest::StopCuCores()
{
    uint read_data;
    LogMessage(MSG_DEBUG_TESTCASE, {"Stop Memory xbtest HW IP"});
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
        WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data | MEM_CTRL_STOP);
    }
    if (WaitCuConfigUpdated(this->m_enabled_ch_indexes) == RET_FAILURE)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Stop Memory xbtest HW IP failed while waiting for configuration update"});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

void MemoryTest::ActivateResetCores()
{
    uint read_data;
    LogMessage(MSG_DEBUG_TESTCASE, {"Activate Memory xbtest HW IP reset"});
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
        WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data | MEM_CTRL_RESET);
    }
}

void MemoryTest::ClearResetCores()
{
    uint read_data;
    LogMessage(MSG_DEBUG_TESTCASE, {"Clear Memory xbtest HW IP reset"});
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
        WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data & ~MEM_CTRL_RESET);
    }
}

void MemoryTest::UpdateCuConfig( const int & ch_idx )
{
    uint read_data;
    // LogMessage(MSG_DEBUG_TESTCASE, {"Update Memory xbtest HW IP configuration " + GetMessageChan(ch_idx)});
    ReadMemoryCu (ch_idx, REG_CTRL_CONFIGURATION, read_data);
    WriteMemoryCu(ch_idx, REG_CTRL_CONFIGURATION, read_data | MEM_CTRL_UPDATE_CFG);
}

bool MemoryTest::WaitCuConfigUpdated( const std::vector<uint> & ch_indexes )
{
    // LogMessage(MSG_DEBUG_TESTCASE, {"Check Memory xbtest HW IP channels configuration updated"});

    const uint64_t MAX_CNT_1S_TICK = 60 * 60; // display message every 60 min
    uint64_t cnt_1s_tick_msg = 60; // display first WARNING after 60 s
    uint cnt_1s_tick = 0;

    // Init
    auto all_cfg_updated = false;
    std::map<uint, bool> cfg_updated;
    for (const auto & ch_idx : ch_indexes)
    {
        cfg_updated[ch_idx] = false;
    }

    while ((!all_cfg_updated) && (!(this->m_abort)))
    {
        ReadMemoryCuStatus();
        for (const auto & ch_idx : ch_indexes)
        {
            if (!cfg_updated[ch_idx] && (this->m_chan_status[ch_idx].cfg_updated == 1))
            {
                cfg_updated[ch_idx] = true;
                LogMessage(MSG_DEBUG_TESTCASE, {"New test configuration is running in the xbtest HW IP " + GetMessageChan(ch_idx)});
            }
        }
        all_cfg_updated = true;
        for (const auto & ch_idx : ch_indexes)
        {
            if (!cfg_updated[ch_idx])
            {
                all_cfg_updated = false;
                LogMessage(MSG_DEBUG_TESTCASE, {"Waiting configuration update in the xbtest HW IP " + GetMessageChan(ch_idx)});
            }
        }
        if (all_cfg_updated)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Configuration updated in the xbtest HW IP for all channels"});
        }
        if ((!all_cfg_updated) && (!(this->m_abort)))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Waiting 1 second before checking configuration updated"});
            WaitSecTick(1);
            cnt_1s_tick++;
            if (cnt_1s_tick == cnt_1s_tick_msg)
            {
                LogMessage(MSG_MEM_044, {std::to_string(cnt_1s_tick)}); // error: config not updated
                cnt_1s_tick_msg *= 2; // next message will be 2 min, then after 4 min, 8 min...
            }
            if (cnt_1s_tick % MAX_CNT_1S_TICK == 0)
            {
                LogMessage(MSG_MEM_045, {std::to_string(MAX_CNT_1S_TICK)}); // error: config not updated
                return RET_FAILURE;
            }
        }
    }
    LogMessage(MSG_DEBUG_TESTCASE, {"Clear errors after configuration is updated"});
    for (const auto & ch_idx : ch_indexes)
    {
        ClearError(ch_idx);
    }
    return RET_SUCCESS;
}

void MemoryTest::SetWrCtrlAddr( const int & ch_idx, const uint & addr_mb )
{
    uint64_t addr_bytes = (uint64_t)(addr_mb) * (uint64_t)(1024) * (uint64_t)(1024);
    // addr_bytes += GetAxiBaseAddress(ch_idx);
    WriteMemoryCu(ch_idx, REG_CTRL_WR_START_ADDR_LSB, (uint)( addr_bytes        & 0xFFFFFFFF));
    WriteMemoryCu(ch_idx, REG_CTRL_WR_START_ADDR_MSB, (uint)((addr_bytes >> 32) & 0xFFFFFFFF));
}

void MemoryTest::SetWrCtrlXferBytes( const int & ch_idx, const uint & value )
{
    WriteMemoryCu(ch_idx, REG_CTRL_WR_BURST_SIZE, (uint)((value - 1) & 0xFFFFFFFF));
}

void MemoryTest::SetWrCtrlNumXfer( const int & ch_idx, const uint & value )
{
    WriteMemoryCu(ch_idx, REG_CTRL_WR_NUM_XFER, (uint)((value - 1) & 0xFFFFFFFF));
}

void MemoryTest::SetRdCtrlAddr( const int & ch_idx, const uint & addr_mb )
{
    uint64_t addr_bytes = (uint64_t)(addr_mb)*(uint64_t)(1024) * (uint64_t)(1024);
    // addr_bytes += GetAxiBaseAddress(ch_idx);
    WriteMemoryCu(ch_idx, REG_CTRL_RD_START_ADDR_LSB, (uint)(addr_bytes & 0xFFFFFFFF));
    WriteMemoryCu(ch_idx, REG_CTRL_RD_START_ADDR_MSB, (addr_bytes >> 32) & 0xFFFFFFFF);
}

void MemoryTest::SetRdCtrlXferBytes( const int & ch_idx, const uint & value )
{
    WriteMemoryCu(ch_idx, REG_CTRL_RD_BURST_SIZE, (uint)((value - 1) & 0xFFFFFFFF));
}

void MemoryTest::SetRdCtrlNumXfer( const int & ch_idx, const uint & value )
{
    WriteMemoryCu(ch_idx, REG_CTRL_RD_NUM_XFER, (uint)((value - 1) & 0xFFFFFFFF));
}

void MemoryTest::CheckDataIntegrity()
{
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            if (!StrMatchNoCase(this->m_TC_Cfg.chan_config[ch_idx].pattern_ctrl, PATTERN_CTRL_PRBS) | !this->m_TC_Cfg.chan_config[ch_idx].check_data_integrity)
            {
                this->m_data_integrity[ch_idx].live   = DATA_INTEG_NC;
                this->m_data_integrity[ch_idx].result = DATA_INTEG_NC;
            }
            else if (CheckPrbsErrorLatches(ch_idx) == RET_FAILURE)
            {
                this->m_data_integrity[ch_idx].live   = DATA_INTEG_KO;
                this->m_data_integrity[ch_idx].result = DATA_INTEG_KO;
                ClearError(ch_idx);
            }
            else
            {
                this->m_data_integrity[ch_idx].live   = DATA_INTEG_OK;
            }
        }
    }
}

void MemoryTest::GetCombinedDataIntegrity()
{
    auto data_integrity_fail = false;
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            if (this->m_data_integrity[ch_idx].live == DATA_INTEG_KO)
            {
                data_integrity_fail = true;
                break;
            }
        }
    }
    if (!this->m_combined_check_data_integrity)
    {
        this->m_combined_data_integrity.live   = DATA_INTEG_NC;
        this->m_combined_data_integrity.result = DATA_INTEG_NC;
    }
    else if (data_integrity_fail)
    {
        this->m_combined_data_integrity.live   = DATA_INTEG_KO;
        this->m_combined_data_integrity.result = DATA_INTEG_KO;
    }
    else
    {
        this->m_combined_data_integrity.live   = DATA_INTEG_OK;
    }
}

void MemoryTest::GetWrPlusRdBw()
{
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        this->m_bw_wr_plus_rd[ch_idx] = RESET_STAT_BURST;
        if (this->m_chan_test_it[ch_idx].is_write)
        {
            this->m_bw_wr_plus_rd[ch_idx].live_bw_mbps    += this->m_chan_status[ch_idx].wr_burst_latency.live_bw_mbps;
            this->m_bw_wr_plus_rd[ch_idx].average_bw_mbps += this->m_chan_status[ch_idx].wr_burst_latency.average_bw_mbps;
        }
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            this->m_bw_wr_plus_rd[ch_idx].live_bw_mbps    += this->m_chan_status[ch_idx].rd_burst_latency.live_bw_mbps;
            this->m_bw_wr_plus_rd[ch_idx].average_bw_mbps += this->m_chan_status[ch_idx].rd_burst_latency.average_bw_mbps;
        }
    }
}

void MemoryTest::GetCombinedWrPlusRdBw()
{
    this->m_combined_bw_wr_plus_rd = RESET_STAT_BURST;
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        this->m_combined_bw_wr_plus_rd.live_bw_mbps    += this->m_bw_wr_plus_rd[ch_idx].live_bw_mbps;
        this->m_combined_bw_wr_plus_rd.average_bw_mbps += this->m_bw_wr_plus_rd[ch_idx].average_bw_mbps;
    }
}

MemoryTest::Stat_Burst_t MemoryTest::GetCombinedBurstStat( const std::string & stat )
{
    Stat_Burst_t combined = RESET_STAT_BURST;
    std::vector<uint> wr_rd_ch_idx;
    if (stat == "write burst latency")
    {
        wr_rd_ch_idx = this->m_is_write_ch_idx;
    }
    else
    {
        wr_rd_ch_idx = this->m_is_read_ch_idx;
    }
    for (const auto & ch_idx : wr_rd_ch_idx)
    {
        Stat_Burst_t stat_burst;
        if (stat == "write burst latency")
        {
            stat_burst = this->m_chan_status[ch_idx].wr_burst_latency;
        }
        else
        {
            stat_burst = this->m_chan_status[ch_idx].rd_burst_latency;
        }
        // combined average burst latency is the average of all channels
        combined.live_inst            += stat_burst.live_inst          / (double)(wr_rd_ch_idx.size());
        combined.live_total_ns        += stat_burst.live_total_ns      / (double)(wr_rd_ch_idx.size());
        combined.live_average_ns      += stat_burst.live_average_ns    / (double)(wr_rd_ch_idx.size());
        combined.live_average_mbps    += stat_burst.live_average_mbps  / (double)(wr_rd_ch_idx.size());
        combined.average_inst         += stat_burst.average_inst       / (double)(wr_rd_ch_idx.size());
        combined.average_ns           += stat_burst.average_ns         / (double)(wr_rd_ch_idx.size());
        combined.average_mbps         += stat_burst.average_mbps       / (double)(wr_rd_ch_idx.size());
        // combined BW is the sum of all channels
        combined.live_bw_mbps     += stat_burst.live_bw_mbps;
        combined.average_bw_mbps  += stat_burst.average_bw_mbps;
    }
    return combined;
}

bool MemoryTest::CheckPrbsErrorLatches( const int & ch_idx )
{
    auto ret = RET_SUCCESS;
    if (this->m_chan_status[ch_idx].term_seed_err == 1)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"Term seed error detected " + GetMessageChan(ch_idx)});
        ret = RET_FAILURE;
    }
    if (this->m_chan_status[ch_idx].gen_seed_err == 1)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"Gen seed error detected " + GetMessageChan(ch_idx)});
        ret = RET_FAILURE;
    }
    if (this->m_chan_status[ch_idx].term_err == 1)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"Data integrity error detected " + GetMessageChan(ch_idx)});
        ret = RET_FAILURE;
    }
    return ret;
}

uint MemoryTest::GetHw1secToggle( const int & ch_idx )
{
    uint read_data;
    ReadMemoryCu(ch_idx, REG_STAT_CONFIGURATION, read_data);
    return (read_data >> 15) & 0x00000001;
}

bool MemoryTest::ReadMemoryCu64b( const int & ch_idx, const uint & addr_lsb, const uint & addr_msb, uint64_t & read_data_64 )
{
    read_data_64 = 0;
    uint read_data;

    if (ReadMemoryCu(ch_idx, addr_msb, read_data) == RET_FAILURE) { return RET_FAILURE; }
    read_data_64 = (uint64_t)read_data;
    read_data_64 = (read_data_64 << 32);

    if (ReadMemoryCu(ch_idx, addr_lsb, read_data) == RET_FAILURE) { return RET_FAILURE; }
    read_data_64 |= (uint64_t)read_data;

    return RET_SUCCESS;
}

uint64_t MemoryTest::GetAxiAddrPtr ( const int & ch_idx )
{
    uint64_t read_data_64;
    ReadMemoryCu64b(ch_idx, REG_STAT_AXI_ADDR_PTR_LSB, REG_STAT_AXI_ADDR_PTR_MSB, read_data_64);
    return read_data_64;
}

bool MemoryTest::SetSequenceAndUpdateCores()
{
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        SetSequence(ch_idx, this->m_chan_test_it[ch_idx]);
    }
    LogMessage(MSG_DEBUG_TESTCASE, {"Update Memory xbtest HW IP configuration"});
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        UpdateCuConfig(ch_idx);
    }
    if (WaitCuConfigUpdated(this->m_enabled_ch_indexes) == RET_FAILURE)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Set Memory xbtest HW IP sequence failed while waiting for configuration update"});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

void MemoryTest::SetSequence( const int & ch_idx, const TestItConfig_t & test_it )
{
    // LogMessage(MSG_DEBUG_TESTCASE, {"Setup Memory xbtest HW IP " + GetMessageChan(ch_idx)});
    SetMode(ch_idx, test_it.mode);

    if (test_it.is_write)
    {
        SetWrCtrlAddr           (ch_idx, test_it.write.start_addr);
        SetWrCtrlXferBytes      (ch_idx, test_it.write.burst_xfer);
        SetWrCtrlNumXfer        (ch_idx, test_it.write.total_xfer);
        SetWrRate               (ch_idx, test_it.write.rate, test_it.write.burst_xfer);
        SetOutstandingWrites    (ch_idx, test_it.write.outstanding);
    }
    if (test_it.is_read)
    {
        SetRdCtrlAddr           (ch_idx, test_it.read.start_addr);
        SetRdCtrlXferBytes      (ch_idx, test_it.read.burst_xfer);
        SetRdCtrlNumXfer        (ch_idx, test_it.read.total_xfer);
        SetRdRate               (ch_idx, test_it.read.rate, test_it.read.burst_xfer);
        SetOutstandingReads     (ch_idx, test_it.read.outstanding);
    }
}

void MemoryTest::WriteToMeasurementFileDetail(
    const std::string &         global_time,
    std::ofstream &             measurement_file,
    std::ofstream &             RT_measurement_file,
    std::string &               RT_measurement_file_name,
    std::string &               RT_measurement_file_head,
    const uint &                test_idx,
    const bool &                seq_override,
    const bool &                is_write,
    const bool &                is_read,
    const TestItConfig_t &      test_it,
    const uint &                timestamp_1_sec,
    const Data_Integrity_t &    data_integrity,
    const Stat_Burst_t &        burst_latency_and_bw_wr,
    const Stat_Burst_t &        burst_latency_and_bw_rd,
    const Stat_Burst_t &        bw_wr_plus_rd
)
{
    if ( (!(this->m_use_outputfile)) || this->m_abort)
    {
        return;
    }
    std::vector<std::string> str_vect;
    str_vect.emplace_back(global_time);
    str_vect.emplace_back(std::to_string(test_idx));
    if (!seq_override)
    {
        str_vect.emplace_back(TestModeEnumToString(test_it.mode));
    }
    else
    {
        str_vect.emplace_back(OVR);
    }
    str_vect.emplace_back(std::to_string(this->m_hw_sec_cnt));
    if (is_read)
    {
        str_vect.emplace_back(data_integrity.live);
        str_vect.emplace_back(data_integrity.result);
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }

    if (is_write || is_read) // Test can be stopped
    {
        str_vect.emplace_back(Float_to_String<double>(bw_wr_plus_rd.live_bw_mbps,    3));
        str_vect.emplace_back(Float_to_String<double>(bw_wr_plus_rd.average_bw_mbps, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }

    if (is_write)
    {
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.live_bw_mbps,    3));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.average_bw_mbps, 3));

        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.live_inst,    0));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.average_inst, 3));

        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.live_average_ns, 3));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.average_ns,      3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (is_read)
    {
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.live_bw_mbps,    3));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.average_bw_mbps, 3));

        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.live_inst,    0));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.average_inst, 3));

        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.live_average_ns, 3));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.average_ns,      3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    str_vect.emplace_back(std::to_string(timestamp_1_sec));

    measurement_file << StrVectToStr(str_vect, ",") << "\n";
    measurement_file.flush();

    (void)OpenRTOutputFile(RT_measurement_file_name, RT_measurement_file);  // Don't check if open worked..
    RT_measurement_file << RT_measurement_file_head + "\n" + StrVectToStr(str_vect, ",");
    RT_measurement_file.flush();
    RT_measurement_file.close();
}

void MemoryTest::WriteToMeasurementFilePower(
    const std::string &     global_time,
    std::ofstream &         measurement_file,
    const uint &            test_idx,
    const bool &            seq_override,
    const bool &            is_write,
    const bool &            is_read,
    const TestItConfig_t &  test_it
)
{
    if ( (!(this->m_use_outputfile)) || this->m_abort)
    {
        return;
    }
    std::vector<std::string> str_vect;
    str_vect.emplace_back(global_time);
    str_vect.emplace_back(std::to_string(test_idx));
    if (!seq_override)
    {
        str_vect.emplace_back(TestModeEnumToString(test_it.mode));
    }
    else
    {
        str_vect.emplace_back(OVR);
    }
    str_vect.emplace_back(std::to_string(this->m_hw_sec_cnt));

    if (is_write)
    {
        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(test_it.write.rate));
        }
        else
        {
            str_vect.emplace_back(OVR);
        }
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (is_read)
    {
        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(test_it.read.rate));
        }
        else
        {
            str_vect.emplace_back(OVR);
        }
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }

    auto sensor = this->m_devicemgt->GetSensorInstant();

    if (sensor.dump_valid)
    {
        str_vect.emplace_back("OK");
    }
    else
    {
        str_vect.emplace_back("KO");
    }
    for (const auto & source : sensor.fan_sources)
    {
        if (source.found && source.info.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.speed_rpm));
            str_vect.emplace_back(               source.info.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
    }
    for (const auto & source : sensor.thermal_sources)
    {
        if (source.found && source.info.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.temp_c));
            str_vect.emplace_back(               source.info.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
    }
    for (const auto & source : sensor.power_consumption_sources)
    {
        if (source.found)
        {
            str_vect.emplace_back(std::to_string(source.info.power_consumption_watts));
            str_vect.emplace_back(               source.info.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
    }
    for (const auto & source : sensor.power_rail_sources)
    {
        if (source.found && source.info.current.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.current.amps));
            str_vect.emplace_back(               source.info.current.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
        if (source.found && source.info.voltage.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.voltage.volts));
            str_vect.emplace_back(               source.info.voltage.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
        if (source.found && source.power_is_present)
        {
            str_vect.emplace_back(std::to_string(source.power));
        }
        else
        {
            str_vect.emplace_back("0.0");
        }
    }
    measurement_file << StrVectToStr(str_vect, ",") << "\n";
    measurement_file.flush();

    // live display in console
    // erase the first 2 elements:
    // std::vector<std::string> str_vect_console = str_vect;
    // str_vect_console.erase (str_vect_console.begin(),str_vect_console.begin()+2);
    // LogMessage(MSG_DEBUG_TESTCASE, {StrVectToStr(str_vect_console, " | ")});
}

void MemoryTest::WriteToMeasurementFileResult(
    std::ofstream &             measurement_file,
    const uint &                test_idx,
    const bool &                seq_override,
    const bool &                is_write,
    const bool &                is_read,
    const TestItConfig_t &      test_it,
    const Data_Integrity_t &    data_integrity,
    const Stat_Burst_t &        burst_latency_and_bw_wr,
    const Stat_Burst_t &        burst_latency_and_bw_rd,
    const Stat_Burst_t &        bw_wr_plus_rd
)
{
    if ( (!(this->m_use_outputfile)) || this->m_abort)
    {
        return;
    }
    std::vector<std::string> str_vect;
    str_vect.emplace_back(std::to_string(test_idx));
    str_vect.emplace_back(std::to_string(test_it.duration));
    if (!seq_override)
    {
        str_vect.emplace_back(TestModeEnumToString(test_it.mode));
    }
    else
    {
        str_vect.emplace_back(OVR);
    }
    if (is_read)
    {
        str_vect.emplace_back(data_integrity.result);
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }

    if (is_write || is_read) // Test can be stopped
    {
        str_vect.emplace_back(Float_to_String<double>(bw_wr_plus_rd.average_bw_mbps, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (is_write)
    {
        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(test_it.write.rate));
            str_vect.emplace_back(std::to_string(test_it.write.bandwidth));
            str_vect.emplace_back(std::to_string(test_it.write.start_addr));
            str_vect.emplace_back(std::to_string(test_it.write.end_addr));
            str_vect.emplace_back(std::to_string(test_it.write.burst_size));
            str_vect.emplace_back(std::to_string(test_it.write.block_size));
            str_vect.emplace_back(std::to_string(test_it.write.outstanding));
        }
        else
        {
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
        }
        str_vect.emplace_back(std::to_string(  this->m_AXI_num_data_bytes));

        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(test_it.write.burst_xfer));
            str_vect.emplace_back(std::to_string(test_it.write.num_burst));
            str_vect.emplace_back(std::to_string(test_it.write.total_xfer));
        }
        else
        {
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
        }
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.average_bw_mbps, 3));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.average_inst, 0));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_wr.average_ns, 3));

    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (is_read)
    {
        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(test_it.read.rate));
            str_vect.emplace_back(std::to_string(test_it.read.bandwidth));
            str_vect.emplace_back(std::to_string(test_it.read.start_addr));
            str_vect.emplace_back(std::to_string(test_it.read.end_addr));
            str_vect.emplace_back(std::to_string(test_it.read.burst_size));
            str_vect.emplace_back(std::to_string(test_it.read.block_size));
            str_vect.emplace_back(std::to_string(test_it.read.outstanding));
        }
        else
        {
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
        }
        str_vect.emplace_back(std::to_string(this->m_AXI_num_data_bytes));
        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(test_it.read.burst_xfer));
            str_vect.emplace_back(std::to_string(test_it.read.num_burst));
            str_vect.emplace_back(std::to_string(test_it.read.total_xfer));
        }
        else
        {
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
        }
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.average_bw_mbps, 3));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.average_inst, 0));
        str_vect.emplace_back(Float_to_String<double>(burst_latency_and_bw_rd.average_ns, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);

        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    measurement_file << StrVectToStr(str_vect, ",") << "\n";
    measurement_file.flush();
}

void MemoryTest::WriteToMeasurementFileMcSummary(
    std::ofstream &                     measurement_file,
    const uint &                        test_idx,
    const bool &                        seq_override,
    std::map<uint, TestItConfig_t> &    chan_test_it,
    std::map<uint, Chan_Status_t> &     chan_status,
    const bool &                        combined_is_write,
    const bool &                        combined_is_read,
    const TestItConfig_t &              combined_test_it,
    const Data_Integrity_t &            combined_data_integrity,
    const Stat_Burst_t &                combined_burst_latency_and_bw_wr,
    const Stat_Burst_t &                combined_burst_latency_and_bw_rd,
    const Stat_Burst_t &                combined_bw_wr_plus_rd
)
{
    if ( (!(this->m_use_outputfile)) || this->m_abort)
    {
        return;
    }
    std::vector<std::string> str_vect;
    str_vect.emplace_back(std::to_string(test_idx));
    str_vect.emplace_back(std::to_string(combined_test_it.duration));
    if (!seq_override)
    {
        str_vect.emplace_back(TestModeEnumToString(combined_test_it.mode));
    }
    else
    {
        str_vect.emplace_back(OVR);
    }

    if (combined_is_write)
    {
        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(combined_test_it.write.rate));
            str_vect.emplace_back(std::to_string(combined_test_it.write.bandwidth));
            str_vect.emplace_back(std::to_string(combined_test_it.write.start_addr));
            str_vect.emplace_back(std::to_string(combined_test_it.write.burst_size));
            str_vect.emplace_back(std::to_string(combined_test_it.write.block_size));
            str_vect.emplace_back(std::to_string(combined_test_it.write.outstanding));
        }
        else
        {
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
        }
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }

    if (combined_is_read)
    {
        if (!seq_override)
        {
            str_vect.emplace_back(std::to_string(combined_test_it.read.rate));
            str_vect.emplace_back(std::to_string(combined_test_it.read.bandwidth));
            str_vect.emplace_back(std::to_string(combined_test_it.read.start_addr));
            str_vect.emplace_back(std::to_string(combined_test_it.read.burst_size));
            str_vect.emplace_back(std::to_string(combined_test_it.read.block_size));
            str_vect.emplace_back(std::to_string(combined_test_it.read.outstanding));
        }
        else
        {
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
            str_vect.emplace_back(OVR);
        }
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    // Combined results
    if (combined_is_write)
    {
        str_vect.emplace_back(Float_to_String<double>(combined_burst_latency_and_bw_wr.average_bw_mbps, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (combined_is_read)
    {
        str_vect.emplace_back(Float_to_String<double>(combined_burst_latency_and_bw_rd.average_bw_mbps, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (combined_is_write)
    {
        str_vect.emplace_back(Float_to_String<double>(combined_burst_latency_and_bw_wr.average_ns, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (combined_is_read)
    {
        str_vect.emplace_back(Float_to_String<double>(combined_burst_latency_and_bw_rd.average_ns, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }
    if (combined_is_read)
    {
        str_vect.emplace_back(combined_data_integrity.result);
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }

    // Combined Write+Read
    if (combined_is_write || combined_is_read) // Test can be stopped
    {
        str_vect.emplace_back(Float_to_String<double>(combined_bw_wr_plus_rd.average_bw_mbps, 3));
    }
    else
    {
        str_vect.emplace_back(NOT_APPLICABLE);
    }

    // Channel Write BW
    str_vect.emplace_back("");
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (chan_test_it[ch_idx].is_write)
        {
            str_vect.emplace_back(Float_to_String<double>(chan_status[ch_idx].wr_burst_latency.average_bw_mbps, 3));
        }
        else
        {
            str_vect.emplace_back(NOT_APPLICABLE);
        }
    }
    // Channel Read BW
    str_vect.emplace_back("");
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (chan_test_it[ch_idx].is_read)
        {
            str_vect.emplace_back(Float_to_String<double>(chan_status[ch_idx].rd_burst_latency.average_bw_mbps, 3));
        }
        else
        {
            str_vect.emplace_back(NOT_APPLICABLE);
        }
    }

    // Channel Write Latency
    str_vect.emplace_back("");
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (chan_test_it[ch_idx].is_write)
        {
            str_vect.emplace_back(Float_to_String<double>(chan_status[ch_idx].wr_burst_latency.average_ns, 3));
        }
        else
        {
            str_vect.emplace_back(NOT_APPLICABLE);
        }
    }
    // Channel Read Latency
    str_vect.emplace_back("");
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (chan_test_it[ch_idx].is_read)
        {
            str_vect.emplace_back(Float_to_String<double>(chan_status[ch_idx].rd_burst_latency.average_ns, 3));
        }
        else
        {
            str_vect.emplace_back(NOT_APPLICABLE);
        }
    }

    measurement_file << StrVectToStr(str_vect, ",") << "\n";
    measurement_file.flush();
}

std::string MemoryTest::GetMessageTagChan( const int & ch_idx )
{
    if (this->m_memory_settings.type == MULTI_CHANNEL)
    {
        return "for channel: " + std::to_string(ch_idx) + " (" + GetCUConnectionVppSptag(ch_idx) + ")";
    }
    return "for tag: " + GetCUConnectionVppSptag(ch_idx);
}

std::string MemoryTest::GetMessageTagChan2( const int & ch_idx )
{
    if (this->m_memory_settings.type == MULTI_CHANNEL)
    {
        return "channel: " + std::to_string(ch_idx) + " (" + GetCUConnectionVppSptag(ch_idx) + ")";
    }
    return "tag: " + GetCUConnectionVppSptag(ch_idx);
}

std::string MemoryTest::GetMessageChan( const int & ch_idx )
{
    if (this->m_memory_settings.type == MULTI_CHANNEL)
    {
        return "for channel: " + std::to_string(ch_idx) + " (" + GetCUConnectionVppSptag(ch_idx) + ")";
    }
    return "";
}

std::string MemoryTest::GetPassMessageChan()
{
    if (this->m_memory_settings.type == MULTI_CHANNEL)
    {
        return "for each of " + std::to_string(this->m_enabled_ch_indexes.size()) + " channel(s)";
    }
    return "";
}

std::string MemoryTest::GetFailMessageChan()
{
    if (this->m_memory_settings.type == MULTI_CHANNEL)
    {
        return "for some of " + std::to_string(this->m_enabled_ch_indexes.size()) + " channel(s)";
    }
    return "";
}

void MemoryTest::DisplayStartExtraParam( const TestItConfig_t & test_it, const std::string & opt_chan_info )
{
    LogMessage(MSG_DEBUG_TESTCASE, {"\t- Extrapolated test sequence" + opt_chan_info + ": " + StrVectToTest(test_it.test_strvect_dbg)});

    std::vector<std::string> exta_param_vect;
    if (test_it.is_write)
    {
        exta_param_vect.emplace_back("\"" + WR_RATE_TEST_SEQ_MEMBER.name           + "\": " + std::to_string(test_it.write.rate));
        exta_param_vect.emplace_back("\"" + WR_BANDWIDTH_TEST_SEQ_MEMBER.name      + "\": " + std::to_string(test_it.write.bandwidth));
        exta_param_vect.emplace_back("\"" + WR_START_ADDR_TEST_SEQ_MEMBER.name     + "\": " + std::to_string(test_it.write.start_addr));
        exta_param_vect.emplace_back("\"" + WR_BURST_SIZE_TEST_SEQ_MEMBER.name     + "\": " + std::to_string(test_it.write.burst_size));
        exta_param_vect.emplace_back("\"" + WR_BLOCK_SIZE_TEST_SEQ_MEMBER.name     + "\": " + std::to_string(test_it.write.block_size));
        exta_param_vect.emplace_back("\"" + WR_OUTSTANDING_TEST_SEQ_MEMBER.name    + "\": " + std::to_string(test_it.write.outstanding));
    }
    if (test_it.is_read)
    {
        exta_param_vect.emplace_back("\"" + RD_RATE_TEST_SEQ_MEMBER.name           + "\": " + std::to_string(test_it.read.rate));
        exta_param_vect.emplace_back("\"" + RD_BANDWIDTH_TEST_SEQ_MEMBER.name      + "\": " + std::to_string(test_it.read.bandwidth));
        exta_param_vect.emplace_back("\"" + RD_START_ADDR_TEST_SEQ_MEMBER.name     + "\": " + std::to_string(test_it.read.start_addr));
        exta_param_vect.emplace_back("\"" + RD_BURST_SIZE_TEST_SEQ_MEMBER.name     + "\": " + std::to_string(test_it.read.burst_size));
        exta_param_vect.emplace_back("\"" + RD_BLOCK_SIZE_TEST_SEQ_MEMBER.name     + "\": " + std::to_string(test_it.read.block_size));
        exta_param_vect.emplace_back("\"" + RD_OUTSTANDING_TEST_SEQ_MEMBER.name    + "\": " + std::to_string(test_it.read.outstanding));
    }
    if (test_it.is_write || test_it.is_read)
    {
        LogMessage(MSG_CMN_042, {"\t- Extra test parameters" + opt_chan_info + ": " + StrVectToStr(exta_param_vect, ", ")});
    }

    if (test_it.is_write)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {WR_BURST_XFER     + opt_chan_info + ": " + std::to_string(test_it.write.burst_xfer)});
        LogMessage(MSG_DEBUG_TESTCASE, {WR_TOTAL_XFER     + opt_chan_info + ": " + std::to_string(test_it.write.total_xfer)});
        LogMessage(MSG_DEBUG_TESTCASE, {WR_NUM_BURST      + opt_chan_info + ": " + std::to_string(test_it.write.num_burst)});
    }
    if (test_it.is_read)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {RD_BURST_XFER     + opt_chan_info + ": " + std::to_string(test_it.read.burst_xfer)});
        LogMessage(MSG_DEBUG_TESTCASE, {RD_TOTAL_XFER     + opt_chan_info + ": " + std::to_string(test_it.read.total_xfer)});
        LogMessage(MSG_DEBUG_TESTCASE, {RD_NUM_BURST      + opt_chan_info + ": " + std::to_string(test_it.read.num_burst)});
    }
}

bool MemoryTest::PreWriteStep()
{
    auto ret = RET_SUCCESS;
    // Set pre-write test sequence
    std::vector<uint> ch_indexes;
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (!(this->m_TC_Cfg.chan_config[ch_idx].disable_prewrite))
        {
            if ((this->m_chan_test_it[ch_idx].mode == ONLY_RD_MODE_VAL) || (this->m_chan_test_it[ch_idx].mode == SIMULTANEOUS_WR_RD_MODE_VAL))
            {
                // The memory area written shall be the same as the memory area which will be read.
                // Use same parameters as read except rate
                LogMessage(MSG_MEM_036, {TestModeEnumToString(this->m_chan_test_it[ch_idx].mode), GetMessageChan(ch_idx)}); // Write memory before next test
                ch_indexes.emplace_back(ch_idx);
            }
        }
    }
    if (ch_indexes.empty())
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"No channel found for pre-write"});
        return ret;
    }
    // Set pre-write test sequence
    for (const auto & ch_idx : ch_indexes)
    {
        TestItConfig_t prewrite_test_it;
        prewrite_test_it.mode       = ONLY_WR_MODE_VAL;
        prewrite_test_it.write      = this->m_chan_test_it[ch_idx].read;
        prewrite_test_it.write.rate = this->m_cu_rate.only_wr.write.nominal;
        prewrite_test_it.is_write   = true;
        prewrite_test_it.is_read    = false;

        SetSequence(ch_idx, prewrite_test_it); // The xbtest HW IP starts as soon as the xbtest HW IP config is updated
    }
    // Update pre-write test configuration
    for (const auto & ch_idx : ch_indexes)
    {
        UpdateCuConfig(ch_idx);
    }
    // Wait pre-write test configuration updated
    ret |= WaitCuConfigUpdated(ch_indexes);

    if ((this->m_abort) || (ret == RET_FAILURE))
    {
        LogMessage(MSG_MEM_038);
    }
    else
    {
        LogMessage(MSG_MEM_037);
    }
    return ret;
}

int MemoryTest::RunTest()
{
    uint test_it_cnt      = 1;
    auto test_failure     = RET_SUCCESS;
    auto pre_test_failure = RET_SUCCESS;

    // Combined check_data_integrity indicates at least one channel checks data integrity
    this->m_combined_check_data_integrity = false;
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (this->m_TC_Cfg.chan_config[ch_idx].check_data_integrity & StrMatchNoCase(this->m_TC_Cfg.chan_config[ch_idx].pattern_ctrl, PATTERN_CTRL_PRBS))
        {
            this->m_combined_check_data_integrity = true;
        }
    }

    if (this->m_num_axi_thread > 1)
    {
        for (const auto & ch_idx : this->m_enabled_ch_indexes)
        {
            if (this->m_TC_Cfg.chan_config[ch_idx].single_axi_thread)
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Setting single ID mode " + GetMessageChan(ch_idx)});
                SetAxiSingleThread(ch_idx);
            }
            else
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Setting multi ID mode " + GetMessageChan(ch_idx)});
                SetAxiMultiThread(ch_idx);
            }
        }
    }

    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (StrMatchNoCase(this->m_TC_Cfg.chan_config[ch_idx].pattern_ctrl, PATTERN_CTRL_FIXED))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Enable " + PATTERN_CTRL_FIXED + " user pattern " + GetMessageChan(ch_idx) + ": 0x" + NumToStrHex(this->m_TC_Cfg.chan_config[ch_idx].pattern_list[0],8)});
            EnablePattern(ch_idx);
            SetPatternA(ch_idx, this->m_TC_Cfg.chan_config[ch_idx].pattern_list[0]);
            SetPatternB(ch_idx, this->m_TC_Cfg.chan_config[ch_idx].pattern_list[0]);
        }
        else if (StrMatchNoCase(this->m_TC_Cfg.chan_config[ch_idx].pattern_ctrl, PATTERN_CTRL_ALTERNATE))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Enable " + PATTERN_CTRL_ALTERNATE + " user pattern " + GetMessageChan(ch_idx) + ": 0x" + NumToStrHex(this->m_TC_Cfg.chan_config[ch_idx].pattern_list[0],8) + "/0x" + NumToStrHex(this->m_TC_Cfg.chan_config[ch_idx].pattern_list[1],8)});
            EnablePattern(ch_idx);
            SetPatternA(ch_idx, this->m_TC_Cfg.chan_config[ch_idx].pattern_list[0]);
            SetPatternB(ch_idx, this->m_TC_Cfg.chan_config[ch_idx].pattern_list[1]);
        }
        else
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Disable user pattern " + GetMessageChan(ch_idx)});
            DisablePattern(ch_idx);
        }
    }

    if (!(this->m_abort))
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Number of test iterations: " + std::to_string(this->m_global_test_it_list.size())});
        this->m_testcase_queue_value.pending = this->m_global_test_it_list.size();
        PushTestcaseQueue();
    }

    // Compare AXI address pointer read using both types of status register access
    // This allow to check both types of status register access are operational
    pre_test_failure |= CheckAxiAddrPtr();

    // Reset the Memory xbtest HW IP
    ActivateResetCores();
    ClearResetCores();

    test_failure |= pre_test_failure;

    for (auto & test_it : this->m_global_test_it_list)
    {
        if ((this->m_abort) || (pre_test_failure == RET_FAILURE))
        {
            break;
        }

        // Init
        auto test_it_setup_failure  = RET_SUCCESS;
        auto test_it_failure        = RET_SUCCESS;
        this->m_combined_data_integrity   = RESET_DATA_INTEGRITY;
        for (const auto & ch_idx : this->m_enabled_ch_indexes)
        {
            this->m_chan_status[ch_idx]     = RESET_CHAN_STATUS;
            this->m_data_integrity[ch_idx]  = RESET_DATA_INTEGRITY;
        }
        // Test parameters to dynamic console
        this->m_testcase_queue_value.remaining_time   = std::to_string(test_it.duration);
        this->m_testcase_queue_value.parameters       = StrVectToTest(test_it.test_strvect);
        PushTestcaseQueue();

        // Get channel test sequence
        this->m_combined_is_write = false;
        this->m_combined_is_read  = false;
        this->m_is_write_ch_idx.clear();
        this->m_is_read_ch_idx.clear();
        for (const auto & ch_idx : this->m_enabled_ch_indexes)
        {
            auto it_chan_list = std::next(this->m_chan_test_it_list[ch_idx].begin(), test_it_cnt-1);
            this->m_chan_test_it[ch_idx] = *it_chan_list;

            if (this->m_chan_test_it[ch_idx].is_write)
            {
                this->m_is_write_ch_idx.emplace_back(ch_idx);
                this->m_combined_is_write = true; // At least one channel is write, so combined is write too
            }
            if (this->m_chan_test_it[ch_idx].is_read)
            {
                this->m_is_read_ch_idx.emplace_back(ch_idx);
                this->m_combined_is_read  = true; // At least one channel is read, so combined is read too
            }
        }

        // Display start test message and test parameters
        LogMessage(MSG_CMN_032, {std::to_string(test_it_cnt), StrVectToTest(test_it.test_strvect)});
        DisplayStartExtraParam(test_it, "");

        // Display test parameters for each channel when the tet_sequence was overwritten
        for (const auto & ch_idx : this->m_enabled_ch_indexes)
        {
            if (this->m_chan_seq_override[ch_idx])
            {
                LogMessage(MSG_CMN_046, {GetMessageTagChan2(ch_idx), std::to_string(test_it_cnt), StrVectToTest(this->m_chan_test_it[ch_idx].test_strvect) }); // start test
                DisplayStartExtraParam(this->m_chan_test_it[ch_idx], " " + GetMessageTagChan2(ch_idx));
            }
        }

        // Compute the divider used to display remaining time one tenth of time
        uint duration_divider = test_it.duration/10;
        if (duration_divider == 0)
        {
            duration_divider = 1;
        }

        // The SW takes care of filling the memory with a PRBS before Only_Rd and Simultaneous_Wr_Rd tests
        if (test_it_setup_failure == RET_SUCCESS)
        {
            test_it_setup_failure |= PreWriteStep();
        }

        // Set xbtest HW IP with new configuration
        if ((!(this->m_abort)) && (test_it_setup_failure == RET_SUCCESS))
        {
            test_it_setup_failure |= SetSequenceAndUpdateCores();
        }

        if (test_it_setup_failure == RET_SUCCESS)
        {
            LogMessage(MSG_MEM_042);
        }
        else
        {
            LogMessage(MSG_MEM_041);
        }
        test_it_failure |= test_it_setup_failure;

        LogMessage(MSG_DEBUG_TESTCASE, {"Let run 2 second and start computing BW and latency"});
        WaitSecTick(2);

        // Use the greatest channel index (read at last) to detect the HW 1sec toggle
        this->m_tog_ch_idx = 0;
        for (const auto & ch_idx : this->m_enabled_ch_indexes)
        {
            if (this->m_tog_ch_idx < ch_idx)
            {
                this->m_tog_ch_idx = ch_idx;
            }
        }
        this->m_tog_1sec          = GetHw1secToggle(this->m_tog_ch_idx);
        this->m_tog_1sec_last     = this->m_tog_1sec;
        this->m_hw_sec_cnt        = 0;
        this->m_sw_sec_cnt        = this->m_timer->GetSecCnt();
        this->m_sw_sec_cnt_start  = this->m_sw_sec_cnt;

        ReadMemoryCuStatus();

        auto display_time = true; // true by default to display message at first iteration
        auto display_time_dynamic = true; // true by default to display message at first iteration
        auto remaining_time = test_it.duration - (this->m_sw_sec_cnt - this->m_sw_sec_cnt_start);

        do
        {
            if (test_it_setup_failure == RET_FAILURE)
            {
                break; // Do not execute do-while if set-up failed
            }

            // Display remaining time in log and classic console mode
            if (display_time && !(this->m_abort))
            {
                LogMessage(MSG_CMN_048, {std::to_string(remaining_time), ""});
                display_time = false;
            }

            // Display remaining time in dynamic console mode
            if (display_time_dynamic && !(this->m_abort))
            {
                this->m_testcase_queue_value.remaining_time = std::to_string(remaining_time);
                PushTestcaseQueue();
                display_time_dynamic = false;
            }

            // Detect (with timeout) a new HW sec toggle before getting new measurements
            WaitHw1SecToggle();

            // From last values read in xbtest HW IP status dump, compute burst stat (latency) and BW
            ComputeStatBurst();
            // PrintStatus();

            // this->m_bw_wr_plus_rd will have the sum of wr + rd BW for each channel. No burst latency info as not applicable for wr+rd
            GetWrPlusRdBw();

            // this->m_combined_bw_wr_plus_rd will have the sum of wr + rd BW for all channel. No burst latency info as not applicable for wr+rd
            GetCombinedWrPlusRdBw();

            // this->m_combined_burst_latency_wr/rd will have the average of burst latency and min/max of all channels
            this->m_combined_burst_latency_and_bw_wr = GetCombinedBurstStat("write burst latency");
            this->m_combined_burst_latency_and_bw_rd = GetCombinedBurstStat("read burst latency");

            // check for error every second while measuring bandwidth
            CheckDataIntegrity();
            GetCombinedDataIntegrity();

            // Write measurement live and results for each channel
            auto global_time  = std::to_string(this->m_timer->GetSecCnt());
            auto seq_override = false;
            for (const auto & ch_idx : this->m_enabled_ch_indexes)
            {
                WriteToMeasurementFileDetail(
                    global_time,
                    this->m_outputfile_detail[ch_idx],
                    this->m_RT_outputfile_detail[ch_idx],
                    this->m_RT_outputfile_detail_name[ch_idx],
                    this->m_RT_outputfile_detail_head[ch_idx],
                    test_it_cnt,
                    seq_override,
                    this->m_chan_test_it[ch_idx].is_write,
                    this->m_chan_test_it[ch_idx].is_read,
                    this->m_chan_test_it[ch_idx],
                    this->m_chan_status[ch_idx].timestamp_1_sec,
                    this->m_data_integrity[ch_idx],
                    this->m_chan_status[ch_idx].wr_burst_latency,
                    this->m_chan_status[ch_idx].rd_burst_latency,
                    this->m_bw_wr_plus_rd[ch_idx]
                );
            }
            // Write combined measurement live and results for multi-channel only
            if (this->m_memory_settings.type == MULTI_CHANNEL)
            {
                WriteToMeasurementFileDetail(
                    global_time,
                    this->m_outputfile_combined_detail,
                    this->m_RT_outputfile_combined_detail,
                    this->m_RT_outputfile_combined_detail_name,
                    this->m_RT_outputfile_combined_detail_head,
                    test_it_cnt,
                    this->m_seq_override,
                    this->m_combined_is_write,
                    this->m_combined_is_read,
                    test_it,
                    this->m_timestamp_1_sec_expected[0],
                    this->m_combined_data_integrity,
                    this->m_combined_burst_latency_and_bw_wr,
                    this->m_combined_burst_latency_and_bw_rd,
                    this->m_combined_bw_wr_plus_rd
                );
            }
            WriteToMeasurementFilePower(
                global_time,
                this->m_outputfile_power,
                test_it_cnt,
                this->m_seq_override,
                this->m_combined_is_write,
                this->m_combined_is_read,
                test_it
            );

            // Compute remaining time and check we need to display it at next iteration
            auto tmp_sw_sec_cnt = this->m_timer->GetSecCnt();
            if (tmp_sw_sec_cnt != this->m_sw_sec_cnt)
            {
                this->m_sw_sec_cnt = tmp_sw_sec_cnt;
                remaining_time = test_it.duration - (this->m_sw_sec_cnt - this->m_sw_sec_cnt_start);

                display_time_dynamic = true;
                if ((remaining_time % duration_divider == 0) && (remaining_time > 0)) // Display regularly, except when test duration is reached
                {
                    display_time = true;
                }
            }
        } while ((this->m_sw_sec_cnt - this->m_sw_sec_cnt_start < test_it.duration) && !(this->m_abort));

        LogMessage(MSG_CMN_049); // test duration reached

        if (!(this->m_abort))
        {
            // Check the this->m_hw_sec_cnt corresponds to the test duration
            uint64_t min_m_hw_sec_cnt = test_it.duration - test_it.duration/HW_SEC_CNT_TOLERANCE;
            if (min_m_hw_sec_cnt == test_it.duration)
            {
                min_m_hw_sec_cnt = test_it.duration - 1;
            }
            if (this->m_hw_sec_cnt < min_m_hw_sec_cnt)
            {
                LogMessage(MSG_MEM_047, {std::to_string(this->m_hw_sec_cnt), std::to_string(test_it.duration)});
            }
        }

        if (test_it_setup_failure == RET_SUCCESS)
        {
            // Write result in CSV output file for each channel
            auto seq_override = false;
            for (const auto & ch_idx : this->m_enabled_ch_indexes)
            {
                WriteToMeasurementFileResult(
                    this->m_outputfile_result[ch_idx],
                    test_it_cnt,
                    seq_override,
                    this->m_chan_test_it[ch_idx].is_write,
                    this->m_chan_test_it[ch_idx].is_read,
                    this->m_chan_test_it[ch_idx],
                    this->m_data_integrity[ch_idx],
                    this->m_chan_status[ch_idx].wr_burst_latency,
                    this->m_chan_status[ch_idx].rd_burst_latency,
                    this->m_bw_wr_plus_rd[ch_idx]
                );
            }
            // Write combined result in CSV output file for multi-channel only

            if (this->m_memory_settings.type == MULTI_CHANNEL)
            {
                // Data will not be written in the file if test sequence "mode" is overwritten for some channels as global mode is use here (test_it)
                WriteToMeasurementFileResult(
                    this->m_outputfile_combined_result,
                    test_it_cnt,
                    this->m_seq_override,
                    this->m_combined_is_write,
                    this->m_combined_is_read,
                    test_it,
                    this->m_combined_data_integrity,
                    this->m_combined_burst_latency_and_bw_wr,
                    this->m_combined_burst_latency_and_bw_rd,
                    this->m_combined_bw_wr_plus_rd
                );
                WriteToMeasurementFileMcSummary(
                    this->m_outputfile_mc_summary,
                    test_it_cnt,
                    this->m_seq_override,
                    this->m_chan_test_it,
                    this->m_chan_status,
                    this->m_combined_is_write,
                    this->m_combined_is_read,
                    test_it,
                    this->m_combined_data_integrity,
                    this->m_combined_burst_latency_and_bw_wr,
                    this->m_combined_burst_latency_and_bw_rd,
                    this->m_combined_bw_wr_plus_rd
                );
            }

            test_it_failure |= CheckTestItResults();

            if (!(this->m_abort))
            {
                std::string mem_type_str;
                if (this->m_memory_settings.type == MULTI_CHANNEL)
                {
                    mem_type_str += this->m_memory_settings.name;
                    mem_type_str += " (" + std::to_string(this->m_enabled_ch_indexes.size()) + " channel(s) sum)";
                }
                else
                {
                    mem_type_str += GetCUConnectionVppSptag(0);
                }
                LogMessage(MSG_DEBUG_TESTCASE, {"FPGA <-> " + mem_type_str + " average total (wr+rd) bandwidth: " + Float_to_String<double>(this->m_combined_bw_wr_plus_rd.average_bw_mbps, 3) + " MBps"});
            }
        }

        // Do not stop or reset xbtest HW IP between test iterations
        // test_it_failure |= StopCuCores();
        // ActivateResetCores();
        // ClearResetCores();

        if (this->m_abort)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            // Set all remaining test result to aborted
            for (uint i = test_it_cnt - 1; i < this->m_global_test_sequence.size(); i++)
            {
                this->m_test_it_results[i] = TITR_ABORTED;
                this->m_testcase_queue_value.completed++;
                this->m_testcase_queue_value.pending--;
                this->m_testcase_queue_value.failed++;
            }
        }
        else if (test_it_failure == RET_FAILURE)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            this->m_test_it_results[test_it_cnt-1] = TITR_FAILED;
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.failed++;
        }
        else
        {
            LogMessage(MSG_CMN_033_PASS, {std::to_string(test_it_cnt)});
            this->m_test_it_results[test_it_cnt-1] = TITR_PASSED;
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.passed++;
        }
        if (this->m_testcase_queue_value.pending == 0)
        {
            this->m_testcase_queue_value.remaining_time    = NOT_APPLICABLE;
            this->m_testcase_queue_value.parameters        = NOT_APPLICABLE;
        }
        PushTestcaseQueue();
        test_it_cnt++;

    }

    test_failure |= StopCuCores();

    // check for overall test failure
    if (this->m_abort)
    {
        return TEST_STATE_ABORT;
    }
    if (test_failure == RET_FAILURE)
    {
        return TEST_STATE_FAILURE;
    }
    return TEST_STATE_PASS;
}

bool MemoryTest::CheckAxiAddrPtr()
{
    LogMessage(MSG_DEBUG_TESTCASE, {"xbtest HW IP AXI base addresses are set to:"});
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        this->m_stat_axi_addr_ptr[ch_idx] = GetAxiAddrPtr(ch_idx);
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - 0x" + NumToStrHex<uint64_t>(this->m_stat_axi_addr_ptr[ch_idx]) + " " + GetMessageChan(ch_idx)});
    }
    ReadMemoryCuStatus();
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        if (this->m_stat_axi_addr_ptr[ch_idx] != this->m_chan_status[ch_idx].axi_addr_ptr)
        {
            LogMessage(MSG_MEM_049, {NumToStrHex<uint64_t>(this->m_stat_axi_addr_ptr[ch_idx]), NumToStrHex<uint64_t>(this->m_chan_status[ch_idx].axi_addr_ptr), GetMessageChan(ch_idx)});
            return RET_FAILURE;
        }
        if (this->m_stat_axi_addr_ptr[ch_idx] != GetAxiBaseAddress(ch_idx))
        {
            auto axi_base_addr = GetAxiBaseAddress(ch_idx);
            LogMessage(MSG_DEBUG_EXCEPTION, {"AXI base address reported from xbtest HW IP (0x" + NumToStrHex<uint64_t>(this->m_stat_axi_addr_ptr[ch_idx]) + ") does not match AXI base address computed by host application (0x" + NumToStrHex<uint64_t>(axi_base_addr) + ") " + GetMessageChan(ch_idx)});
        }
    }
    return RET_SUCCESS;
}

bool MemoryTest::CheckTestConfigWrRd ( const std::string & direction, Test_Cfg_Wr_Rd_t & test_it_wr_rd )
{
    std::string param_burst;
    std::string param_block;
    if (direction == "write")
    {
        param_burst = WR_BURST_SIZE;
        param_block = WR_BLOCK_SIZE;
    }
    else
    {
        param_burst = RD_BURST_SIZE;
        param_block = RD_BLOCK_SIZE;
    }

    // end_addr
    test_it_wr_rd.end_addr = test_it_wr_rd.start_addr + test_it_wr_rd.block_size - 1;

    // burst_xfer
    if (test_it_wr_rd.burst_size % this->m_AXI_num_data_bytes != 0)
    {
        LogMessage(MSG_MEM_039, {param_burst, std::to_string(test_it_wr_rd.burst_size), "Bytes", std::to_string(this->m_AXI_num_data_bytes)} );
        return RET_FAILURE;
    }
    test_it_wr_rd.burst_xfer = test_it_wr_rd.burst_size / this->m_AXI_num_data_bytes;

    // total_xfer
    uint64_t block_size_bytes = (uint64_t)(test_it_wr_rd.block_size) * (uint64_t)(1024) * (uint64_t)(1024);
    if (block_size_bytes % (uint64_t)(this->m_AXI_num_data_bytes) != 0)
    {
        LogMessage(MSG_MEM_039, {param_block, std::to_string(test_it_wr_rd.block_size), "MB", std::to_string(this->m_AXI_num_data_bytes)} );
        return RET_FAILURE;
    }
    test_it_wr_rd.total_xfer = (uint) ( block_size_bytes / (uint64_t)(this->m_AXI_num_data_bytes) );

    // num_burst
    if ((test_it_wr_rd.burst_size == 0) || ( block_size_bytes % (uint64_t)(test_it_wr_rd.burst_size) != 0 ) )
    {
        LogMessage(MSG_MEM_004, {param_block, std::to_string(test_it_wr_rd.block_size), param_burst, std::to_string(test_it_wr_rd.burst_size)} );
        return RET_FAILURE;
    }
    test_it_wr_rd.num_burst  = (uint) ( test_it_wr_rd.block_size * (uint64_t)(1024) * (uint64_t)(1024) / (uint64_t)(test_it_wr_rd.burst_size) ); // Already check multple

    return RET_SUCCESS;
}

bool MemoryTest::CheckWrRdMemoryAreas( const TestItConfig_t & test_it )
{
    auto wr_start_addr_bytes = (uint64_t)(test_it.write.start_addr) * (uint64_t)(1024) * (uint64_t)(1024);
    auto rd_start_addr_bytes = (uint64_t)(test_it.read.start_addr)  * (uint64_t)(1024) * (uint64_t)(1024);
    auto wr_end_addr_bytes   = (uint64_t)(test_it.write.end_addr)   * (uint64_t)(1024) * (uint64_t)(1024);
    auto rd_end_addr_bytes   = (uint64_t)(test_it.read.end_addr)    * (uint64_t)(1024) * (uint64_t)(1024);

    if (test_it.mode == ALTERNATE_WR_RD_MODE_VAL)
    {
        // Check the two memory areas are the same
        if (
               (wr_start_addr_bytes != rd_start_addr_bytes) || (wr_end_addr_bytes != rd_end_addr_bytes)
        )
        {
            LogMessage(MSG_MEM_035, {"Write and read memory areas must be equal for mode " + TestModeEnumToString(test_it.mode),
                                     std::to_string(test_it.write.start_addr),
                                     std::to_string(wr_end_addr_bytes),
                                     std::to_string(rd_start_addr_bytes),
                                     std::to_string(rd_end_addr_bytes),
                                     });
            return RET_FAILURE;
        }
    }
    else if (test_it.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
    {
        // Check the two memory areas do not overlap

        // Case 1: rd in after wr  = [wr_start_addr_bytes <= rd_start_addr_bytes <= wr_end_addr_bytes <= rd_end_addr_bytes]
        // Case 2: wr in after rd  = [rd_start_addr_bytes <= wr_start_addr_bytes <= rd_end_addr_bytes <= wr_end_addr_bytes]
        // Case 3: rd in wr        = [wr_start_addr_bytes <= rd_start_addr_bytes <= rd_end_addr_bytes <= wr_end_addr_bytes]
        // Case 4: wr in rd        = [rd_start_addr_bytes <= wr_start_addr_bytes <= wr_end_addr_bytes <= rd_end_addr_bytes]

        if (
               ((wr_start_addr_bytes <= rd_start_addr_bytes) && (rd_start_addr_bytes <= wr_end_addr_bytes) && (wr_end_addr_bytes <= rd_end_addr_bytes))  // Case 1
            || ((rd_start_addr_bytes <= wr_start_addr_bytes) && (wr_start_addr_bytes <= rd_end_addr_bytes) && (rd_end_addr_bytes <= wr_end_addr_bytes)) // Case 2
            || ((wr_start_addr_bytes <= rd_start_addr_bytes) && (rd_start_addr_bytes <= rd_end_addr_bytes) && (rd_end_addr_bytes <= wr_end_addr_bytes)) // Case 3
            || ((rd_start_addr_bytes <= wr_start_addr_bytes) && (wr_start_addr_bytes <= wr_end_addr_bytes) && (wr_end_addr_bytes <= rd_end_addr_bytes))  // Case 4
        )
        {
            LogMessage(MSG_MEM_035, {"Write and read memory areas cannot overlap for mode " + TestModeEnumToString(test_it.mode),
                                     std::to_string(wr_start_addr_bytes),
                                     std::to_string(wr_end_addr_bytes),
                                     std::to_string(rd_start_addr_bytes),
                                     std::to_string(rd_end_addr_bytes),
                                     });
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool MemoryTest::CheckTestItConfig( TestItConfig_t & test_it )
{
    if (test_it.is_write)
    {
        if (CheckTestConfigWrRd("write", test_it.write) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    if (test_it.is_read)
    {
        if (CheckTestConfigWrRd("read", test_it.read) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }

    if (CheckWrRdMemoryAreas(test_it) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        Mem_Thresh_WrRd_t lat_thresh;
        Mem_Thresh_WrRd_t bw_thresh;
        if (test_it.mode == ALTERNATE_WR_RD_MODE_VAL)
        {
            bw_thresh.read.low    = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.read.low;
            bw_thresh.read.high   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.read.high;
            bw_thresh.write.low   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.write.low;
            bw_thresh.write.high  = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.alt_wr_rd.write.high;

            lat_thresh.read.low    = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.read.low;
            lat_thresh.read.high   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.read.high;
            lat_thresh.write.low   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.write.low;
            lat_thresh.write.high  = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.alt_wr_rd.write.high;
        }
        else if (test_it.mode == ONLY_RD_MODE_VAL)
        {
            bw_thresh.read.low    = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_rd.read.low;
            bw_thresh.read.high   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_rd.read.high;

            lat_thresh.read.low    = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_rd.read.low;
            lat_thresh.read.high   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_rd.read.high;
        }
        else if (test_it.mode == ONLY_WR_MODE_VAL)
        {
            bw_thresh.write.low   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_wr.write.low;
            bw_thresh.write.high  = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.only_wr.write.high;

            lat_thresh.write.low   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_wr.write.low;
            lat_thresh.write.high  = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.only_wr.write.high;
        }
        else if (test_it.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
        {
            bw_thresh.read.low    = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.read.low;
            bw_thresh.read.high   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.read.high;
            bw_thresh.write.low   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.write.low;
            bw_thresh.write.high  = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_bw.simul_wr_rd.write.high;

            lat_thresh.read.low    = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.read.low;
            lat_thresh.read.high   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.read.high;
            lat_thresh.write.low   = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.write.low;
            lat_thresh.write.high  = (double)this->m_TC_Cfg.chan_config[ch_idx].cu_latency.simul_wr_rd.write.high;
        }
        test_it.bw_thresh[ch_idx]  = bw_thresh;
        test_it.lat_thresh[ch_idx] = lat_thresh;
    }
    return RET_SUCCESS;
}

bool MemoryTest::GetTestSequence()
{
    this->m_test_it_results.clear();
    auto it = FindJsonParam(this->m_test_parameters.global_config, TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_global_test_sequence = TestcaseParamCast<std::vector<Memory_Test_Sequence_Parameters_t>>(it->second);
        for (uint ii=0; ii<this->m_global_test_sequence.size(); ii++)
        {
            this->m_test_it_results.emplace_back(TITR_NOT_TESTED);
        }
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

bool MemoryTest::ParseTestSequenceSettings( const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence, std::list<TestItConfig_t> & test_list )
{
    auto parse_failure = RET_SUCCESS;
    uint parse_error_cnt = 0;
    int  test_cnt = 0;

    LogMessage(MSG_CMN_022);
    std::vector<std::string> test_seq_strvect;
    std::vector<std::string> test_sequence_dbg;

    for (const auto & test_seq_param : test_sequence)
    {
        if (this->m_abort)
        {
            break;
        }

        auto parse_it_failure = RET_SUCCESS;
        TestItConfig_t test_it_cfg;

        test_cnt++;

        test_it_cfg.test_strvect = {}; // Displays what user entered
        test_it_cfg.test_strvect_dbg = {}; // Displays with extrapolated parameters

        test_it_cfg.duration = test_seq_param.duration.value;
        test_it_cfg.test_strvect.emplace_back(    "\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));

        test_it_cfg.mode = TestModeStringToEnum(test_seq_param.mode.value);
        test_it_cfg.is_write = false;
        test_it_cfg.is_read  = false;
        if (IsTestModeWrite(test_it_cfg.mode))
        {
            test_it_cfg.is_write = true;
        }
        if (IsTestModeRead(test_it_cfg.mode))
        {
            test_it_cfg.is_read = true;
        }

        test_it_cfg.test_strvect.emplace_back(    "\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");

        // Extrapolation

        // Check rate and bandwidth request are not both set
        if (test_it_cfg.is_write)
        {
            if (test_seq_param.write.rate.exists & test_seq_param.write.bandwidth.exists)
            {
                LogMessage(MSG_MEM_054, {"write"});
                parse_it_failure = RET_FAILURE;
            }
        }
        if (test_it_cfg.is_read)
        {
            if (test_seq_param.read.rate.exists & test_seq_param.read.bandwidth.exists)
            {
                LogMessage(MSG_MEM_054, {"read"});
                parse_it_failure = RET_FAILURE;
            }
        }
        // Write rate
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_write)
            {
                if (test_seq_param.write.rate.exists)
                {
                    test_it_cfg.write.rate = test_seq_param.write.rate.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + WR_RATE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.rate));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.write.rate = this->m_cu_rate.simul_wr_rd.write.nominal;
                    }
                    else if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.write.rate = this->m_cu_rate.alt_wr_rd.write.nominal;
                    }
                    else
                    {
                        test_it_cfg.write.rate = this->m_cu_rate.only_wr.write.nominal;
                    }
                }
            }
        }
        // Read rate
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_read)
            {
                if (test_seq_param.read.rate.exists)
                {
                    test_it_cfg.read.rate = test_seq_param.read.rate.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + RD_RATE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.rate));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.rate  = this->m_cu_rate.simul_wr_rd.read.nominal;
                    }
                    else if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.rate  = this->m_cu_rate.alt_wr_rd.read.nominal;
                    }
                    else
                    {
                        test_it_cfg.read.rate = this->m_cu_rate.only_rd.read.nominal;
                    }
                }
            }
        }
        // Write bandwidth
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_write)
            {
                if (test_seq_param.write.bandwidth.exists)
                {
                    test_it_cfg.write.bandwidth = test_seq_param.write.bandwidth.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + WR_BANDWIDTH_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.bandwidth));

                    // Compute xbtest HW IP rate as bandwidth is provided
                    auto rate = (double)(test_it_cfg.write.bandwidth) / (double)(this->m_max.bandwidth) * (double)(100);
                    if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        rate *= (double)(2);
                    }
                    if (rate > 100.0)
                    {
                        rate = (double)(100);
                    }
                    test_it_cfg.write.rate = rate;
                }
                else // Default bandwidth depends on xbtest HW IP rate
                {
                    auto bandwidth = (double)(test_it_cfg.write.rate) * (double)(this->m_max.bandwidth) / (double)(100);
                    if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        bandwidth /= (double)(2);
                    }
                    test_it_cfg.write.bandwidth = (uint)(bandwidth);
                }
            }
        }
        // Read bandwidth
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_read)
            {
                if (test_seq_param.read.bandwidth.exists)
                {
                    test_it_cfg.read.bandwidth = test_seq_param.read.bandwidth.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + RD_BANDWIDTH_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.bandwidth));

                    // Compute xbtest HW IP rate as bandwidth is provided
                    auto rate = (double)(test_it_cfg.read.bandwidth) / (double)(this->m_max.bandwidth) * (double)(100);
                    if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        rate *= (double)(2);
                    }
                    if (rate > 100.0)
                    {
                        rate = (double)(100);
                    }
                    test_it_cfg.read.rate = rate;
                }
                else // Default bandwidth depends depend on xbtest HW IP rate
                {
                    auto bandwidth = (double)(test_it_cfg.read.rate) * (double)(this->m_max.bandwidth) / (double)(100);
                    if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        bandwidth /= (double)(2);
                    }
                    test_it_cfg.read.bandwidth = (uint)(bandwidth);
                }
            }
        }
        // Add xbtest HW IP rate and bandwith to debug test sequence
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_write)
            {
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + WR_RATE_TEST_SEQ_MEMBER.name      + "\": " + std::to_string(test_it_cfg.write.rate));
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + WR_BANDWIDTH_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.bandwidth));
            }
        }
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_read)
            {
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + RD_RATE_TEST_SEQ_MEMBER.name      + "\": " + std::to_string(test_it_cfg.read.rate));
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + RD_BANDWIDTH_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.bandwidth));
            }
        }
        // Write burst size
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_write)
            {
                if (test_seq_param.write.burst_size.exists)
                {
                    test_it_cfg.write.burst_size = test_seq_param.write.burst_size.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + WR_BURST_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.burst_size));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.write.burst_size = this->m_cu_burst_size.simul_wr_rd.write.nominal;
                    }
                    else if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.write.burst_size = this->m_cu_burst_size.alt_wr_rd.write.nominal;
                    }
                    else
                    {
                        test_it_cfg.write.burst_size = this->m_cu_burst_size.only_wr.write.nominal;
                    }
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + WR_BURST_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.burst_size));
            }
        }
        // Read burst size
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_read)
            {
                if (test_seq_param.read.burst_size.exists)
                {
                    test_it_cfg.read.burst_size = test_seq_param.read.burst_size.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + RD_BURST_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.burst_size));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.burst_size  = this->m_cu_burst_size.simul_wr_rd.read.nominal;
                    }
                    else if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.burst_size  = this->m_cu_burst_size.alt_wr_rd.read.nominal;
                    }
                    else
                    {
                        test_it_cfg.read.burst_size = this->m_cu_burst_size.only_rd.read.nominal;
                    }
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + RD_BURST_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.burst_size));
            }
        }
        // Write start address
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_write)
            {
                if (test_seq_param.write.start_addr.exists)
                {
                    test_it_cfg.write.start_addr = test_seq_param.write.start_addr.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + WR_START_ADDR_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.start_addr));
                }
                else
                {
                    test_it_cfg.write.start_addr = this->m_min.start_addr;
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + WR_START_ADDR_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.start_addr));
            }
        }
        // Read start address
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_read)
            {
                if (test_seq_param.read.start_addr.exists)
                {
                    test_it_cfg.read.start_addr = test_seq_param.read.start_addr.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + RD_START_ADDR_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.start_addr));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.start_addr  = this->m_max.block_size/2;
                    }
                    else
                    {
                        test_it_cfg.read.start_addr = this->m_min.start_addr;
                    }
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + RD_START_ADDR_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.start_addr));
            }
        }
        // Write block size
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_write)
            {
                if (test_seq_param.write.block_size.exists)
                {
                    test_it_cfg.write.block_size = test_seq_param.write.block_size.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + WR_BLOCK_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.block_size));
                }
                else
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.write.block_size = this->m_max.block_size/2;
                    }
                    else
                    {
                        test_it_cfg.write.block_size = this->m_max.block_size;
                    }
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + WR_BLOCK_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.block_size));
            }
        }
        // Read block size
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_read)
            {
                if (test_seq_param.read.block_size.exists)
                {
                    test_it_cfg.read.block_size = test_seq_param.read.block_size.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + RD_BLOCK_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.block_size));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.block_size = this->m_max.block_size/2;
                    }
                    else
                    {
                        test_it_cfg.read.block_size = this->m_max.block_size;
                    }
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + RD_BLOCK_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.block_size));
            }
        }

        // Write outstanding
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_write)
            {
                if (test_seq_param.write.outstanding.exists)
                {
                    test_it_cfg.write.outstanding = test_seq_param.write.outstanding.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + WR_OUTSTANDING_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.outstanding));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.write.outstanding = this->m_cu_outstanding.simul_wr_rd.write.nominal;
                    }
                    else if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.write.outstanding = this->m_cu_outstanding.alt_wr_rd.write.nominal;
                    }
                    else
                    {
                        test_it_cfg.write.outstanding = this->m_cu_outstanding.only_wr.write.nominal;
                    }
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + WR_OUTSTANDING_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.write.outstanding));
            }
        }
        // Read outstanding
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.is_read)
            {
                if (test_seq_param.read.outstanding.exists)
                {
                    test_it_cfg.read.outstanding = test_seq_param.read.outstanding.value;
                    test_it_cfg.test_strvect.emplace_back("\"" + RD_OUTSTANDING_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.outstanding));
                }
                else // Default values depend on test mode
                {
                    if (test_it_cfg.mode == SIMULTANEOUS_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.outstanding  = this->m_cu_outstanding.simul_wr_rd.read.nominal;
                    }
                    else if (test_it_cfg.mode == ALTERNATE_WR_RD_MODE_VAL)
                    {
                        test_it_cfg.read.outstanding  = this->m_cu_outstanding.alt_wr_rd.read.nominal;
                    }
                    else
                    {
                        test_it_cfg.read.outstanding = this->m_cu_outstanding.only_rd.read.nominal;
                    }
                }
                test_it_cfg.test_strvect_dbg.emplace_back("\"" + RD_OUTSTANDING_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.read.outstanding));
            }
        }

        // Check parameters
        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= CheckParam<uint>(DURATION_TEST_SEQ_MEMBER.name, test_it_cfg.duration, MIN_DURATION, MAX_DURATION);
        }

        // Check parameters also when default values are used
        // Number of transfer boundaries depends on the current start address and burst size
        // Find minimum block size: first multiple of current burst size >= this->m_min.block_size
        //  - burst size cannot be 0 so while loop ok
        // Find max start addr: max block size - current block size
        if (test_it_cfg.is_write)
        {
            if (parse_it_failure == RET_SUCCESS)
            {
                auto min_rate = this->m_min.rate;
                if (test_seq_param.write.bandwidth.exists)
                {
                    min_rate = 0.0;
                }
                parse_it_failure |= CheckParam<double>(WR_RATE_TEST_SEQ_MEMBER.name, test_it_cfg.write.rate, min_rate, this->m_max.rate);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                parse_it_failure |= CheckParam<uint>(WR_BANDWIDTH_TEST_SEQ_MEMBER.name, test_it_cfg.write.bandwidth, this->m_min.bandwidth, this->m_max.bandwidth);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                parse_it_failure |= CheckParam<uint>(WR_BURST_SIZE_TEST_SEQ_MEMBER.name, test_it_cfg.write.burst_size, this->m_min.burst_size, this->m_max.burst_size);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                uint64_t min_wr_block_size = (uint64_t)(2) * (uint64_t)(test_it_cfg.write.burst_size);
                while (
                        (min_wr_block_size / (uint64_t)(1024) / (uint64_t)(1024) < (uint64_t)(this->m_min.block_size))
                     && (min_wr_block_size % ( (uint64_t)(1024) * (uint64_t)(1024) ) != 0)
                    )
                {
                    min_wr_block_size += (uint64_t)(test_it_cfg.write.burst_size);
                }
                min_wr_block_size = min_wr_block_size / (uint64_t)(1024) / (uint64_t)(1024);
                parse_it_failure |= CheckParam<uint>(WR_BLOCK_SIZE_TEST_SEQ_MEMBER.name, test_it_cfg.write.block_size, (uint)(min_wr_block_size), this->m_max.block_size);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                uint max_wr_start_addr = this->m_max.block_size - test_it_cfg.write.block_size;
                parse_it_failure |= CheckParam<uint>(WR_START_ADDR_TEST_SEQ_MEMBER.name, test_it_cfg.write.start_addr, this->m_min.start_addr, max_wr_start_addr);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                parse_it_failure |= CheckParam<uint>(WR_OUTSTANDING_TEST_SEQ_MEMBER.name, test_it_cfg.write.outstanding, MIN_OUTSTANDING, MAX_OUTSTANDING);
            }
        }
        if (test_it_cfg.is_read)
        {
            if (parse_it_failure == RET_SUCCESS)
            {
                auto min_rate = this->m_min.rate;
                if (test_seq_param.read.bandwidth.exists)
                {
                    min_rate = 0.0;
                }
                parse_it_failure |= CheckParam<double>(RD_RATE_TEST_SEQ_MEMBER.name, test_it_cfg.read.rate, min_rate, this->m_max.rate);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                parse_it_failure |= CheckParam<uint>(RD_BANDWIDTH_TEST_SEQ_MEMBER.name, test_it_cfg.read.bandwidth, this->m_min.bandwidth, this->m_max.bandwidth);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                parse_it_failure |= CheckParam<uint>(RD_BURST_SIZE_TEST_SEQ_MEMBER.name, test_it_cfg.read.burst_size, this->m_min.burst_size, this->m_max.burst_size);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                uint64_t min_rd_block_size = (uint64_t)(2) * (uint64_t)(test_it_cfg.read.burst_size);

                while (
                        (min_rd_block_size / (uint64_t)(1024) / (uint64_t)(1024) < (uint64_t)(this->m_min.block_size))
                     && (min_rd_block_size % ( (uint64_t)(1024) * (uint64_t)(1024) ) != 0)
                    )
                {
                    min_rd_block_size += (uint64_t)(test_it_cfg.read.burst_size);
                }
                min_rd_block_size = min_rd_block_size / (uint64_t)(1024) / (uint64_t)(1024);
                parse_it_failure |= CheckParam<uint>(RD_BLOCK_SIZE_TEST_SEQ_MEMBER.name, test_it_cfg.read.block_size, (uint)(min_rd_block_size), this->m_max.block_size);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                uint max_rd_start_addr = this->m_max.block_size - test_it_cfg.read.block_size;
                parse_it_failure |= CheckParam<uint>(RD_START_ADDR_TEST_SEQ_MEMBER.name, test_it_cfg.read.start_addr, this->m_min.start_addr, max_rd_start_addr);
            }
            if (parse_it_failure == RET_SUCCESS)
            {
                parse_it_failure |= CheckParam<uint>(RD_OUTSTANDING_TEST_SEQ_MEMBER.name, test_it_cfg.read.outstanding, MIN_OUTSTANDING, MAX_OUTSTANDING);
            }
        }
        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= CheckTestItConfig(test_it_cfg);
        }

        parse_failure |= parse_it_failure;

        if (parse_it_failure == RET_FAILURE)
        {
            LogMessage(MSG_CMN_025, {std::to_string(test_cnt)});
            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + " contains erroneous parameters");
            parse_error_cnt ++;

            if (parse_error_cnt > MAX_NUM_PARSER_ERROR)
            {
                break;
            }
        }
        else
        {
            // the test is valid; add it to the test_list
            test_list.emplace_back(test_it_cfg);
            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + ". " + std::to_string(test_seq_param.num_param) + " parameters provided");
        }
        test_seq_strvect.emplace_back(StrVectToTest(test_it_cfg.test_strvect));
    }

    // display the entire test sequence
    LogMessage(MSG_DEBUG_TESTCASE, {"Extrapolated test sequence:"});
    for (const auto & test : test_sequence_dbg)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {test});
    }

    if (parse_failure == RET_SUCCESS)
    {
        LogMessage(MSG_CMN_047, {std::to_string(test_seq_strvect.size()), "[ " + StrVectToStr(test_seq_strvect, ", ") + " ]"});
        LogMessage(MSG_CMN_023);
    }
    else
    {
        LogMessage(MSG_CMN_024);
    }

    return parse_failure;
}

bool MemoryTest::CheckTestItResults()
{
    auto ret = RET_SUCCESS;
    auto check_bw_disabled = false;
    auto check_lat_disabled = false;

    uint col_size_1 = 8;
    uint col_size_2 = 10;
    uint col_size_3 = 15;
    uint col_size_4 = 14;
    uint col_size_5 = 18;
    uint col_size_6 = 17;
    uint col_size_7 = 11;


    std::vector<std::string> results_header;
    results_header.emplace_back(pad(           "Channel", ' ', col_size_1, PAD_ON_RIGHT));
    results_header.emplace_back(pad(               "Tag", ' ', col_size_2, PAD_ON_RIGHT));
    results_header.emplace_back(pad(   "Write BW (MBps)", ' ', col_size_3, PAD_ON_RIGHT));
    results_header.emplace_back(pad(    "Read BW (MBps)", ' ', col_size_4, PAD_ON_RIGHT));
    results_header.emplace_back(pad("Write latency (ns)", ' ', col_size_5, PAD_ON_RIGHT));
    results_header.emplace_back(pad( "Read latency (ns)", ' ', col_size_6, PAD_ON_RIGHT));
    results_header.emplace_back(pad(       "Data integ.", ' ', col_size_7, PAD_ON_RIGHT));

    std::vector<std::vector<std::string>> all_chan_results;
    for (const auto & ch_idx : this->m_enabled_ch_indexes)
    {
        std::vector<std::string> chan_results;
        chan_results.emplace_back(pad( std::to_string(ch_idx), ' ', col_size_1, PAD_ON_RIGHT));
        chan_results.emplace_back(pad(GetCUConnectionVppSptag(ch_idx), ' ', col_size_2, PAD_ON_RIGHT));

        // Nominal settings
        Test_Cfg_Wr_Rd_t nominal_write;
        nominal_write.block_size   = this->m_max.block_size;
        nominal_write.rate         = this->m_cu_rate.only_wr.write.nominal;
        nominal_write.burst_size   = this->m_cu_burst_size.only_wr.write.nominal;
        nominal_write.outstanding  = this->m_cu_outstanding.only_wr.write.nominal;

        Test_Cfg_Wr_Rd_t nominal_read;
        nominal_read.block_size    = this->m_max.block_size;
        nominal_read.rate          = this->m_cu_rate.only_rd.read.nominal;
        nominal_read.burst_size    = this->m_cu_burst_size.only_rd.read.nominal;
        nominal_read.outstanding   = this->m_cu_outstanding.only_rd.read.nominal;

        if (this->m_chan_test_it[ch_idx].mode == SIMULTANEOUS_WR_RD_MODE_VAL)
        {
            nominal_write.block_size    = this->m_max.block_size/2;
            nominal_write.rate          = this->m_cu_rate.simul_wr_rd.write.nominal;
            nominal_write.burst_size    = this->m_cu_burst_size.simul_wr_rd.write.nominal;
            nominal_write.outstanding   = this->m_cu_outstanding.simul_wr_rd.write.nominal;

            nominal_read.block_size     = this->m_max.block_size/2;
            nominal_read.rate           = this->m_cu_rate.simul_wr_rd.read.nominal;
            nominal_read.burst_size     = this->m_cu_burst_size.simul_wr_rd.read.nominal;
            nominal_read.outstanding    = this->m_cu_outstanding.simul_wr_rd.read.nominal;
        }
        else if (this->m_chan_test_it[ch_idx].mode == ALTERNATE_WR_RD_MODE_VAL)
        {
            nominal_write.rate         = this->m_cu_rate.alt_wr_rd.write.nominal;
            nominal_write.burst_size   = this->m_cu_burst_size.alt_wr_rd.write.nominal;
            nominal_write.outstanding  = this->m_cu_outstanding.alt_wr_rd.write.nominal;

            nominal_read.rate         = this->m_cu_rate.alt_wr_rd.read.nominal;
            nominal_read.burst_size   = this->m_cu_burst_size.alt_wr_rd.read.nominal;
            nominal_read.outstanding  = this->m_cu_outstanding.alt_wr_rd.read.nominal;
        }

        // See if check BW/latency enabled
        auto check_bw_exists    = this->m_TC_Cfg.chan_config[ch_idx].check_bw_exists;
        auto check_lat_exists   = this->m_TC_Cfg.chan_config[ch_idx].check_latency_exists;
        auto default_check_bw   = this->m_TC_Cfg.chan_config[ch_idx].check_bw;
        auto default_check_lat  = this->m_TC_Cfg.chan_config[ch_idx].check_latency;
        auto check_bw           = default_check_bw;
        auto check_lat          = default_check_lat;

        // check block_size
        if (this->m_chan_test_it[ch_idx].is_write)
        {
            if (this->m_chan_test_it[ch_idx].write.block_size != nominal_write.block_size)
            {
                LogMessage(MSG_CMN_051, {WR_BLOCK_SIZE_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].write.block_size) + ") of test does not equal " + std::to_string(nominal_write.block_size) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            if (this->m_chan_test_it[ch_idx].read.block_size != nominal_read.block_size)
            {
                LogMessage(MSG_CMN_051, {RD_BLOCK_SIZE_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].read.block_size) + ") of test does not equal " + std::to_string(nominal_read.block_size) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }

        // check rate
        if (this->m_chan_test_it[ch_idx].is_write)
        {
            if (this->m_chan_test_it[ch_idx].write.rate != nominal_write.rate)
            {
                LogMessage(MSG_CMN_051, {WR_RATE_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].write.rate) + ") of test does not equal " + std::to_string(nominal_write.rate) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            if (this->m_chan_test_it[ch_idx].read.rate != nominal_read.rate)
            {
                LogMessage(MSG_CMN_051, {RD_RATE_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].read.rate) + ") of test does not equal " + std::to_string(nominal_read.rate) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }

        // check burst_size
        if (this->m_chan_test_it[ch_idx].is_write)
        {
            if (this->m_chan_test_it[ch_idx].write.burst_size != nominal_write.burst_size)
            {
                LogMessage(MSG_CMN_051, {WR_BURST_SIZE_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].write.burst_size) + ") of test does not equal " + std::to_string(nominal_write.burst_size) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            if (this->m_chan_test_it[ch_idx].read.burst_size != nominal_read.burst_size)
            {
                LogMessage(MSG_CMN_051, {RD_BURST_SIZE_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].read.burst_size) + ") of test does not equal " + std::to_string(nominal_read.burst_size) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }

        // check outstanding
        if (this->m_chan_test_it[ch_idx].is_write)
        {
            if (this->m_chan_test_it[ch_idx].write.outstanding != nominal_write.outstanding)
            {
                LogMessage(MSG_CMN_051, {WR_OUTSTANDING_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].write.outstanding) + ") of test does not equal " + std::to_string(nominal_write.outstanding) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            if (this->m_chan_test_it[ch_idx].read.outstanding != nominal_read.outstanding)
            {
                LogMessage(MSG_CMN_051, {RD_OUTSTANDING_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].read.outstanding) + ") of test does not equal " + std::to_string(nominal_read.outstanding) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
                IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
                IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
            }
        }
        // check duration
        if (this->m_chan_test_it[ch_idx].duration < MIN_RESULTS_CHECK_DURATION)
        {
            LogMessage(MSG_CMN_051, {DURATION_TEST_SEQ_MEMBER.name + " (" + std::to_string(this->m_chan_test_it[ch_idx].duration) + ") of test is lower than " + std::to_string(MIN_RESULTS_CHECK_DURATION) + " " + GetMessageTagChan(ch_idx), "bandwidth and latency"});
            IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
            IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
        }
        // check test aborted
        if (this->m_abort)
        {
            LogMessage(MSG_CMN_051, {"test is aborted", "bandwidth and latency"});
            IsCheckDisabled(check_bw_exists,   default_check_bw,  check_bw,  check_bw_disabled);
            IsCheckDisabled(check_lat_exists, default_check_lat, check_lat, check_lat_disabled);
        }
        // BW
        // Check write BW
        if (this->m_chan_test_it[ch_idx].is_write)
        {
            double bw_thresh_wr_lo = this->m_chan_test_it[ch_idx].bw_thresh[ch_idx].write.low;
            double bw_thresh_wr_hi = this->m_chan_test_it[ch_idx].bw_thresh[ch_idx].write.high;
            auto   bw_wr           = this->m_chan_status[ch_idx].wr_burst_latency.average_bw_mbps;
            auto   bw_wr_str       = Float_to_String<double>(bw_wr, 3);

            if ((check_bw) && ((bw_wr < bw_thresh_wr_lo) || (bw_wr > bw_thresh_wr_hi)))
            {
                ret = RET_FAILURE;
                chan_results.emplace_back(pad("***" + bw_wr_str, ' ', col_size_3, PAD_ON_RIGHT));
            }
            else
            {
                chan_results.emplace_back(pad(bw_wr_str, ' ', col_size_3, PAD_ON_RIGHT));
            }
        }
        else
        {
            chan_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_3, PAD_ON_RIGHT));
        }

        // Check read BW
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            double bw_thresh_rd_lo = this->m_chan_test_it[ch_idx].bw_thresh[ch_idx].read.low;
            double bw_thresh_rd_hi = this->m_chan_test_it[ch_idx].bw_thresh[ch_idx].read.high;
            auto   bw_rd           = this->m_chan_status[ch_idx].rd_burst_latency.average_bw_mbps;
            auto   bw_rd_str       = Float_to_String<double>(bw_rd, 3);

            if ((check_bw) && ((bw_rd < bw_thresh_rd_lo) || (bw_rd > bw_thresh_rd_hi)))
            {
                ret = RET_FAILURE;
                chan_results.emplace_back(pad("***" + bw_rd_str, ' ', col_size_4, PAD_ON_RIGHT));
            }
            else
            {
                chan_results.emplace_back(pad(bw_rd_str, ' ', col_size_4, PAD_ON_RIGHT));
            }
        }
        else
        {
            chan_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_4, PAD_ON_RIGHT));
        }

        // Latency

        // Check write latency
        if (this->m_chan_test_it[ch_idx].is_write)
        {
            double lat_thresh_wr_lo    = this->m_chan_test_it[ch_idx].lat_thresh[ch_idx].write.low;
            double lat_thresh_wr_hi    = this->m_chan_test_it[ch_idx].lat_thresh[ch_idx].write.high;
            auto   lat_wr              = this->m_chan_status[ch_idx].wr_burst_latency.average_ns;
            auto   lat_wr_str          = Float_to_String<double>(lat_wr, 1);

            if ((check_lat) && ((lat_wr < lat_thresh_wr_lo) || (lat_wr > lat_thresh_wr_hi)))
            {
                ret = RET_FAILURE;
                chan_results.emplace_back(pad("***" + lat_wr_str, ' ', col_size_5, PAD_ON_RIGHT));
            }
            else
            {
                chan_results.emplace_back(pad(lat_wr_str, ' ', col_size_5, PAD_ON_RIGHT));
            }
        }
        else
        {
            chan_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_5, PAD_ON_RIGHT));
        }

        // Check read latency
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            double lat_thresh_rd_lo    = this->m_chan_test_it[ch_idx].lat_thresh[ch_idx].read.low;
            double lat_thresh_rd_hi    = this->m_chan_test_it[ch_idx].lat_thresh[ch_idx].read.high;
            auto   lat_rd              = this->m_chan_status[ch_idx].rd_burst_latency.average_ns;
            auto   lat_rd_str          = Float_to_String<double>(lat_rd, 1);

            if ((check_lat) && ((lat_rd < lat_thresh_rd_lo) || (lat_rd > lat_thresh_rd_hi)))
            {
                ret = RET_FAILURE;
                chan_results.emplace_back(pad("***" + lat_rd_str, ' ', col_size_6, PAD_ON_RIGHT));
            }
            else
            {
                chan_results.emplace_back(pad(lat_rd_str, ' ', col_size_6, PAD_ON_RIGHT));
            }
        }
        else
        {
            chan_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_6, PAD_ON_RIGHT));
        }

        // data integrity
        if (this->m_chan_test_it[ch_idx].is_read)
        {
            chan_results.emplace_back(pad(this->m_data_integrity[ch_idx].result, ' ', col_size_7, PAD_ON_RIGHT));
        }
        else
        {
            chan_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_7, PAD_ON_RIGHT));
        }

        all_chan_results.emplace_back(chan_results);
    }

    if (this->m_memory_settings.type == MULTI_CHANNEL)
    {
        std::vector<std::string> combined_results;
        combined_results.emplace_back(pad("combined", ' ', col_size_1, PAD_ON_RIGHT));
        combined_results.emplace_back(pad("all",      ' ', col_size_2, PAD_ON_RIGHT));

        // Combined BW

        if (this->m_combined_is_write)
        {
            auto combined_bw_wr     = this->m_combined_burst_latency_and_bw_wr.average_bw_mbps;
            auto combined_bw_wr_str = Float_to_String<double>(combined_bw_wr, 3);
            combined_results.emplace_back(pad(combined_bw_wr_str, ' ', col_size_3, PAD_ON_RIGHT));
        }
        else
        {
            combined_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_3, PAD_ON_RIGHT));
        }

        if (this->m_combined_is_read)
        {
            auto combined_bw_rd     = this->m_combined_burst_latency_and_bw_rd.average_bw_mbps;
            auto combined_bw_rd_str = Float_to_String<double>(combined_bw_rd, 3);
            combined_results.emplace_back(pad(combined_bw_rd_str, ' ', col_size_4, PAD_ON_RIGHT));
        }
        else
        {
            combined_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_4, PAD_ON_RIGHT));
        }

        // Combined latency
        if (this->m_combined_is_write)
        {
            auto combined_lat_wr     = this->m_combined_burst_latency_and_bw_wr.average_ns;
            auto combined_lat_wr_str = Float_to_String<double>(combined_lat_wr, 1);
            combined_results.emplace_back(pad(combined_lat_wr_str, ' ', col_size_5, PAD_ON_RIGHT));
        }
        else
        {
            combined_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_5, PAD_ON_RIGHT));
        }

        if (this->m_combined_is_read)
        {
            auto combined_lat_rd     = this->m_combined_burst_latency_and_bw_rd.average_ns;
            auto combined_lat_rd_str = Float_to_String<double>(combined_lat_rd, 1);
            combined_results.emplace_back(pad(combined_lat_rd_str, ' ', col_size_6, PAD_ON_RIGHT));
        }
        else
        {
            combined_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_6, PAD_ON_RIGHT));
        }

        // combined data integrity
        if (this->m_combined_is_read)
        {
            combined_results.emplace_back(pad(this->m_combined_data_integrity.result, ' ', col_size_7, PAD_ON_RIGHT));
        }
        else
        {
            combined_results.emplace_back(pad(NOT_APPLICABLE, ' ', col_size_7, PAD_ON_RIGHT));
        }

        all_chan_results.emplace_back(combined_results);
    }

    LogMessage(MSG_MEM_040, {StrVectToStr(results_header, " | ")});
    for (const auto & chan_results: all_chan_results)
    {
        LogMessage(MSG_MEM_040, {StrVectToStr(chan_results, " | ")});
    }

    if (check_bw_disabled)
    {
        LogMessage(MSG_CMN_052, {"bandwidth"});
    }
    if (check_lat_disabled)
    {
        LogMessage(MSG_CMN_052, {"latency"});
    }

    // Report BW and latency result
    if (ret == RET_SUCCESS)
    {
        LogMessage(MSG_MEM_026, {GetPassMessageChan()});
    }
    else
    {
        LogMessage(MSG_MEM_025, {GetFailMessageChan()});
        if (this->m_power_tc_exists)
        {
            LogMessage(MSG_MEM_051);
        }
        ret = RET_FAILURE;
    }

    // Report data integrity result
    if (this->m_combined_is_read & this->m_combined_check_data_integrity)
    {
        if (this->m_combined_data_integrity.result == DATA_INTEG_KO)
        {
            LogMessage(MSG_MEM_023, {GetFailMessageChan()});
            ret = RET_FAILURE;
        }
        else
        {
            LogMessage(MSG_MEM_024, {GetPassMessageChan()});
        }
    }
    else
    {
        LogMessage(MSG_MEM_022);
    }

    return ret;
}

std::string MemoryTest::TestModeEnumToString( const uint & type )
{
    std::string type_str;
    switch (type)
    {
        case ALTERNATE_WR_RD_MODE_VAL:       type_str = ALTERNATE_WR_RD;     break;
        case ONLY_WR_MODE_VAL:               type_str = ONLY_WR;             break;
        case ONLY_RD_MODE_VAL:               type_str = ONLY_RD;             break;
        case SIMULTANEOUS_WR_RD_MODE_VAL:    type_str = SIMULTANEOUS_WR_RD;  break;
        default:                             type_str = "UNKNOWN";           break;
    }
    return  type_str;
}

uint MemoryTest::TestModeStringToEnum( const std::string & type )
{
    if (StrMatchNoCase(type, ALTERNATE_WR_RD))
    {
        return ALTERNATE_WR_RD_MODE_VAL;
    }
    if (StrMatchNoCase(type, ONLY_WR))
    {
        return ONLY_WR_MODE_VAL;
    }
    if (StrMatchNoCase(type, ONLY_RD))
    {
        return ONLY_RD_MODE_VAL;
    }
    if (StrMatchNoCase(type, SIMULTANEOUS_WR_RD))
    {
        return SIMULTANEOUS_WR_RD_MODE_VAL;
    }
    return UNKNOWN_MODE_VAL;
}

bool MemoryTest::IsTestModeWrite( const uint & mode_in )
{
    for (const auto & mode : {ONLY_WR_MODE_VAL, ALTERNATE_WR_RD_MODE_VAL, SIMULTANEOUS_WR_RD_MODE_VAL})
    {
        if (mode_in == mode)
        {
            return true;
        }
    }
    return false;
}

bool MemoryTest::IsTestModeRead ( const uint & mode_in )
{
    for (const auto & mode : {ONLY_RD_MODE_VAL, ALTERNATE_WR_RD_MODE_VAL, SIMULTANEOUS_WR_RD_MODE_VAL})
    {
        if (mode_in == mode)
        {
            return true;
        }
    }
    return false;
}

} // namespace
