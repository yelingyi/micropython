# Select the variant to build for.
VARIANT ?= standard

# If the build directory is not given, make it reflect the variant name.
BUILD ?= build-$(VARIANT)

VARIANT_DIR ?= variants/$(VARIANT)
ifeq ($(wildcard $(VARIANT_DIR)/.),)
$(error Invalid VARIANT specified: $(VARIANT_DIR))
endif

include ../../py/mkenv.mk
-include mpconfigport.mk
include $(VARIANT_DIR)/mpconfigvariant.mk

FROZEN_MANIFEST ?= variants/manifest.py

# Define main target
# This should be configured by the mpconfigvariant.mk
PROG ?= micropython

# qstr definitions (must come before including py.mk)
QSTR_DEFS = ../unix/qstrdefsport.h
QSTR_GLOBAL_DEPENDENCIES = $(VARIANT_DIR)/mpconfigvariant.h

# include py core make definitions
include $(TOP)/py/py.mk

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(VARIANT_DIR)

# compiler settings
CFLAGS = $(INC) -Wall -Wpointer-arith -Wdouble-promotion -Werror -std=gnu99 -DUNIX -D__USE_MINGW_ANSI_STDIO=1 $(CFLAGS_MOD) $(COPT) $(CFLAGS_EXTRA)
LDFLAGS = $(LDFLAGS_MOD) -lm $(LDFLAGS_EXTRA)

# Debugging/Optimization
ifdef DEBUG
CFLAGS += -g
COPT = -O0
else
COPT = -Os #-DNDEBUG
endif

# source files
SRC_C = \
	shared/libc/printf.c \
	shared/runtime/gchelper_generic.c \
	ports/unix/main.c \
	ports/unix/input.c \
	ports/unix/modos.c \
	ports/unix/modmachine.c \
	ports/unix/modtime.c \
	ports/unix/gccollect.c \
	ports/unix/mpbtstackport_common.c \
	ports/unix/mpbtstackport_h4.c \
	ports/unix/mpbtstackport_usb.c \
	ports/unix/mpnimbleport.c \
	mpbthciport.c \
	windows_mphal.c \
	realpath.c \
	init.c \
	sleep.c \
	fmode.c \
	$(SRC_MOD) \
	$(wildcard $(VARIANT_DIR)/*.c)

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))

ifeq ($(MICROPY_USE_READLINE),1)
CFLAGS_MOD += -DMICROPY_USE_READLINE=1
SRC_C += shared/readline/readline.c
endif

LIB += -lws2_32

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_CXX) $(SHARED_SRC_C) $(EXTMOD_SRC_C)
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS +=

ifneq ($(FROZEN_MANIFEST),)
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool -DMICROPY_MODULE_FROZEN_MPY=1 -DMPZ_DIG_SIZE=16
endif

# If the variant enables it, enable modbluetooth.
ifeq ($(MICROPY_PY_BLUETOOTH),1)

# Only one stack can be enabled.
ifeq ($(MICROPY_BLUETOOTH_NIMBLE),1)
ifeq ($(MICROPY_BLUETOOTH_BTSTACK),1)
$(error Cannot enable both NimBLE and BTstack at the same time)
endif
endif

# Default to btstack, but a variant (or make command line) can set NimBLE
# explicitly (which is always via H4 UART).
ifneq ($(MICROPY_BLUETOOTH_NIMBLE),1)
ifneq ($(MICROPY_BLUETOOTH_BTSTACK),1)
MICROPY_BLUETOOTH_BTSTACK ?= 1
endif
endif

CFLAGS_MOD += -mno-ms-bitfields
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH=1
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH_ENABLE_CENTRAL_MODE=1

ifeq ($(MICROPY_BLUETOOTH_BTSTACK),1)

# Figure out which BTstack transport to use.
ifeq ($(MICROPY_BLUETOOTH_BTSTACK_H4),1)
ifeq ($(MICROPY_BLUETOOTH_BTSTACK_USB),1)
$(error Cannot enable BTstack support for USB and H4 UART at the same time)
endif
else
ifeq ($(HAVE_LIBUSB),1)
# Default to btstack-over-usb.
MICROPY_BLUETOOTH_BTSTACK_USB ?= 1
else
# Fallback to HCI controller via a H4 UART (e.g. Zephyr on nRF) over a /dev/tty serial port.
MICROPY_BLUETOOTH_BTSTACK_H4 ?= 1
endif
endif

# BTstack is enabled.
GIT_SUBMODULES += lib/btstack
include $(TOP)/extmod/btstack/btstack.mk
SRC_BTSTACK += lib/btstack/platform/embedded/btstack_run_loop_embedded.c

else

# NimBLE is enabled.
GIT_SUBMODULES += lib/mynewt-nimble
CFLAGS_MOD += -DMICROPY_PY_BLUETOOTH_ENABLE_L2CAP_CHANNELS=1
include $(TOP)/extmod/nimble/nimble.mk

endif

endif

include $(TOP)/py/mkrules.mk

.PHONY: test test_full

RUN_TESTS_SKIP += -e math_fun -e float2int_double -e float_parse -e math_domain_special

test: $(PROG) $(TOP)/tests/run-tests.py
	$(eval DIRNAME=ports/$(notdir $(CURDIR)))
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) $(PYTHON) ./run-tests.py $(RUN_TESTS_SKIP)

test_full: test
	$(eval DIRNAME=ports/$(notdir $(CURDIR)))
	cd $(TOP)/tests && MICROPY_MICROPYTHON=../$(DIRNAME)/$(PROG) $(PYTHON) ./run-tests.py --via-mpy $(RUN_TESTS_MPY_CROSS_FLAGS) $(RUN_TESTS_SKIP) -d basics float micropython
