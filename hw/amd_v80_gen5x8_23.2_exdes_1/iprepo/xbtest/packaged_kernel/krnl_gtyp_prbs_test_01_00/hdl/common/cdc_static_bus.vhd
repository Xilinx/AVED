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

entity cdc_static_bus is
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
end entity cdc_static_bus;

architecture rtl of cdc_static_bus is


    signal  src_send_d1         : std_logic                                 := '0';
    signal  src_send_tog        : std_logic                                 := '0';
    signal  src_send_tog_cdc    : std_logic                                 := '0';
    signal  src_send_tog_cdc_d1 : std_logic                                 := '0';


    signal  src_in_latch        : std_logic_vector(WIDTH-1 downto 0)        := (others => '0');
    signal  dest_out_cdc        : std_logic_vector(WIDTH-1 downto 0)        := (others => '0');

    -- synthesis translate_off

    -- synthesis translate_on

begin


CDC_data:
xpm_cdc_array_single
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => WIDTH,
        SRC_INPUT_REG  => 0
    )
    port map (
        src_clk         => src_clk,
        src_in          => src_in_latch,

        dest_clk        => dest_clk,
        dest_out        => dest_out_cdc
    );

CDC_ctrl: xpm_cdc_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF+2,
        SRC_INPUT_REG  => 0
    )
    port map (
        src_clk              => src_clk,
        src_in               => src_send_tog,
        dest_clk             => dest_clk,
        dest_out             => src_send_tog_cdc
    );


process(src_clk)
begin
    if (rising_edge(src_clk)) then
        src_send_d1 <= src_send;

        -- rising edge
        if (src_send = '1' and src_send_d1 = '0') then
            src_send_tog <= not src_send_tog;
            src_in_latch <= src_in;
        end if;
    end if;
end process;

process(dest_clk)
begin
    if (rising_edge(dest_clk)) then
        src_send_tog_cdc_d1 <= src_send_tog_cdc;

        dest_ack <= '0';
        -- detect change of level
        if ( (src_send_tog_cdc_d1 xor src_send_tog_cdc) = '1') then
            dest_ack <= '1';
            dest_out <= dest_out_cdc;
        end if;
    end if;
end process;


end architecture rtl;