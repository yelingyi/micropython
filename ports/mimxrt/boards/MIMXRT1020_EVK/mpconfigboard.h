#define MICROPY_HW_BOARD_NAME "i.MX RT1020 EVK"
#define MICROPY_HW_MCU_NAME   "MIMXRT1021DAG5A"

#define MICROPY_PY_NETWORK_HOSTNAME_DEFAULT "mpy-1020evk"

// i.MX RT1020 EVK has 1 board LED
// Todo: think about replacing the define with searching in the generated pins?
#define MICROPY_HW_LED1_PIN (pin_GPIO_AD_B0_05)
#define MICROPY_HW_LED_ON(pin) (mp_hal_pin_low(pin))
#define MICROPY_HW_LED_OFF(pin) (mp_hal_pin_high(pin))

#define MICROPY_HW_NUM_PIN_IRQS (3 * 32)

// Define mapping logical UART # to hardware UART #
// RX/TX      HW-UART    Logical UART
// DEBUG USB  LPUART1 -> 0
// D3/D5      LPUART1    Not usable, Since D3 is blocked.
// D0/D1      LPUART2 -> 1
// D6/D9      LPUART3 -> 2
// D10/D12    LPUART5 -> 3
// D14/D15    LPUART8 -> 4
// A0/A1      LPUART4 -> 5

#define MICROPY_HW_UART_NUM     (sizeof(uart_index_table) / sizeof(uart_index_table)[0])
#define MICROPY_HW_UART_INDEX   { 1, 2, 3, 5, 8, 4 }

#define IOMUX_TABLE_UART \
    { IOMUXC_GPIO_AD_B0_06_LPUART1_TX }, { IOMUXC_GPIO_AD_B0_07_LPUART1_RX }, \
    { IOMUXC_GPIO_AD_B1_08_LPUART2_TX }, { IOMUXC_GPIO_AD_B1_09_LPUART2_RX }, \
    { IOMUXC_GPIO_AD_B0_14_LPUART3_TX }, { IOMUXC_GPIO_AD_B0_15_LPUART3_RX }, \
    { IOMUXC_GPIO_AD_B1_10_LPUART4_TX }, { IOMUXC_GPIO_AD_B1_11_LPUART4_RX }, \
    { IOMUXC_GPIO_AD_B0_10_LPUART5_TX }, { IOMUXC_GPIO_AD_B0_11_LPUART5_RX }, \
    { 0 }, { 0 }, \
    { 0 }, { 0 }, \
    { IOMUXC_GPIO_SD_B1_02_LPUART8_TX }, { IOMUXC_GPIO_SD_B1_03_LPUART8_RX },

#define IOMUX_TABLE_UART_CTS_RTS \
    { IOMUXC_GPIO_AD_B0_08_LPUART1_CTS_B }, { IOMUXC_GPIO_AD_B0_09_LPUART1_RTS_B }, \
    { IOMUXC_GPIO_AD_B1_06_LPUART2_CTS_B }, { IOMUXC_GPIO_AD_B1_07_LPUART2_RTS_B }, \
    { IOMUXC_GPIO_AD_B0_12_LPUART3_CTS_B }, { IOMUXC_GPIO_AD_B0_13_LPUART3_RTS_B }, \
    { IOMUXC_GPIO_EMC_00_LPUART4_CTS_B }, { IOMUXC_GPIO_EMC_01_LPUART4_RTS_B }, \
    { IOMUXC_GPIO_EMC_36_LPUART5_CTS_B }, { IOMUXC_GPIO_EMC_37_LPUART5_RTS_B }, \
    { 0 }, { 0 }, \
    { 0 }, { 0 }, \
    { IOMUXC_GPIO_EMC_24_LPUART8_CTS_B }, { IOMUXC_GPIO_EMC_25_LPUART8_RTS_B },

#define MICROPY_HW_SPI_INDEX { 1, 3 }

#define IOMUX_TABLE_SPI \
    { pin_GPIO_AD_B0_10, IOMUXC_GPIO_AD_B0_10_LPSPI1_SCK }, { pin_GPIO_AD_B0_11, IOMUXC_GPIO_AD_B0_11_LPSPI1_PCS0 }, \
    { pin_GPIO_AD_B0_12, IOMUXC_GPIO_AD_B0_12_LPSPI1_SDO }, { pin_GPIO_AD_B0_13, IOMUXC_GPIO_AD_B0_13_LPSPI1_SDI }, \
    { 0 }, { 0 }, \
    { 0 }, \
    { 0 }, { 0 }, \
    { 0 }, { 0 }, \
    { 0 }, { 0 }, \
    { 0 }, \
    { pin_GPIO_AD_B1_12, IOMUXC_GPIO_AD_B1_12_LPSPI3_SCK }, { pin_GPIO_AD_B1_13, IOMUXC_GPIO_AD_B1_13_LPSPI3_PCS0 }, \
    { pin_GPIO_AD_B1_14, IOMUXC_GPIO_AD_B1_14_LPSPI3_SDO }, { pin_GPIO_AD_B1_15, IOMUXC_GPIO_AD_B1_15_LPSPI3_SDI }, \
    { pin_GPIO_AD_B1_09, IOMUXC_GPIO_AD_B1_09_LPSPI3_PCS1 }, { 0 }, \
    { 0 }

#define DMA_REQ_SRC_RX { 0, kDmaRequestMuxLPSPI1Rx, kDmaRequestMuxLPSPI2Rx, \
                         kDmaRequestMuxLPSPI3Rx, kDmaRequestMuxLPSPI4Rx }

#define DMA_REQ_SRC_TX { 0, kDmaRequestMuxLPSPI1Tx, kDmaRequestMuxLPSPI2Tx, \
                         kDmaRequestMuxLPSPI3Tx, kDmaRequestMuxLPSPI4Tx }

// Define mapping hardware I2C # to logical I2C #
// SDA/SCL  HW-I2C    Logical I2C
// D14/D15  LPI2C4 ->    0
// A4/A5    LPI2C1 ->    1
// D0/D1    LPI2C2 ->    2

#define MICROPY_HW_I2C_INDEX   { 4, 1, 2 }

#define IOMUX_TABLE_I2C \
    { IOMUXC_GPIO_AD_B1_14_LPI2C1_SCL }, { IOMUXC_GPIO_AD_B1_15_LPI2C1_SDA }, \
    { IOMUXC_GPIO_AD_B1_08_LPI2C2_SCL }, { IOMUXC_GPIO_AD_B1_09_LPI2C2_SDA }, \
    { 0 }, { 0 }, \
    { IOMUXC_GPIO_SD_B1_02_LPI2C4_SCL }, { IOMUXC_GPIO_SD_B1_03_LPI2C4_SDA },

#define MICROPY_PY_MACHINE_I2S (1)
#define MICROPY_HW_I2S_NUM (1)
#define I2S_CLOCK_MUX { 0, kCLOCK_Sai1Mux, kCLOCK_Sai2Mux }
#define I2S_CLOCK_PRE_DIV { 0, kCLOCK_Sai1PreDiv, kCLOCK_Sai2PreDiv }
#define I2S_CLOCK_DIV { 0, kCLOCK_Sai1Div, kCLOCK_Sai2Div }
#define I2S_IOMUXC_GPR_MODE { 0, kIOMUXC_GPR_SAI1MClkOutputDir, kIOMUXC_GPR_SAI2MClkOutputDir }
#define I2S_DMA_REQ_SRC_RX { 0, kDmaRequestMuxSai1Rx, kDmaRequestMuxSai2Rx }
#define I2S_DMA_REQ_SRC_TX { 0, kDmaRequestMuxSai1Tx, kDmaRequestMuxSai2Tx }
#define I2S_AUDIO_PLL_CLOCK (2U)

#define I2S_GPIO(_hwid, _fn, _mode, _pin, _iomux) \
    { \
        .hw_id = _hwid, \
        .fn = _fn, \
        .mode = _mode, \
        .name = MP_QSTR_##_pin, \
        .iomux = {_iomux}, \
    }

#define I2S_GPIO_MAP \
    { \
        I2S_GPIO(1, MCK, TX, GPIO_AD_B1_00, IOMUXC_GPIO_AD_B1_00_SAI1_MCLK), \
        I2S_GPIO(1, SCK, RX, GPIO_AD_B1_06, IOMUXC_GPIO_AD_B1_06_SAI1_RX_BCLK), \
        I2S_GPIO(1, WS, RX, GPIO_AD_B1_04, IOMUXC_GPIO_AD_B1_04_SAI1_RX_SYNC), \
        I2S_GPIO(1, SD, RX, GPIO_AD_B1_05, IOMUXC_GPIO_AD_B1_05_SAI1_RX_DATA00),  \
        I2S_GPIO(1, SCK, TX, GPIO_AD_B1_01, IOMUXC_GPIO_AD_B1_01_SAI1_TX_BCLK), \
        I2S_GPIO(1, WS, TX, GPIO_AD_B1_02, IOMUXC_GPIO_AD_B1_02_SAI1_TX_SYNC),  \
        I2S_GPIO(1, SD, TX, GPIO_AD_B1_03, IOMUXC_GPIO_AD_B1_03_SAI1_TX_DATA00), \
    }


#define USDHC_DUMMY_PIN NULL, 0
#define MICROPY_USDHC1 \
    { \
        .cmd = {GPIO_SD_B0_02_USDHC1_CMD}, \
        .clk = { GPIO_SD_B0_03_USDHC1_CLK }, \
        .cd_b = { GPIO_SD_B0_06_USDHC1_CD_B }, \
        .data0 = { GPIO_SD_B0_04_USDHC1_DATA0 }, \
        .data1 = { GPIO_SD_B0_05_USDHC1_DATA1 }, \
        .data2 = { GPIO_SD_B0_00_USDHC1_DATA2 }, \
        .data3 = { GPIO_SD_B0_01_USDHC1_DATA3 }, \
    }

// --- SEMC --- //
#define MIMXRT_IOMUXC_SEMC_DATA00 IOMUXC_GPIO_EMC_00_SEMC_DATA00
#define MIMXRT_IOMUXC_SEMC_DATA01 IOMUXC_GPIO_EMC_01_SEMC_DATA01
#define MIMXRT_IOMUXC_SEMC_DATA02 IOMUXC_GPIO_EMC_02_SEMC_DATA02
#define MIMXRT_IOMUXC_SEMC_DATA03 IOMUXC_GPIO_EMC_03_SEMC_DATA03
#define MIMXRT_IOMUXC_SEMC_DATA04 IOMUXC_GPIO_EMC_04_SEMC_DATA04
#define MIMXRT_IOMUXC_SEMC_DATA05 IOMUXC_GPIO_EMC_05_SEMC_DATA05
#define MIMXRT_IOMUXC_SEMC_DATA06 IOMUXC_GPIO_EMC_06_SEMC_DATA06
#define MIMXRT_IOMUXC_SEMC_DATA07 IOMUXC_GPIO_EMC_07_SEMC_DATA07
#define MIMXRT_IOMUXC_SEMC_DATA08 IOMUXC_GPIO_EMC_32_SEMC_DATA08
#define MIMXRT_IOMUXC_SEMC_DATA09 IOMUXC_GPIO_EMC_33_SEMC_DATA09
#define MIMXRT_IOMUXC_SEMC_DATA10 IOMUXC_GPIO_EMC_34_SEMC_DATA10
#define MIMXRT_IOMUXC_SEMC_DATA11 IOMUXC_GPIO_EMC_35_SEMC_DATA11
#define MIMXRT_IOMUXC_SEMC_DATA12 IOMUXC_GPIO_EMC_36_SEMC_DATA12
#define MIMXRT_IOMUXC_SEMC_DATA13 IOMUXC_GPIO_EMC_37_SEMC_DATA13
#define MIMXRT_IOMUXC_SEMC_DATA14 IOMUXC_GPIO_EMC_38_SEMC_DATA14
#define MIMXRT_IOMUXC_SEMC_DATA15 IOMUXC_GPIO_EMC_39_SEMC_DATA15

#define MIMXRT_IOMUXC_SEMC_ADDR00 IOMUXC_GPIO_EMC_16_SEMC_ADDR00
#define MIMXRT_IOMUXC_SEMC_ADDR01 IOMUXC_GPIO_EMC_17_SEMC_ADDR01
#define MIMXRT_IOMUXC_SEMC_ADDR02 IOMUXC_GPIO_EMC_18_SEMC_ADDR02
#define MIMXRT_IOMUXC_SEMC_ADDR03 IOMUXC_GPIO_EMC_19_SEMC_ADDR03
#define MIMXRT_IOMUXC_SEMC_ADDR04 IOMUXC_GPIO_EMC_20_SEMC_ADDR04
#define MIMXRT_IOMUXC_SEMC_ADDR05 IOMUXC_GPIO_EMC_21_SEMC_ADDR05
#define MIMXRT_IOMUXC_SEMC_ADDR06 IOMUXC_GPIO_EMC_22_SEMC_ADDR06
#define MIMXRT_IOMUXC_SEMC_ADDR07 IOMUXC_GPIO_EMC_23_SEMC_ADDR07
#define MIMXRT_IOMUXC_SEMC_ADDR08 IOMUXC_GPIO_EMC_24_SEMC_ADDR08
#define MIMXRT_IOMUXC_SEMC_ADDR09 IOMUXC_GPIO_EMC_25_SEMC_ADDR09
#define MIMXRT_IOMUXC_SEMC_ADDR10 IOMUXC_GPIO_EMC_15_SEMC_ADDR10
#define MIMXRT_IOMUXC_SEMC_ADDR11 IOMUXC_GPIO_EMC_26_SEMC_ADDR11
#define MIMXRT_IOMUXC_SEMC_ADDR12 IOMUXC_GPIO_EMC_27_SEMC_ADDR12

#define MIMXRT_IOMUXC_SEMC_BA0 IOMUXC_GPIO_EMC_13_SEMC_BA0
#define MIMXRT_IOMUXC_SEMC_BA1 IOMUXC_GPIO_EMC_14_SEMC_BA1
#define MIMXRT_IOMUXC_SEMC_CAS IOMUXC_GPIO_EMC_10_SEMC_CAS
#define MIMXRT_IOMUXC_SEMC_CKE IOMUXC_GPIO_EMC_29_SEMC_CKE
#define MIMXRT_IOMUXC_SEMC_CLK IOMUXC_GPIO_EMC_30_SEMC_CLK
#define MIMXRT_IOMUXC_SEMC_DM00 IOMUXC_GPIO_EMC_08_SEMC_DM00
#define MIMXRT_IOMUXC_SEMC_DM01 IOMUXC_GPIO_EMC_31_SEMC_DM01
#define MIMXRT_IOMUXC_SEMC_DQS IOMUXC_GPIO_EMC_28_SEMC_DQS
#define MIMXRT_IOMUXC_SEMC_RAS IOMUXC_GPIO_EMC_11_SEMC_RAS
#define MIMXRT_IOMUXC_SEMC_WE IOMUXC_GPIO_EMC_09_SEMC_WE

#define MIMXRT_IOMUXC_SEMC_CS0 IOMUXC_GPIO_EMC_12_SEMC_CS0

// Network definitions
// Transceiver Phy Address
#define ENET_PHY_ADDRESS    (2)
#define ENET_PHY_OPS        phyksz8081_ops

#define IOMUX_TABLE_ENET \
    { IOMUXC_GPIO_AD_B0_08_ENET_REF_CLK1, 1, 0xB0E9u }, \
    { IOMUXC_GPIO_AD_B0_09_ENET_RDATA01, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_AD_B0_10_ENET_RDATA00, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_AD_B0_11_ENET_RX_EN, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_AD_B0_12_ENET_RX_ER, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_AD_B0_13_ENET_TX_EN, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_AD_B0_14_ENET_TDATA00, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_AD_B0_15_ENET_TDATA01, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_EMC_40_ENET_MDIO, 0, 0xB0E9u }, \
    { IOMUXC_GPIO_EMC_41_ENET_MDC, 0, 0xB0E9u },
