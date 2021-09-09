MCU_SERIES = MIMXRT1062
MCU_VARIANT = MIMXRT1062DVJ6A

MICROPY_FLOAT_IMPL = double
MICROPY_PY_MACHINE_SDCARD = 1
MICROPY_HW_FLASH_TYPE = qspi_nor_flash
MICROPY_HW_FLASH_SIZE = 0x800000  # 8MB
MICROPY_HW_FLASH_RESERVED ?= 0x1000  # 4KB

MICROPY_BOOT_BUFFER_SIZE = (64 * 1024)
MICROPY_PY_LWIP = 1
MICROPY_PY_USSL = 1
MICROPY_SSL_MBEDTLS = 1

deploy: $(BUILD)/firmware.hex
	teensy_loader_cli --mcu=imxrt1062 -v -w $<
