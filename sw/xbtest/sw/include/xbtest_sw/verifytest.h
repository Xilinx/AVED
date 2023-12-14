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

#ifndef _VERIFYTEST_H
#define _VERIFYTEST_H

#include "testinterface.h"
#include "devicemgt.h"

namespace xbtest
{

class VerifyTest: public TestInterface
{

public:
    VerifyTest( Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt );
    ~VerifyTest();

    bool    PreSetup() override;
    void    Run() override;
    void    PostTeardown() override;
    void    Abort() override;
    int     RunTest() override;

    bool PreTestChecks();
    bool GetPostTestChecksResult();

private:

    const uint  DNA_READ_TIMEOUT    = 10;

    const uint  VERIFY_CTRL_REG_ADDR        = 0x20;
    const uint  VERIFY_DNA_0_REG_ADDR       = 0x21;
    const uint  VERIFY_DNA_1_REG_ADDR       = 0x22;
    const uint  VERIFY_DNA_2_REG_ADDR       = 0x23;

    const uint  VERIFY_DNA_READ_AVAILABLE   = (0x1 << 0);
    const uint  VERIFY_DNA_READ_REQUEST     = (0x1 << 1);
    // done also means that it's available
    const uint  VERIFY_DNA_READ_DONE        = (0x1 << 2) | VERIFY_DNA_READ_AVAILABLE;

    uint  m_cu_idx = 0; // only 1 xbtest HW IP so far ...

    int test_state = TEST_STATE_FAILURE;

    DeviceMgt *        m_devicemgt = nullptr;

    bool WriteVerifyCu( const uint & address, const uint & value);
    bool ReadVerifyCu ( const uint & address, uint & read_data );

    bool CheckVersion          ( const std::string & cu_name, const double & hw_version, const double & sw_version, const double & min_hw_version );
    bool CheckVerifyCu         ( Build_Info & build_info, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckPowerCu          ( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckGtmacCu          ( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckGtlpbkCu         ( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckGtPrbsCu         ( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckGtfPrbsCu        ( Build_Info & build_info, const uint & cu_idx, const int & gt_idx, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckGtmPrbsCu        ( Build_Info & build_info, const uint & cu_idx, const int & gt_idx, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckGtypPrbsCu       ( Build_Info & build_info, const uint & cu_idx, const int & gt_idx, uint & verify_pass_cnt, uint & verify_fail_cnt);
    bool CheckMemoryCu         ( Build_Info & build_info, const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt, const int & ch_idx);
    void PrintVerifyCuSummary  ( const std::string & cu_name, const uint & verify_pass_cnt, const uint & verify_fail_cnt );

    bool StartCU() override;
    bool EnableWatchdogClkThrotDetection() override;
    bool StopCU() override;
    bool CheckWatchdogAndClkThrottlingAlarms() override;

    bool ReadDNA();
    bool RunPreTests();
    bool RunPostTests();
    bool PostTestChecks();

    bool pretest_done = false;
    bool post_test_checks_result = RET_FAILURE;

    template<typename T> bool VerifyBIValue( const Build_Info & build_info, const std::string & param_name, const T & param_read, const T & param_expected, uint & verify_pass_cnt, uint & verify_fail_cnt )
    {
        if (param_read != param_expected)
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, param_name, std::to_string(param_read), std::to_string(param_expected)});
            verify_fail_cnt++;
            return RET_FAILURE;
        }
        verify_pass_cnt++;
        return RET_SUCCESS;
    }

};

} // namespace

#endif /* _VERIFYTEST_H */
