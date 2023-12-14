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

library ieee;
    use ieee.std_logic_1164.all;
    use ieee.std_logic_unsigned.all;
    use ieee.std_logic_arith.all;
    use ieee.numeric_std.all;

library unisim;
    use unisim.vcomponents.all;

entity dna_read_wrappper is
    generic (
        SIM_DNA_VALUE   : std_logic_vector(95 downto 0) := (others => '0')
    );
    port (
        Clk             : in  std_logic;
        DNA_Read_Rq_Tog : in  std_logic;
        DNA_96          : out std_logic_vector(95 downto 0);
        Done_Tog        : out std_logic
    );

end dna_read_wrappper;

architecture rtl of dna_read_wrappper is

    constant ST_LOAD        : std_logic_vector(0 downto 0)  := "0";
    constant ST_SHIFT       : std_logic_vector(0 downto 0)  := "1";

    signal State            : std_logic_vector(0 downto 0)  := ST_LOAD;

    signal DNA_i            : std_logic_vector(95 downto 0) := (others => '0');
    signal Done_i           : std_logic                     := '0';

    signal Shift_Cnt        : std_logic_vector(7 downto 0)  := (others => '0');

    signal Read_Rq_Tog_d1   : std_logic                     := '0';
    signal DNA_Read         : std_logic                     := '0';
    signal DNA_Shift        : std_logic                     := '0';
    signal DNA_Dout         : std_logic                     := '0';

begin

--    To access the Device DNA data, you must first load the shift register by setting the active-High
--    READ signal for one clock cycle. After the shift register is loaded, the data can be synchronously
--    shifted out by enabling the active-High SHIFT input and capturing the data out the DOUT output

process(Clk)
begin

    if rising_edge(Clk) then

        DNA_Read  <= '0';
        DNA_Shift <= '0';

        Read_Rq_Tog_d1 <= DNA_Read_Rq_Tog;

        case State is

            when ST_LOAD =>
                if ( DNA_Read_Rq_Tog /= Read_Rq_Tog_d1 ) then

                    State    <= ST_SHIFT;
                    DNA_Read <= '1';
                    Shift_Cnt <= conv_std_logic_vector(33, Shift_Cnt'length);

                end if;

            when ST_SHIFT =>

                DNA_Shift <= '1';
                Shift_Cnt <= Shift_Cnt + 1;

                if (Shift_Cnt(Shift_Cnt'high) = '1') then

                    State <= ST_LOAD;
                    Done_i <= not Done_i;

                end if;

            when others =>

                null;

        end case;

        if (DNA_Shift = '1') then

            DNA_i <= DNA_Dout & DNA_i(DNA_i'HIGH downto 1);

        end if;

    end if;

end process;


DNA_96      <= DNA_i;
Done_Tog    <= Done_i;

DNA_INST: DNA_PORTE2
    generic map (
        SIM_DNA_VALUE => SIM_DNA_VALUE
    )
    port map (

        Clk   => Clk,

        Read  => DNA_Read,
        Shift => DNA_Shift,

        Din   => '0',
        Dout  => DNA_Dout
    );

end rtl;
