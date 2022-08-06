# cmake file for Arduino Nano RP2040 Connect.

# This is a 16MB board? Was an 8MB firmware size intentional?
# https://github.com/raspberrypi/pico-sdk/blob/master/src/boards/include/boards/arduino_nano_rp2040_connect.h#L76
set(MICROPY_HW_FLASH_SIZE_BYTES "16 * 1024 * 1024")
set(MICROPY_HW_APP_SIZE_BYTES    "8 * 1024 * 1024")

set(MICROPY_PY_BLUETOOTH  1)
set(MICROPY_BLUETOOTH_NIMBLE  1)
set(MICROPY_PY_NETWORK_NINAW10 1)
set(MICROPY_HW_ENABLE_DOUBLE_TAP 1)
set(MICROPY_FROZEN_MANIFEST ${MICROPY_BOARD_DIR}/manifest.py)
