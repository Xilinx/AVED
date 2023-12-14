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

entity gt_prbs_25 is
    generic (
        C_GT_IP_SEL         : integer := 0;
        DEST_SYNC_FF        : integer := 4
    );
    port (
        Rx_Usr_Clk          : in    std_logic;
        Tx_Usr_Clk          : in    std_logic;

        Tx_Enable           : in    std_logic;
        Tx_Error_Inject     : in    std_logic;

        Rx_Enable           : in    std_logic;
        Clear_Rx_PRBS_Error : in    std_logic;
        Clear_Rx_Seed_Zero  : in    std_logic;
        Clear_Tx_Seed_Zero  : in    std_logic;

        Disable_PRBS_Ref    : in    std_logic;

        -- Rx Traffic Interface

        Rx_Data         : in    std_logic_vector(63 downto 0);
        Rx_Header       : in    std_logic_vector(1 downto 0);

        Rx_Slip         : out   std_logic   := '0';

        Rx_PRBS_Error   : out   std_logic   := '0';
        Rx_Seed_Zero    : out   std_logic   := '0';

        Rx_Word_Cnt     : out   std_logic_vector(47 downto 0);
        Rx_Err_Word_Cnt : out   std_logic_vector(47 downto 0);

        -- Tx Traffic Interface

        Tx_Data         : out   std_logic_vector(63 downto 0);
        Tx_Header       : out   std_logic_vector(1 downto 0);

        Tx_Seed_Zero    : out std_logic := '0'
    );
end entity gt_prbs_25;

architecture rtl of gt_prbs_25 is

-------------------------------------------------------------------------------
--
--      Constants (simple)
--
-------------------------------------------------------------------------------

constant COUNT_MODE             : boolean := false;
constant FIXED_VALUE            : boolean := false;

constant RAW_PRBS               : boolean := true;

constant ZERO   : std_logic_vector(63 downto 0) := (others => '0');
constant ONE    : std_logic_vector(0 downto 0)  := (others => '1');

constant SLIP_TEST_SH               : std_logic_vector(0 downto 0)  := "0";
constant SLIP_ST_SLIP               : std_logic_vector(0 downto 0)  := "1";

constant PRBS_SEED_31       : std_logic_vector(30 downto 0)         := (30 => '1', 27 => '1', others => '0'); -- X"48000000"
constant C_RST_PRBS_SEED    : std_logic_vector(PRBS_SEED_31'range)  := (others => '1');

type PRBS_Data_Type is array (integer range <>) of std_logic_vector(PRBS_SEED_31'RANGE);
type    std_logic_2d_3      is array (integer range <>) of std_logic_vector( 2 downto 0);
type    std_logic_2d_4      is array (integer range <>) of std_logic_vector( 3 downto 0);
type    std_logic_2d_5      is array (integer range <>) of std_logic_vector( 4 downto 0);
type    std_logic_2d_66     is array (integer range <>) of std_logic_vector(65 downto 0);

function fn_if(Test : boolean; Arg1 : integer; Arg2 : integer) return integer is
    begin

        if (Test) then

            return Arg1;

        else

            return Arg2;

        end if;

end function fn_if;

-- in RAW_PRBS mode data is on all available bits data + header
constant DATA_SIZE  : integer := fn_if(RAW_PRBS, Tx_Data'length + Tx_Header'length, Tx_Data'length);

function fn_Init_Poly_2_Data(CRC_POLY : in std_logic_vector; DATA_WIDTH : in integer) return PRBS_Data_Type is
    variable Res        : PRBS_Data_Type(DATA_WIDTH-1 downto 0) := (others => (others => '0'));
    variable Poly_var   : std_logic_vector(CRC_POLY'RANGE);
    begin

        -- For each bit in the input CRC, calculate the output CRC bits that it affects

        for n in CRC_POLY'RANGE loop

            -- Initialise the CRC from this bit

            Poly_var     := (others => '0');
            Poly_var(n)  := '1';

            -- Loop for each data bit

            for m in DATA_WIDTH-1 downto 0 loop

                Poly_var    := Poly_var(Poly_var'HIGH-1 downto 0) & xor_reduce(Poly_var and CRC_POLY);

                Res(m)(n)   := Poly_var(0);

            end loop;

        end loop;

        return Res;

end function fn_Init_Poly_2_Data;

constant PRBS_CHECK         : PRBS_Data_Type(DATA_SIZE-1 downto 0)       := fn_Init_Poly_2_Data(PRBS_SEED_31, DATA_SIZE);

-------------------------------------------------------------------------------
--
--      Type Definitions
--
-------------------------------------------------------------------------------

type In_Pipe_Type is
    record
        Data            : std_logic_vector(DATA_SIZE-1 downto 0);
        Enable          : std_logic;
        PRBS_Test       : std_logic_vector(DATA_SIZE-1 downto 0);
        PRBS_Error      : std_logic_vector(10 downto 0);
        PRBS_Error_Cnt  : std_logic_2d_3(10 downto 0);
        Error           : std_logic;
        Clear_Cnt       : std_logic;
    end record;

type In_Pipe_Array_Type is array (integer range <>) of In_Pipe_Type;

constant RST_IN_PIPE_TYPE : In_Pipe_Type := (
    (others => '0'),
    '0',
    (others => '0'),
    (others => '0'),
    (others => (others => '0')),
    '0',
    '0'
    );

type Out_Pipe_Type is
    record
        Enable          : std_logic;
        Header          : std_logic_vector(1 downto 0);
        Data            : std_logic_vector(Tx_Data'range);
        PRBS_Seed       : std_logic_vector(PRBS_SEED_31'range);
    end record;

type Out_Pipe_Array_Type is array (integer range <>) of Out_Pipe_Type;

constant RST_OUT_PIPE_TYPE : Out_Pipe_Type := (
    '0',
    (others => '0'),
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

function fn_Log2(Arg : natural) return natural is
    variable Result : natural := 0;
    variable Work   : natural := natural(Arg) - 1;
begin

    if (Work /= 0) then

        while (Work /= 0) loop

            Work := Work / 2;
            Result  := Result + 1;

        end loop;

    end if;

    return Result;

end function fn_Log2;

function fn_Reverse_Bits(vector : std_logic_vector) return std_logic_vector is
    variable result : std_logic_vector(vector'LENGTH-1 downto 0);
    begin

        for i in result'range loop

            result(i) := vector(vector'HIGH-i);

        end loop;

        return result;

end function fn_Reverse_Bits;


function fn_Count_Ones(Input : std_logic_vector) return std_logic_vector is
    variable Cnt    : integer := 0;
    variable Res    : std_logic_vector(fn_Log2(Input'LENGTH+1)-1 downto 0)  := (others => 'X');
begin

    for n in Input'RANGE loop

        if (Input(n) = '1') then

        Cnt := Cnt + 1;

        end if;

    end loop;

    Res := conv_std_logic_vector(Cnt, Res'LENGTH);
    return Res;

end function fn_Count_Ones;

-------------------------------------------------------------------------------
--
--      Signals
--
-------------------------------------------------------------------------------


signal  PRBS_Lock          : std_logic                     := '0';

signal  In_Pipe         : In_Pipe_Array_Type(0 to 6)    := (others => RST_IN_PIPE_TYPE);

signal  Out_Pipe        : Out_Pipe_Array_Type(0 to 2)   := (others => RST_OUT_PIPE_TYPE);

signal  Rx_En_cdc       : std_logic                     := '0';
signal  Tx_En_cdc       : std_logic                     := '0';

signal  Tx_Error_Inject_cdc     : std_logic            := '0';
signal  Tx_Error_Inject_d1      : std_logic            := '0';

signal Clear_Rx_PRBS_Error_cdc  : std_logic            := '0';
signal Clear_Rx_PRBS_Error_d1   : std_logic            := '0';
signal Clear_Rx_Seed_Zero_cdc   : std_logic            := '0';
signal Clear_Rx_Seed_Zero_d1    : std_logic            := '0';
signal Clear_Tx_Seed_Zero_cdc   : std_logic            := '0';
signal Clear_Tx_Seed_Zero_d1    : std_logic            := '0';

signal Disable_PRBS_Ref_cdc     : std_logic            := '0';

signal Tx_PRBS_Seed         : std_logic_vector(PRBS_SEED_31'range)  := C_RST_PRBS_SEED;
signal Tx_Seed_Zero_i       : std_logic                             := '0';

signal Rx_PRBS_Seed         : std_logic_vector(PRBS_SEED_31'range)  := C_RST_PRBS_SEED;
signal Rx_Ref_PRBS_Seed     : std_logic_vector(PRBS_SEED_31'range)  := C_RST_PRBS_SEED;

signal Rx_CDC_In            : std_logic_vector(3 downto 0)          := (others => '0');
signal Rx_CDC_Out           : std_logic_vector(Rx_CDC_In'range)     := (others => '0');
signal Tx_CDC_In            : std_logic_vector(2 downto 0)          := (others => '0');
signal Tx_CDC_Out           : std_logic_vector(Tx_CDC_In'range)     := (others => '0');

signal Rx_Word_Cnt_i        : std_logic_vector(47 downto 0)         := (others => '0');
signal Rx_Err_Word_Cnt_i    : std_logic_vector(47 downto 0)         := (others => '0');


signal PRBS_Error_Cnt_Acc_1 : std_logic_2d_4(4 downto 0)           := (others => (others => '0'));
signal PRBS_Error_Cnt_Acc_2 : std_logic_2d_5(2 downto 0)           := (others => (others => '0'));
signal PRBS_Error_Cnt_Acc  : std_logic_vector(6 downto 0)          := (others => '0');

-- synthesis translate_off
signal dbg_Rx_Data          : std_logic_vector(DATA_SIZE-1 downto 0)    := (others => '0');
signal dbg_Tx_Data          : std_logic_vector(DATA_SIZE-1 downto 0)    := (others => '0');
signal dbg_rx_shift_reg     : std_logic_2d_66(65 downto 0)              := (others => (others => '0'));
-- synthesis translate_on


begin

    Rx_CDC_In <= Disable_PRBS_Ref & Rx_Enable & Clear_Rx_PRBS_Error & Clear_Rx_Seed_Zero ;

    Rx_CDC: xpm_cdc_array_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF,
        SRC_INPUT_REG  => 0,
        WIDTH          => Rx_CDC_In'length
    )
    port map (
        src_clk        => '0',
        src_in         => Rx_CDC_In,
        dest_clk       => Rx_Usr_Clk,
        dest_out       => Rx_CDC_Out
    );

    Disable_PRBS_Ref_cdc  <= Rx_CDC_Out(3);
    Rx_En_cdc <= Rx_CDC_Out(2);
    Clear_Rx_PRBS_Error_cdc <= Rx_CDC_Out(1);
    Clear_Rx_Seed_Zero_cdc <= Rx_CDC_Out(0);

    In_Pipe(0).Enable   <= Rx_En_cdc;

    -- versal bit ordering is different from VUp (am002 vs. ug578)
    --    Both sends first header word then data word
    --      VUp: Header (MSB to LSB) then data (MSB to LSB)
    --      Versal: Header (LSB to MSB) followed by Data (LSB to MSB)
    --

    In_Pipe(0).Data     <= (fn_Reverse_Bits(Rx_Header) & fn_Reverse_Bits(Rx_Data)) when C_GT_IP_SEL = 1 else (Rx_Header & Rx_Data);

-- synthesis translate_off
BitShift:
process(Rx_Usr_Clk)
    variable shift_reg_var : std_logic_vector(2*DATA_SIZE-1 downto 0);
begin

    if (rising_edge(Rx_Usr_Clk)) then
        shift_reg_var := In_Pipe(1).Data & In_Pipe(0).Data;
        for i in 0 to dbg_rx_shift_reg'high loop
            dbg_rx_shift_reg(i) <= shift_reg_var(shift_reg_var'high-i downto shift_reg_var'high-i-(DATA_SIZE-1));
        end loop;
    end if;
end process;
-- synthesis translate_on

WQ:
process(Rx_Usr_Clk)
    variable PRBS_var       : std_logic_vector(PRBS_SEED_31'range);
    variable OR_Reducer_var : std_logic;
begin

    if (rising_edge(Rx_Usr_Clk)) then

        -- useful for testing
        -- deposit a '1' in simulation to create lots of error resulting a PRBS out of lock
        --   check if error accumulation pipeline works and if PRBS re-aquires lock
        Rx_Slip     <= '0';

        ---------------------------------------------------
        -- Pipeline
        ---------------------------------------------------
        In_Pipe(1 to In_Pipe'HIGH)  <= In_Pipe(0 to In_Pipe'HIGH-1);


        ---------------------------------------------------
        -- Stage 1: create expected PRBS
        ---------------------------------------------------
        if (In_Pipe(0).Enable = '1') then

            Rx_PRBS_Seed <= In_Pipe(0).Data(PRBS_SEED_31'range);

            if PRBS_Lock = '1' and Disable_PRBS_Ref_cdc /= '1' then

                for n in PRBS_SEED_31'range loop

                    Rx_Ref_PRBS_Seed(n) <= xor_reduce(PRBS_CHECK(n) and Rx_Ref_PRBS_Seed);

                end loop;

            else

                Rx_Ref_PRBS_Seed <= In_Pipe(0).Data(PRBS_SEED_31'range);

            end if;

        end if;


        for n in DATA_SIZE-1 downto 0 loop

            In_Pipe(1).PRBS_Test(n) <= xor_reduce(PRBS_CHECK(n) and Rx_Ref_PRBS_Seed);

        end loop;

        -- pseudo-code equivalent
        -- PRBS_var := Rx_PRBS_Seed;
        -- for n in 63 downto 0 loop
        --     -- generate the Test Word
        --     In_Pipe(2).PRBS_Test(n)  <=     xor_reduce(PRBS_var and PRBS_SEED_31);
        --     -- Update the Seed
        --     PRBS_var := PRBS_var(PRBS_var'HIGH-1 downto 0) & not In_Pipe(1).Data(n);
        -- end loop;
        -- Rx_PRBS_Seed <= PRBS_var;

        if nor_reduce(Rx_PRBS_Seed) = '1' then
            Rx_Seed_Zero <= '1';
        end if;

        Clear_Rx_Seed_Zero_d1 <= Clear_Rx_Seed_Zero_cdc;
        if (Clear_Rx_Seed_Zero_d1 /= Clear_Rx_Seed_Zero_cdc) then
            Rx_Seed_Zero    <= '0';
        end if;

        if (COUNT_MODE = true) then
            Rx_PRBS_Seed <= In_Pipe(0).Data(62 downto 32) + 1;
            In_Pipe(1).PRBS_Test(30 downto 0) <= Rx_PRBS_Seed;
            In_Pipe(1).PRBS_Test(31) <= '0';
            In_Pipe(1).PRBS_Test(62 downto 32) <= Rx_PRBS_Seed + 1;
            In_Pipe(1).PRBS_Test(63) <= '0';
            Rx_Seed_Zero    <= '0';
        end if;


        ---------------------------------------------------
        -- Stage 2: compare data with expected PRBS
        ---------------------------------------------------
        In_Pipe(2).PRBS_Test <= In_Pipe(1).PRBS_Test xor In_Pipe(1).Data;


        ---------------------------------------------------
        -- Stage 3: Reduce the PRBS errors using LUT6s: 64 bits reduced into 11 bits
        ---------------------------------------------------
        OR_Reducer_var  := '0';

        for n in DATA_SIZE-1 downto 0 loop

            -- "aggregate" error over 6 bits
            OR_Reducer_var  := OR_Reducer_var or In_Pipe(2).PRBS_Test(n);

            -- every 6 bits store the accumlation result and reset the accumulator
            if ((n mod 6) = 0) then

                In_Pipe(3).PRBS_Error(n / 6) <= OR_Reducer_var;

                OR_Reducer_var  := '0';

            end if;

        end loop;

        for n in 10 downto 0 loop
            -- "count" error over 6 bits
            In_Pipe(3).PRBS_Error_Cnt(n) <= fn_Count_Ones( In_Pipe(2).PRBS_Test(n*6+5 downto n*6) );

        end loop;


        ---------------------------------------------------
        -- Stage 4: Reduce the PRBS errors using LUT6s: 11bits reduced to 2
        ---------------------------------------------------
        OR_Reducer_var  := '0';

        for n in In_Pipe(3).PRBS_Error'range loop

            -- "aggregate" error over 6 bits
            OR_Reducer_var  := OR_Reducer_var or In_Pipe(3).PRBS_Error(n);

            -- every 6 bits store the accumlation result and reset the accumulator
            if ((n mod 6) = 0) then

                In_Pipe(4).PRBS_Error(n / 6) <= OR_Reducer_var;

                OR_Reducer_var  := '0';

            end if;

        end loop;

        -- "accumalate" error counters per pair
        -- 10 error cnt accumalated 2 by 2. MSB error counter will be added later
        for n in 4 downto 0 loop

            PRBS_Error_Cnt_Acc_1(n) <= ext(In_Pipe(3).PRBS_Error_Cnt(2*n+1),4) + ext(In_Pipe(3).PRBS_Error_Cnt(2*n),4);

        end loop;


        ---------------------------------------------------
        -- Stage 5: final reduction
        ---------------------------------------------------
        if (In_Pipe(4).Enable = '1') then

            if or_reduce(In_Pipe(4).PRBS_Error(1 downto 0)) = '1' then
                Rx_PRBS_Error <= '1';
                In_Pipe(5).Error <= '1';
            end if;

        end if;

        Clear_Rx_PRBS_Error_d1 <= Clear_Rx_PRBS_Error_cdc;
        if (Clear_Rx_PRBS_Error_cdc /= Clear_Rx_PRBS_Error_d1) then
            Rx_PRBS_Error <= '0';
            In_Pipe(5).Clear_Cnt <= '1';
        end if;

        -- "accumalate" previously added pairs & include top error counter
        PRBS_Error_Cnt_Acc_2(2) <= ext(PRBS_Error_Cnt_Acc_1(4),5) + ext( In_Pipe(4).PRBS_Error_Cnt(10),5);
        for n in 1 downto 0 loop

            PRBS_Error_Cnt_Acc_2(n) <= ext(PRBS_Error_Cnt_Acc_1(2*n+1),5) + ext(PRBS_Error_Cnt_Acc_1(2*n),5);

        end loop;


        ---------------------------------------------------
        -- Stage 6: BERT counters
        ---------------------------------------------------
        if (In_Pipe(5).Enable = '1') then
            Rx_Word_Cnt_i <= Rx_Word_Cnt_i + 1;

            if (In_Pipe(5).Error = '1') then
                Rx_Err_Word_Cnt_i <= Rx_Err_Word_Cnt_i + 1;
            end if;

        end if;

        if (In_Pipe(5).Clear_Cnt = '1' or In_Pipe(5).Enable /= '1') then
            Rx_Word_Cnt_i       <= (others => '0');
            Rx_Err_Word_Cnt_i   <= (others => '0');
        end if;

        -- final stage of error accumulation
        PRBS_Error_Cnt_Acc <= ext(PRBS_Error_Cnt_Acc_2(2),7) + ext(PRBS_Error_Cnt_Acc_2(1),7) + ext(PRBS_Error_Cnt_Acc_2(0),7);


        ---------------------------------------------------
        -- Stage 7: PRBS_Lock
        ---------------------------------------------------
        if ( (PRBS_Error_Cnt_Acc <= 6) and (In_Pipe(6).Enable = '1') ) then
            PRBS_Lock <= '1';
        else
            PRBS_Lock <= '0';
        end if;

    end if;

end process;

Rx_Word_Cnt     <= Rx_Word_Cnt_i;
Rx_Err_Word_Cnt <= Rx_Err_Word_Cnt_i;


Tx_CDC_In <= Tx_Enable & Clear_Tx_Seed_Zero & Tx_Error_Inject;

Tx_CDC: xpm_cdc_array_single
generic map (
    DEST_SYNC_FF   => DEST_SYNC_FF,
    SRC_INPUT_REG  => 0,
    WIDTH          => Tx_CDC_In'length
)
port map (
    src_clk        => '0',
    src_in         => Tx_CDC_In,
    dest_clk       => Tx_Usr_Clk,
    dest_out       => Tx_CDC_Out
);

Tx_En_cdc               <= Tx_CDC_Out(2);
Clear_Tx_Seed_Zero_cdc  <= Tx_CDC_Out(1);
Tx_Error_Inject_cdc     <= Tx_CDC_Out(0);


Tx_Seed_Zero <= Tx_Seed_Zero_i;

Out_Pipe(0).Enable <= Tx_En_cdc;

RQ:
process(Tx_Usr_Clk)
    variable tx_data_var    : std_logic_vector(DATA_SIZE-1 downto 0);
    variable Tx_Seed_Zero_var : std_logic;

begin

    if (rising_edge(Tx_Usr_Clk)) then

        ---------------------------------------------------
        -- Pipeline
        ---------------------------------------------------

        Out_Pipe(1 to Out_Pipe'HIGH)  <= Out_Pipe(0 to Out_Pipe'HIGH-1);


        if Out_Pipe(0).Enable = '1' then

            Out_Pipe(1).PRBS_Seed   <= Tx_PRBS_Seed;

            for n in Tx_PRBS_Seed'RANGE loop
                Tx_PRBS_Seed(n)  <= xor_reduce(PRBS_CHECK(n) and Tx_PRBS_Seed);
            end loop;

        end if;

        -- Check for all zero seed & reset seed if detected
        Tx_Seed_Zero_var := nor_reduce(Tx_PRBS_Seed);
        if (Tx_Seed_Zero_var = '1') then

            Tx_PRBS_Seed    <= C_RST_PRBS_SEED;

        end if;

        if Tx_Seed_Zero_var = '1' then
            Tx_Seed_Zero_i    <= '1';
        end if;

        Clear_Tx_Seed_Zero_d1 <= Clear_Tx_Seed_Zero_cdc;
        if Clear_Tx_Seed_Zero_d1 /= Clear_Tx_Seed_Zero_cdc then
            Tx_Seed_Zero_i <= '0';
        end if;

        if (COUNT_MODE = true) then
            if Out_Pipe(0).Enable = '1' then
                Tx_PRBS_Seed <= Tx_PRBS_Seed + 2;
                Tx_Seed_Zero_i <= '0';
            end if;
        end if;


        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------
        Tx_Error_Inject_d1 <= Tx_Error_Inject_cdc;

        if (COUNT_MODE = true) then
            Out_Pipe(2).Data(30 downto 0) <= Tx_PRBS_Seed;
            Out_Pipe(2).Data(31) <= '0';
            Out_Pipe(2).Data(62 downto 32) <= Tx_PRBS_Seed + 1;
            Out_Pipe(2).Data(63) <= '0';
        end if;

        if (RAW_PRBS = true) then

            -- PRBS word generated from MSB to LSB
            for n in tx_data_var'range loop
                -- inject a single error
                if (n = tx_data_var'high) and (Tx_Error_Inject_d1 /= Tx_Error_Inject_cdc) then
                    tx_data_var(n)  := not (xor_reduce(PRBS_CHECK(n) and Out_Pipe(1).PRBS_Seed) );
                else
                    tx_data_var(n)  := xor_reduce(PRBS_CHECK(n) and Out_Pipe(1).PRBS_Seed);
                end if;
            end loop;

            -- synthesis translate_off
            dbg_Tx_Data <= tx_data_var;
            -- synthesis translate_on

            -- versal bit ordering is different from VUp (am002 vs. ug578)
            --    Both sends first header word then data word
            --      VUp: Header (MSB to LSB) then data (MSB to LSB)
            --      Versal: Header (LSB to MSB) followed by Data (LSB to MSB)
            --
            if C_GT_IP_SEL = 1 then
                Out_Pipe(2).Data    <= fn_Reverse_Bits(tx_data_var(Tx_Data'range));
                Out_Pipe(2).Header  <= fn_Reverse_Bits(tx_data_var(DATA_SIZE-1 downto Tx_Data'length));
            else
                Out_Pipe(2).Data    <= tx_data_var(Tx_Data'range);
                Out_Pipe(2).Header  <= tx_data_var(DATA_SIZE-1 downto Tx_Data'length);
            end if;

        end if;

        if (FIXED_VALUE = true) then
            Out_Pipe(2).Data    <= X"FFFF0007000B0003";
            Out_Pipe(2).Header  <= "01";
        end if;


    end if;

end process;

    Tx_Data     <= Out_Pipe(Out_Pipe'HIGH).Data;
    Tx_Header   <= Out_Pipe(Out_Pipe'HIGH).Header;

end architecture rtl;
