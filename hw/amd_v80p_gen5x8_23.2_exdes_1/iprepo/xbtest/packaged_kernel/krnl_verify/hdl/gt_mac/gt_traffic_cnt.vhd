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

entity gt_traffic_cnt is
    generic(
        DEST_SYNC_FF    : integer
    );
    port (
        clk_300                 : in    std_logic;
        rst_300                 : in    std_logic;
        toggle_1sec             : in    std_logic;

        clk_axis                : in    std_logic;
        rst_axis                : in    std_logic;

        -- Static Configuration (clk_300)
        Latch_and_Clear_Cnt     : in    std_logic;

        Config_Upd              : in    std_logic;
        Source_Addr             : in    std_logic_vector(47 downto 0);
        Dest_Addr               : in    std_logic_vector(47 downto 0);

        -- AXI Streaming Output (clk_axis)
        axi_tvalid              : in    std_logic;
        axi_tdata               : in    std_logic_vector(63 downto 0);
        axi_tkeep               : in    std_logic_vector(7 downto 0);
        axi_tlast               : in    std_logic;
        axi_tuser               : in    std_logic;

        Cnt_RAM_Rd_Addr         : in    std_logic_vector(3 downto 0);
        Cnt_RAM_Out             : out   std_logic_vector(31 downto 0)

    );
end entity gt_traffic_cnt;

architecture rtl of gt_traffic_cnt is

    attribute dont_touch    : string;

-------------------------------------------------------------------------------
--
--      Component
--
-------------------------------------------------------------------------------
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

COMPONENT axis_ila_0
  PORT (
    clk : IN STD_LOGIC;
    probe0 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe1 : IN STD_LOGIC_VECTOR(63 DOWNTO 0);
    probe2 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe3 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe4 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe5 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe6 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe7 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe8 : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    probe9 : IN STD_LOGIC_VECTOR(47 DOWNTO 0);
    probe10 : IN STD_LOGIC_VECTOR(47 DOWNTO 0);
    probe11 : IN STD_LOGIC_VECTOR(47 DOWNTO 0)
  );
END COMPONENT;

-------------------------------------------------------------------------------
--
--      Constants (simple)
--
-------------------------------------------------------------------------------

constant ZERO           : std_logic_vector(0 downto 0)  := (others => '0');
constant CHIPSCOPE_EN   : boolean  := false;

------------------------------------------------------------------------------
--
--      Type Definitions
--
-------------------------------------------------------------------------------

type Pipe_Type is
    record
        Valid           : std_logic;
        Data            : std_logic_vector(63 downto 0);
        Keep            : integer range 0 to 8;
        Last            : std_logic;
        User            : std_logic;
        First_Word      : std_logic;
        Second_Word     : std_logic;
        Valid_Addr      : std_logic;
        Dest_Addr_Valid : std_logic;
        Dest_Addr_Error : std_logic;
        Src_Addr_Valid  : std_logic;
        Src_Addr_Error  : std_logic;
        Byte_Cnt        : std_logic_vector(15 downto 0);
    end record;

type Pipe_Array_Type is array (integer range <>) of Pipe_Type;

constant RST_PIPE_TYPE : Pipe_Type := (
    '0',
    (others => '0'),
     0,
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    '0',
    (others => '0')
    );


type  std_logic_2d_64     is array (integer range <>) of std_logic_vector(63 downto 0);

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
--      Signals
--
-------------------------------------------------------------------------------
signal  Source_Addr_cdc     : std_logic_vector(47 downto 0)             := (others => '0');
signal  Dest_Addr_cdc       : std_logic_vector(47 downto 0)             := (others => '0');
signal  Latch_cdc           : std_logic                                 := '0';
signal  Latch_cdc_d1        : std_logic                                 := '0';
signal  Latch_cdc_d         : std_logic_vector(DEST_SYNC_FF-1 downto 0) := (others => '0');

signal  Clear_Cnt           : std_logic                                 := '0';

signal  Pipe                : Pipe_Array_Type(0 to 2)                   := (others => RST_PIPE_TYPE);
signal  Frame_Header        : std_logic_vector(1 downto 0)              := (others => '1');

signal  Bad_Packet_Cnt      : std_logic_vector(47 downto 0)             := (others => '0');
signal  Good_Packet_Cnt     : std_logic_vector(47 downto 0)             := (others => '0');
signal  Good_Byte_Cnt       : std_logic_vector(47 downto 0)             := (others => '0');
signal  Packet_Cnt          : std_logic_vector(47 downto 0)             := (others => '0');
signal  Bad_Dest_Addr_Cnt   : std_logic_vector(47 downto 0)             := (others => '0');
signal  Bad_Src_Addr_Cnt    : std_logic_vector(47 downto 0)             := (others => '0');

signal Rcv_Source_Addr_LSB  : std_logic_vector(15 downto 0)             := (others => '0');
signal Rcv_Source_Addr      : std_logic_vector(47 downto 0)             := (others => '0');
signal Rcv_Dest_Addr        : std_logic_vector(47 downto 0)             := (others => '0');

signal toggle_1sec_cdc      : std_logic                                 := '0';
signal toggle_1sec_d        : std_logic                                 := '0';

signal freq_count           : std_logic_vector(31 downto 0)             := (others => '0');
signal freq_count_latch     : std_logic_vector(31 downto 0)             := (others => '0');

signal  Cnt_Sel             : std_logic_vector(4 downto 0)              := (others => '0');
signal  Cnt_Sel_d1          : std_logic_vector(Cnt_Sel'range)           := (others => '0');
signal  Cnt_RAM             : std_logic_2d_32(0 to 15)                  := (others => (others => '0'));
signal  Cnt_RAM_In          : std_logic_vector(31 downto 0)             := (others => '0');

signal Cnt_RAM_Rd_Addr_d1   : std_logic_vector(Cnt_RAM_Rd_Addr'range)   := (others => '0');
signal Cnt_RAM_Rd_Addr_d2   : std_logic_vector(Cnt_RAM_Rd_Addr'range)   := (others => '0');
signal RAM_Out              : std_logic_vector(Cnt_RAM_Out'range)       := (others => '0');
signal RAM_Out_d1           : std_logic_vector(Cnt_RAM_Out'range)       := (others => '0');

-- keep pipeline for helping timing closure as MAC is spread across the SLR
attribute dont_touch  of Cnt_RAM_Rd_Addr_d1     : signal is "true";
attribute dont_touch  of Cnt_RAM_Rd_Addr_d2     : signal is "true";
attribute dont_touch  of RAM_Out_d1             : signal is "true";

-- synthesis translate_off

-- synthesis translate_on

begin

CS_En : if (CHIPSCOPE_EN) generate
    u_cs : axis_ila_0
    PORT MAP (
        clk => clk_axis,

        probe0(0)   => Pipe(2).Valid,
        probe1      => Pipe(2).Data ,
        probe2(0)   => Pipe(2).Last ,
        probe3(0)   => Pipe(2).User ,
        probe4(0)   => Pipe(2).First_Word,
        probe5(0)   => Pipe(2).Second_Word,
        probe6(0)   => Pipe(2).Dest_Addr_Error,
        probe7(0)   => Pipe(2).Src_Addr_Error,
        probe8(0)   => Pipe(2).Valid_Addr,
        probe9      => Packet_Cnt,
        probe10     => Bad_Dest_Addr_Cnt,
        probe11     => Bad_Src_Addr_Cnt
    );

end generate;


CDC_toggle_1sec: xpm_cdc_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF,
        SRC_INPUT_REG  => 1
    )
    port map (
        src_clk              => clk_300,
        src_in               => toggle_1sec,
        dest_clk             => clk_axis,
        dest_out             => toggle_1sec_cdc
    );

-- CDC_CONF: cdc_static_bus
--     generic map (
--         DEST_SYNC_FF    => DEST_SYNC_FF,
--         WIDTH           => 48 + 48
--     )
--     port map (
--         src_clk                 => clk_300,
--         src_send                => Config_Upd,
--         src_in( 95 downto  48)  => Source_Addr,
--         src_in( 47 downto   0)  => Dest_Addr,
--
--         dest_clk                => clk_axis,
--         dest_out(95 downto  48) => Source_Addr_cdc,
--         dest_out(47 downto   0) => Dest_Addr_cdc
--     );

CDC_CONF: xpm_cdc_array_single
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => 48 + 48,
        SRC_INPUT_REG   => 1
    )
    port map (
        src_clk                 => clk_300,
        src_in( 95 downto  48)  => Source_Addr,
        src_in( 47 downto   0)  => Dest_Addr,

        dest_clk                => clk_axis,
        dest_out(95 downto  48) => Source_Addr_cdc,
        dest_out(47 downto   0) => Dest_Addr_cdc
    );


CDC_LaC: xpm_cdc_single
    generic map (
        DEST_SYNC_FF   => DEST_SYNC_FF,
        SRC_INPUT_REG  => 1
    )
    port map (
        src_clk              => clk_300,
        src_in               => Latch_and_Clear_Cnt,
        dest_clk             => clk_axis,
        dest_out             => Latch_cdc
    );


FC:process(clk_axis)
begin
    if (rising_edge(clk_axis)) then
        toggle_1sec_d <= toggle_1sec_cdc;
        freq_count <= freq_count + 1;

        if (toggle_1sec_d xor toggle_1sec_cdc) = '1' then
            freq_count_latch <= freq_count;
            freq_count <= (others => '0');
        end if;

    end if;
end process;

Pipe(0).Valid <= axi_tvalid;
Pipe(0).Data  <= axi_tdata;
Pipe(0).Last  <= axi_tlast;
Pipe(0).User  <= axi_tuser;

-- keep bits are de asserted (= '0') when a byte is not used
-- keep is only used when last ='1'
Pipe(0).Keep <= fn_Count_Ones(axi_tkeep);

SQ:
process(clk_axis)
begin

    if (rising_edge(clk_axis)) then

        Pipe(1 to Pipe'HIGH)    <= Pipe(0 to Pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 1
        ---------------------------------------------------

        Pipe(1).First_Word  <= '0';
        Pipe(1).Second_Word <= '0';

        Pipe(1).Dest_Addr_Valid <= Pipe(1).Dest_Addr_Valid;
        Pipe(1).Src_Addr_Valid  <= Pipe(1).Src_Addr_Valid;
        Pipe(1).Dest_Addr_Error <= Pipe(1).Dest_Addr_Error;
        Pipe(1).Src_Addr_Error  <= Pipe(1).Src_Addr_Error;

        if Pipe(0).Valid = '1' then

            case Frame_Header is

                when "11" =>
                    -- first cycle: Destination address and LSB of Source Address
                    Pipe(1).First_Word  <= '1';
                    if (Pipe(0).Data(47 downto 0) = Dest_Addr_cdc) then
                        Pipe(1).Dest_Addr_Valid <= '1';
                        Pipe(1).Dest_Addr_Error <= '0';
                    else
                        Pipe(1).Dest_Addr_Valid <= '0';
                        Pipe(1).Dest_Addr_Error <= '1';
                    end if;

                    if (Pipe(0).Data(63 downto 48) = Source_Addr_cdc(15 downto 0)) then
                        Pipe(1).Src_Addr_Valid <= '1';
                        Pipe(1).Src_Addr_Error <= '0';
                    else
                        Pipe(1).Src_Addr_Valid <= '0';
                        Pipe(1).Src_Addr_Error <= '1';
                    end if;

                when "10" =>
                    -- second cycle: Source MSB & ignore the reset of the data
                    Pipe(1).Second_Word  <= '1';

                    --  if equal keep previous comparison result as part of the address could have been wrong in the first word
                    if (Pipe(0).Data(31 downto 0) = Source_Addr_cdc(47 downto 16)) then
                        if Pipe(1).Src_Addr_Valid = '1' then
                            Pipe(1).Src_Addr_Valid <= '1';
                            Pipe(1).Src_Addr_Error <= '0';
                        end if;
                    else
                        Pipe(1).Src_Addr_Valid <= '0';
                        Pipe(1).Src_Addr_Error <= '1';
                    end if;

                when others => null;
            end case;

            if Pipe(0).Last = '1' then
                -- the next 2 cycles are the MAC address
                Frame_Header    <= (others => '1');

                -- not needed but there for debug purpose
                Pipe(1).Dest_Addr_Valid <= '0';
                Pipe(1).Dest_Addr_Error <= '0';
                Pipe(1).Src_Addr_Valid <= '0';
                Pipe(1).Src_Addr_Error <= '0';
            else
                -- shift in '0'
                Frame_Header(0) <= '0';
                Frame_Header(1) <= Frame_Header(0);
            end if;

        end if;


        ---------------------------------------------------
        -- Stage 2: count the byte
        ---------------------------------------------------
        Pipe(2).Byte_Cnt <= Pipe(2).Byte_Cnt;
        if Pipe(1).Valid = '1' then

            if Pipe(1).Last = '1' then
                Pipe(2).Byte_Cnt <= Pipe(2).Byte_Cnt + conv_std_logic_vector(Pipe(1).Keep, Pipe(2).Byte_Cnt'length);
            else
                Pipe(2).Byte_Cnt <= Pipe(2).Byte_Cnt + conv_std_logic_vector(8, Pipe(2).Byte_Cnt'length);
            end if;

            if Pipe(1).Second_Word = '1' then
                -- 16 first bytes have been received (Dest & source Address + ethertype)
                -- already add the 4 FCS byte, received later with tlast
                Pipe(2).Byte_Cnt <= conv_std_logic_vector(16+4, Pipe(2).Byte_Cnt'length);
            end if;

        end if;

        -- latch only when addresses are different
        if Pipe(1).Valid = '1' then

            -- first part of the header
            if Pipe(1).First_Word = '1' then

                Packet_Cnt <= Packet_Cnt + 1;

                if Pipe(1).Dest_Addr_Error = '1' then
                    Rcv_Dest_Addr   <= Pipe(1).Data(47 downto 0) xor Dest_Addr_cdc;
                    Bad_Dest_Addr_Cnt <= Bad_Dest_Addr_Cnt + 1;
                end if;

                -- save the source address in all cases
                Rcv_Source_Addr_LSB <= Pipe(1).Data(63 downto 48) xor Source_Addr_cdc(15 downto 0);

            end if;

            -- second part of the header
            if Pipe(1).Second_Word = '1' then

                if Pipe(1).Src_Addr_Error = '1' then
                    Rcv_Source_Addr (15 downto 0)  <= Rcv_Source_Addr_LSB;
                    Rcv_Source_Addr (47 downto 16) <= Pipe(1).Data(31 downto 0) xor Source_Addr_cdc(47 downto 16);
                    Bad_Src_Addr_Cnt <= Bad_Src_Addr_Cnt + 1;
                end if;

            end if;

        end if;

        -- save the fact that both addresses were correct
        Pipe(2).Valid_Addr <= Pipe(2).Valid_Addr;
        if Pipe(1).Valid = '1' then
            if Pipe(1).Second_Word = '1' then
                if Pipe(1).Src_Addr_Valid = '1' and Pipe(1).Dest_Addr_Valid = '1' then
                    Pipe(2).Valid_Addr <= '1';
                else
                    Pipe(2).Valid_Addr <= '0';
                end if;
            end if;
        end if;


        ---------------------------------------------------
        -- Stage 3: udpate counters at the end of the frame
        ---------------------------------------------------
        if Pipe(2).Valid = '1' then
            if Pipe(2).Last = '1' then
                if Pipe(2).User = '1' then
                    Bad_Packet_Cnt <= Bad_Packet_Cnt + 1;
                else
                    if Pipe(2).Valid_Addr = '1' then
                        Good_Packet_Cnt <= Good_Packet_Cnt + 1;
                        Good_Byte_Cnt   <= Good_Byte_Cnt + ext(Pipe(2).Byte_Cnt, Good_Byte_Cnt'length);
                    end if;
                end if;
            end if;
        end if;

        if rst_axis = '1' then
            Frame_Header   <= (others => '1');
        end if;


        -- counter storage
        Latch_cdc_d1 <= Latch_cdc;

        if ((Latch_cdc_d1 xor Latch_cdc) = '1') or (rst_axis = '1') then
            Cnt_Sel(Cnt_Sel'high) <= '0';
        end if;

        if Cnt_Sel(Cnt_Sel'high) /= '1' then
            Cnt_Sel <= Cnt_Sel + 1;
        end if;
        Cnt_Sel_d1 <= Cnt_Sel;

        case Cnt_Sel(3 downto 0) is
            when "0000"  => Cnt_RAM_In <= Good_Packet_Cnt(31 downto 0);
            when "0001"  => Cnt_RAM_In <= ext(Good_Packet_Cnt(47 downto 32),32);
            when "0010"  => Cnt_RAM_In <= Good_Byte_Cnt(31 downto 0);
            when "0011"  => Cnt_RAM_In <= ext(Good_Byte_Cnt(47 downto 32),32);
            when "0100"  => Cnt_RAM_In <= Packet_Cnt(31 downto 0);
            when "0101"  => Cnt_RAM_In <= ext(Packet_Cnt(47 downto 32),32);
            when "0110"  => Cnt_RAM_In <= Bad_Dest_Addr_Cnt(31 downto 0);
            when "0111"  => Cnt_RAM_In <= ext(Bad_Dest_Addr_Cnt(47 downto 32),32);
            when "1000"  => Cnt_RAM_In <= Rcv_Dest_Addr(31 downto 0);
            when "1001"  => Cnt_RAM_In <= ext(Rcv_Dest_Addr(47 downto 32),32);
            when "1010"  => Cnt_RAM_In <= Bad_Src_Addr_Cnt(31 downto 0);
            when "1011"  => Cnt_RAM_In <= ext(Bad_Src_Addr_Cnt(47 downto 32),32);
            when "1100"  => Cnt_RAM_In <= Rcv_Source_Addr(31 downto 0);
            when "1101"  => Cnt_RAM_In <= ext(Rcv_Source_Addr(47 downto 32),32);
            --when "1110"  => Cnt_RAM_In <= Bad_Packet_Cnt(31 downto 0);
            --when "1111"  => Cnt_RAM_In <= ext(Bad_Packet_Cnt(47 downto 32),32);
            when "1110"  => Cnt_RAM_In <= freq_count_latch;
            ----when "1111"  => Cnt_RAM_In <= freq_count;
            when others => Cnt_RAM_In <= (others => '0');
        end case;

        if (Cnt_Sel_d1(Cnt_Sel_d1'high) = '0') then
            Cnt_RAM(conv_integer(Cnt_Sel_d1(3 downto 0))) <= Cnt_RAM_In;
        end if;

        -- clear counter after they've been stored
        if (Cnt_Sel(Cnt_Sel'high) = '1' and Cnt_Sel_d1(Cnt_Sel_d1'high) = '0') or  rst_axis = '1' then
            Bad_Packet_Cnt      <= (others => '0');
            Good_Packet_Cnt     <= (others => '0');
            Good_Byte_Cnt       <= (others => '0');
            Packet_Cnt          <= (others => '0');
            Bad_Dest_Addr_Cnt   <= (others => '0');
            Bad_Src_Addr_Cnt    <= (others => '0');
            Rcv_Dest_Addr       <= (others => '0');
            Rcv_Source_Addr     <= (others => '0');
        end if;

    end if;

end process;

Cnt_RAM_Out <= RAM_Out_d1;

RM:
process(clk_300)
begin
    if (rising_edge(clk_300)) then
        Cnt_RAM_Rd_Addr_d1 <= Cnt_RAM_Rd_Addr;
        Cnt_RAM_Rd_Addr_d2 <= Cnt_RAM_Rd_Addr;

        RAM_Out <= Cnt_RAM(conv_integer(Cnt_RAM_Rd_Addr_d2));
        RAM_Out_d1 <= RAM_Out;

    end if;
end process;

end architecture rtl;
