MCU_SERIES = f4
CMSIS_MCU = STM32F412Rx
AF_FILE = boards/stm32f412_af.csv
LD_FILES = boards/stm32f412rg.ld boards/common_ifs.ld
TEXT0_ADDR = 0x08000000
TEXT1_ADDR = 0x08020000
MICROPY_HW_ENABLE_ISR_UART_FLASH_FUNCS_IN_RAM = 1
