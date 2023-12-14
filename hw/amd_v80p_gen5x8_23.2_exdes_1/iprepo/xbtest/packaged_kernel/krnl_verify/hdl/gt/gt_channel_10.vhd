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

library ieee, unisim;
    use ieee.std_logic_1164.all;
    use ieee.std_logic_unsigned.all;
    use ieee.std_logic_arith.all;
    use ieee.std_logic_misc.all;
    use unisim.vcomponents.all;

library xpm;
    use xpm.vcomponents.all;

entity gt_channel_10 is
--  generic (
--  );
    port (
        Rx_Usr_Clk      : in    std_logic;
        Tx_Usr_Clk      : in    std_logic;

        ctrl_scramb_dis : in    std_logic;
        ctrl_retime_dis : in    std_logic;
        ctrl_align_dis  : in    std_logic;

        -- Rx Traffic Interface

        Rx_Data         : in    std_logic_vector(31 downto 0);
        Rx_Hdr_Val      : in    std_logic;
        Rx_Header       : in    std_logic_vector(1 downto 0);

        Rx_Slip         : out   std_logic;

        -- Rx Traffic Interface

        Tx_Data         : out   std_logic_vector(31 downto 0);
        Tx_Hdr_Val      : out   std_logic;
        Tx_Header       : out   std_logic_vector(1 downto 0)
    );
end entity gt_channel_10;

architecture rtl of gt_channel_10 is

-------------------------------------------------------------------------------
--
--      Constants (simple)
--
-------------------------------------------------------------------------------

constant ZERO   : std_logic_vector(31 downto 0) := (others => '0');
constant ONE    : std_logic_vector(0 downto 0)  := (others => '1');

constant SLIP_TEST_SH               : std_logic_vector(2 downto 0)  := "000";
constant SLIP_ST_CHK_GOOD           : std_logic_vector(2 downto 0)  := "001";
constant SLIP_ST_CHK_BAD_LOCKED     : std_logic_vector(2 downto 0)  := "010";
constant SLIP_ST_CHK_BAD_UNLOCKED   : std_logic_vector(2 downto 0)  := "011";
constant SLIP_ST_SLIP               : std_logic_vector(2 downto 0)  := "100";

-------------------------------------------------------------------------------
--
--      Type Definitions
--
-------------------------------------------------------------------------------

type Pipe_Type is
    record
        FO_Wr_En        : std_logic;
        Hdr_Val         : std_logic;
        Ctrl_Hdr_Val    : std_logic;
        Idle            : std_logic;
        Ins_Pos         : std_logic;
        Header          : std_logic_vector(1 downto 0);
        Data            : std_logic_vector(31 downto 0);
        Locked          : std_logic;
    end record;

type Pipe_Array_Type is array (integer range <>) of Pipe_Type;

constant RST_PIPE_TYPE : Pipe_Type := (
    '0',                -- : std_logic
    '0',                -- : std_logic
    '0',                -- : std_logic
    '0',                -- : std_logic
    '0',                -- : std_logic
    (others => '0'),    -- : std_logic_vector(1 downto 0);
    (others => '0'),    -- : std_logic_vector(31 downto 0);
    '0');               -- : std_logic

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

function fn_Reverse_Bits(vector : std_logic_vector) return std_logic_vector is
variable result : std_logic_vector(vector'LENGTH-1 downto 0);
begin

    for i in result'range loop

        result(i) := vector(vector'HIGH-i);

    end loop;

    return result;

end function fn_Reverse_Bits;

-------------------------------------------------------------------------------
--
--      Signals
--
-------------------------------------------------------------------------------

signal  SH_Good             : std_logic                     := '0';
signal  SH_Bad              : std_logic                     := '0';
signal  SH_Test             : std_logic                     := '0';
signal  SH_Cnt              : std_logic_vector(6 downto 0)  := (others => '0');
signal  SH_Inv_Cnt          : std_logic_vector(4 downto 0)  := (others => '0');
signal  SH_Inv_Cnt_Zero     : std_logic                     := '0';
signal  Slip_Sate           : std_logic_vector(2 downto 0)  := SLIP_TEST_SH;
signal  Block_Lock          : std_logic                     := '0';

signal  In_Pipe             : Pipe_Array_Type(0 to 4)       := (others => RST_PIPE_TYPE);

signal  Descramb            : std_logic_vector(57 downto 0) := (others => '0');

signal  FO_Rd_En            : std_logic                     := '0';
signal  Pad_Next            : std_logic                     := '0';
signal  FO_Pad              : std_logic_vector(0 to 1)      := (others => '0');

signal  FO_Rst              : std_logic                     := '0';
signal  FO_RDRSTBUSY        : std_logic                     := '0';
signal  FO_EMPTY            : std_logic                     := '0';
signal  FO_LOW              : std_logic                     := '0';
signal  FO_HIGH             : std_logic                     := '0';
signal  FO_FULL             : std_logic                     := '0';

signal  Out_Pipe            : Pipe_Array_Type(0 to 5)       := (others => RST_PIPE_TYPE);

signal  Scramb              : std_logic_vector(57 downto 0) := (others => '0');


signal  ctrl_scramb_dis_rx  : std_logic                     := '0';
signal  ctrl_scramb_dis_tx  : std_logic                     := '0';
signal  ctrl_retime_dis_rx  : std_logic                     := '0';
signal  ctrl_retime_dis_tx  : std_logic                     := '0';
signal  ctrl_align_dis_rx   : std_logic                     := '0';

-- synthesis translate_off

signal  FO_WRERR            : std_logic                     := '0';
signal  dbg_Slip_Cnt        : integer                       := 0;
signal  dbg_Descramb        : std_logic_vector(31 downto 0) := (others => '0');

-- synthesis translate_on

begin


    CDC_ctrl_scramb_dis_rx: xpm_cdc_single
    generic map(

        DEST_SYNC_FF    => 4,
        SIM_ASSERT_CHK  => 0,
        SRC_INPUT_REG   => 0

    )
    port map(

        src_clk   => '0',
        src_in    => ctrl_scramb_dis,
        dest_clk  => Rx_Usr_Clk,
        dest_out  => ctrl_scramb_dis_rx
    );

    CDC_ctrl_retime_dis_rx: xpm_cdc_single
    generic map(

        DEST_SYNC_FF    => 4,
        SIM_ASSERT_CHK  => 0,
        SRC_INPUT_REG   => 0

    )
    port map(

        src_clk   => '0',
        src_in    => ctrl_retime_dis,
        dest_clk  => Rx_Usr_Clk,
        dest_out  => ctrl_retime_dis_rx
    );

    CDC_ctrl_align_dis: xpm_cdc_single
    generic map(

        DEST_SYNC_FF    => 4,
        SIM_ASSERT_CHK  => 0,
        SRC_INPUT_REG   => 0

    )
    port map(

        src_clk   => '0',
        src_in    => ctrl_align_dis,
        dest_clk  => Rx_Usr_Clk,
        dest_out  => ctrl_align_dis_rx
    );

    CDC_ctrl_scramb_dis_tx: xpm_cdc_single
    generic map(

        DEST_SYNC_FF    => 4,
        SIM_ASSERT_CHK  => 0,
        SRC_INPUT_REG   => 0

    )
    port map(

        src_clk   => '0',
        src_in    => ctrl_scramb_dis,
        dest_clk  => Tx_Usr_Clk,
        dest_out  => ctrl_scramb_dis_tx
    );

    CDC_ctrl_retime_dis_tx: xpm_cdc_single
    generic map(

        DEST_SYNC_FF    => 4,
        SIM_ASSERT_CHK  => 0,
        SRC_INPUT_REG   => 0

    )
    port map(

        src_clk   => '0',
        src_in    => ctrl_retime_dis,
        dest_clk  => Tx_Usr_Clk,
        dest_out  => ctrl_retime_dis_tx
    );


    In_Pipe(0).Hdr_Val  <= Rx_Hdr_Val;
    In_Pipe(0).Header   <= Rx_Header;
    In_Pipe(0).Data     <= fn_Reverse_Bits(Rx_Data);
    In_Pipe(0).Locked   <= Block_Lock;
    In_Pipe(0).FO_Wr_En <= '1';

WQ:
process(Rx_Usr_Clk)
variable Descramb_var   : std_logic_vector(89 downto 0);
begin

    if (rising_edge(Rx_Usr_Clk)) then

        -- 66b Alignment State Machine

        SH_Good <=                         In_Pipe(0).Header(0) xor  In_Pipe(0).Header(1);
        SH_Bad  <= In_Pipe(0).Hdr_Val and (In_Pipe(0).Header(0) xnor In_Pipe(0).Header(1));
        SH_Test <= In_Pipe(0).Hdr_Val;

        -- Count All and Bad SHs

        if (SH_Test = '1') then

            SH_Cnt  <= SH_Cnt + 1;

        end if;

        if (SH_Bad = '1') then

            SH_Inv_Cnt  <= SH_Inv_Cnt + 1;

        end if;

        SH_Inv_Cnt_Zero <= nor_reduce(SH_Inv_Cnt);

        -- Implement the State Machine

        Rx_Slip <= '0';

        case Slip_Sate is

            when SLIP_TEST_SH =>

                if (SH_Test = '1') then

                    if (SH_Good = '1') then

                        Slip_Sate   <= SLIP_ST_CHK_GOOD;

                    else

                        if (Block_Lock = '1') then

                            Slip_Sate   <= SLIP_ST_CHK_BAD_LOCKED;

                        else

                            Slip_Sate   <= SLIP_ST_CHK_BAD_UNLOCKED;

                        end if;

                    end if;

                end if;

            when SLIP_ST_CHK_GOOD =>

                Slip_Sate   <= SLIP_TEST_SH;

                -- See if we have checked 64 SHs

                if (SH_Cnt(6) = '1') then

                    -- Have they all been good ?

                    if (SH_Inv_Cnt_Zero = '1') then

                        -- Go into Block Lock

                        Block_Lock  <= '1';

                        -- synthesis translate_off
                        dbg_Slip_Cnt    <= 0;
                        -- synthesis translate_on

                    end if;

                    -- Check the next block

                    SH_Cnt      <= (others => '0');
                    SH_Inv_Cnt  <= (others => '0');

                end if;

            when SLIP_ST_CHK_BAD_LOCKED =>

                Slip_Sate   <= SLIP_TEST_SH;

                -- See if we have seen 16 bad

                if (SH_Inv_Cnt(4) = '1') then

                    -- Go unlocked, and Slip the transceiver

                    SH_Cnt      <= (others => '0');
                    SH_Inv_Cnt  <= (others => '0');
                    Block_Lock  <= '0';
                    Rx_Slip     <= '1';
                    Slip_Sate   <= SLIP_ST_SLIP;

                    -- synthesis translate_off
                    dbg_Slip_Cnt    <= dbg_Slip_Cnt + 1;
                    -- synthesis translate_on

                elsif (SH_Cnt(6) = '1') then

                    -- If we have checked 64 SHs, without getting 16 bad
                    -- Check the next block

                    SH_Cnt      <= (others => '0');
                    SH_Inv_Cnt  <= (others => '0');

                end if;

            when SLIP_ST_CHK_BAD_UNLOCKED =>

                -- A Bad SH while unlocked, Slip and try again

                SH_Cnt      <= (others => '0');
                SH_Inv_Cnt  <= (others => '0');
                Rx_Slip     <= '1';
                Slip_Sate   <= SLIP_ST_SLIP;

                -- synthesis translate_off
                dbg_Slip_Cnt    <= dbg_Slip_Cnt + 1;
                -- synthesis translate_on

            when SLIP_ST_SLIP =>

                -- Wait for 4 SHs before we start testing the new block

                if (SH_Cnt(2) = '1') then

                    SH_Cnt      <= (others => '0');
                    SH_Inv_Cnt  <= (others => '0');
                    Slip_Sate   <= SLIP_TEST_SH;

                end if;

            when others =>

                SH_Cnt      <= (others => '0');
                SH_Inv_Cnt  <= (others => '0');
                Slip_Sate   <= SLIP_TEST_SH;

        end case;

        if (ctrl_align_dis_rx = '1') then

            Rx_Slip     <= '0';

        end if;

        ---------------------------------------------------
        -- Pipeline
        ---------------------------------------------------

        In_Pipe(1 to In_Pipe'HIGH)  <= In_Pipe(0 to In_Pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 1
        ---------------------------------------------------

        -- De-scramble the 66b Data

        Descramb_var    := In_Pipe(0).Data & Descramb;

        In_Pipe(1).Data <= Descramb_var(89 downto 58) xor Descramb_var(89-39 downto 58-39) xor Descramb_var(89-58 downto 58-58);
        Descramb        <= Descramb_var(89 downto 32);

        if ((In_Pipe(0).Hdr_Val = '1') and (In_Pipe(0).Header = "10")) then

            In_Pipe(1).Ctrl_Hdr_Val <= '1';

        end if;

        if (ctrl_scramb_dis_rx = '1') then

            In_Pipe(1).Data <= In_Pipe(0).Data;

        end if;

        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------

        -- Detect Idles

        if ((In_Pipe(1).Ctrl_Hdr_Val = '1') and (In_Pipe(1).Data(7 downto 0) = X"1E")) then

            In_Pipe(2).Idle <= '1';

        end if;

        ---------------------------------------------------
        -- Stage 3
        ---------------------------------------------------

        -- if Stages 2 and 4 are Idles, we can delete the one in stage 2

        if ((ctrl_retime_dis_rx = '0') and (In_Pipe(2).Idle = '1') and (In_Pipe(4).Idle = '1') and (FO_HIGH = '1')) then

            In_Pipe(2).FO_Wr_En <= '0';
            In_Pipe(3).FO_Wr_En <= '0';
            In_Pipe(3).Idle     <= '0';

        end if;


    end if;

end process;

FB:
block

signal  FO_Din  : std_logic_vector(35 downto 0) := (others => '0');
signal  FO_Dout : std_logic_vector(35 downto 0) := (others => '0');

begin

    FO_Din  <= EXT(In_Pipe(In_Pipe'HIGH).Locked & In_Pipe(In_Pipe'HIGH).Idle & In_Pipe(In_Pipe'HIGH).Hdr_Val & In_Pipe(In_Pipe'HIGH).Header(0) & In_Pipe(In_Pipe'HIGH).Data, FO_Din'LENGTH);

FO:
FIFO18E2
    generic map (
        WRITE_WIDTH             => 36,
        READ_WIDTH              => 36,
        REGISTER_MODE           => "DO_PIPELINED",
        CLOCK_DOMAINS           => "INDEPENDENT",
        PROG_EMPTY_THRESH       => 128,                     -- 1/4
        PROG_FULL_THRESH        => 384,                     -- 3/4
--      PROG_EMPTY_THRESH       => 8,
--      PROG_FULL_THRESH        => 24,
        CASCADE_ORDER           => "NONE",
        -- synthesis translate_off
        RDCOUNT_TYPE            => "SIMPLE_DATACOUNT",      -- (For SIM only)
        -- synthesis translate_on
        FIRST_WORD_FALL_THROUGH => "FALSE"
    )
    port map (
        WRCLK               => Rx_Usr_Clk,
        RST                 => FO_Rst,
        WREN                => In_Pipe(In_Pipe'HIGH).FO_Wr_En,
        DIN                 => FO_Din(31 downto  0),
        DINP                => FO_Din(35 downto 32),
        -- synthesis translate_off
        WRERR               => FO_WRERR,
        -- synthesis translate_on

        RDCLK               => Tx_Usr_Clk,
        RSTREG              => ZERO(0),
        RDRSTBUSY           => FO_RDRSTBUSY,
        RDEN                => FO_Rd_En,
        REGCE               => ONE(0),
        DOUT                => FO_Dout(31 downto  0),
        DOUTP               => FO_Dout(35 downto 32),
--      RDERR               => FO_RdErr,

        EMPTY               => FO_EMPTY,
        PROGEMPTY           => FO_LOW,
        PROGFULL            => FO_HIGH,
        FULL                => FO_FULL,

        CASDIN              => ZERO(31 downto 0),
        CASDINP             => ZERO(3 downto 0),
        CASDOMUX            => ZERO(0),
        CASDOMUXEN          => ZERO(0),
        CASNXTRDEN          => ZERO(0),
        CASOREGIMUX         => ZERO(0),
        CASOREGIMUXEN       => ZERO(0),
        CASPRVEMPTY         => ZERO(0),
        SLEEP               => ZERO(0)
    );

    Out_Pipe(0).Locked  <= FO_Dout(35);
    Out_Pipe(0).Idle    <= FO_Dout(34);
    Out_Pipe(0).Ins_Pos <= FO_Dout(34) or (FO_Dout(33) and (not FO_Dout(35)));
    Out_Pipe(0).Hdr_Val <= FO_Dout(33);
    Out_Pipe(0).Header  <= (not FO_Dout(32)) & FO_Dout(32);
    Out_Pipe(0).Data    <= FO_Dout(31 downto  0);

end block FB;

RQ:
process(Tx_Usr_Clk)
variable Scramb_var : std_logic_vector(57 downto 0);
-- synthesis translate_off
variable Descramb_var   : std_logic_vector(89 downto 0);
-- synthesis translate_on
begin

    if (rising_edge(Tx_Usr_Clk)) then

        FO_Pad  <= (not FO_Rd_En) & FO_Pad(0);

        ---------------------------------------------------
        -- Pipeline
        ---------------------------------------------------

        Out_Pipe(1 to Out_Pipe'HIGH)  <= Out_Pipe(0 to Out_Pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------

        -- Skip reads if the FIFO is Low, and we are unlocked (and replacing) or the output is an idle

        FO_Rd_En    <= '1';
        Pad_Next    <= '0';

        if ((FO_Rd_En = '1') and (FO_LOW = '1') and (Out_Pipe(1).Ins_Pos = '1')) then

            FO_Rd_En    <= '0';
            Pad_Next    <= '1';

        end if;

        if (Pad_Next = '1') then

            FO_Rd_En    <= '0';

        end if;

        if (ctrl_retime_dis_tx = '1') then

            FO_Rd_En    <= '1';
            Pad_Next    <= '0';

        end if;

        ---------------------------------------------------
        -- Stage 3
        ---------------------------------------------------

        -- Whenever we Pad, insert an Idle pattern

        if (FO_Pad(1) = '1') then

            Out_Pipe(1 to 3)    <= Out_Pipe(1 to 3);

            Out_Pipe(3).Header   <= "10";

            if (FO_Pad(0) = '1') then

                Out_Pipe(3).Idle     <= 'H';
                Out_Pipe(3).Hdr_Val  <= 'H';
                Out_Pipe(3).Data     <= X"0000001E";

            else

                Out_Pipe(3).Idle     <= 'L';
                Out_Pipe(3).Hdr_Val  <= 'L';
                Out_Pipe(3).Data     <= X"00000000";

            end if;

        end if;

        ---------------------------------------------------
        -- Stage 4
        ---------------------------------------------------

        -- Re-scramble the Data

        Scramb_var  := Scramb;

        for n in Out_Pipe(3).Data'LOW to Out_Pipe(3).Data'HIGH loop

            Scramb_var  := Scramb_var(56 downto 0) & (Scramb_var(57) xor Scramb_var(38) xor Out_Pipe(3).Data(n));

            Out_Pipe(4).Data(n) <= Scramb_var(0);

        end loop;

        if (ctrl_scramb_dis_tx = '1') then

            Out_Pipe(4).Data <= Out_Pipe(3).Data;

        end if;

        Scramb  <= Scramb_var;

        Out_Pipe(4).Hdr_Val <= not Out_Pipe(3).Hdr_Val;

        ---------------------------------------------------
        -- Stage 5
        ---------------------------------------------------

        Out_Pipe(5).Data    <= fn_Reverse_Bits(Out_Pipe(4).Data);


        -- synthesis translate_off

        -- Test De-scramble the data

        Descramb_var    := Out_Pipe(4).Data & Descramb_var(89 downto 32);
        dbg_Descramb    <= Descramb_var(89 downto 58) xor Descramb_var(89-39 downto 58-39) xor Descramb_var(89-58 downto 58-58);

        -- synthesis translate_on

    end if;

end process;

    Tx_Data     <= Out_Pipe(Out_Pipe'HIGH).Data;
    Tx_Header   <= Out_Pipe(Out_Pipe'HIGH).Header;
    Tx_Hdr_Val  <= Out_Pipe(Out_Pipe'HIGH).Hdr_Val;

end architecture rtl; -- of gt_channel_10
