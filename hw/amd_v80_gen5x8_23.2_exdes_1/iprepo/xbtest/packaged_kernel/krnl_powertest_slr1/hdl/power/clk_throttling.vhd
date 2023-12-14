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
    use unisim.vcomponents.all;
    use xpm.vcomponents.all;

entity clk_throttling is
    generic (
        C_THROTTLE_MODE     : integer               := 1; -- 1: clock throttling with BUFG, 0: throttle clock enable of sites FF input oscillator
        SYNTH_SIZE          : integer               := 8;
        RST_ACTIVE_HIGH     : integer range 0 to 1  := 1; -- 1 means Active High, 0 means Active Low
        DEST_SYNC_FF        : integer               := 4;
        SIM_DEVICE          : string                := "ULTRASCALE_PLUS"
    );
    port (
        Clk_In          : in    std_logic;  -- Clk_Out_Fast frequency
        Rst_In          : in    std_logic;
        Enable          : in    std_logic;

        Rate_Upd_Tog    : in    std_logic;  -- Rate update Toggle (asynchronous)
        Rate_In         : in    std_logic_vector(SYNTH_SIZE-1 downto 0);

        Clk_Out         : out   std_logic;
        Throttle        : out   std_logic
    );
end entity clk_throttling;

architecture rtl of clk_throttling is

-------------------------------------------------------------------------------
--      Constants (simple)
-------------------------------------------------------------------------------
constant STARTUP_SYNTH : std_logic := '0'; -- '1': 100 %; '0': 0 %

constant ZERO       : std_logic_vector(0 downto 0)  := (others => '0');
constant ONE        : std_logic_vector(0 downto 0)  := (others => '1');

constant CONST_MAX  : std_logic_vector(SYNTH_SIZE-1 downto 0)   := "1" & EXT("0", SYNTH_SIZE-1);

-------------------------------------------------------------------------------
--      Type Definitions
-------------------------------------------------------------------------------
attribute dont_touch    : string;
attribute RLOC          : string;

-------------------------------------------------------------------------------
--      Signals
-------------------------------------------------------------------------------
signal  Clk_Int                 : std_logic                                 := '0';

signal  Rate_Upd_Tog_CDC        : std_logic                                 := '0';
signal  Rate_Upd_Tog_CDC_d1     : std_logic                                 := '0';
signal  Rate_Upd                : std_logic                                 := '0';
signal  Rate_In_cdc             : std_logic_vector(SYNTH_SIZE-1 downto 0)   := (others => STARTUP_SYNTH);
signal  Rate_int                : std_logic_vector(SYNTH_SIZE-1 downto 0)   := (others => STARTUP_SYNTH);
signal  Synth                   : std_logic_vector(SYNTH_SIZE-1 downto 0)   := (others => STARTUP_SYNTH);

signal  Rst_In_rsync            : std_logic                                 := '0';

signal  Gate_Fast               : std_logic                                 := '0';
signal  Gate_Fast_d1            : std_logic                                 := '0';
attribute dont_touch  of Gate_Fast    : signal is "true";
attribute dont_touch  of Gate_Fast_d1 : signal is "true";

-- synthesis translate_off

-- synthesis translate_on

begin

-------------------------------------------------------------------------------
--      CDC onto the local domain
-------------------------------------------------------------------------------
-- if clk_in = ap_clk, ok this could have been removed...
rate_cdc: xpm_cdc_array_single
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => SYNTH_SIZE,
        SRC_INPUT_REG   => 0
    )
    port map (
        src_clk         => ZERO(0),
        src_in          => Rate_In,

        dest_clk        => Clk_Int,
        dest_out        => Rate_In_cdc
    );

rate_upd_cdc: xpm_cdc_single
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF + 1,
        SRC_INPUT_REG   => 0
    )
    port map (
        src_clk     => ZERO(0),
        src_in      => Rate_Upd_Tog,

        dest_clk    => Clk_Int,
        dest_out    => Rate_Upd_Tog_CDC
    );

RPS : xpm_cdc_async_rst
    generic map(
        DEST_SYNC_FF      => DEST_SYNC_FF,
        RST_ACTIVE_HIGH   => RST_ACTIVE_HIGH
    )
    port map(
        src_arst    => Rst_In,
        dest_clk    => Clk_Int,
        dest_arst   => Rst_In_rsync
    );

-------------------------------------------------------------------------------
--      Instantiate the Clk_Out Gated Clock Buffer
-------------------------------------------------------------------------------
CB: if ( C_THROTTLE_MODE = 1 ) generate

    signal In_Bufg_CE   : std_logic := '1';
    attribute dont_touch  of In_Bufg_CE    : signal is "true";
    attribute dont_touch  of i_CE_LUT      : label is "true";
    attribute dont_touch  of i_BUFG        : label is "true";

begin
    -- force the LUT to avoid any optimization of i_BUFG.
    -- Depending on the shell clock structure (UCS in BLP), vivado trims away cascaded buffer if they are not in the same partition
    i_CE_LUT : LUT1
    generic map (
        INIT => X"3"
    )
    port map (
        I0  => '1',
        O   => In_Bufg_CE
    );

    i_BUFG: BUFGCE
    generic map (
        SIM_DEVICE => SIM_DEVICE,
        CE_TYPE => "ASYNC"
    ) port map (
        I   => Clk_In,
        CE  => In_Bufg_CE,
        O   => Clk_Int
    );

    -- From UG949 - Because the BUFGCE and BUFGCE_DIV do not have the same cell delay, Xilinx recommends using the same clock buffer for both synchronous clocks
    -- Generate BUFGCE_DIV for Fast clock, contrinuous clock and DIV clock when divided clock is enabled
    FCLK: BUFGCE
    generic map (
        SIM_DEVICE => SIM_DEVICE
    ) port map (
        I   => Clk_Int,
        CE  => Gate_Fast_d1,

        O   => Clk_Out
    );

    Throttle <= '1';
end generate;

NCB: if ( C_THROTTLE_MODE = 0 ) generate
    Clk_Int  <= Clk_In;

    Throttle <= Gate_Fast_d1;
    Clk_Out  <= Clk_Int;
end generate;

-------------------------------------------------------------------------------
--      Controller Logic
-------------------------------------------------------------------------------
SQ: process(Clk_Int)
begin

    if (rising_edge(Clk_Int)) then

        -- Wait for a change of state of the Rate_Upd_Tog input, and reload the Rate signal

        Rate_Upd_Tog_CDC_d1   <= Rate_Upd_Tog_CDC;

        Rate_Upd    <= Rate_Upd_Tog_CDC xor Rate_Upd_Tog_CDC_d1;

        if (Rate_Upd = '1') then

            Rate_int <= Rate_In_cdc;

        end if;

        -- Implement N/M Synthesiser
        -- Treat all values >= 128 as 100%

        if (Rate_int(Rate_int'HIGH) = '1') then

            Synth   <= ("0" & Synth(Synth'HIGH-1 downto 0)) + CONST_MAX;

        else

            Synth   <= ("0" & Synth(Synth'HIGH-1 downto 0)) + Rate_int;

        end if;

        Gate_Fast <= Synth(Synth'HIGH);
        -- Flop the gate signals to assist timing closure
        Gate_Fast_d1 <= Gate_Fast;

        if ( (Enable = '0') or (((RST_ACTIVE_HIGH = 1) and (Rst_In_rsync = '1')) or ((RST_ACTIVE_HIGH = 0) and (Rst_In_rsync = '0'))) ) then

            Rate_int        <= (others => STARTUP_SYNTH); -- Default to 0 %
            Synth           <= (others => STARTUP_SYNTH);
            Gate_Fast       <= STARTUP_SYNTH;
            Gate_Fast_d1    <= STARTUP_SYNTH;

        end if;

    end if;

end process;

end architecture rtl; -- of clk_throttling
