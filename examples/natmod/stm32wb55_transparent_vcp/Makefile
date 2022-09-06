# This module is intended for use on stm32wb55 microcontrollers
# and shows how a native module can be used to extend hardware
# functionality.

# Note: `pip install pyelftools` is required to build native modules.

# Location of top-level MicroPython directory
MPY_DIR = ../../..

# Name of module
MOD = rfcore_transparent

# Architecture to build for (x86, x64, armv7m, xtensa, xtensawin)
ARCH = armv7m

# Source files (.c or .py)
SRC = \
  stm32wb55_transparent.c \
  stm32wb55_local_commands.c \
  rfcore_transparent.py

MCU_SERIES = wb
CMSIS_MCU = STM32WB55xx

CFLAGS += -Os
CFLAGS += -D$(CMSIS_MCU) -DUSE_FULL_LL_DRIVER $(CFLAGS_MCU_$(MCU_SERIES))

CFLAGS += -I$(MPY_DIR)/ports/stm32
CFLAGS += -I$(MPY_DIR)/ports/stm32/boards/NUCLEO_WB55
CFLAGS += -I$(MPY_DIR)/lib/cmsis/inc
CFLAGS += -I$(MPY_DIR)/lib/stm32lib/STM32WBxx_HAL_Driver/Inc
CFLAGS += -I$(MPY_DIR)/lib/stm32lib/CMSIS/STM32WBxx/Include

include $(MPY_DIR)/py/dynruntime.mk
