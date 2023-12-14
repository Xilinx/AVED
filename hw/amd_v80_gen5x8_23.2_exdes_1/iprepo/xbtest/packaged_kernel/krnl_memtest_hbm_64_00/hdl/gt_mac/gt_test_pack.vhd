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
    use ieee.std_logic_misc.all;

-- ---------------------------------------------------------------------------------------------------------------------

package gt_test_pack is

-------------------------------------------------------------------------------
--      Constants
-------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------
--  Types
------------------------------------------------------------------------------------------------------------------------

type    Real_Array          is array (integer range <>) of real;

type    std_logic_2d_2      is array (integer range <>) of std_logic_vector(  1 downto 0);
type    std_logic_2d_4      is array (integer range <>) of std_logic_vector(  3 downto 0);
type    std_logic_2d_5      is array (integer range <>) of std_logic_vector(  4 downto 0);
type    std_logic_2d_7      is array (integer range <>) of std_logic_vector(  6 downto 0);
type    std_logic_2d_12     is array (integer range <>) of std_logic_vector( 11 downto 0);
type    std_logic_2d_14     is array (integer range <>) of std_logic_vector( 13 downto 0);
type    std_logic_2d_16     is array (integer range <>) of std_logic_vector( 15 downto 0);
type    std_logic_2d_32     is array (integer range <>) of std_logic_vector( 31 downto 0);
type    std_logic_2d_48     is array (integer range <>) of std_logic_vector( 47 downto 0);
type    std_logic_2d_64     is array (integer range <>) of std_logic_vector( 63 downto 0);

--------------------------------------------------------------------------------------------------------------------------
----  Function Prototypes
--------------------------------------------------------------------------------------------------------------------------

function fn_Log2(Arg : natural) return natural;
function fn_if(Test : boolean; Arg1 : real; Arg2 : real) return real;
function fn_Count_Ones(Input : std_logic_vector) return integer;
function fn_Get_Index (Data : in std_logic_2d_12; Search : in std_logic_vector) return integer;

end package gt_test_pack;

------------------------------------------------------------------------------------------------------------------------
--  Package Body
------------------------------------------------------------------------------------------------------------------------

package body gt_test_pack is

--------------------------------------------------------------------------------------------------------------------------
----  Functions
--------------------------------------------------------------------------------------------------------------------------

function fn_Log2(Arg : natural) return natural is
  variable Result : natural := 0;
  variable Work   : natural := natural(Arg) - 1;
begin

  assert Arg >= 1
  report "Argument in call to fn_Log2 is <= 0. " &
         "This is no longer supported and will be removed in the future. " &
         "Please update the code."
  severity warning;

    -- While loop only entered if Work is non-zero to avoid an XST warning

    if (Work /= 0) then

        while (Work /= 0) loop

            Work := Work / 2;
            Result  := Result + 1;

        end loop;

    end if;

    return Result;

end function fn_Log2;

function fn_if(Test : boolean; Arg1 : real; Arg2 : real) return real is
begin

    if (Test) then

        return Arg1;

    else

        return Arg2;

    end if;

end function fn_if;


function fn_Count_Ones(Input : std_logic_vector) return integer is
    variable Result : integer := 0;
begin

    for n in Input'RANGE loop

        if (Input(n) = '1') then

            Result := Result + 1;

        end if;

    end loop;

    return Result;

end function fn_Count_Ones;

function fn_Get_Index (Data : in std_logic_2d_12; Search : in std_logic_vector) return integer is
variable Res    : integer   := -1;
begin

    for n in Data'RANGE loop

        if (Data(n) = Search) then

            Res := n;

        end if;

    end loop;

    return Res;

end function fn_Get_Index;


end package body gt_test_pack;
