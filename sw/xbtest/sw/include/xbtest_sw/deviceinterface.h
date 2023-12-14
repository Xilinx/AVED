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

#ifndef _DEVICEINTERFACE_H
#define _DEVICEINTERFACE_H

#include <fcntl.h>
#include <sys/mman.h>
#include <mutex>

#include "xbtestswconfig.h"
#include "deviceinterfacestructures.h"

namespace xbtest
{

class DeviceInterface_Base
{

public:

    bool m_cl_fail = false;
    bool m_abort_dev_access_msg = true;

    DeviceInterface_Base( Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const  bool & p2p_target );
    ~DeviceInterface_Base();

    //INTERLEAVED
    bool        SetupComputeUnits();

    //VIRTUAL
    virtual bool        CheckXclbinDownloadTime();


    //ABSTRACT VIRTUAL
    virtual bool        SelectDevice() = 0;
    virtual bool        SetupDevice() = 0;
    virtual bool        WriteGtmacCuCmd         ( const uint & cu_idx, const uint & value ) = 0;
    virtual bool        WriteGtmacCuTrafficCfg  ( const uint & cu_idx, uint32_t * traffic_cfg ) = 0;
    virtual bool        ReadGtmacCuTrafficCfg   ( const uint & cu_idx, uint32_t * read_buffer ) = 0;



    bool                CheckDeviceInfo();
    uint                GetNumSkippedCu();
    uint                GetNumCompanionCu();
    uint                GetNumVerifyCu();
    uint                GetNumPowerCu();
    uint                GetNumGtmacCu();
    uint                GetNumGtlpbkCu();
    uint                GetNumGtPrbsCu();
    uint                GetNumGtfPrbsCu();
    uint                GetNumGtmPrbsCu();
    uint                GetNumGtypPrbsCu();
    uint                GetNumUnknownCu();
    uint                GetNumMemoryCu          ( const std::string & memory_type, const std::string & memory_name );
    std::string         GetSkippedCuName        ( const uint & cu_idx );
    std::string         GetCompanionCuName      ( const uint & cu_idx );
    std::string         GetVerifyCuName         ( const uint & cu_idx );
    std::string         GetPwrCuName            ( const uint & cu_idx );
    std::string         GetGTMACCuName          ( const uint & cu_idx );
    std::string         GetGTLpbkCuName         ( const uint & cu_idx );
    std::string         GetGTPrbsCuName         ( const uint & cu_idx );
    std::string         GetGTFPrbsCuName        ( const uint & cu_idx );
    std::string         GetGTMPrbsCuName        ( const uint & cu_idx );
    std::string         GetGTYPPrbsCuName       ( const uint & cu_idx );
    std::string         GetUnknownCuName        ( const uint & cu_idx );
    std::string         GetMemoryCuName         ( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx );
    uint                GetVerifyCuDnaRead      ( const uint & cu_idx );
    uint                GetPowerCuSlrIndex      ( const uint & cu_idx );
    uint                GetGtmacCuGTIndex       ( const uint & cu_idx );
    uint                GetGtlpbkCuGTIndex      ( const uint & cu_idx );
    uint                GetGtPrbsCuGTIndex      ( const uint & cu_idx );
    std::vector<uint>   GetGtfPrbsCuGTFIndexes  ( const uint & cu_idx );
    std::vector<uint>   GetGtmPrbsCuGTMIndexes  ( const uint & cu_idx );
    std::vector<uint>   GetGtypPrbsCuGTYPIndexes( const uint & cu_idx );
    bool                ReadVerifyCu            ( const uint & cu_idx, const uint & address, uint & read_data );
    bool                ReadPowerCu             ( const uint & cu_idx, const uint & address, uint & read_data );
    bool                ReadGtmacCu             ( const uint & cu_idx, const uint & address, uint & read_data );
    bool                ReadGtlpbkCu            ( const uint & cu_idx, const uint & address, uint & read_data );
    bool                ReadGtPrbsCu            ( const uint & cu_idx, const uint & address, uint & read_data );
    bool                ReadGtfPrbsCu           ( const uint & cu_idx, const int & gt_idx, const uint & address, uint & read_data );
    bool                ReadGtmPrbsCu           ( const uint & cu_idx, const int & gt_idx, const uint & address, uint & read_data );
    bool                ReadGtypPrbsCu          ( const uint & cu_idx, const int & gt_idx, const uint & address, uint & read_data );
    bool                ReadGtfPrbsCuStatus     ( const uint & cu_idx, const int & gt_idx, uint32_t * read_buffer, const uint64_t & size );
    bool                ReadGtmPrbsCuStatus     ( const uint & cu_idx, const int & gt_idx, uint32_t * read_buffer, const uint64_t & size );
    bool                ReadGtypPrbsCuStatus    ( const uint & cu_idx, const int & gt_idx, uint32_t * read_buffer, const uint64_t & size );
    bool                ReadMemoryCu            ( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, uint & read_data );
    bool                ReadMemoryCuStatus      ( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, uint32_t * read_buffer, const uint64_t & size );
    bool                WriteVerifyCu           ( const uint & cu_idx, const uint & address, const uint & value );
    bool                WritePowerCu            ( const uint & cu_idx, const uint & address, const uint & value );
    bool                WriteGtmacCu            ( const uint & cu_idx, const uint & address, const uint & value );
    bool                WriteGtlpbkCu           ( const uint & cu_idx, const uint & address, const uint & value );
    bool                WriteGtPrbsCu           ( const uint & cu_idx, const uint & address, const uint & value );
    bool                WriteGtfPrbsCu          ( const uint & cu_idx, const int & gt_idx, const uint & address, const uint & value );
    bool                WriteGtmPrbsCu          ( const uint & cu_idx, const int & gt_idx, const uint & address, const uint & value );
    bool                WriteGtypPrbsCu         ( const uint & cu_idx, const int & gt_idx, const uint & address, const uint & value );
    bool                WriteMemoryCu           ( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, const uint & value );

    Build_Info          GetVerifyCuBI           ( const uint & cu_idx );
    Build_Info          GetPowerCuBI            ( const uint & cu_idx );
    Build_Info          GetGtmacCuBI            ( const uint & cu_idx );
    Build_Info          GetGtlpbkCuBI           ( const uint & cu_idx );
    Build_Info          GetGtPrbsCuBI           ( const uint & cu_idx );
    Build_Info          GetGtfPrbsCuBI          ( const uint & cu_idx, const int & gt_idx );
    Build_Info          GetGtmPrbsCuBI          ( const uint & cu_idx, const int & gt_idx );
    Build_Info          GetGtypPrbsCuBI         ( const uint & cu_idx, const int & gt_idx );
    Build_Info          GetMemoryCuBI           ( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx );

    uint                GetGTFNumLanes          ( const uint & cu_idx, const int & gt_idx);
    uint                GetGTMNumLanes          ( const uint & cu_idx, const int & gt_idx);
    uint                GetGTYPNumLanes         ( const uint & cu_idx, const int & gt_idx);

    void        LockGTRstMtx                    (const int & gt_idx);
    void        UnLockGTRstMtx                  (const int & gt_idx);

    uint64_t            RoundUpToMultiple       ( const uint64_t & value, const uint64_t & multiple );

protected:

    //VIRTUAL
    virtual bool CheckCUsNotDeadlocked();
    virtual bool SetupPsComputeUnits();
    virtual bool ConfigurePlramShadowBuffers();
    virtual bool CreateAllM00AXIBuffers();
    virtual bool InitCUArgs();

    //ABSTRACT VIRTUAL
    virtual bool        InitStruct                      (const Xbtest_Compute_Units_t & compute_unit, Cl_Compute_Unit_t & cl_compute_unit) = 0;
    virtual bool        ConfigureM00AxiBuffers          ( std::vector<Cl_Compute_Unit_t> & cl_compute_units ) = 0;
    virtual bool        SetCuM00AxiPtrArgument          ( std::vector<Cl_Compute_Unit_t> & cl_compute_units ) = 0;
    virtual bool        CreateCuMxxAxiBuffers           ( std::vector<Cl_Compute_Unit_t> & cl_compute_units ) = 0;
    virtual bool        ReadComputeUnit                 ( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, uint & read_data ) = 0;
    virtual bool        ReadMemoryCuStatusInner         ( Cl_Compute_Unit_t & cl_compute_unit, uint32_t * read_buffer, const uint64_t & size) = 0;
    virtual bool        WriteComputeUnit                ( Cl_Compute_Unit_t & cl_compute_unit, const uint & core_address, const uint & value ) = 0;
    virtual bool        ReadMultiGtStatus               ( Cl_Compute_Unit_t & cl_compute_unit, const uint & core_address, uint32_t * read_buffer, const uint64_t & size) = 0;


    Message_t MSG_ITF_017_ERROR  = MSG_ITF_017; // Error by default in message definition
    Message_t MSG_ITF_017_WARN   = MSG_ITF_017;

    const uint64_t      M_AXI_BASE_SIZE_BYTES   = (uint64_t)(1024) * (uint64_t)(1024); // mXX_axi buffer pointers MB
    const uint64_t      MAX_OCL_BUFFER_SIZE_MB  = (uint64_t)(4095); // MB

    std::string                                             m_log_header = LOG_HEADER_DEVICE;
    std::atomic<bool> *                                     m_abort = nullptr;
    Logging *                                               m_log = nullptr;
    XbtestSwConfig *                                        m_xbtest_sw_config = nullptr;
    bool                                                    m_p2p_target;
    std::string                                             m_device_bdf;

    std::vector<Cl_Compute_Unit_t>                          m_cl_skipped_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_companion_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_verify_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_pwr_compute_units;
    std::map<std::string,std::vector<Cl_Compute_Unit_t>>    m_cl_mem_sc_compute_units;
    std::map<std::string,std::vector<Cl_Compute_Unit_t>>    m_cl_mem_mc_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_gt_mac_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_gt_lpbk_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_gt_prbs_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_gtf_prbs_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_gtm_prbs_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_gtyp_prbs_compute_units;
    std::vector<Cl_Compute_Unit_t>                          m_cl_unknown_compute_units;
    std::vector<Cl_Buffer_t>                                cl_m00_axi_buffers;

    bool                                                    m_use_ps_kernels = false;

    double                                                  m_xclbin_download_time = 0.0;

    void        LogMessage                      ( const Message_t & message, const std::vector<std::string> & arg_list );
    void        LogMessage                      ( const Message_t & message );
    //bool        CreateCuMxxAxiBuffersNoOffset   ( std::vector<Cl_Compute_Unit_t> & cl_compute_units );
    uint        GetNumMemoryScCu                ( const std::string & memory_name );
    uint        GetNumMemoryMcCu                ( const std::string & memory_name );
    std::string GetMemSCCuName                  ( const std::string & memory_name, const uint & cu_idx );
    std::string GetMemMCCuName                  ( const std::string & memory_name, const uint & cu_idx );
    Build_Info  GetMemoryScCuBI                 ( const std::string & memory_name, const uint & cu_idx, const int & ch_idx  );
    Build_Info  GetMemoryMcCuBI                 ( const std::string & memory_name, const uint & cu_idx, const int & ch_idx  );
    uint        GetMemoryCuAddrOffset           ( const int & ch_idx );
    uint        GetMultiGtCuAddrOffset          ( const int & gt_idx );
    bool        ReadMemoryScCu                  ( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, uint & read_data );
    bool        ReadMemoryMcCu                  ( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, uint & read_data );
    bool        ReadMemoryScCuStatus            ( const std::string & memory_name, const uint & cu_idx, uint32_t * read_buffer, const uint64_t & size );
    bool        ReadMemoryMcCuStatus            ( const std::string & memory_name, const uint & cu_idx, uint32_t * read_buffer, const uint64_t & size );
    bool        WriteMemoryScCu                 ( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, const uint & value );
    bool        WriteMemoryMcCu                 ( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, const uint & value );
    bool        ReadVerifyCuBI                  ( const uint & cu_idx );
    bool        ReadPowerCuBI                   ( const uint & cu_idx );
    bool        ReadGtmacCuBI                   ( const uint & cu_idx );
    bool        ReadGTLpbkCuBI                  ( const uint & cu_idx );
    bool        ReadGTPrbsCuBI                  ( const uint & cu_idx );
    bool        ReadGTFPrbsCuBI                 ( const uint & cu_idx );
    bool        ReadGTFPrbsCuInternBI           ( const uint & cu_idx, const int & gt_idx, Build_Info & build_info );
    bool        ReadGTMPrbsCuBI                 ( const uint & cu_idx );
    bool        ReadGTMPrbsCuInternBI           ( const uint & cu_idx, const int & gt_idx, Build_Info & build_info );
    bool        ReadGTYPPrbsCuBI                ( const uint & cu_idx );
    bool        ReadGTYPPrbsCuInternBI          ( const uint & cu_idx, const int & gt_idx, Build_Info & build_info );
    bool        ReadMemoryCuBI                  ( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx );
    bool        ReadMemoryCuInternBI            ( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx, Build_Info & build_info );
    void        PrintCommonCuBI                 ( const Build_Info & build_info, const int & ch_idx );
    void        PrintVerifyCuBI                 ( const Build_Info & build_info );
    void        PrintPowerCuBI                  ( const Build_Info & build_info );
    void        PrintMemoryCuBI                 ( const Build_Info & build_info, const int & ch_idx );
    void        PrintGtmacCuBI                  ( const Build_Info & build_info );
    void        PrintGTLpbkCuBI                 ( const Build_Info & build_info );
    void        PrintGTPrbsCuBI                 ( const Build_Info & build_info );
    void        PrintGTFPrbsCuBI                ( const Build_Info & build_info, const int & gt_idx );
    void        PrintGTMPrbsCuBI                ( const Build_Info & build_info, const int & gt_idx );
    void        PrintGTYPPrbsCuBI               ( const Build_Info & build_info, const int & gt_idx );

    std::mutex  m_gt_rst_mtx;
};

} // namespace

#endif /* _DEVICEINTERFACE_H */
