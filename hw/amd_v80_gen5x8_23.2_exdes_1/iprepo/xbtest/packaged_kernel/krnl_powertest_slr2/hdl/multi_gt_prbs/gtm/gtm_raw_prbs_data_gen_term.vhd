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

entity gtm_raw_prbs_data_gen_term is
    generic (
        DEST_SYNC_FF        : integer := 4;
        DATA_SIZE           : integer := 128  -- only tested with 128 bits
    );
    port (
        Rx_Usr_Clk          : in    std_logic;
        Tx_Usr_Clk          : in    std_logic;

        Tx_Error_Inject     : in    std_logic;

        Rx_Enable           : in    std_logic;
        clear_status        : in    std_logic;

        Disable_PRBS_Ref    : in    std_logic;

        latch_status        : in    std_logic;
        toggle_1_sec        : in    std_logic;

        -- Rx Traffic Interface

        Rx_Data         : in    std_logic_vector(DATA_SIZE-1 downto 0);

        Rx_PRBS_Error   : out   std_logic   := '0';
        Rx_Seed_Zero    : out   std_logic   := '0';
        Rx_Word_Cnt     : out   std_logic_vector(47 downto 0);
        Rx_Err_Bit_Cnt  : out   std_logic_vector(47 downto 0);
        Tx_Word_Cnt     : out   std_logic_vector(47 downto 0);

        -- Tx Traffic Interface

        Tx_Data         : out   std_logic_vector(DATA_SIZE-1 downto 0);
        Tx_Seed_Zero    : out std_logic := '0'

    );
end entity gtm_raw_prbs_data_gen_term;

architecture rtl of gtm_raw_prbs_data_gen_term is

-------------------------------------------------------------------------------
--
--      Constants (simple)
--
-------------------------------------------------------------------------------

-- these constant can't be set at true simultaneously
constant COUNT_MODE             : boolean := false; -- only tested with 40bits
constant FIXED_VALUE            : boolean := false; -- only tested with 40bits
constant RAW_PRBS               : boolean := true;

constant PRBS_SEED_31       : std_logic_vector(30 downto 0)         := (30 => '1', 27 => '1', others => '0'); -- X"48000000"
constant C_RST_PRBS_SEED    : std_logic_vector(PRBS_SEED_31'range)  := (others => '1');

type PRBS_Data_Type is array (integer range <>) of std_logic_vector(PRBS_SEED_31'RANGE);
type    std_logic_2d_3      is array (integer range <>) of std_logic_vector( 2 downto 0);
type    std_logic_2d_4      is array (integer range <>) of std_logic_vector( 3 downto 0);
type    std_logic_2d_5      is array (integer range <>) of std_logic_vector( 4 downto 0);
type    std_logic_2d_6      is array (integer range <>) of std_logic_vector( 5 downto 0);

function fn_if(Test : boolean; Arg1 : integer; Arg2 : integer) return integer is
    begin

        if (Test) then

            return Arg1;

        else

            return Arg2;

        end if;

end function fn_if;

function fn_if(Test : boolean; Arg1 : boolean; Arg2 : boolean) return boolean is
    begin

        if (Test) then

            return Arg1;

        else

            return Arg2;

        end if;

end function fn_if;

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
        Data                : std_logic_vector(DATA_SIZE-1 downto 0);
        Enable              : std_logic;
        PRBS_Test           : std_logic_vector(DATA_SIZE-1 downto 0);
        PRBS_Error          : std_logic_vector(DATA_SIZE/6 downto 0);     -- 128bits data requires 22 or_reduction (each or_reduce is done on 6 bits)
        PRBS_Error_Cnt      : std_logic_2d_3(DATA_SIZE/6 downto 0);
        PRBS_Seed           : std_logic_vector(PRBS_SEED_31'range);
        PRBS_Seed_Error     : std_logic_vector(5 downto 0);
        PRBS_Zero_Seed      : std_logic;
        PRBS_Too_Many_Error : std_logic;
        Error               : std_logic;
    end record;

type In_Pipe_Array_Type is array (integer range <>) of In_Pipe_Type;

constant RST_IN_PIPE_TYPE : In_Pipe_Type := (
    (others => '0'),
    '0',
    (others => '0'),
    (others => '0'),
    (others => (others => '0')),
    PRBS_SEED_31,
    (others => '0'),
    '0',
    '0',
    '0'
    );

type Out_Pipe_Type is
    record
        Enable          : std_logic;
        Data            : std_logic_vector(DATA_SIZE-1 downto 0);
        PRBS_Seed       : std_logic_vector(PRBS_SEED_31'range);
        PRBS_Seed_Error : std_logic_vector(5 downto 0);
        PRBS_Zero_Seed  : std_logic;
    end record;

type Out_Pipe_Array_Type is array (integer range <>) of Out_Pipe_Type;

constant RST_OUT_PIPE_TYPE : Out_Pipe_Type := (
    '0',
    (others => '0'),
    PRBS_SEED_31,
    (others => '0'),
    '0'
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

function fn_Inverse_Bits(vector : std_logic_vector; ctrl : std_logic_vector) return std_logic_vector is
    variable result : std_logic_vector(vector'LENGTH-1 downto 0);
    begin

        for i in result'range loop

            if (ctrl(i) = '1') then
                result(i) := not vector(i);
            else
                result(i) := vector(i);
            end if;

        end loop;

        return result;

    end function fn_Inverse_Bits;

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

signal  In_Pipe         : In_Pipe_Array_Type(0 to 8)    := (others => RST_IN_PIPE_TYPE);

signal  Out_Pipe        : Out_Pipe_Array_Type(0 to 3)   := (others => RST_OUT_PIPE_TYPE);

signal  Rx_En_cdc       : std_logic                     := '0';
signal  Tx_En_cdc       : std_logic                     := '0';

signal toggle_1_sec_rx_cdc    : std_logic                 := '0';
signal toggle_1_sec_tx_cdc    : std_logic                 := '0';
signal toggle_1_sec_rx_cdc_d  : std_logic                 := '0';
signal toggle_1_sec_tx_cdc_d  : std_logic                 := '0';

signal Tx_Error_Inject_cdc      : std_logic            := '0';
signal Tx_Error_Inject_d1       : std_logic            := '0';

signal clear_status_rx_cdc      : std_logic            := '0';
signal clear_status_rx_d1       : std_logic            := '0';
signal clear_status_tx_cdc      : std_logic            := '0';
signal clear_status_tx_d1       : std_logic            := '0';

signal latch_status_tx_cdc      : std_logic            := '0';
signal latch_status_tx_d        : std_logic            := '0';

signal Disable_PRBS_Ref_cdc     : std_logic            := '0';

signal Tx_PRBS_Seed         : std_logic_vector(PRBS_SEED_31'range)  := C_RST_PRBS_SEED;
signal Tx_Seed_Zero_i       : std_logic                             := '0';
signal Rst_Tx_Seed          : std_logic                             := '0';

signal Rx_PRBS_Seed         : std_logic_vector(PRBS_SEED_31'range)  := C_RST_PRBS_SEED;

signal Rx_CDC_In            : std_logic_vector(3 downto 0)          := (others => '0');
signal Rx_CDC_Out           : std_logic_vector(Rx_CDC_In'range)     := (others => '0');
signal Tx_CDC_In            : std_logic_vector(3 downto 0)          := (others => '0');
signal Tx_CDC_Out           : std_logic_vector(Tx_CDC_In'range)     := (others => '0');

signal Word_Cnt             : std_logic_vector(4 downto 0)          := (others => '0');

signal Rx_Cnt_Rec           : std_logic := '0';
signal Rx_Word_Cnt_i        : std_logic_vector(47 downto 0)         := (others => '0');
signal Rx_Err_Bit_Cnt_i     : std_logic_vector(47 downto 0)         := (others => '0');

-- 128 DATA_SIZE bits;
--   error accumulated per 6 bits into 22 3-bit counters
--   Acc_1:       22 3-bit counters added 2 by 2 into 11 4-bit counters
--   Acc_2:       11 4-bit counters added 2 by 2 into  6 5-bit counters => no accummulation for 11th counter, it is "passed" to next stage in 6th counter
--   Acc_3:        6 5-bit counters added 2 by 2 into  3 6-bit counters
--   final stage:  3 6-bit counters added        into  1 8-bit counters

signal PRBS_Error_Cnt_Acc_1 : std_logic_2d_4(10 downto 0)          := (others => (others => '0'));
signal PRBS_Error_Cnt_Acc_2 : std_logic_2d_5(5 downto 0)           := (others => (others => '0'));
signal PRBS_Error_Cnt_Acc_3 : std_logic_2d_6(2 downto 0)           := (others => (others => '0'));
signal PRBS_Error_Cnt_Acc   : std_logic_vector(7 downto 0)         := (others => '0');

signal Tx_Cnt_Rec           : std_logic := '0';
signal Tx_Word_Cnt_i        : std_logic_vector(47 downto 0)         := (others => '0');
signal Tx_Word_Cnt_sec      : std_logic_vector(47 downto 0)         := (others => '0');

-- synthesis translate_off
signal dbg_PRBS_Error_Cnt   : std_logic_vector(fn_Log2(DATA_SIZE+1)-1 downto 0 ) := (others => '0');
signal dbg_Rx_Word_Cnt      : std_logic_vector(47 downto 0)         := (others => '0');
signal dbg_Rx_Word_Diff     : std_logic_vector(47 downto 0)         := (others => '0');

signal dbg_Rx_PRBS_Seed     : std_logic_vector(PRBS_SEED_31'range)      := C_RST_PRBS_SEED;
signal dbg_PRBS_Test        : std_logic_vector(DATA_SIZE-1 downto 0)    := (others => '0');
-- synthesis translate_on

signal dbg_Bit_Error_Injection : std_logic_vector(DATA_SIZE-1 downto 0) := (others => '0');
signal Rx_Data_In: std_logic_vector(DATA_SIZE-1 downto 0) := (others => '0');

begin

    Rx_CDC_In <=  toggle_1_sec & Disable_PRBS_Ref & Rx_Enable & clear_status;

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

    toggle_1_sec_rx_cdc     <= Rx_CDC_Out(3);
    Disable_PRBS_Ref_cdc    <= Rx_CDC_Out(2);
    Rx_En_cdc               <= Rx_CDC_Out(1);
    clear_status_rx_cdc     <= Rx_CDC_Out(0);



    In_Pipe(0).Enable   <= Rx_En_cdc;
    -- PRBS checker works from MSB to LSB, but raw mode sends data from LSB to MSB
    Rx_Data_In <=  fn_Reverse_Bits(Rx_Data);
    -- allow precise bit error injection for simulation
    In_Pipe(0).Data     <= fn_Inverse_Bits(Rx_Data_In, dbg_Bit_Error_Injection);

RX:
process(Rx_Usr_Clk)
    variable PRBS_var       : std_logic_vector(PRBS_SEED_31'range);
    variable OR_Reducer_var : std_logic;
    variable PRBS_Test_var  : std_logic_vector(DATA_SIZE-1 downto 0);
    variable Temp_Test      : std_logic_vector((DATA_SIZE/6 + 1)*6 - 1 downto 0); -- size: next multiple of 6 bigger than DATA_SIZE
begin

    if (rising_edge(Rx_Usr_Clk)) then

        ---------------------------------------------------
        -- Pipeline
        ---------------------------------------------------
        In_Pipe(1 to In_Pipe'HIGH)  <= In_Pipe(0 to In_Pipe'HIGH-1);


        ---------------------------------------------------
        -- Stage 1: create expected PRBS
        ---------------------------------------------------
        if (In_Pipe(0).Enable = '1') then

            if PRBS_Lock = '1' and Disable_PRBS_Ref_cdc /= '1' then

                for n in PRBS_SEED_31'range loop

                    Rx_PRBS_Seed(n) <= xor_reduce(PRBS_CHECK(n) and Rx_PRBS_Seed);

                end loop;

            else

                Rx_PRBS_Seed <= In_Pipe(0).Data(PRBS_SEED_31'range);

            end if;

            -- create reference data
            for n in DATA_SIZE-1 downto 0 loop

                In_Pipe(1).PRBS_Test(n) <= xor_reduce(PRBS_CHECK(n) and Rx_PRBS_Seed);

            end loop;

        end if;

        In_Pipe(1).PRBS_Seed <= Rx_PRBS_Seed;

        -- synthesis translate_off
        -- pseudo-code equivalent
        -- if (In_Pipe(0).Enable = '1') then
        --     PRBS_var := dbg_Rx_PRBS_Seed;
        --     for n in DATA_SIZE-1 downto 0 loop
        --         -- generate the Test Word
        --         dbg_PRBS_Test(n)  <=     xor_reduce(PRBS_var and PRBS_SEED_31);
        --         -- Update the Seed
        --         PRBS_var := PRBS_var(PRBS_var'HIGH-1 downto 0) & not In_Pipe(0).Data(n);
        --     end loop;
        --     dbg_Rx_PRBS_Seed <= PRBS_var;
        -- end if;
        -- synthesis translate_on

        if (COUNT_MODE = true) then
            if (In_Pipe(0).Enable = '1') then

                Rx_PRBS_Seed <= ext(In_Pipe(0).Data(31 downto 16),Rx_PRBS_Seed'length)  + 1;
                In_Pipe(1).PRBS_Seed <= Rx_PRBS_Seed;
                In_Pipe(1).PRBS_Test <= (others => '0');
                In_Pipe(1).PRBS_Test(15 downto 0)   <= Rx_PRBS_Seed(15 downto 0);
                In_Pipe(1).PRBS_Test(31 downto 16)  <= Rx_PRBS_Seed(15 downto 0) + 1;

            end if;
        end if;


        ---------------------------------------------------
        -- Stage 2: compare data with expected PRBS
        ---------------------------------------------------
        In_Pipe(2).PRBS_Test <= In_Pipe(1).PRBS_Test xor In_Pipe(1).Data;

        -- check PRBS seed is not null
        OR_Reducer_var  := '0';
        for n in PRBS_SEED_31'range loop
            -- "aggregate" ones over 6 bits
            OR_Reducer_var  := OR_Reducer_var or In_Pipe(1).PRBS_Seed(n);
            -- every 6 bits store the accumlation result and reset the accumulator
            if ((n mod 6) = 0) then
                In_Pipe(2).PRBS_Seed_Error(n / 6) <= OR_Reducer_var;
                OR_Reducer_var  := '0';
            end if;
        end loop;


        ---------------------------------------------------
        -- Stage 3: Reduce the PRBS errors using LUT6s: 128 bits reduced into 22 bits
        ---------------------------------------------------
        OR_Reducer_var  := '0';
        PRBS_Test_var := In_Pipe(2).PRBS_Test;

        for n in DATA_SIZE-1 downto 0 loop

            -- "aggregate" error over 6 bits
            OR_Reducer_var  := OR_Reducer_var or PRBS_Test_var(n);

            -- every 6 bits store the accumlation result and reset the accumulator
            if ((n mod 6) = 0) then

                In_Pipe(3).PRBS_Error(n / 6) <= OR_Reducer_var;

                OR_Reducer_var  := '0';

            end if;

        end loop;

        Temp_Test := ext(PRBS_Test_var,Temp_Test'length); -- cast data in vector which has a mulitple of 6 size
        for n in DATA_SIZE/6 downto 0 loop
            -- "count" error over 6 bits
            In_Pipe(3).PRBS_Error_Cnt(n) <= fn_Count_Ones( Temp_Test(n*6+5 downto n*6) );
        end loop;

        -- synthesis translate_off
        dbg_PRBS_Error_Cnt <= fn_Count_Ones( PRBS_Test_var );
        -- synthesis translate_on

        -- check that not all bits are null
        In_Pipe(3).PRBS_Zero_Seed <= '0';
        if (nor_reduce(In_Pipe(2).PRBS_Seed_Error) = '1') and (RAW_PRBS = true) then
            Rx_Seed_Zero <= '1';
            In_Pipe(3).PRBS_Zero_Seed <= '1';
        end if;

        if (clear_status_rx_d1 /= clear_status_rx_cdc) then
            Rx_Seed_Zero    <= '0';
        end if;

        ---------------------------------------------------
        -- Stage 4: Reduce the PRBS errors using LUT6s: 22 error bits reduced to 4 error bits
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

        -- "accumulate" error counters per pair
        for n in PRBS_Error_Cnt_Acc_1'range loop

            PRBS_Error_Cnt_Acc_1(n) <= ext(In_Pipe(3).PRBS_Error_Cnt(2*n+1),4) + ext(In_Pipe(3).PRBS_Error_Cnt(2*n),4);

        end loop;


        ---------------------------------------------------
        -- Stage 5: final reduction of the 4 error bits
        ---------------------------------------------------
        if (In_Pipe(4).Enable = '1') then

            if or_reduce(In_Pipe(4).PRBS_Error(3 downto 0)) = '1' then
                Rx_PRBS_Error <= '1';
                In_Pipe(5).Error <= '1';
            end if;

        else
            In_Pipe(5).Error <= '0';
        end if;

        clear_status_rx_d1 <= clear_status_rx_cdc;
        if (clear_status_rx_d1 /= clear_status_rx_cdc) then
            Rx_PRBS_Error <= '0';
        end if;

        -- "accumulate" previously added pairs, but 11 is not a multiple of 2, last value is simply passed to the next stage
        for n in PRBS_Error_Cnt_Acc_2'high -1 downto 0 loop

            PRBS_Error_Cnt_Acc_2(n) <= ext(PRBS_Error_Cnt_Acc_1(2*n+1),5) + ext(PRBS_Error_Cnt_Acc_1(2*n),5);

        end loop;
        PRBS_Error_Cnt_Acc_2(PRBS_Error_Cnt_Acc_2'high) <=  ext(PRBS_Error_Cnt_Acc_1(PRBS_Error_Cnt_Acc_1'high),5);


        ---------------------------------------------------
        -- Stage 6: BERT counters
        ---------------------------------------------------
        -- "accumulate" error counters per pair
        for n in PRBS_Error_Cnt_Acc_3'range loop

            PRBS_Error_Cnt_Acc_3(n) <= ext(PRBS_Error_Cnt_Acc_2(2*n+1),6) + ext(PRBS_Error_Cnt_Acc_2(2*n),6);

        end loop;


        ---------------------------------------------------
        -- Stage 7: final stage of error accumulation
        ---------------------------------------------------
        PRBS_Error_Cnt_Acc <= ext(PRBS_Error_Cnt_Acc_3(2),8) + ext(PRBS_Error_Cnt_Acc_3(1),8) + ext(PRBS_Error_Cnt_Acc_3(0),8);


        ---------------------------------------------------
        -- Stage 8: check for more than 10% error
        ---------------------------------------------------
        if ( PRBS_Error_Cnt_Acc > (DATA_SIZE/10) ) then
            In_Pipe(8).PRBS_Too_Many_Error <= '1';
        else
            In_Pipe(8).PRBS_Too_Many_Error <= '0';
        end if;


        if (Rx_Cnt_Rec = '1') then  -- In_Pipe(7).Enable already part of Rx_Cnt_Rec

            if (In_Pipe(7).Error = '1') then

                Rx_Err_Bit_Cnt_i <= Rx_Err_Bit_Cnt_i + ext(PRBS_Error_Cnt_Acc, Rx_Err_Bit_Cnt_i'length);

            end if;

            Rx_Word_Cnt_i <= Rx_Word_Cnt_i + 1;

        end if;

        toggle_1_sec_rx_cdc_d <= toggle_1_sec_rx_cdc;
        -- only start to record on second tick so rate computation are precise
        if ( (In_Pipe(7).Enable = '1') and (toggle_1_sec_rx_cdc_d /= toggle_1_sec_rx_cdc) ) then
            Rx_Cnt_Rec          <= '1';
            Rx_Word_Cnt_i       <= conv_std_logic_vector(1, Rx_Word_Cnt_i'length);
            Rx_Err_Bit_Cnt_i    <= ext(PRBS_Error_Cnt_Acc, Rx_Err_Bit_Cnt_i'length);
        end if;

        -- only report value accumulated over a plain second
        if ( (Rx_Cnt_Rec = '1') and (toggle_1_sec_rx_cdc_d /= toggle_1_sec_rx_cdc) )  then
            Rx_Word_Cnt     <= Rx_Word_Cnt_i;
            Rx_Err_Bit_Cnt  <= Rx_Err_Bit_Cnt_i;
        end if;

        if (In_Pipe(7).Enable /= '1') then
            Rx_Cnt_Rec          <= '0';
            Rx_Word_Cnt_i       <= (others => '0');
            Rx_Err_Bit_Cnt_i    <= (others => '0');
            Rx_Word_Cnt         <= (others => '0');
            Rx_Err_Bit_Cnt      <= (others => '0');
        end if;


        ---------------------------------------------------
        -- Stage 9: PRBS_Lock
        ---------------------------------------------------
        -- loose lock if too many errors or PRBS null detected or not enable
        if In_Pipe(8).Enable = '1' then

            if ( In_Pipe(8).Enable = '1') then
                if (In_Pipe(8).PRBS_Too_Many_Error = '1') or (In_Pipe(8).PRBS_Zero_Seed = '1')  then
                    PRBS_Lock <= '0';
                else
                    PRBS_Lock <= '1';
                end if;
            end if;

        else
            PRBS_Lock <= '0';
        end if;

    end if;

end process;

Tx_CDC_In <= latch_status & toggle_1_sec & clear_status & Tx_Error_Inject;

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

Tx_En_cdc               <= '1'; -- always enable the TX, so with loopback module RX always gets something for its reset
latch_status_tx_cdc     <= Tx_CDC_Out(3);
toggle_1_sec_tx_cdc     <= Tx_CDC_Out(2);
clear_status_tx_cdc     <= Tx_CDC_Out(1);
Tx_Error_Inject_cdc     <= Tx_CDC_Out(0);


Tx_Seed_Zero <= Tx_Seed_Zero_i;

Out_Pipe(0).Enable  <= Tx_En_cdc;

TX:
process(Tx_Usr_Clk)
    variable tx_data_var    : std_logic_vector(DATA_SIZE-1 downto 0);
    variable Tx_Seed_Zero_var : std_logic;
    variable OR_Reducer_var : std_logic;
begin

    if (rising_edge(Tx_Usr_Clk)) then

        ---------------------------------------------------
        -- Pipeline
        ---------------------------------------------------

        Out_Pipe(1 to Out_Pipe'HIGH)  <= Out_Pipe(0 to Out_Pipe'HIGH-1);

        Out_Pipe(1).PRBS_Seed <= Out_Pipe(1).PRBS_Seed;
        if Out_Pipe(0).Enable = '1' then

            Out_Pipe(1).PRBS_Seed   <= Tx_PRBS_Seed;

            for n in Tx_PRBS_Seed'RANGE loop
                Tx_PRBS_Seed(n)  <= xor_reduce(PRBS_CHECK(n) and Tx_PRBS_Seed);
            end loop;

        end if;

        -- Check for all zero seed & reset seed if detected
        if (Rst_Tx_Seed = '1') then

            Tx_PRBS_Seed    <= C_RST_PRBS_SEED;

        end if;


        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------

        Out_Pipe(2).Data <= Out_Pipe(2).Data;

        if (COUNT_MODE = true) then

            if Out_Pipe(1).Enable = '1' then
                Tx_PRBS_Seed <= Tx_PRBS_Seed + 2;
                if Tx_PRBS_Seed(16) = '1' then
                    Tx_PRBS_Seed <= (others => '0');
                end if;
                Out_Pipe(2).Data  <= (others => '0');
                Out_Pipe(2).Data(15 downto 0)  <= Tx_PRBS_Seed(15 downto 0);
                Out_Pipe(2).Data(31 downto 16) <= Tx_PRBS_Seed(15 downto 0) + 1;

            end if;
        end if;

        if (RAW_PRBS = true) then

            if Out_Pipe(1).Enable = '1' then

                Tx_Error_Inject_d1 <= Tx_Error_Inject_cdc;

                for n in tx_data_var'range loop
                    -- inject a single error
                    if (n = tx_data_var'high) and (Tx_Error_Inject_d1 /= Tx_Error_Inject_cdc) then
                        tx_data_var(n)  := not (xor_reduce(PRBS_CHECK(n) and Out_Pipe(1).PRBS_Seed) );
                    else
                        tx_data_var(n)  := xor_reduce(PRBS_CHECK(n) and Out_Pipe(1).PRBS_Seed);
                    end if;
                end loop;

                Out_Pipe(2).Data    <= tx_data_var;

            end if;

            -- check PRBS seed is not null
            OR_Reducer_var  := '0';
            for n in PRBS_SEED_31'range loop
                -- "aggregate" ones over 6 bits
                OR_Reducer_var  := OR_Reducer_var or Out_Pipe(1).PRBS_Seed(n);
                -- every 6 bits store the accumlation result and reset the accumulator
                if ((n mod 6) = 0) then
                    Out_Pipe(2).PRBS_Seed_Error(n / 6) <= OR_Reducer_var;
                    OR_Reducer_var  := '0';
                end if;
            end loop;


        end if;

        if (FIXED_VALUE = true) then
            Out_Pipe(2).Data  <= (others => '0');
            Out_Pipe(2).Data(31 downto 0)    <= X"FF007002";
        end if;

        ---------------------------------------------------
        -- Stage 3
        ---------------------------------------------------

        if (Tx_Cnt_Rec = '1') then

            Tx_Word_Cnt_i <= Tx_Word_Cnt_i + 1;

        end if;

        toggle_1_sec_tx_cdc_d <= toggle_1_sec_tx_cdc;
        -- only start to record on second tick so rate computation are precise
        --  store counter value every second
        if (toggle_1_sec_tx_cdc_d /= toggle_1_sec_tx_cdc) then

            Tx_Cnt_Rec      <= '1';
            Tx_Word_Cnt_sec <= Tx_Word_Cnt_i;

        end if;

        latch_status_tx_d <= latch_status_tx_cdc;
        -- only report value accumulated over plain second
        if (latch_status_tx_d /= latch_status_tx_cdc) then

            Tx_Word_Cnt     <= Tx_Word_Cnt_sec;

        end if;


        -- check that not all bits are null
        if (nor_reduce(Out_Pipe(2).PRBS_Seed_Error) = '1') and (RAW_PRBS = true) then
            Tx_Seed_Zero_i  <= '1';
            Rst_Tx_Seed     <= '1';
        else
            Rst_Tx_Seed <= '0';
        end if;

        clear_status_tx_d1 <= clear_status_tx_cdc;
        if clear_status_tx_d1 /= clear_status_tx_cdc then
            Tx_Seed_Zero_i  <= '0';
            Tx_Cnt_Rec      <= '0';
            Tx_Word_Cnt_i   <= (others => '0');
            Tx_Word_Cnt     <= (others => '0');
        end if;

    end if;

end process;
    -- PRBS is generated from MSB to LSB, but gtm raw transmit LSB first
    Tx_Data     <=  fn_Reverse_Bits(Out_Pipe(2).Data);


end architecture rtl;
