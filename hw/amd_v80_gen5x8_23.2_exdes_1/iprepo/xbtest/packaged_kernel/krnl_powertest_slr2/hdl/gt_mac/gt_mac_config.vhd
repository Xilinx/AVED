-- Copyright (C) 2022 Xilinx, Inc.
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--   http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

library ieee, unisim, xpm;
    use ieee.std_logic_1164.all;
    use ieee.std_logic_unsigned.all;
    use ieee.std_logic_arith.all;
    use ieee.std_logic_misc.all;
    use work.gt_test_pack.all;
    use unisim.vcomponents.all;
    use xpm.vcomponents.all;


entity gt_mac_config is
    generic (
        C_MAJOR_VERSION         : integer;
        C_MINOR_VERSION         : integer;
        DEST_SYNC_FF            : integer;
        C_BUILD_VERSION         : integer;
        C_CLOCK0_FREQ           : integer;
        C_CLOCK1_FREQ           : integer;
        C_BLOCK_ID              : integer;
        C_GT_INDEX              : integer;
        C_GT_NUM_GT             : integer;
        C_GT_NUM_LANE           : integer;
        C_GT_RATE               : integer;
        C_GT_TYPE               : integer;
        C_GT_MAC_ENABLE_RSFEC   : integer
    );
    port (
        clk_300             : in    std_logic;
        clk_300_cont        : in    std_logic;
        rst_300_raw         : in    std_logic;
        rst_300_gate        : in    std_logic;

        axi_clk             : in    std_logic;
        axi_clk_rst_n       : in    std_logic;

        watchdog_alarm_in   : in    std_logic;

        rst_allow           : out   std_logic;

        Start               : in    std_logic;
        Done                : out   std_logic;
        AXI00_Ptr0          : in    std_logic_vector(63 downto 0);  -- PLRAM Offset
        Scalar00            : in    std_logic_vector(31 downto 0);  -- Run Configuration
        Scalar01            : in    std_logic_vector(31 downto 0);  -- Run Configuration

        PLRAM_awready       : in    std_logic;
        PLRAM_awvalid       : out   std_logic;
        PLRAM_awaddr        : out   std_logic_vector(63 downto 0);
        PLRAM_awlen         : out   std_logic_vector(7 downto 0);

        PLRAM_wready        : in    std_logic;
        PLRAM_wvalid        : out   std_logic;
        PLRAM_wdata         : out   std_logic_vector(31 downto 0);
        PLRAM_wstrb         : out   std_logic_vector(3 downto 0);
        PLRAM_wlast         : out   std_logic;

        PLRAM_bvalid        : in    std_logic;
        PLRAM_bready        : out   std_logic;

        PLRAM_arready       : in    std_logic;
        PLRAM_arvalid       : out   std_logic;
        PLRAM_araddr        : out   std_logic_vector(63 downto 0);
        PLRAM_arlen         : out   std_logic_vector(7 downto 0);

        PLRAM_rready        : out   std_logic;
        PLRAM_rvalid        : in    std_logic;
        PLRAM_rlast         : in    std_logic;
        PLRAM_rdata         : in    std_logic_vector(31 downto 0);

        MAC_aresetn         : out   std_logic;
        MAC_local_fault     : in    std_logic_vector(0 to 3);

        MAC_Sel_Mode        : in    std_logic_vector(0 to 3);
        MAC_Mode_Change     : out   std_logic_vector(0 to 3);
        MAC_PM_Tick         : out   std_logic_vector(0 to 3);
        MAC_rx_reset        : out   std_logic_vector(0 to 3);
        MAC_tx_reset        : out   std_logic_vector(0 to 3);

        MAC_awready         : in    std_logic_vector(0 to 3);
        MAC_awvalid         : out   std_logic_vector(0 to 3);
        MAC_awaddr_0        : out   std_logic_vector(31 downto 0);
        MAC_awaddr_1        : out   std_logic_vector(31 downto 0);
        MAC_awaddr_2        : out   std_logic_vector(31 downto 0);
        MAC_awaddr_3        : out   std_logic_vector(31 downto 0);

        MAC_wready          : in    std_logic_vector(0 to 3);
        MAC_wvalid          : out   std_logic_vector(0 to 3);
        MAC_wdata_0         : out   std_logic_vector(31 downto 0);
        MAC_wdata_1         : out   std_logic_vector(31 downto 0);
        MAC_wdata_2         : out   std_logic_vector(31 downto 0);
        MAC_wdata_3         : out   std_logic_vector(31 downto 0);
        MAC_wstrb_0         : out   std_logic_vector(3 downto 0);
        MAC_wstrb_1         : out   std_logic_vector(3 downto 0);
        MAC_wstrb_2         : out   std_logic_vector(3 downto 0);
        MAC_wstrb_3         : out   std_logic_vector(3 downto 0);

        MAC_arready         : in    std_logic_vector(0 to 3);
        MAC_arvalid         : out   std_logic_vector(0 to 3);
        MAC_araddr_0        : out   std_logic_vector(31 downto 0);
        MAC_araddr_1        : out   std_logic_vector(31 downto 0);
        MAC_araddr_2        : out   std_logic_vector(31 downto 0);
        MAC_araddr_3        : out   std_logic_vector(31 downto 0);

        MAC_rready          : out   std_logic_vector(0 to 3);
        MAC_rvalid          : in    std_logic_vector(0 to 3);
        MAC_rdata_0         : in    std_logic_vector(31 downto 0);
        MAC_rdata_1         : in    std_logic_vector(31 downto 0);
        MAC_rdata_2         : in    std_logic_vector(31 downto 0);
        MAC_rdata_3         : in    std_logic_vector(31 downto 0);
        MAC_rresp_0         : in    std_logic_vector(1 downto 0);
        MAC_rresp_1         : in    std_logic_vector(1 downto 0);
        MAC_rresp_2         : in    std_logic_vector(1 downto 0);
        MAC_rresp_3         : in    std_logic_vector(1 downto 0);

        Config_Upd          : out   std_logic_vector(0 to 3);
        Dest_MAC_Addr_0     : out   std_logic_vector(47 downto 0);
        Dest_MAC_Addr_1     : out   std_logic_vector(47 downto 0);
        Dest_MAC_Addr_2     : out   std_logic_vector(47 downto 0);
        Dest_MAC_Addr_3     : out   std_logic_vector(47 downto 0);
        Source_MAC_Addr_0   : out   std_logic_vector(47 downto 0);
        Source_MAC_Addr_1   : out   std_logic_vector(47 downto 0);
        Source_MAC_Addr_2   : out   std_logic_vector(47 downto 0);
        Source_MAC_Addr_3   : out   std_logic_vector(47 downto 0);
        MAC_Ethertype_0     : out   std_logic_vector(15 downto 0);
        MAC_Ethertype_1     : out   std_logic_vector(15 downto 0);
        MAC_Ethertype_2     : out   std_logic_vector(15 downto 0);
        MAC_Ethertype_3     : out   std_logic_vector(15 downto 0);
        MAC_Traffic_Type_0  : out   std_logic_vector(1 downto 0);
        MAC_Traffic_Type_1  : out   std_logic_vector(1 downto 0);
        MAC_Traffic_Type_2  : out   std_logic_vector(1 downto 0);
        MAC_Traffic_Type_3  : out   std_logic_vector(1 downto 0);

        GT_Rx_LPM_En        : out   std_logic_vector(0 to 3);
        GT_TX_Polarity      : out   std_logic_vector(0 to 3);
        GT_Tx_Main_Cursor_0 : out   std_logic_vector(6 downto 0);
        GT_Tx_Main_Cursor_1 : out   std_logic_vector(6 downto 0);
        GT_Tx_Main_Cursor_2 : out   std_logic_vector(6 downto 0);
        GT_Tx_Main_Cursor_3 : out   std_logic_vector(6 downto 0);
        GT_Tx_Post_Cursor_0 : out   std_logic_vector(4 downto 0);
        GT_Tx_Post_Cursor_1 : out   std_logic_vector(4 downto 0);
        GT_Tx_Post_Cursor_2 : out   std_logic_vector(4 downto 0);
        GT_Tx_Post_Cursor_3 : out   std_logic_vector(4 downto 0);
        GT_Tx_Pre_Cursor_0  : out   std_logic_vector(4 downto 0);
        GT_Tx_Pre_Cursor_1  : out   std_logic_vector(4 downto 0);
        GT_Tx_Pre_Cursor_2  : out   std_logic_vector(4 downto 0);
        GT_Tx_Pre_Cursor_3  : out   std_logic_vector(4 downto 0);
        GT_Tx_Diff_Ctrl_0   : out   std_logic_vector(4 downto 0);
        GT_Tx_Diff_Ctrl_1   : out   std_logic_vector(4 downto 0);
        GT_Tx_Diff_Ctrl_2   : out   std_logic_vector(4 downto 0);
        GT_Tx_Diff_Ctrl_3   : out   std_logic_vector(4 downto 0);

        Gen_Pkt             : out   std_logic_vector(0 to 3);
        Gen_Length_0        : out   std_logic_vector(13 downto 0);
        Gen_Length_1        : out   std_logic_vector(13 downto 0);
        Gen_Length_2        : out   std_logic_vector(13 downto 0);
        Gen_Length_3        : out   std_logic_vector(13 downto 0);
        Gen_Half_Full       : in    std_logic_vector(0 to 3);

        Mac_Rst_Status      : in    std_logic_vector(11 downto 0);

        Traffic_Cnt_Latch   : out   std_logic;

        Cnt_RAM_Rd_Addr_0   : out   std_logic_vector(3 downto 0);
        Cnt_RAM_Rd_Addr_1   : out   std_logic_vector(3 downto 0);
        Cnt_RAM_Rd_Addr_2   : out   std_logic_vector(3 downto 0);
        Cnt_RAM_Rd_Addr_3   : out   std_logic_vector(3 downto 0);
        Cnt_RAM_Data_0      : in    std_logic_vector(31 downto 0);
        Cnt_RAM_Data_1      : in    std_logic_vector(31 downto 0);
        Cnt_RAM_Data_2      : in    std_logic_vector(31 downto 0);
        Cnt_RAM_Data_3      : in    std_logic_vector(31 downto 0)

    );
end entity gt_mac_config;

architecture rtl of gt_mac_config is

-------------------------------------------------------------------------------
--
--      Constants (simple)
--
-------------------------------------------------------------------------------

constant SIM_DIVIDER            : integer
-- synthesis translate_off
                                := 500;
constant DUMMY_DIVIDER          : integer
-- synthesis translate_on
                                := 1;

constant MAC_STATUS_REGS    : std_logic_2d_12(0 to 85)      := (X"500", X"504", X"648", X"64C", X"660", X"664", X"670", X"674",
                                                                X"678", X"67C", X"680", X"684", X"808", X"80C", X"810", X"814",
                                                                X"818", X"81C", X"820", X"824", X"828", X"82C", X"830", X"834",
                                                                X"838", X"83C", X"840", X"844", X"848", X"84C", X"850", X"854",
                                                                X"858", X"85C", X"860", X"864", X"868", X"86C", X"870", X"874",
                                                                X"878", X"87C", X"880", X"884", X"888", X"88C", X"890", X"894",
                                                                X"898", X"89C", X"8A0", X"8A4", X"8A8", X"8AC", X"8B0", X"8B4",
                                                                X"8B8", X"8BC", X"8C0", X"8C4", X"8C8", X"8CC", X"8D0", X"8D4",
                                                                X"8D8", X"8DC", X"8E0", X"8E4", X"8E8", X"8EC", X"8F0", X"8F4",
                                                                X"908", X"90C", X"910", X"914", X"918", X"91C", X"920", X"924",
                                                                X"928", X"92C", X"700", X"704", X"710", X"714");

constant STAT_BASE          : integer                       :=  64;
constant STAT_INTERVAL      : integer                       :=  96;

constant NUM_STAT_REGS      : integer                       := MAC_STATUS_REGS'LENGTH;
constant NUM_BIT_STAT       : integer                       := fn_Log2(NUM_STAT_REGS);

constant CONF_BASE          : std_logic_vector(31 downto 0) := X"00000004"; -- Leave 0x00000000 free for BuildInfo reads
constant NUM_INT_REGS       : integer                       := 7;
constant MAC_CONF_REGS      : std_logic_2d_12(0 to 7)       := (X"008", X"00C", X"014", X"018", X"01C", X"0D0", X"0D4", X"138");
constant NUM_REG            : integer                       := MAC_CONF_REGS'LENGTH + NUM_INT_REGS;

constant NUM_BIT_INT_REGS   : integer                       := fn_Log2(NUM_INT_REGS);
constant NUM_BIT_EXT_REGS   : integer                       := fn_Log2(MAC_CONF_REGS'LENGTH);
constant NUM_BIT_REG        : integer                       := fn_Log2(NUM_REG);

constant ST_IDLE            : std_logic_vector(3 downto 0)  := "0000";
constant ST_START           : std_logic_vector(3 downto 0)  := "0001";
constant ST_GET_STATUS      : std_logic_vector(3 downto 0)  := "0010";
constant ST_GET_STATUS_DONE : std_logic_vector(3 downto 0)  := "0011";
constant ST_SEND_CONF       : std_logic_vector(3 downto 0)  := "0100";
constant ST_SEND_CONF_WAIT  : std_logic_vector(3 downto 0)  := "0101";
constant ST_SEND_CONF_DONE  : std_logic_vector(3 downto 0)  := "0110";
constant ST_RUN_SCRIPT      : std_logic_vector(3 downto 0)  := "0111";
constant ST_STOP_SCRIPT     : std_logic_vector(3 downto 0)  := "1000";
constant ST_WAIT_TX_RST     : std_logic_vector(3 downto 0)  := "1001";
constant ST_WAIT_RX_RST     : std_logic_vector(3 downto 0)  := "1010";

-- Default Transceiver Physical Interface settings

constant DEF_RX_LPM_EN      : std_logic                     := '0';         -- DFE Enabled
constant DEF_TX_MAIN_CURSOR : std_logic_vector(6 downto 0)  := "1010000";   -- Not currently used TX_MAINCURSOR_SEL = 1'b0
constant DEF_TX_POST_CURSOR : std_logic_vector(4 downto 0)  := "00000";     -- 0 dB
constant DEF_TX_PRE_CURSOR  : std_logic_vector(4 downto 0)  := "00000";     -- 0 dB
constant DEF_TX_DIFF_CTRL   : std_logic_vector(4 downto 0)  := "01101";     -- 730 mVpp
constant DEF_TX_POLARITY    : std_logic                     := '0';         -- Not inverted

-------------------------------------------------------------------------------
--
--      Type Definitions
--
-------------------------------------------------------------------------------

type PLRD_Pipe_Type is
    record
        Conf_Val    : std_logic;
        Scr_Val     : std_logic;
        Valid       : std_logic;
        Chan        : std_logic_vector(1 downto 0);
        Reg         : std_logic_vector(NUM_BIT_REG-1 downto 0);
        Rd_Addr     : std_logic_vector(31 downto 0);
        Rd_Ptr      : std_logic_vector(15 downto 0);
    end record;

type PLRD_Pipe_Array_Type is array (integer range <>) of PLRD_Pipe_Type;

constant RST_PLRD_PIPE_TYPE : PLRD_Pipe_Type := (
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    (others => '0'),    -- std_logic_vector(1 downto 0);;
    (others => '0'),    -- std_logic_vector(NUM_BIT_REG-1 downto 0);
    (others => '0'),    -- std_logic_vector(31 downto 0);
    (others => '0'));   -- std_logic_vector(15 downto 0);

type PLWR_Pipe_Type is
    record
        Valid       : std_logic;
        Valid_A     : std_logic;
        Valid_D     : std_logic;
        Cmd_Rd      : std_logic;
        Cmd_Wr      : std_logic;
        Cmd_Ad      : std_logic_vector(31 downto 0);
        Cmd_Da      : std_logic_vector(31 downto 0);
        Chan        : std_logic_vector(1 downto 0);
        Wr_Addr     : std_logic_vector(31 downto 0);
        Data        : std_logic_vector(31 downto 0);
    end record;

type PLWR_Pipe_Array_Type is array (integer range <>) of PLWR_Pipe_Type;

constant RST_PLWR_PIPE_TYPE : PLWR_Pipe_Type := (
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    (others => '0'),    -- std_logic_vector(2 downto 0);;
    (others => '0'),    -- std_logic_vector(31 downto 0);;
    (others => '0'),    -- std_logic_vector(1 downto 0);;
    (others => '0'),    -- std_logic_vector(31 downto 0);
    (others => '0'));   -- std_logic_vector(31 downto 0);

type MR_Pipe_Type is
    record
        Valid   : std_logic;
        Rd_Addr : std_logic_vector(31 downto 0);
        Data    : std_logic_vector(31 downto 0);
        Wr_Addr : std_logic_vector(31 downto 0);
    end record;

type MR_Pipe_Array_Type is array (integer range <>) of MR_Pipe_Type;

constant RST_MR_PIPE_TYPE : MR_Pipe_Type := (
    '0',                -- std_logic;
    (others => '0'),    -- std_logic_vector(31 downto 0);
    (others => '0'),    -- std_logic_vector(31 downto 0);
    (others => '0'));   -- std_logic_vector(31 downto 0);

type MW_Pipe_Type is
    record
        Valid   : std_logic;
        Scr_Val : std_logic;
        Val_Int : std_logic;
        Val_Ext : std_logic;
        Valid_A : std_logic;
        Valid_D : std_logic;
        Reg_Int : std_logic_vector(NUM_BIT_INT_REGS-1 downto 0);
        Reg_Ext : std_logic_vector(NUM_BIT_REG downto 0);
        Data    : std_logic_vector(31 downto 0);
        Wr_Addr : std_logic_vector(31 downto 0);
    end record;

type MW_Pipe_Array_Type is array (integer range <>) of MW_Pipe_Type;

constant RST_MW_PIPE_TYPE : MW_Pipe_Type := (
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    '0',                -- std_logic;
    (others => '0'),    -- std_logic_vector(NUM_BIT_INT_REGS-1 downto 0);
    (others => '0'),    -- std_logic_vector(NUM_BIT_REG downto 0);
    (others => '0'),    -- std_logic_vector(31 downto 0);
    (others => '0'));   -- std_logic_vector(31 downto 0);

type TCM_pipe_Type is
    record
        Addr            : std_logic_vector(5 downto 0);
        Cnt             : std_logic_vector(31 downto 0);
    end record;

type TCM_pipe_Array_Type is array (integer range <>) of TCM_pipe_Type;

constant RST_TCM_pipe_TYPE : TCM_pipe_Type := (
    (others => '0'),
    (others => '0')
    );


-------------------------------------------------------------------------------
--
--      Constants (complex)
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
--
--      Functions
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
--
--      Component
--
-------------------------------------------------------------------------------
component build_info_v4_0 is
    generic (
        C_MAJOR_VERSION     : integer := 0;
        C_MINOR_VERSION     : integer := 0;
        C_BUILD_VERSION     : integer := 0;
        C_BLOCK_ID          : integer := 0
    );
    port (
        Clk         : in  std_logic;
        Rst         : in  std_logic;

        Info_1      : in  std_logic_vector(15 downto 0);
        Info_2      : in  std_logic_vector(15 downto 0);
        Info_3      : in  std_logic_vector(15 downto 0);
        Info_4      : in  std_logic_vector(15 downto 0);
        Info_5      : in  std_logic_vector(15 downto 0);
        Info_6      : in  std_logic_vector(15 downto 0);
        Info_7      : in  std_logic_vector(15 downto 0);
        Info_8      : in  std_logic_vector(15 downto 0);

        We          : in  std_logic;
        Addr        : in  std_logic_vector(2 downto 0);
        Data_In     : in  std_logic_vector(31 downto 0);
        Data_Out    : out std_logic_vector(31 downto 0)
    );

end component;

component common_ctrl_status is
    generic (
        C_CLOCK_FREQ            : integer     := 300; -- Frequency in MHz of the common_ctrl_status clock
        DEST_SYNC_FF            : integer     := 4;
        C_CLK_TROTTLE_DETECT_EN : integer     := 2;
        C_WATCHDOG_ENABLE       : integer     := 1;
        C_EXT_TOGGLE_1_SEC      : integer     := 0  -- 1: use toggle_1_sec input, 0: generate internally
    );
    port (
        ap_clk          : in  std_logic;
        ap_clk_cont     : in  std_logic;
        ap_rst          : in  std_logic;
        ap_clk_2        : in  std_logic;
        ap_clk_2_cont   : in  std_logic;
        ap_rst_2        : in  std_logic;

        toggle_1sec     : in  std_logic;
        rst_watchdog    : in  std_logic;
        watchdog_alarm  : out std_logic;

        We              : in  std_logic;
        Addr            : in  std_logic_vector(2 downto 0);
        Data_In         : in  std_logic_vector(31 downto 0);
        User_Status_1   : in  std_logic_vector(31 downto 0);
        Data_Out        : out std_logic_vector(31 downto 0)
    );
end component;

component timer is
    generic (
        C_CLOCK_FREQ          : integer := 300;      -- Frequency in MHz of the timer clock
        C_TIMER_RELOAD        : integer := 2999999;  -- Example value for a 10ms timer at 300MHz
        C_1_SEC_TIMER_EN      : integer := 0;        -- 1: Enable 1 sec timer; 0: disable
        C_USER_TIMER_EN       : integer := 0;        -- 1: Enable User-defined timer; 0: disable
        C_CLK_2_EN            : integer := 1;        -- 1: Enable clk CDC; 0: disable
        DEST_SYNC_FF          : integer := 4;
        C_SIM_DIVIDER         : integer := 1
    );
    port (
        clk_1           : in    std_logic;
        clk_2           : in    std_logic;

        toggle_1_sec_1  : out   std_logic;
        toggle_1_sec_2  : out   std_logic;

        timer_rst       : in    std_logic;
        timer_end_1     : out   std_logic;
        timer_end_2     : out   std_logic
    );
end component timer;


component cdc_static_bus is
    generic(
        DEST_SYNC_FF    : integer;
        WIDTH           : integer
    );
    port (
        src_clk         : in    std_logic;
        src_send        : in    std_logic;
        src_in          : in    std_logic_vector(WIDTH-1 downto 0);

        dest_clk        : in    std_logic;
        dest_ack        : out   std_logic;
        dest_out        : out   std_logic_vector(WIDTH-1 downto 0)
    );
end component cdc_static_bus;

-------------------------------------------------------------------------------
--
--      Signals
--
-------------------------------------------------------------------------------

signal  Core_State              : std_logic_vector(3 downto 0)              := ST_IDLE;

signal  Traffic_Cnt_Latch_i     : std_logic                                 := '0';
signal  Start_Status            : std_logic                                 := '0';
signal  Start_Status_tog        : std_logic                                 := '0';
signal  Start_Status_tog_axi    : std_logic                                 := '0';
signal  Start_Status_tog_axi_d  : std_logic                                 := '0';
signal  Run_Status              : std_logic                                 := '0';
signal  Run_Conf                : std_logic                                 := '0';
signal  Run_Script              : std_logic                                 := '0';
signal  Status_Done             : std_logic                                 := '0';
signal  Conf_Done               : std_logic                                 := '0';
signal  Script_Done             : std_logic                                 := '0';
signal  Done_int                : std_logic                                 := '0';
signal  MAC_RX_reset_int        : std_logic                                 := '0';
signal  MAC_TX_reset_int        : std_logic                                 := '0';
signal  rst_allow_int           : std_logic                                 := '1';

signal  PLRD_Pipe               : PLRD_Pipe_Array_Type(1 to 6)              := (others => RST_PLRD_PIPE_TYPE);

signal  Conf_Ctr                : std_logic_vector(NUM_BIT_REG+2 downto 0)  := (others => '1');
signal  Conf_FIFO_Full          : std_logic                                 := '0';

signal  Script_Start            : std_logic_vector(0 to 3)                  := (others => '0');
signal  Start_Sel               : std_logic                                 := '0';
signal  Len_Check               : std_logic_vector(16 downto 0)             := (others => '0');

signal  Rd_Ptr_RAM              : std_logic_2d_16(0 to 15)                  := (others => (others => '0'));
signal  Rd_Ptr_Out              : std_logic_vector(15 downto 0)             := (others => '0');
signal  Addr_Sel                : std_logic_vector(15 downto 0)             := (others => '0');
signal  Length_Sel              : std_logic_vector(15 downto 0)             := (others => '0');
signal  Length_Zero             : std_logic                                 := '0';

signal  PLRD_FIFO               : PLRD_Pipe_Array_Type(0 to 15)             := (others => RST_PLRD_PIPE_TYPE);
signal  PLRD_FIFO_Rd_En         : std_logic                                 := '0';
signal  PLRD_FIFO_Rd_Ptr        : std_logic_vector(4 downto 0)              := (others => '1');
signal  PLRD_FIFO_Empty         : std_logic                                 := '0';
signal  PLRD_FIFO_Full          : std_logic                                 := '0';

signal  Chan_FIFO               : PLRD_Pipe_Array_Type(0 to 15)             := (others => RST_PLRD_PIPE_TYPE);
signal  Chan_FIFO_Rd_En         : std_logic                                 := '0';
signal  Chan_FIFO_Rd_Ptr        : std_logic_vector(4 downto 0)              := (others => '1');
signal  Chan_FIFO_Out           : PLRD_Pipe_Type                            := RST_PLRD_PIPE_TYPE;
signal  Chan_FIFO_Full          : std_logic                                 := '0';

signal  MD_FIFO_Empty           : std_logic_vector(0 to 3)                  := (others => '1');
signal  MD_FIFO_Rd_En           : std_logic_vector(0 to 3)                  := (others => '0');
signal  MD_FIFO_Out             : MR_Pipe_Array_Type(0 to 3)                := (others => RST_MR_PIPE_TYPE);

signal  MW_FIFO_Empty           : std_logic_vector(0 to 3)                  := (others => '1');
signal  MW_FIFO_Full            : std_logic_vector(0 to 3)                  := (others => '0');
signal  All_MW_FIFO_Empty       : std_logic                                 := '0';
signal  Empty_Cnt               : std_logic_vector(2 downto 0)              := (others => '0');

signal  Script_FIFO_Empty       : std_logic_vector(0 to 3)                  := (others => '1');
signal  All_Script_FIFO_Empty   : std_logic                                 := '1';
signal  Script_FIFO_Full        : std_logic_vector(0 to 3)                  := (others => '0');
signal  Script_FIFO_Full_Sel    : std_logic                                 := '0';

signal  Script_Cfg_Len          : std_logic_2d_16(0 to 3)                   := (others => (others => '0'));
signal  Script_Cfg_Base         : std_logic_2d_16(0 to 3)                   := (others => (others => '0'));

signal  Stat_Val                : std_logic                                 := '0';
signal  Stat_Ctr                : std_logic_vector(NUM_BIT_STAT downto 0)   := (others => '0');
signal  Stat_Rd_Addr            : std_logic_vector(11 downto 0)             := (others => '0');

signal  MR_FIFO_Empty           : std_logic_vector(0 to 3)                  := (others => '1');
signal  All_MR_FIFO_Empty       : std_logic                                 := '0';
signal  MR_FIFO_Full            : std_logic_vector(0 to 3)                  := (others => '0');
signal  Any_MR_FIFO_Full        : std_logic                                 := '0';

signal  Status_Done_En          : std_logic                                 := '0';
signal  Status_Done_Ctr         : std_logic_vector(4 downto 0)              := (others => '0');

signal  PLWR_Pipe               : PLWR_Pipe_Array_Type(1 to 5)              := (others => RST_PLWR_PIPE_TYPE);

signal  Scan_Chan               : std_logic_vector(1 downto 0)              := (others => '0');
signal  Scratch                 : std_logic                                 := '0';
signal  CU_Start                : std_logic                                 := '0';
signal  CU_Already_Started      : std_logic                                 := '0';
signal  Rst_Detect              : std_logic_vector(2 downto 0)              := (others => '0');
signal  Read_Pending            : std_logic                                 := '0';
signal  Done_RW                 : std_logic                                 := '0';


signal  PLWR_FIFO               : PLWR_Pipe_Array_Type(0 to 15)             := (others => RST_PLWR_PIPE_TYPE);
signal  PLWR_FIFO_Rd_En         : std_logic                                 := '0';
signal  PLWR_FIFO_Rd_Ptr        : std_logic_vector(4 downto 0)              := (others => '1');
signal  PLWR_FIFO_Empty         : std_logic                                 := '0';
signal  PLWR_FIFO_Full          : std_logic                                 := '0';

signal  TCM_pipe                : TCM_pipe_Array_Type(0 to 5)               := (others => RST_TCM_pipe_TYPE);
signal  TCM_Cnt_Sel             : std_logic_vector(5 downto 0)              := (others => '0');
signal  TCM_Cnt_RAM             : std_logic_2d_32(0 to 63)                  := (others => (others => '0'));
signal  TCM_Cnt_RAM_Out          : std_logic_vector(31 downto 0)            := (others => '0');

-- 2D Signals which Vivado does not support for VHDL/Verilog Interfaces

signal  MAC_awaddr              : std_logic_2d_32(0 to 3)                   := (others => (others => '0'));
signal  MAC_wdata               : std_logic_2d_32(0 to 3)                   := (others => (others => '0'));
signal  MAC_wstrb               : std_logic_2d_4(0 to 3)                    := (others => (others => '0'));
signal  MAC_araddr              : std_logic_2d_32(0 to 3)                   := (others => (others => '0'));
signal  MAC_rdata               : std_logic_2d_32(0 to 3)                   := (others => (others => '0'));
signal  Dest_MAC_Addr           : std_logic_2d_48(0 to 3)                   := (others => (others => '0'));
signal  Source_MAC_Addr         : std_logic_2d_48(0 to 3)                   := (others => (others => '0'));
signal  MAC_Ethertype           : std_logic_2d_16(0 to 3)                   := (others => (others => '0'));
signal  MAC_Traffic_Type        : std_logic_2d_2(0 to 3)                    := (others => (others => '0'));
signal  Gen_Length              : std_logic_2d_14(0 to 3)                   := (others => (others => '0'));

signal  GT_Tx_Post_Cursor       : std_logic_2d_5(0 to 3)                    := (others => (others => '0'));
signal  GT_Tx_Main_Cursor       : std_logic_2d_7(0 to 3)                    := (others => (others => '0'));
signal  GT_Tx_Pre_Cursor        : std_logic_2d_5(0 to 3)                    := (others => (others => '0'));
signal  GT_Tx_Diff_Ctrl         : std_logic_2d_5(0 to 3)                    := (others => (others => '0'));


signal rdata_bi                 : std_logic_vector(31 downto 0)             := (others => '0');
signal rdata_ctrl_status        : std_logic_vector(31 downto 0)             := (others => '0');
signal rst_300                  : std_logic;

signal timer_rst                : std_logic;
signal timer_end                : std_logic;
signal status                   : std_logic_vector(31 downto 0)             := (others => '0');
signal PLWR_Cmd                 : std_logic                                 := '0';


signal MAC_wvalid_i             : std_logic_vector(0 to 3)                  := (others => '0');
signal MAC_awvalid_i            : std_logic_vector(0 to 3)                  := (others => '0');
signal MAC_arvalid_i            : std_logic_vector(0 to 3)                  := (others => '0');

signal MAC_Sel_Mode_ap_clk      : std_logic_vector(MAC_Sel_Mode'range)      := (others => '0');

signal watchdog_alarm_cdc       : std_logic                                 := '0';

-- synthesis translate_off


-- synthesis translate_on

begin

u_cdc_sel_mode: xpm_cdc_array_single
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => MAC_Sel_Mode'length,
        SRC_INPUT_REG  => 0
    )
    port map (
        src_clk         => axi_clk ,
        src_in          => MAC_Sel_Mode,

        dest_clk        => clk_300,
        dest_out        => MAC_Sel_Mode_ap_clk
    );

u_build_info: component build_info_v4_0
    generic map(
        C_MAJOR_VERSION     => C_MAJOR_VERSION,
        C_MINOR_VERSION     => C_MINOR_VERSION,
        C_BUILD_VERSION     => C_BUILD_VERSION,
        C_BLOCK_ID          => C_BLOCK_ID
    )
    port map (
        clk                 => clk_300,
        rst                 => rst_300_raw,

        Info_1              => (others => '0'), -- reserved for future use
        Info_2              => conv_std_logic_vector(C_GT_INDEX*(2**8),16), -- shift left by 8 bits, prior converting to vector
        Info_3              => conv_std_logic_vector(C_GT_TYPE + C_GT_NUM_GT*(2**2) + C_GT_NUM_LANE*(2**4) + C_GT_RATE*(2**7) + C_GT_MAC_ENABLE_RSFEC*(2**9),16), -- shift by 4 bits NUM_LANE, shit 7bits Rate
        Info_4              => (others => '0'),
        Info_5              => (others => '0'),
        Info_6              => (others => '0'),
        Info_7              => EXT(conv_std_logic_vector(C_CLOCK0_FREQ,16),16),
        Info_8              => EXT(conv_std_logic_vector(C_CLOCK1_FREQ,16),16),

        We                  => PLWR_Pipe(1).Cmd_Wr,
        Addr                => Scalar00(6 downto 4),
        Data_In             => PLWR_Pipe(1).Cmd_Da,
        Data_Out            => rdata_bi
    );

    rst_300 <= rst_300_raw or rst_300_gate;

    status <= ext(MAC_Sel_Mode_ap_clk & Mac_Rst_Status, status'length);

    u_common_ctrl_status: component common_ctrl_status
    generic map(
        C_CLOCK_FREQ            => C_CLOCK0_FREQ,
        DEST_SYNC_FF            => DEST_SYNC_FF,
        C_CLK_TROTTLE_DETECT_EN => 0,
        C_WATCHDOG_ENABLE       => 0,
        C_EXT_TOGGLE_1_SEC      => 0  -- 1: use toggle_1_sec input, 0: generate internally
    )
    port map (
        ap_clk              => clk_300,
        ap_clk_cont         => clk_300_cont,
        ap_rst              => rst_300,

        ap_clk_2            => '0',
        ap_clk_2_cont       => '0',
        ap_rst_2            => '0',

        toggle_1sec         => '0',

        rst_watchdog        => '0',
        watchdog_alarm      => open,

        We                  => PLWR_Pipe(1).Cmd_Wr,
        Addr                => Scalar00(6 downto 4),
        Data_In             => PLWR_Pipe(1).Cmd_Da,
        User_Status_1       => status,
        Data_Out            => rdata_ctrl_status
    );

    -- 10ms timer, it requires C_CLOCK0_FREQ * 10.000 clock cycles at C_CLOCK0_FREQ MHz
    u_timer: component timer
    generic map (
        C_USER_TIMER_EN     => 1,
        C_TIMER_RELOAD      => C_CLOCK0_FREQ * 10000 - 1,
        DEST_SYNC_FF        => DEST_SYNC_FF,
        C_SIM_DIVIDER       => SIM_DIVIDER
    )
    port map (
        clk_1           => clk_300_cont,
        clk_2           => clk_300,

        toggle_1_sec_1  => open,
        toggle_1_sec_2  => open,

        timer_rst       => timer_rst,
        timer_end_1     => open,
        timer_end_2     => timer_end
    );

    MAC_rdata   <= (MAC_rdata_0, MAC_rdata_1, MAC_rdata_2, MAC_rdata_3);


process(clk_300)
begin
    if (rising_edge(clk_300)) then
        TCM_Cnt_RAM_Out <= TCM_Cnt_RAM(conv_integer(Scalar00(9 downto 4)));
    end if;
end process;

u_cdc_watchdog: xpm_cdc_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF,
        SRC_INPUT_REG  => 0
    )
    port map (
        src_clk              => clk_300,
        src_in               => watchdog_alarm_in,
        dest_clk             => clk_300,
        dest_out             => watchdog_alarm_cdc
    );


Cnt_RAM_Rd_Addr_0 <= TCM_Cnt_Sel(5 downto 2);
Cnt_RAM_Rd_Addr_1 <= TCM_Cnt_Sel(5 downto 2);
Cnt_RAM_Rd_Addr_2 <= TCM_Cnt_Sel(5 downto 2);
Cnt_RAM_Rd_Addr_3 <= TCM_Cnt_Sel(5 downto 2);


TCM: -- Traffic Counter Memory
    -- from mutiple arrays of counters, gradually select 1 word of 32b and store it
process(clk_300)
    variable Cnt_64_var : std_logic_vector(63 downto 0);
    variable Cnt_32_var : std_logic_vector(31 downto 0);
begin

    if (rising_edge(clk_300)) then

        TCM_pipe(1 to TCM_pipe'HIGH)    <= TCM_pipe(0 to TCM_pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 1 : select counter type
        ---------------------------------------------------
        TCM_Cnt_Sel <= TCM_Cnt_Sel + 1;
        TCM_pipe(1).Addr <= TCM_Cnt_Sel;

        ---------------------------------------------------
        -- Stage 2-3 : wait for data to come back
        ---------------------------------------------------
        ---------------------------------------------------
        -- Stage 4 : select lane
        ---------------------------------------------------

        case TCM_pipe(3).Addr(1 downto 0) is
            when "00"   => TCM_pipe(4).Cnt <= Cnt_RAM_Data_0;
            when "01"   => TCM_pipe(4).Cnt <= Cnt_RAM_Data_1;
            when "10"   => TCM_pipe(4).Cnt <= Cnt_RAM_Data_2;
            when others => TCM_pipe(4).Cnt <= Cnt_RAM_Data_3;
        end case;

        ---------------------------------------------------
        -- Stage 5 : store into RAM
        ---------------------------------------------------
        TCM_Cnt_RAM(conv_integer(TCM_pipe(4).Addr)) <= TCM_pipe(4).Cnt;

    end if;
end process;


-------------------------------------------------------------------------------
--
--      Control State Machine
--
-------------------------------------------------------------------------------

SQ:
process(clk_300)
begin

    if (rising_edge(clk_300)) then

        All_MW_FIFO_Empty   <= and_reduce(MW_FIFO_Empty) and PLRD_FIFO_Empty;

        -- Ensure All FIFOs have been empty for 4 cycles

        if (All_MW_FIFO_Empty = '0') then

            Empty_Cnt   <= (others => '0');

        elsif (Empty_Cnt(Empty_Cnt'HIGH) = '0') then

            Empty_Cnt   <= Empty_Cnt + 1;

        end if;

        -- Implement the PLRAM Read Sequence State Machine

        Start_Status    <= '0';
        Run_Status      <= '0';
        Run_Conf        <= '0';
        Run_Script      <= '0';
        Done_int        <= '0';
        MAC_aresetn     <= '1';
        MAC_RX_reset_int   <= '0';
        MAC_TX_reset_int   <= '0';
        timer_rst <= '0';

        case Core_State is

            when ST_IDLE =>

                if ((Scalar00(19) = '1') and (Start = '1')) then

                    Core_State  <= ST_START;

                    rst_allow_int   <= Scalar00(20);

                end if;

            when ST_START =>

                -- Check if TX or a TX reset has been requested

                if (Scalar00(21) = '1') then
                    -- initiate TX reset procedure
                    Core_State          <= ST_WAIT_TX_RST;
                    MAC_TX_reset_int    <= '1';
                    timer_rst           <= '1';

                elsif (Scalar00(22) = '1') then
                        -- initiate RX reset procedure
                        Core_State          <= ST_WAIT_RX_RST;
                        MAC_RX_reset_int    <= '1';
                        timer_rst           <= '1';
                else
                    -- Check if a Status Read has been requested
                    if (Scalar00(16) = '1') then

                        Start_Status        <= '1';
                        Start_Status_tog    <= not Start_Status_tog;
                        Traffic_Cnt_Latch_i <= not Traffic_Cnt_Latch_i;
                        Core_State          <= ST_GET_STATUS;

                    else    -- Check Next Option

                        Core_State  <= ST_GET_STATUS_DONE;

                    end if;

                end if;

            when ST_WAIT_TX_RST =>

                if (timer_end = '1') then

                    -- Check if a RX reset has been requested
                    if (Scalar00(22) = '1') then

                        Core_State          <= ST_WAIT_RX_RST;
                        MAC_RX_reset_int    <= '1';
                        timer_rst           <= '1';

                    else

                        Core_State  <= ST_SEND_CONF_DONE;

                    end if;

                end if;

            when ST_WAIT_RX_RST =>

                if (timer_end = '1') then

                    Core_State  <= ST_SEND_CONF_DONE;

                end if;

            when ST_GET_STATUS =>

                Run_Status  <= '1';

                if (Status_Done = '1') then

                    Core_State    <= ST_GET_STATUS_DONE;

                end if;

            when ST_GET_STATUS_DONE =>

                -- Check if a configuration update has been requested

                if (Scalar00(17) ='1') then

                    Core_State    <= ST_SEND_CONF;

                else

                    Core_State    <= ST_SEND_CONF_DONE;

                end if;

            when ST_SEND_CONF =>

                -- Send a new configuration

                Run_Conf    <= '1';

                -- Conf_Done means all the addresses have been requested

                if (Conf_Done = '1') then

                    Core_State          <= ST_SEND_CONF_WAIT;

                end if;

            when ST_SEND_CONF_WAIT =>

                -- Wait for all the AXI Transactions to complete

                if (Empty_Cnt(Empty_Cnt'HIGH) = '1') then

                    Core_State  <= ST_SEND_CONF_DONE;

                end if;

            when ST_SEND_CONF_DONE =>

                -- Signal that the kernel has finished

                Done_int  <= '1';

                -- If we want to start running the script jump to its state

                if (Scalar00(18) ='1') then

                    Core_State  <= ST_RUN_SCRIPT;

                else    -- Otherwise just jump back to Idle

                    Core_State  <= ST_IDLE;

                end if;

            when ST_RUN_SCRIPT =>

                Run_Script  <= '1';

                -- Stop the Script when a new Start is received

                if ( ((Scalar00(19) = '1') and (Start = '1')) or watchdog_alarm_cdc = '1') then

                    Core_State    <= ST_STOP_SCRIPT;

                    if (watchdog_alarm_cdc /= '1')  then
                        rst_allow_int   <= Scalar00(20);
                    end if;

                end if;

            when ST_STOP_SCRIPT =>

                -- Wait for all the scripts to finish, and jump back to Idle

                if (Script_Done = '1') then

                    Core_State  <= ST_START;

                end if;

            when others =>

        end case;

        if (Done_RW = '1') then

            Done_int    <= '1';

        end if;

    end if;

end process;

u_cdc_status: xpm_cdc_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF,
        SRC_INPUT_REG  => 0
    )
    port map (
        src_clk              => clk_300,
        src_in               => Start_Status_tog,
        dest_clk             => axi_clk,
        dest_out             => Start_Status_tog_axi
    );

    SS:
    process(axi_clk)
    variable Start_Status_var : std_logic;
    begin
        if (rising_edge(axi_clk)) then
            Start_Status_tog_axi_d <= Start_Status_tog_axi;

            Start_Status_var := Start_Status_tog_axi_d xor Start_Status_tog_axi;
            MAC_PM_Tick     <= (others => Start_Status_var);

        end if;
    end process;

    rst_allow       <= '1'; -- rst_allow_int;
    Done            <= Done_int;

    MAC_rx_reset    <= (others => MAC_RX_reset_int);
    MAC_tx_reset    <= (others => MAC_TX_reset_int);

    Traffic_Cnt_Latch  <= Traffic_Cnt_Latch_i;
-------------------------------------------------------------------------------
--
--      PLRAM Read Logic
--
-------------------------------------------------------------------------------

    PLRD_FIFO_Rd_En <= '1' when ((PLRD_FIFO_Rd_Ptr(PLRD_FIFO_Rd_Ptr'HIGH) = '0') and (PLRD_Pipe(6).Valid = '0')) else '0';
    Chan_FIFO_Rd_En <= PLRAM_rvalid;

RQ:
process(clk_300)
begin

    if (rising_edge(clk_300)) then

        PLRD_Pipe   <= RST_PLRD_PIPE_TYPE & PLRD_Pipe(1 to PLRD_Pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 1
        ---------------------------------------------------

        -- Translate the Scan into reads for the 4 channels

        if (Run_Script = '1') then

            -- Process the script every other cycle

            if ((PLRD_Pipe(1).Scr_Val = '0') and (PLRD_FIFO_Full = '0') and (Chan_FIFO_Full = '0')) then

                PLRD_Pipe(1).Scr_Val    <= '1';
                PLRD_Pipe(1).Chan       <= PLRD_Pipe(1).Chan + 1;

            else

                PLRD_Pipe(1).Scr_Val    <= '0';
                PLRD_Pipe(1).Chan       <= PLRD_Pipe(1).Chan;

            end if;

        else

            Script_Start    <= (others => '1');

        end if;

        All_Script_FIFO_Empty   <= and_reduce(Script_FIFO_Empty);

        Script_Done <= PLRD_FIFO_Empty and All_Script_FIFO_Empty;

        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------

        Rd_Ptr_Out  <= Rd_Ptr_RAM(conv_integer(PLRD_Pipe(1).Chan));
        Addr_Sel    <= Script_Cfg_Base(conv_integer(PLRD_Pipe(1).Chan));

        ---------------------------------------------------
        -- Stage 3
        ---------------------------------------------------

        PLRD_Pipe(3).Rd_Ptr     <= Rd_Ptr_Out + 1;

        PLRD_Pipe(3).Rd_Addr    <= EXT(Addr_Sel & "00", PLRD_Pipe(3).Rd_Addr'LENGTH) + EXT(Rd_Ptr_Out & "00", PLRD_Pipe(3).Rd_Addr'LENGTH);

        Length_Sel              <= Script_Cfg_Len(conv_integer(PLRD_Pipe(2).Chan));
        Script_FIFO_Full_Sel    <= Script_FIFO_Full(conv_integer(PLRD_Pipe(2).Chan));
        Start_Sel               <= Script_Start(conv_integer(PLRD_Pipe(2).Chan));

        Script_Start(conv_integer(PLRD_Pipe(2).Chan))   <= '0';

        ---------------------------------------------------
        -- Stage 4
        ---------------------------------------------------

        -- Keep the MW_FIFOs at half full
        -- Suppress writes when the Full flag is asserted

        if (Script_FIFO_Full_Sel = '1') then

            PLRD_Pipe(4).Scr_Val    <= '0';

        end if;

        -- Check if we have got to the end of the script

        Len_Check   <= ("0" & PLRD_Pipe(3).Rd_Ptr) - ("0" & Length_Sel);

        -- During startup, force the counters back to 0, suppress memory reads

        if (Start_Sel = '1') then

            PLRD_Pipe(4).Scr_Val    <= PLRD_Pipe(3).Scr_Val;
            Len_Check               <= (others => '0');

        end if;

        Length_Zero <= nor_reduce(Length_Sel);

        -- During Configuration, Generate the pipeline here based on the Sequence Counter

        Conf_FIFO_Full  <= PLRD_FIFO_Full or Chan_FIFO_Full or or_reduce(MW_FIFO_Full);

        if (Run_Conf = '1') then

            if ((Conf_FIFO_Full = '0') and (Conf_Ctr(Conf_Ctr'HIGH) = '0')) then

                Conf_Ctr <= Conf_Ctr + 1;

                if ((signed(Conf_Ctr) >= 0) and (signed(Conf_Ctr) < (NUM_REG * 4))) then

                    PLRD_Pipe(4).Conf_Val   <= '1';

                end if;

            end if;

            PLRD_Pipe(4).Chan       <= Conf_Ctr(1 downto 0);
            PLRD_Pipe(4).Reg        <= Conf_Ctr(Conf_Ctr'HIGH-1 downto 2);
            PLRD_Pipe(4).Rd_Addr    <= EXT(Conf_Ctr(1 downto 0) & Conf_Ctr(Conf_Ctr'HIGH-1 downto 2) & "00", PLRD_Pipe(4).Rd_Addr'LENGTH) + CONF_BASE;

        else

            Conf_Ctr    <= (others => '0');

        end if;

        ---------------------------------------------------
        -- Stage 5
        ---------------------------------------------------

        if (Len_Check(Len_Check'HIGH) = '0') then

            PLRD_Pipe(5).Rd_Ptr <= (others => '0');

        end if;

        -- Setting Length = 0 disables script execution

        if (PLRD_Pipe(4).Scr_Val = '1') then

            PLRD_Pipe(5).Valid  <= not Length_Zero;

        end if;

        if (PLRD_Pipe(4).Conf_Val = '1') then

            PLRD_Pipe(5).Valid  <= '1';

        end if;

        -- Apply the PLRAM Offset for this Kernel Instance

        PLRD_Pipe(5).Rd_Addr    <= PLRD_Pipe(4).Rd_Addr + (AXI00_Ptr0(31 downto 2) & "00");

        ---------------------------------------------------
        -- Stage 6
        ---------------------------------------------------

        if (PLRD_Pipe(5).Scr_Val = '1') then

            Rd_Ptr_RAM(conv_integer(PLRD_Pipe(5).Chan)) <= PLRD_Pipe(5).Rd_Ptr;

        end if;

        -- FIFO the Read Requests to the PLRAM AXI Bus

        if (PLRD_Pipe(5).Valid = '1') then

            PLRD_FIFO   <= PLRD_Pipe(5) & PLRD_FIFO(0 to PLRD_FIFO'HIGH-1);

            if (PLRD_FIFO_Rd_En = '0') then

                PLRD_FIFO_Rd_Ptr    <= PLRD_FIFO_Rd_Ptr + 1;

                assert (signed(PLRD_FIFO_Rd_Ptr) /= PLRD_FIFO'HIGH)
                    report "PLRD_FIFO Overflow in gt_mac_config"
                    severity note;

            end if;

        elsif (PLRD_FIFO_Rd_En = '1') then

            PLRD_FIFO_Rd_Ptr    <= PLRD_FIFO_Rd_Ptr - 1;

        end if;

        PLRD_Pipe(6)    <= PLRD_Pipe(6);

        if (PLRAM_arready = '1') then

            PLRD_Pipe(6).Valid  <= '0';

        end if;

        if (PLRD_FIFO_Rd_En = '1') then

            PLRD_Pipe(6)        <= PLRD_FIFO(conv_integer(PLRD_FIFO_Rd_Ptr(PLRD_FIFO_Rd_Ptr'HIGH-1 downto 0)));
            PLRD_Pipe(6).Valid  <= '1';

        end if;

        if (signed(PLRD_FIFO_Rd_Ptr) >= (PLRD_FIFO'LENGTH / 2)) then

            PLRD_FIFO_Full  <= '1';

        else

            PLRD_FIFO_Full  <= '0';

        end if;

        if (Rst_300_Gate = '1') then

            PLRD_FIFO_Rd_Ptr    <= (others => '1');

        end if;

        -- FIFO the Channel indicators, and correlate between Read Address and Read Data busses

        if (PLRD_Pipe(5).Valid = '1') then

            Chan_FIFO   <= PLRD_Pipe(5) & Chan_FIFO(0 to Chan_FIFO'HIGH-1);

            if (Chan_FIFO_Rd_En = '0') then

                Chan_FIFO_Rd_Ptr    <= Chan_FIFO_Rd_Ptr + 1;

                assert (signed(Chan_FIFO_Rd_Ptr) /= Chan_FIFO'HIGH)
                    report "Chan_FIFO Overflow in gt_mac_config"
                    severity note;

            end if;

        elsif (Chan_FIFO_Rd_En = '1') then

            Chan_FIFO_Rd_Ptr    <= Chan_FIFO_Rd_Ptr - 1;

        end if;

        Chan_FIFO_Out   <= Chan_FIFO(conv_integer(Chan_FIFO_Rd_Ptr(Chan_FIFO_Rd_Ptr'HIGH-1 downto 0)));

        if (signed(Chan_FIFO_Rd_Ptr) >= (Chan_FIFO'LENGTH / 2)) then

            Chan_FIFO_Full  <= '1';

        else

            Chan_FIFO_Full  <= '0';

        end if;

        if (Rst_300_Gate = '1') then

            Chan_FIFO_Rd_Ptr    <= (others => '1');

        end if;

    end if;

end process;

    PLRAM_rready    <= '1';

    PLRAM_arvalid   <= PLRD_Pipe(6).Valid;
    PLRAM_araddr    <= AXI00_Ptr0(63 downto 32) & PLRD_Pipe(6).Rd_Addr;
    PLRAM_arlen     <= EXT("0", PLRAM_arlen'LENGTH);

    PLRD_FIFO_Empty <= PLRD_FIFO_Rd_Ptr(PLRD_FIFO_Rd_Ptr'HIGH);

    Conf_Done       <= Conf_Ctr(Conf_Ctr'HIGH);

-------------------------------------------------------------------------------
--
--      MAC Common Read Logic
--
-------------------------------------------------------------------------------

MQ:
process(clk_300)
begin

    if (rising_edge(clk_300)) then

        Stat_Val        <= '0';
        Stat_Rd_Addr    <= (others => '-');

        -- Generate the read addresses of all the Status Registers of interest

        Any_MR_FIFO_Full    <= or_reduce(MR_FIFO_Full);
        All_MR_FIFO_Empty   <= and_reduce(MR_FIFO_Empty);

        if (Run_Status = '1') then

            if ((Any_MR_FIFO_Full = '0') and (Stat_Ctr(Stat_Ctr'HIGH) = '0')) then

                Stat_Ctr <= Stat_Ctr + 1;

                if ((signed(Stat_Ctr) >= 0) and (signed(Stat_Ctr) < NUM_STAT_REGS)) then

                    Stat_Val        <= '1';
                    Stat_Rd_Addr    <= EXT(MAC_STATUS_REGS(conv_integer(Stat_Ctr(Stat_Ctr'HIGH-1 downto 0))), Stat_Rd_Addr'LENGTH);

                end if;

            end if;

        else

            Stat_Ctr    <= (others => '0');

        end if;

        Status_Done_En  <= Stat_Ctr(Stat_Ctr'HIGH) and All_MR_FIFO_Empty and PLWR_FIFO_Empty;

        Status_Done_Ctr <= Status_Done_Ctr + EXT(not Status_Done_Ctr(Status_Done_Ctr'HIGH downto Status_Done_Ctr'HIGH), Status_Done_Ctr'LENGTH);

        if (Status_Done_En = '0') then

            Status_Done_Ctr <= (others => '0');

        end if;

    end if;

end process;

    Status_Done <= Status_Done_Ctr(Status_Done_Ctr'HIGH);

-------------------------------------------------------------------------------
--
--      PLRAM Write Logic
--
-------------------------------------------------------------------------------

    PLWR_FIFO_Rd_En <= '1' when ((PLWR_FIFO_Rd_Ptr(PLWR_FIFO_Rd_Ptr'HIGH) = '0') and (PLWR_Pipe(5).Valid_A = '0') and (PLWR_Pipe(5).Valid_D = '0')) else '0';

WQ:
process(clk_300)
begin

    if (rising_edge(clk_300)) then

        PLWR_Pipe   <= RST_PLWR_PIPE_TYPE & PLWR_Pipe(1 to PLWR_Pipe'HIGH-1);

        Done_RW <= '0';

        ---------------------------------------------------
        -- Stage 1
        ---------------------------------------------------

        PLWR_Pipe(1).Valid  <= '0';
        PLWR_Pipe(1).Chan   <= Scan_Chan;

        if ((MD_FIFO_Empty(conv_integer(Scan_Chan)) = '0') and (PLWR_FIFO_Full = '0')) then

            PLWR_Pipe(1).Valid  <= '1';
            Scan_Chan           <= Scan_Chan + 1;

        else

            Scan_Chan           <= Scan_Chan + 1;

        end if;

        -- Generate a 1 cycle BuildInfo read strobe when requested by the host application

        PLWR_Pipe(1).Cmd_Rd <= '0';
        PLWR_Pipe(1).Cmd_Wr <= '0';
        PLWR_Pipe(1).Cmd_Ad <= Scalar00;
        PLWR_Pipe(1).Cmd_Da <= (others => '-');

        -- lower nibble use to select command type
        if (Scalar00(0) = '1') then

            PLWR_Pipe(1).Cmd_Rd <= '1';

        else    -- Handle Writes here as well

            PLWR_Pipe(1).Cmd_Wr <= '1';
            PLWR_Pipe(1).Cmd_Da <= Scalar01;

        end if;

        if ((Scalar00(19) = '1') or (Scalar00(3 downto 1) /= "000") or (Start = '0')) then

            PLWR_Pipe(1).Cmd_Rd <= '0';
            PLWR_Pipe(1).Cmd_Wr <= '0';

        end if;

        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------
        PLWR_Cmd <= PLWR_Pipe(1).Cmd_Wr or PLWR_Pipe(1).Cmd_Rd;

        MD_FIFO_Rd_En                                   <= (others => '0');
        MD_FIFO_Rd_En(conv_integer(PLWR_Pipe(1).Chan))  <= PLWR_Pipe(1).Valid;

        -- lower nibble of Cmd_Ad is the command type
        --  address decoding only starts with Scalar00(4)
        if PLWR_Pipe(1).Cmd_Ad(10) = '0' then  -- 0x40
            -- address < 0x40
            if PLWR_Pipe(1).Cmd_Ad(8) = '0' then
                -- address < 0x10
                PLWR_Pipe(2).Data <= rdata_bi;
            else
                -- address > 0x10
                PLWR_Pipe(2).Data <= rdata_ctrl_status;
            end if;
        else
            -- address > 0x40
            PLWR_Pipe(2).Data <= TCM_Cnt_RAM_Out;
        end if;


        PLWR_Pipe(2).Wr_Addr    <= AXI00_Ptr0(31 downto 0);

        if (PLWR_Pipe(1).Cmd_Wr = '1') then

            Done_RW <= '1';

        end if;

        if (rst_300_gate = '1') then

            Rst_Detect(1)   <= '0';

        end if;

        if (rst_300_raw = '1') then

            Rst_Detect(0)   <= '0';

        end if;

        ---------------------------------------------------
        -- Stage 4
        ---------------------------------------------------

        if (PLWR_Pipe(3).Cmd_Rd = '1') then

            PLWR_Pipe(4).Valid      <= '1';

            assert (PLWR_Pipe(3).Valid = '0')
                report "Simultaneous Buildinfo read and Status read in gt_mac_config"
                severity note;

        else

            PLWR_Pipe(4).Data       <= MD_FIFO_Out(conv_integer(PLWR_Pipe(3).Chan)).Data;
            PLWR_Pipe(4).Wr_Addr    <= EXT(MD_FIFO_Out(conv_integer(PLWR_Pipe(3).Chan)).Wr_Addr, PLWR_Pipe(4).Wr_Addr'LENGTH);

        end if;

        ---------------------------------------------------
        -- Stage 5
        ---------------------------------------------------

        -- FIFO the Read Requests to the MAC Write AXI Bus

        if (PLWR_Pipe(4).Valid = '1') then

            PLWR_FIFO <= PLWR_Pipe(4) & PLWR_FIFO(0 to PLWR_FIFO'HIGH-1);

            if (PLWR_FIFO_Rd_En = '0') then

                PLWR_FIFO_Rd_Ptr  <= PLWR_FIFO_Rd_Ptr + 1;

                assert (signed(PLWR_FIFO_Rd_Ptr) /= PLWR_FIFO'HIGH)
                    report "PLWR_FIFO Overflow in gt_mac_config"
                    severity note;

            end if;

        elsif (PLWR_FIFO_Rd_En = '1') then

            PLWR_FIFO_Rd_Ptr  <= PLWR_FIFO_Rd_Ptr - 1;

        end if;

        PLWR_Pipe(5)  <= PLWR_Pipe(5);

        if (PLRAM_awready = '1') then

            PLWR_Pipe(5).Valid_A  <= '0';

        end if;

        if (PLRAM_wready = '1') then

            PLWR_Pipe(5).Valid_D  <= '0';

        end if;

        if (PLWR_FIFO_Rd_En = '1') then

            PLWR_Pipe(5)          <= PLWR_FIFO(conv_integer(PLWR_FIFO_Rd_Ptr(PLWR_FIFO_Rd_Ptr'HIGH-1 downto 0)));
            PLWR_Pipe(5).Valid_A  <= '1';
            PLWR_Pipe(5).Valid_D  <= '1';

        end if;

        if (signed(PLWR_FIFO_Rd_Ptr) >= (PLWR_FIFO'LENGTH / 2)) then

            PLWR_FIFO_Full  <= '1';

        else

            PLWR_FIFO_Full  <= '0';

        end if;

        if (Rst_300_Gate = '1') then

            PLWR_FIFO_Rd_Ptr    <= (others => '1');

        end if;

        -- Wait until the PLRAM write is acknowledged before sending Done on read requests

        if ((PLWR_Pipe(5).Valid_A = '1') and (PLWR_Pipe(5).Cmd_Rd = '1')) then

            Read_Pending    <= '1';

        end if;

        if ((Read_Pending = '1') and (PLRAM_bvalid = '1')) then

            Read_Pending    <= '0';
            Done_RW         <= '1';

        end if;

    end if;

end process;

    PLWR_FIFO_Empty <= PLWR_FIFO_Rd_Ptr(PLWR_FIFO_Rd_Ptr'HIGH);

    PLRAM_awvalid   <= PLWR_Pipe(5).Valid_A;
    PLRAM_awaddr    <= AXI00_Ptr0(63 downto 32) & PLWR_Pipe(5).Wr_Addr;
    PLRAM_awlen     <= EXT("0", PLRAM_awlen'LENGTH);

    PLRAM_wvalid    <= PLWR_Pipe(5).Valid_D;
    PLRAM_wdata     <= PLWR_Pipe(5).Data;
    PLRAM_wstrb     <= "1111";
    PLRAM_wlast     <= '1';

    PLRAM_bready    <= '1';

-------------------------------------------------------------------------------
--
--      Per MAC Instance Logic
--
-------------------------------------------------------------------------------

G1:
for n in 0 to C_GT_NUM_LANE-1 generate

signal  MR_Pipe                 : MR_Pipe_Array_Type(0 to 3)                := (others => RST_MR_PIPE_TYPE);

signal  MR_FIFO                 : MR_Pipe_Array_Type(0 to 15)               := (others => RST_MR_PIPE_TYPE);
signal  MR_FIFO_Rd_En           : std_logic                                 := '0';
signal  MR_FIFO_Rd_Ptr          : std_logic_vector(4 downto 0)              := (others => '1');
signal  MR_FIFO_Full_int        : std_logic                                 := '0';

signal  MD_FIFO                 : MR_Pipe_Array_Type(0 to 15)               := (others => RST_MR_PIPE_TYPE);
signal  MD_FIFO_Rd_Ptr          : std_logic_vector(4 downto 0)              := (others => '1');
signal  MD_FIFO_Full            : std_logic                                 := '0';

signal  MW_Pipe                 : MW_Pipe_Array_Type(0 to 3)                := (others => RST_MW_PIPE_TYPE);

signal  MW_FIFO                 : MW_Pipe_Array_Type(0 to 15)               := (others => RST_MW_PIPE_TYPE);
signal  MW_FIFO_Rd_En           : std_logic                                 := '0';
signal  MW_FIFO_Rd_Ptr          : std_logic_vector(4 downto 0)              := (others => '1');
signal  MW_FIFO_Full_int        : std_logic                                 := '0';

signal  MR_Wr_Offs              : std_logic_vector(NUM_BIT_STAT-1 downto 0) := (others => '0');

signal  Script_FIFO             : MW_Pipe_Array_Type(0 to 15)               := (others => RST_MW_PIPE_TYPE);
signal  Script_FIFO_Out         : MW_Pipe_Type                              := RST_MW_PIPE_TYPE;
signal  Script_FIFO_Rd_En       : std_logic                                 := '0';
signal  Script_FIFO_Rd_Ptr      : std_logic_vector(4 downto 0)              := (others => '1');
signal  Script_FIFO_Full_int    : std_logic                                 := '0';

signal  Script_Cnt              : std_logic_vector(21 downto 0)             := (others => '1');

signal  Mode_Change_int         : std_logic                                 := '0';

signal  Script_Cfg_Len_int      : std_logic_vector(15 downto 0)             := (others => '0');
signal  Script_Cfg_Base_int     : std_logic_vector(15 downto 0)             := (others => '0');
signal  Config_Upd_int          : std_logic                                 := '0';
signal  Dest_MAC_Addr_int       : std_logic_vector(47 downto 0)             := (others => '0');
signal  Source_MAC_Addr_int     : std_logic_vector(47 downto 0)             := (others => '0');
signal  MAC_Ethertype_int       : std_logic_vector(15 downto 0)             := (others => '0');
signal  MAC_Traffic_Type_int    : std_logic_vector(1 downto 0)              := (others => '0');

signal  GT_Rx_LPM_En_int        : std_logic                                 := DEF_RX_LPM_EN;
signal  GT_Tx_Main_Cursor_int   : std_logic_vector(6 downto 0)              := DEF_TX_MAIN_CURSOR;
signal  GT_Tx_Post_Cursor_int   : std_logic_vector(4 downto 0)              := DEF_TX_POST_CURSOR;
signal  GT_Tx_Pre_Cursor_int    : std_logic_vector(4 downto 0)              := DEF_TX_PRE_CURSOR;
signal  GT_Tx_Diff_Ctrl_int     : std_logic_vector(4 downto 0)              := DEF_TX_DIFF_CTRL;
signal  GT_TX_Polarity_int      : std_logic                                 := DEF_TX_POLARITY;

signal  Gen_Pkt_int             : std_logic                                 := '0';
signal  Gen_Length_int          : std_logic_vector(13 downto 0)             := (others => '0');


signal aw_addr_data             : std_logic_vector(MAC_araddr_0'length + MAC_wdata_0'length -1 downto 0)      := (others => '0');
signal aw_addr_data_cdc         : std_logic_vector(MAC_araddr_0'length + MAC_wdata_0'length -1 downto 0)      := (others => '0');
signal Wr_Req                   : std_logic                                 := '0';
signal Wr_Req_clk_axi           : std_logic                                 := '0';
signal Wr_Addr_Done             : std_logic                                 := '0';
signal Wr_Data_Done             : std_logic                                 := '0';
signal Wr_Done_tog              : std_logic                                 := '0';
signal Wr_Done_tog_clk_300      : std_logic                                 := '0';
signal Wr_Done_tog_clk_300_d    : std_logic                                 := '0';
signal Wr_Busy                  : std_logic                                 := '0';


signal araddr                   : std_logic_vector(MAC_araddr_0'range)      := (others => '0');
signal Rd_Req                   : std_logic                                 := '0';
signal Rd_Req_clk_axi           : std_logic                                 := '0';
signal Rd_Done_tog              : std_logic                                 := '0';
signal Rd_Done_tog_clk_300      : std_logic                                 := '0';
signal Rd_Done_tog_clk_300_d    : std_logic                                 := '0';
signal Rd_Busy                  : std_logic                                 := '0';
signal rvalid                   : std_logic                                 := '0';
signal rdata                    : std_logic_vector(MAC_rdata_0'range)       := (others => '0');

begin

-------------------------------------------------------------------------------
--
--      MAC Read Functions
--
-------------------------------------------------------------------------------

    MR_Pipe(0).Valid    <= Stat_Val;
    MR_Pipe(0).Rd_Addr  <= EXT(Stat_Rd_Addr, MR_Pipe(0).Rd_Addr'LENGTH);

    MR_FIFO_Rd_En       <= '1' when ((MR_FIFO_Rd_Ptr(MR_FIFO_Rd_Ptr'HIGH) = '0') and (Rd_Busy = '0') and (MD_FIFO_Full = '0')) else '0';

RQ:
process(clk_300)
variable case_3_var : std_logic_vector(2 downto 0);
variable case_5_var : std_logic_vector(4 downto 0);

begin

    if (rising_edge(clk_300)) then

        MR_Pipe(1 to MR_Pipe'HIGH)  <= MR_Pipe(0 to MR_Pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 1
        ---------------------------------------------------

        -- FIFO the Read Requests to the MAC Instance AXI Bus

        if (MR_Pipe(0).Valid = '1') then

            MR_FIFO <= MR_Pipe(0) & MR_FIFO(0 to MR_FIFO'HIGH-1);

            if (MR_FIFO_Rd_En = '0') then

                MR_FIFO_Rd_Ptr  <= MR_FIFO_Rd_Ptr + 1;

                assert (signed(MR_FIFO_Rd_Ptr) /= MR_FIFO'HIGH)
                    report "MR_FIFO Overflow in gt_mac_config"
                    severity note;

            end if;

        elsif (MR_FIFO_Rd_En = '1') then

            MR_FIFO_Rd_Ptr  <= MR_FIFO_Rd_Ptr - 1;

        end if;

        MR_Pipe(1)  <= MR_Pipe(1);

        Rd_Done_tog_clk_300_d <= Rd_Done_tog_clk_300;
        if (Rd_Done_tog_clk_300_d /= Rd_Done_tog_clk_300) then
            Rd_Busy <= '0';
        end if;

        Rd_Req <= '0';
        if (MR_FIFO_Rd_En = '1') then

            MR_Pipe(1) <= MR_FIFO(conv_integer(MR_FIFO_Rd_Ptr(MR_FIFO_Rd_Ptr'HIGH-1 downto 0)));

            Rd_Req <= '1';
            Rd_Busy <= '1';

        end if;

        if (signed(MR_FIFO_Rd_Ptr) >= (MR_FIFO'LENGTH / 2)) then

            MR_FIFO_Full_int    <= '1';

        else

            MR_FIFO_Full_int  <= '0';

        end if;

        if (Rst_300_Gate = '1') then

            MR_FIFO_Rd_Ptr  <= (others => '1');

        end if;

        -- Get the base address of the PLRAM instance and channel instance

        MR_Pipe(1).Wr_Addr  <= (AXI00_Ptr0(31 downto 2) & "00") + (conv_std_logic_vector(STAT_BASE + (STAT_INTERVAL * n), 30) & "00");

        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------

        -- Capture the Data received from the MAC

        MR_Pipe(2)          <= RST_MR_PIPE_TYPE;

        MR_Pipe(2).Valid    <= rvalid;
        MR_Pipe(2).Data     <= rdata;
        MR_Pipe(2).Wr_Addr  <= MR_Pipe(1).Wr_Addr + EXT(MR_Wr_Offs & "00", 32);

        -- Generate the Write PLRAM Write Address

        if (rvalid = '1') then

            MR_Wr_Offs  <= MR_Wr_Offs + 1;

        end if;

        if (Start_Status = '1') then

            MR_Wr_Offs  <= (others => '0');

        end if;

        ---------------------------------------------------
        -- Stage 3
        ---------------------------------------------------

        -- FIFO the Data read from the MAC

        if (MR_Pipe(2).Valid = '1') then

            MD_FIFO <= MR_Pipe(2) & MD_FIFO(0 to MD_FIFO'HIGH-1);

            if (MD_FIFO_Rd_En(n) = '0') then

                MD_FIFO_Rd_Ptr  <= MD_FIFO_Rd_Ptr + 1;

                assert (signed(MD_FIFO_Rd_Ptr) /= MD_FIFO'HIGH)
                    report "MD_FIFO Overflow in gt_mac_config"
                    severity note;

            end if;

        elsif (MD_FIFO_Rd_En(n) = '1') then

            MD_FIFO_Rd_Ptr  <= MD_FIFO_Rd_Ptr - 1;

        end if;

        MR_Pipe(3)  <= MR_Pipe(3);

        if (MD_FIFO_Rd_En(n) = '1') then

            MR_Pipe(3)          <= MD_FIFO(conv_integer(MD_FIFO_Rd_Ptr(MD_FIFO_Rd_Ptr'HIGH-1 downto 0)));
            MR_Pipe(3).Valid    <= '1';

        end if;

        if (signed(MD_FIFO_Rd_Ptr) >= (MD_FIFO'LENGTH / 2)) then

            MD_FIFO_Full    <= '1';

        else

            MD_FIFO_Full    <= '0';

        end if;

        if (Rst_300_Gate = '1') then

            MD_FIFO_Rd_Ptr  <= (others => '1');

        end if;

    end if;

end process;

RA:
process(axi_clk)
begin
    if (rising_edge(axi_clk)) then

        if (MAC_arready(n) = '1') then
            MAC_arvalid_i(n) <= '0';
            Rd_Done_tog <= not Rd_Done_tog;
        end if;

        if (Rd_Req_clk_axi = '1') then
            MAC_arvalid_i(n) <= '1';
            MAC_araddr(n)    <= araddr;
        end if;

    end if;
end process;

u_cdc_araddr: component cdc_static_bus
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => MAC_araddr_0'length
    )
    port map (
        src_clk         => clk_300,
        src_send        => Rd_Req,
        src_in          => MR_Pipe(1).Rd_Addr,

        dest_clk        => axi_clk,
        dest_ack        => Rd_Req_clk_axi,
        dest_out        => araddr
    );

u_cdc_rd_done: xpm_cdc_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF,
        SRC_INPUT_REG  => 0
    )
    port map (
        src_clk              => axi_clk,
        src_in               => Rd_Done_tog,
        dest_clk             => clk_300,
        dest_out             => Rd_Done_tog_clk_300
    );

u_cdc_rdata: component cdc_static_bus
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => MAC_rdata_0'length
    )
    port map (
        src_clk         => axi_clk,
        src_send        => MAC_rvalid(n),
        src_in          => MAC_rdata(n),

        dest_clk        => clk_300,
        dest_ack        => rvalid,
        dest_out        => rdata
    );


    MR_FIFO_Empty(n)    <= MR_FIFO_Rd_Ptr(MR_FIFO_Rd_Ptr'HIGH);
    MR_FIFO_Full(n)     <= MR_FIFO_Full_int;

    MD_FIFO_Empty(n)    <= MD_FIFO_Rd_Ptr(MD_FIFO_Rd_Ptr'HIGH);


    MAC_arvalid(n)      <= MAC_arvalid_i(n);
    MD_FIFO_Out(n)      <= MR_Pipe(3);

    MAC_rready(n)       <= '1';

-------------------------------------------------------------------------------
--
--      MAC Write Functions
--
-------------------------------------------------------------------------------

    MW_Pipe(0).Valid    <= PLRAM_rvalid when (Chan_FIFO_Out.Chan = n) else '0';
    MW_Pipe(0).Scr_Val  <= Chan_FIFO_Out.Scr_Val;
    MW_Pipe(0).Data     <= PLRAM_rdata;
    MW_Pipe(0).Reg_Ext  <= ("0" & Chan_FIFO_Out.Reg) - NUM_INT_REGS;
    MW_Pipe(0).Reg_Int  <= Chan_FIFO_Out.Reg(NUM_BIT_INT_REGS-1 downto 0);

    MW_FIFO_Rd_En       <= '1' when ((MW_FIFO_Rd_Ptr(MW_FIFO_Rd_Ptr'HIGH) = '0') and (Wr_Busy = '0')) else '0';

    Script_FIFO_Rd_En   <= '1' when ((Script_FIFO_Rd_Ptr(Script_FIFO_Rd_Ptr'HIGH) = '0') and (Script_Cnt(Script_Cnt'HIGH) = '1') and (Script_FIFO_Out.Valid = '0') and (Gen_Half_Full(n) = '0')) else '0';

WQ:
process(clk_300)
variable case_3_var : std_logic_vector(2 downto 0);
variable case_5_var : std_logic_vector(4 downto 0);
begin

    if (rising_edge(clk_300)) then

        MW_Pipe(1 to MW_Pipe'HIGH)  <= MW_Pipe(0 to MW_Pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------

        -- Translate the Register Index to a MAC Configuration Register

        if (MW_Pipe(1).Reg_Ext(MW_Pipe(1).Reg_Ext'HIGH) = '0') then

            MW_Pipe(2).Val_Ext  <= MW_Pipe(1).Valid and (not MW_Pipe(1).Scr_Val);
            MW_Pipe(2).Wr_Addr  <= EXT(MAC_CONF_REGS(conv_integer(MW_Pipe(1).Reg_Ext(NUM_BIT_EXT_REGS-1 downto 0))), MW_Pipe(2).Wr_Addr'LENGTH);

        else

            MW_Pipe(2).Val_Int  <= MW_Pipe(1).Valid and (not MW_Pipe(1).Scr_Val);

        end if;

        if (MW_Pipe(1).Valid = '0') then

            MW_Pipe(2).Scr_Val   <= '0';

        end if;

        ---------------------------------------------------
        -- Stage 3
        ---------------------------------------------------

        -- FIFO the Read Requests to the MAC Write AXI Bus

        if (MW_Pipe(2).Val_Ext = '1') then

            MW_FIFO <= MW_Pipe(2) & MW_FIFO(0 to MW_FIFO'HIGH-1);

            if (MW_FIFO_Rd_En = '0') then

                MW_FIFO_Rd_Ptr  <= MW_FIFO_Rd_Ptr + 1;

                assert (signed(MW_FIFO_Rd_Ptr) /= MW_FIFO'HIGH)
                    report "MW_FIFO Overflow in gt_mac_config"
                    severity note;

            end if;

        elsif (MW_FIFO_Rd_En = '1') then

            MW_FIFO_Rd_Ptr  <= MW_FIFO_Rd_Ptr - 1;

        end if;

        MW_Pipe(3)  <= MW_Pipe(3);

        MW_Pipe(3).Valid    <= '0';

        Wr_Done_tog_clk_300_d <= Wr_Done_tog_clk_300;
        if (Wr_Done_tog_clk_300_d /= Wr_Done_tog_clk_300) then
            Wr_Busy <= '0';
        end if;

        Wr_Req  <= '0';
        if (MW_FIFO_Rd_En = '1') then

            MW_Pipe(3)          <= MW_FIFO(conv_integer(MW_FIFO_Rd_Ptr(MW_FIFO_Rd_Ptr'HIGH-1 downto 0)));
            MW_Pipe(3).Valid    <= '1';

            Wr_Req  <= '1';
            Wr_Busy <= '1';

        end if;

        if (signed(MW_FIFO_Rd_Ptr) >= (MW_FIFO'LENGTH / 2)) then

            MW_FIFO_Full_int    <= '1';

        else

            MW_FIFO_Full_int  <= '0';

        end if;

        if (Rst_300_Gate = '1') then

            MW_FIFO_Rd_Ptr  <= (others => '1');

        end if;

        -- Implement Internal Registers

        Config_Upd_int  <= '0';

        if (MW_Pipe(2).Val_Int = '1') then

            case_3_var  := MW_Pipe(2).Reg_Int;

            case case_3_var is

                when "000"  =>

                    Script_Cfg_Len_int                  <= MW_Pipe(2).Data(31 downto 16);
                    Script_Cfg_Base_int                 <= MW_Pipe(2).Data(15 downto  0);

                when "001"  =>

                    Dest_MAC_Addr_int(31 downto 0)      <= MW_Pipe(2).Data;

                when "010"  =>

                    Dest_MAC_Addr_int(47 downto 32)     <= MW_Pipe(2).Data(15 downto  0);
                    Source_MAC_Addr_int(15 downto 0)    <= MW_Pipe(2).Data(31 downto 16);

                when "011"  =>

                    Source_MAC_Addr_int(47 downto 16)   <= MW_Pipe(2).Data;

                when "100"  =>

                    MAC_Ethertype_int                   <= MW_Pipe(2).Data(15 downto  0);
                    MAC_Traffic_Type_int                <= MW_Pipe(2).Data(17 downto 16);

                when "101"  =>

                    GT_TX_Polarity_int                  <= MW_Pipe(2).Data(22);

                    GT_Tx_Post_Cursor_int               <= MW_Pipe(2).Data(21 downto 17);
                    GT_Tx_Main_Cursor_int               <= MW_Pipe(2).Data(16 downto 10);
                    GT_Tx_Pre_Cursor_int                <= MW_Pipe(2).Data( 9 downto  5);
                    GT_Tx_Diff_Ctrl_int                 <= MW_Pipe(2).Data( 4 downto  0);

                when "110"  =>

                    GT_Rx_LPM_En_int                    <= MW_Pipe(2).Data(0);

                    Config_Upd_int                      <= '1';

                when others =>

            end case;

        end if;

        -- FIFO Script Requests

        if (MW_Pipe(2).Scr_Val = '1') then

            Script_FIFO <= MW_Pipe(2) & Script_FIFO(0 to Script_FIFO'HIGH-1);

            if (Script_FIFO_Rd_En = '0') then

                Script_FIFO_Rd_Ptr  <= Script_FIFO_Rd_Ptr + 1;

                assert (signed(Script_FIFO_Rd_Ptr) /= Script_FIFO'HIGH)
                    report "Script_FIFO Overflow in gt_mac_config"
                    severity note;

            end if;

        elsif (Script_FIFO_Rd_En = '1') then

            Script_FIFO_Rd_Ptr  <= Script_FIFO_Rd_Ptr - 1;

        end if;

        Script_FIFO_Out.Valid   <= '0';

        if (Script_FIFO_Rd_En = '1') then

            Script_FIFO_Out         <= Script_FIFO(conv_integer(Script_FIFO_Rd_Ptr(Script_FIFO_Rd_Ptr'HIGH-1 downto 0)));
            Script_FIFO_Out.Valid   <= '1';

        end if;

        if (signed(Script_FIFO_Rd_Ptr) >= (Script_FIFO'LENGTH / 2)) then

            Script_FIFO_Full_int    <= '1';

        else

            Script_FIFO_Full_int  <= '0';

        end if;

        if (Rst_300_Gate = '1') then

            Script_FIFO_Rd_Ptr  <= (others => '1');

        end if;

        ---------------------------------------------------
        -- Stage 4
        ---------------------------------------------------

        -- Intercept writes to the Rate Change register (0x138 in MAC address map)
        -- Stretch the pulse as it has to be read on the 150 MHz DRP clock

        Mode_Change_int <= '0';

        if ((MW_Pipe(2).Val_Ext = '1') and (MW_Pipe(2).Reg_Ext = fn_Get_Index(MAC_CONF_REGS, X"138"))) then

            -- See if the requested rate matches the current rate, if no switch it.

            if (MW_Pipe(2).Data(0) /= MAC_Sel_Mode_ap_clk(n)) then

                Mode_Change_int <= '1';

            end if;

        end if;

        Gen_Pkt_int     <= '0';
        Gen_Length_int  <= (others => '-');

        if (Script_FIFO_Out.Valid = '1') then

            Script_Cnt  <= "0" & Script_FIFO_Out.Data(31 downto 11);

            -- Size 0 .. 63 do noting
            -- Size 64 .. 1535 set packet size
            -- Size 1536 .. 2047 translate to 9500 to 10011 byte packets
            -- Reduce the size by 4 bytes to allow for the FCS added by the MAC

            Gen_Pkt_int <= '1';

            case_5_var := Script_FIFO_Out.Data(10 downto 6);

            case case_5_var is

                when "00000"                                =>  Gen_Pkt_int  <= '0';

                when           "00001" | "00010" | "00011" |
                     "00100" | "00101" | "00110" | "00111" |
                     "01000" | "01001" | "01010" | "01011" |
                     "01100" | "01101" | "01110" | "01111" |
                     "10000" | "10001" | "10010" | "10011" |
                     "10100" | "10101" | "10110" | "10111" =>   Gen_Length_int  <= ("000" & Script_FIFO_Out.Data(10 downto 0)) - 4;
                                                                -- Size 1536 .. 2047 translate to 9000 to 10022 byte packets (in steps of 2 bytes)
                when others                                =>   Gen_Length_int  <= ("00" & Script_FIFO_Out.Data(10 downto 0) & "0") + 5928 - 4;
            end case;

        elsif (Script_Cnt(Script_Cnt'HIGH) = '0') then

            Script_Cnt  <= Script_Cnt - 1;

        end if;

    end if;

end process;

WA:
process(axi_clk)
begin
    if (rising_edge(axi_clk)) then

        if (MAC_awready(n) = '1') then

            MAC_awvalid_i(n)    <= '0';
            Wr_Addr_Done        <= '1';

        end if;

        if (MAC_wready(n) = '1') then

            MAC_wvalid_i(n)     <= '0';
            Wr_Data_Done        <= '1';

        end if;

        -- both addr and data have been acknowledged
        if ((Wr_Addr_Done = '1') and (Wr_Data_Done = '1')) then
            Wr_Done_tog <= not Wr_Done_tog;

            -- done sent .. clear status
            Wr_Addr_Done     <= '0';
            Wr_Data_Done     <= '0';
        end if;

        if (Wr_Req_clk_axi = '1') then
            Wr_Addr_Done     <= '0';
            Wr_Data_Done     <= '0';

            MAC_awvalid_i(n)  <= '1';
            MAC_awaddr(n)     <= aw_addr_data_cdc(aw_addr_data_cdc'high downto MAC_wdata_0'length);
            MAC_wvalid_i(n)   <= '1';
            MAC_wdata(n)      <= aw_addr_data_cdc(MAC_wdata_0'range);

        end if;

    end if;
end process;

aw_addr_data <= MW_Pipe(3).Wr_Addr & MW_Pipe(3).Data;

u_cdc_aw: component cdc_static_bus
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => aw_addr_data'length
    )
    port map (
        src_clk         => clk_300,
        src_send        => Wr_Req,
        src_in          => aw_addr_data,

        dest_clk        => axi_clk,
        dest_ack        => Wr_Req_clk_axi,
        dest_out        => aw_addr_data_cdc
    );

u_cdc_wr_done: xpm_cdc_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF,
        SRC_INPUT_REG  => 0
    )
    port map (
        src_clk              => axi_clk,
        src_in               => Wr_Done_tog,
        dest_clk             => clk_300,
        dest_out             => Wr_Done_tog_clk_300
    );


    MAC_Mode_Change(n)      <= Mode_Change_int;

    Script_Cfg_Len(n)       <= Script_Cfg_Len_int;
    Script_Cfg_Base(n)      <= Script_Cfg_Base_int;

    Config_Upd(n)           <= Config_Upd_int;
    Dest_MAC_Addr(n)        <= Dest_MAC_Addr_int;
    Source_MAC_Addr(n)      <= Source_MAC_Addr_int;
    MAC_Ethertype(n)        <= MAC_Ethertype_int;
    MAC_Traffic_Type(n)     <= MAC_Traffic_Type_int;

    MAC_awvalid(n)          <= MAC_awvalid_i(n);
    MAC_wvalid(n)           <= MAC_wvalid_i(n);
    MAC_wstrb(n)            <= "1111";

    MW_FIFO_Full(n)         <= MW_FIFO_Full_int;
    MW_FIFO_Empty(n)        <= MW_FIFO_Rd_Ptr(MW_FIFO_Rd_Ptr'HIGH);

    Script_FIFO_Empty(n)    <= Script_FIFO_Rd_Ptr(Script_FIFO_Rd_Ptr'HIGH);
    Script_FIFO_Full(n)     <= Script_FIFO_Full_int;

    Gen_Pkt(n)              <= Gen_Pkt_int;
    Gen_Length(n)           <= Gen_Length_int;

    GT_Rx_LPM_En(n)         <= GT_Rx_LPM_En_int;
    GT_Tx_Post_Cursor(n)    <= GT_Tx_Post_Cursor_int;
    GT_Tx_Main_Cursor(n)    <= GT_Tx_Main_Cursor_int;
    GT_Tx_Pre_Cursor(n)     <= GT_Tx_Pre_Cursor_int;
    GT_Tx_Diff_Ctrl(n)      <= GT_Tx_Diff_Ctrl_int;
    GT_TX_Polarity(n)       <= GT_TX_Polarity_int;

end generate G1;

    MAC_awaddr_0        <= MAC_awaddr(0);
    MAC_awaddr_1        <= MAC_awaddr(1);
    MAC_awaddr_2        <= MAC_awaddr(2);
    MAC_awaddr_3        <= MAC_awaddr(3);

    MAC_wdata_0         <= MAC_wdata(0);
    MAC_wdata_1         <= MAC_wdata(1);
    MAC_wdata_2         <= MAC_wdata(2);
    MAC_wdata_3         <= MAC_wdata(3);

    MAC_wstrb_0         <= MAC_wstrb(0);
    MAC_wstrb_1         <= MAC_wstrb(1);
    MAC_wstrb_2         <= MAC_wstrb(2);
    MAC_wstrb_3         <= MAC_wstrb(3);

    MAC_araddr_0        <= MAC_araddr(0);
    MAC_araddr_1        <= MAC_araddr(1);
    MAC_araddr_2        <= MAC_araddr(2);
    MAC_araddr_3        <= MAC_araddr(3);

    Dest_MAC_Addr_0     <= Dest_MAC_Addr(0);
    Dest_MAC_Addr_1     <= Dest_MAC_Addr(1);
    Dest_MAC_Addr_2     <= Dest_MAC_Addr(2);
    Dest_MAC_Addr_3     <= Dest_MAC_Addr(3);

    Source_MAC_Addr_0   <= Source_MAC_Addr(0);
    Source_MAC_Addr_1   <= Source_MAC_Addr(1);
    Source_MAC_Addr_2   <= Source_MAC_Addr(2);
    Source_MAC_Addr_3   <= Source_MAC_Addr(3);

    MAC_Ethertype_0     <= MAC_Ethertype(0);
    MAC_Ethertype_1     <= MAC_Ethertype(1);
    MAC_Ethertype_2     <= MAC_Ethertype(2);
    MAC_Ethertype_3     <= MAC_Ethertype(3);

    MAC_Traffic_Type_0  <= MAC_Traffic_Type(0);
    MAC_Traffic_Type_1  <= MAC_Traffic_Type(1);
    MAC_Traffic_Type_2  <= MAC_Traffic_Type(2);
    MAC_Traffic_Type_3  <= MAC_Traffic_Type(3);

    Gen_Length_0        <= Gen_Length(0);
    Gen_Length_1        <= Gen_Length(1);
    Gen_Length_2        <= Gen_Length(2);
    Gen_Length_3        <= Gen_Length(3);

    GT_Tx_Main_Cursor_0 <= GT_Tx_Main_Cursor(0);
    GT_Tx_Main_Cursor_1 <= GT_Tx_Main_Cursor(1);
    GT_Tx_Main_Cursor_2 <= GT_Tx_Main_Cursor(2);
    GT_Tx_Main_Cursor_3 <= GT_Tx_Main_Cursor(3);
    GT_Tx_Post_Cursor_0 <= GT_Tx_Post_Cursor(0);
    GT_Tx_Post_Cursor_1 <= GT_Tx_Post_Cursor(1);
    GT_Tx_Post_Cursor_2 <= GT_Tx_Post_Cursor(2);
    GT_Tx_Post_Cursor_3 <= GT_Tx_Post_Cursor(3);
    GT_Tx_Pre_Cursor_0  <= GT_Tx_Pre_Cursor(0);
    GT_Tx_Pre_Cursor_1  <= GT_Tx_Pre_Cursor(1);
    GT_Tx_Pre_Cursor_2  <= GT_Tx_Pre_Cursor(2);
    GT_Tx_Pre_Cursor_3  <= GT_Tx_Pre_Cursor(3);
    GT_Tx_Diff_Ctrl_0   <= GT_Tx_Diff_Ctrl(0);
    GT_Tx_Diff_Ctrl_1   <= GT_Tx_Diff_Ctrl(1);
    GT_Tx_Diff_Ctrl_2   <= GT_Tx_Diff_Ctrl(2);
    GT_Tx_Diff_Ctrl_3   <= GT_Tx_Diff_Ctrl(3);





end architecture rtl; -- of gt_mac_config
