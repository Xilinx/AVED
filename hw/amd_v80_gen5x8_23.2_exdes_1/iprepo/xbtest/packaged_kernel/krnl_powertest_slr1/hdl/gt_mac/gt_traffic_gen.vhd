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

entity gt_traffic_gen is
    generic(
        DEST_SYNC_FF    : integer
    );
    port (
        clk_300         : in    std_logic;
        rst_300         : in    std_logic;

        clk_axis        : in    std_logic;

        -- Static Configuration (clk_300)

        Config_Upd      : in    std_logic;
        Dest_Addr       : in    std_logic_vector(47 downto 0);
        Source_Addr     : in    std_logic_vector(47 downto 0);
        Ether_Type      : in    std_logic_vector(15 downto 0);
        Payload_Type    : in    std_logic_vector(1 downto 0);

        -- Generation Request Interface (clk_300)

        Gen_Pkt         : in    std_logic;
        Gen_Length      : in    std_logic_vector(13 downto 0);

        Gen_Half_Full   : out   std_logic;
        Gen_Full        : out   std_logic;

        -- AXI Streaming Output (clk_axis)

        tx_tready       : in    std_logic;
        tx_tvalid       : out   std_logic;
        tx_tdata        : out   std_logic_vector(63 downto 0);
        tx_tkeep        : out   std_logic_vector(7 downto 0);
        tx_tlast        : out   std_logic;
        tx_tuser        : out   std_logic
    );
end entity gt_traffic_gen;

architecture rtl of gt_traffic_gen is

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

-------------------------------------------------------------------------------
--
--      Constants (simple)
--
-------------------------------------------------------------------------------

constant ZERO   : std_logic_vector(0 downto 0)  := (others => '0');

-------------------------------------------------------------------------------
--
--      Type Definitions
--
-------------------------------------------------------------------------------

type Pipe_Type is
    record
        Valid   : std_logic;
        Cycles  : std_logic_vector(11 downto 0);
        Phase   : std_logic_vector(1 downto 0);
        Last    : std_logic_vector(2 downto 0);
        Keep    : std_logic_vector(7 downto 0);
        Data    : std_logic_vector(63 downto 0);
    end record;

type Pipe_Array_Type is array (integer range <>) of Pipe_Type;

constant RST_PIPE_TYPE : Pipe_Type := (
    '0',                -- : std_logic
    (others => '0'),    -- : std_logic_vector(11 downto 0);
    (others => '0'),    -- : std_logic_vector(1 downto 0);
    (others => '0'),    -- : std_logic_vector(2 downto 0);
    (others => '0'),    -- : std_logic_vector(7 downto 0);
    (others => '0'));   -- : std_logic_vector(63 downto 0);

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

function fn_if(Test : boolean; Arg1 : integer; Arg2 : integer) return integer is
begin

    if (Test) then

        return Arg1;

    else

        return Arg2;

    end if;

end function fn_if;

-------------------------------------------------------------------------------
--
--      Signals
--
-------------------------------------------------------------------------------

signal  Dest_Addr_cdc       : std_logic_vector(47 downto 0) := (others => '0');
signal  Source_Addr_cdc     : std_logic_vector(47 downto 0) := (others => '0');
signal  Ether_Type_cdc      : std_logic_vector(15 downto 0) := (others => '0');
signal  Payload_Type_cdc    : std_logic_vector(1 downto 0)  := (others => '0');

signal  Pipe                : Pipe_Array_Type(1 to 4)       := (others => RST_PIPE_TYPE);

signal  Tx_FIFO             : Pipe_Array_Type(0 to 15)      := (others => RST_PIPE_TYPE);
signal  Tx_FIFO_Rd_En       : std_logic                     := '0';
signal  Tx_FIFO_Rd_Ptr      : std_logic_vector(4 downto 0)  := (others => '1');
signal  Tx_FIFO_Half_Full   : std_logic                     := '0';

signal  FO_Val              : std_logic                     := '0';
signal  FO_Length           : std_logic_vector(13 downto 0) := (others => '0');
signal  FO_Ack              : std_logic                     := '0';

-- synthesis translate_off


-- synthesis translate_on

begin

-------------------------------------------------------------------------------
--
--      Clock Domain Crossing
--
-------------------------------------------------------------------------------

-- CDC_CONF: cdc_static_bus
--     generic map (
--         DEST_SYNC_FF    => DEST_SYNC_FF,
--         WIDTH           => 2 + 16 + 48 + 48
--     )
--     port map (
--         src_clk                 => clk_300,
--         src_send                => Config_Upd,
--         src_in(113 downto 112)  => Payload_Type,
--         src_in(111 downto  96)  => Ether_Type,
--         src_in( 95 downto  48)  => Source_Addr,
--         src_in( 47 downto   0)  => Dest_Addr,
--
--         dest_clk                    => clk_axis,
--         dest_out(113 downto 112)    => Payload_Type_cdc,
--         dest_out(111 downto  96)    => Ether_Type_cdc,
--         dest_out( 95 downto  48)    => Source_Addr_cdc,
--         dest_out( 47 downto   0)    => Dest_Addr_cdc
--     );

CDC_CONF: xpm_cdc_array_single
    generic map (
        DEST_SYNC_FF    => DEST_SYNC_FF,
        WIDTH           => 2 + 16 + 48 + 48,
        SRC_INPUT_REG   => 1
    )
    port map (
        src_clk                 => clk_300,
        src_in(113 downto 112)  => Payload_Type,
        src_in(111 downto  96)  => Ether_Type,
        src_in( 95 downto  48)  => Source_Addr,
        src_in( 47 downto   0)  => Dest_Addr,

        dest_clk                    => clk_axis,
        dest_out(113 downto 112)    => Payload_Type_cdc,
        dest_out(111 downto  96)    => Ether_Type_cdc,
        dest_out( 95 downto  48)    => Source_Addr_cdc,
        dest_out( 47 downto   0)    => Dest_Addr_cdc
    );

FO:
xpm_fifo_async
    generic map (
        FIFO_MEMORY_TYPE    => "distributed",
        FIFO_WRITE_DEPTH    => 16,
        PROG_FULL_THRESH    => 8,
        READ_DATA_WIDTH     => 14,
        READ_MODE           => "fwft",
        USE_ADV_FEATURES    => "1002",
        WRITE_DATA_WIDTH    => 14
    )
    port map (
        wr_clk          => clk_300,
        rst             => rst_300,

        wr_en           => Gen_Pkt,
        din             => Gen_Length,

        rd_clk          => clk_axis,

        rd_en           => FO_Ack,
        dout            => FO_Length,
        data_valid      => FO_Val,

        prog_full       => Gen_Half_Full,
        full            => Gen_Full,

        injectdbiterr   => ZERO(0),
        injectsbiterr   => ZERO(0),
        sleep           => ZERO(0)
    );

    Tx_FIFO_Rd_En   <= tx_tready and (not Tx_FIFO_Rd_Ptr(Tx_FIFO_Rd_Ptr'HIGH));

SQ:
process(clk_axis)
variable case_4_var : std_logic_vector(3 downto 0);
begin

    if (rising_edge(clk_axis)) then

        Pipe    <= Pipe(1) & Pipe(1 to Pipe'HIGH-1);

        ---------------------------------------------------
        -- Stage 1
        ---------------------------------------------------

        FO_Ack    <= '0';

        if (Tx_FIFO_Half_Full = '1') then

            -- Freeze while the output FIFO empties

            Pipe(1).Valid   <= '0';

        elsif ((Pipe(1).Cycles(Pipe(1).Cycles'HIGH) = '0') and (FO_Val = '1')) then

            Pipe(1).Valid   <= '1';
            Pipe(1).Cycles  <= ("1" & FO_Length(FO_Length'HIGH downto 3)) - fn_if(FO_Length(2 downto 0) = "000", 2, 1);
            Pipe(1).Last    <= FO_Length(2 downto 0);
            Pipe(1).Phase   <= "00";
            FO_Ack            <= '1';

        elsif (Pipe(1).Cycles(Pipe(1).Cycles'HIGH) = '1') then

            Pipe(1).Valid   <= '1';
            Pipe(1).Cycles  <= Pipe(1).Cycles - 1;

            if (Pipe(1).Phase(1) = '0') then

                Pipe(1).Phase   <= Pipe(1).Phase + 1;

            end if;

        else

            Pipe(1).Valid   <= '0';

        end if;

        ---------------------------------------------------
        -- Stage 2
        ---------------------------------------------------

        case Pipe(1).Phase is

            when "00"   =>

                Pipe(2).Data    <= Source_Addr_cdc(15 downto 0) & Dest_Addr_cdc;

            when "01"   =>

                Pipe(2).Data    <= X"01" & X"00" & Ether_Type_cdc & Source_Addr_cdc(47 downto 16);

            when others =>

                for n in 7 downto 0 loop

                    Pipe(2).Data((n * 8) + 7 downto n * 8)  <= Pipe(2).Data(63 downto 56) + 1 + n;

                end loop;

        end case;

        case Pipe(1).Last is

            when "000"  => Pipe(2).Keep <= "11111111";
            when "001"  => Pipe(2).Keep <= "00000001";
            when "010"  => Pipe(2).Keep <= "00000011";
            when "011"  => Pipe(2).Keep <= "00000111";
            when "100"  => Pipe(2).Keep <= "00001111";
            when "101"  => Pipe(2).Keep <= "00011111";
            when "110"  => Pipe(2).Keep <= "00111111";
            when others => Pipe(2).Keep <= "01111111";

        end case;

        if (Pipe(1).Cycles(Pipe(1).Cycles'HIGH) = '1') then

            Pipe(2).Keep    <= (others => '1');

        end if;

        ---------------------------------------------------
        -- Stage 3
        ---------------------------------------------------

        case_4_var  := Payload_Type_cdc & Pipe(2).Phase;

        case case_4_var is

            when "0001" => Pipe(3).Data(63 downto 48)   <= (others => '0');
            when "0010" => Pipe(3).Data                 <= (others => '0');
            when "0011" => Pipe(3).Data                 <= (others => '-');
            when "1001" => Pipe(3).Data(63 downto 48)   <= X"55" & X"00";
            when "1010" => Pipe(3).Data                 <= X"55" & X"00" & X"FF" & X"AA" & X"55" & X"00" & X"FF" & X"AA";
            when "1011" => Pipe(3).Data                 <= (others => '-');
            when "1101" => Pipe(3).Data(63 downto 48)   <= (others => '1');
            when "1110" => Pipe(3).Data                 <= (others => '1');
            when "1111" => Pipe(3).Data                 <= (others => '-');
            when others => null;

        end case;

        -- Invert Cycles to act as TLAST

        Pipe(3).Cycles(Pipe(3).Cycles'HIGH) <= not Pipe(2).Cycles(Pipe(2).Cycles'HIGH);

        ---------------------------------------------------
        -- Stage 4
        ---------------------------------------------------

        -- FIFO the Data into the MAC based on tready

        if (Pipe(3).Valid = '1') then

            Tx_FIFO <= Pipe(3) & Tx_FIFO(0 to Tx_FIFO'HIGH-1);

            if (Tx_FIFO_Rd_En = '0') then

                Tx_FIFO_Rd_Ptr  <= Tx_FIFO_Rd_Ptr + 1;

            end if;

        elsif (Tx_FIFO_Rd_En = '1') then

            Tx_FIFO_Rd_Ptr  <= Tx_FIFO_Rd_Ptr - 1;

        end if;

        Pipe(4) <= Pipe(4);

        if (tx_tready = '1') then

            Pipe(4).Valid   <= '0';
            Pipe(4).Keep    <= (others => '0');
            Pipe(4).Cycles  <= (others => '0');

        end if;

        if (Tx_FIFO_Rd_En = '1') then

            Pipe(4)         <= Tx_FIFO(conv_integer(Tx_FIFO_Rd_Ptr(Tx_FIFO_Rd_Ptr'HIGH-1 downto 0)));
            Pipe(4).Valid   <= '1';

        end if;

        if (signed(Tx_FIFO_Rd_Ptr) >= (Tx_FIFO'LENGTH / 2)) then

            Tx_FIFO_Half_Full   <= '1';

        else

            Tx_FIFO_Half_Full   <= '0';

        end if;

    end if;

end process;

    tx_tvalid   <= Pipe(Pipe'HIGH).Valid;
    tx_tdata    <= Pipe(Pipe'HIGH).Data;
    tx_tkeep    <= Pipe(Pipe'HIGH).Keep;
    tx_tlast    <= Pipe(Pipe'HIGH).Cycles(Pipe(1).Cycles'HIGH);
    tx_tuser    <= '0';

end architecture rtl; -- of gt_traffic_gen
