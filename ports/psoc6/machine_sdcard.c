/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2021 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// std includes
#include <string.h>

// micropython includes
#include "py/runtime.h"
#include "extmod/vfs.h"
#include "modpsoc6.h"
#include "mplogger.h"
#include "mphalport.h"

// port-specific includes
#include "modmachine.h"
#include "machine_pin_phy.h"
#include "mplogger.h"

// MTB includes
#include "cyhal.h"
#include "cybsp.h"

#define SDHC_DEFAULT_BUS_WIDTH                  (4U)
#define SDHC_BLOCK_SIZE                         (512UL)
#define SDHC_NUM_OF_SLOTS                       (2)

static cyhal_sdhc_t sdhc_obj[SDHC_NUM_OF_SLOTS];

typedef struct _machine_sdcard_obj_t {
    mp_obj_base_t base;
    uint16_t slot_num;
    machine_pin_phy_obj_t *wp;
    machine_pin_phy_obj_t *cmd;
    machine_pin_phy_obj_t *dat0;
    machine_pin_phy_obj_t *dat1;
    machine_pin_phy_obj_t *dat2;
    machine_pin_phy_obj_t *dat3;
    machine_pin_phy_obj_t *clk;
    machine_pin_phy_obj_t *cd;
    uint32_t block_count;
} machine_sdcard_obj_t;

enum {
    ARG_slot,
    ARG_width,
    ARG_cd,
    ARG_wp,
    ARG_cmd,
    ARG_dat0,
    ARG_dat1,
    ARG_dat2,
    ARG_dat3,
    ARG_clk,
    ARG_freq,
};


static void sd_card_allocate_pin(machine_sdcard_obj_t *self, mp_arg_val_t *args) {
    if (args[ARG_cmd].u_obj != mp_const_none) {
        self->cmd = pin_phy_realloc(args[ARG_cmd].u_obj, PIN_PHY_FUNC_SDHC);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Cmd pin must be provided"));
    }

    if (args[ARG_dat0].u_obj != mp_const_none) {
        self->dat0 = pin_phy_realloc(args[ARG_dat0].u_obj, PIN_PHY_FUNC_SDHC);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Data 0 pin must be provided"));
    }

    if (args[ARG_dat1].u_obj != mp_const_none) {
        self->dat1 = pin_phy_realloc(args[ARG_dat1].u_obj, PIN_PHY_FUNC_SDHC);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Data 1 pin must be provided"));
    }

    if (args[ARG_dat2].u_obj != mp_const_none) {
        self->dat2 = pin_phy_realloc(args[ARG_dat2].u_obj, PIN_PHY_FUNC_SDHC);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Data 2 pin must be provided"));
    }

    if (args[ARG_dat3].u_obj != mp_const_none) {
        self->dat3 = pin_phy_realloc(args[ARG_dat3].u_obj, PIN_PHY_FUNC_SDHC);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Data 3 pin must be provided"));
    }

    if (args[ARG_clk].u_obj != mp_const_none) {
        self->clk = pin_phy_realloc(args[ARG_clk].u_obj, PIN_PHY_FUNC_SDHC);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Clk pin must be provided"));
    }

    if (args[ARG_cd].u_obj != mp_const_none) {
        self->cd = pin_phy_realloc(args[ARG_cd].u_obj, PIN_PHY_FUNC_SDHC);
    }

    if (args[ARG_wp].u_obj != mp_const_none) {
        self->wp = pin_phy_realloc(args[ARG_wp].u_obj, PIN_PHY_FUNC_SDHC);
    }
}

static cy_rslt_t sd_card_init_helper(machine_sdcard_obj_t *self, mp_arg_val_t *args) {

    cyhal_sdhc_config_t sdhc_config;

    sdhc_config.enableLedControl = false;
    sdhc_config.lowVoltageSignaling = false;
    sdhc_config.isEmmc = false;
    sdhc_config.busWidth = SDHC_DEFAULT_BUS_WIDTH;

    cy_rslt_t result = CY_RSLT_SUCCESS;

    cyhal_clock_t clock;
    cyhal_resource_inst_t rsc = {CYHAL_RSC_CLOCK, 18, 2};
    cyhal_clock_get(&clock, &rsc);

    sd_card_allocate_pin(self, args);
    result = cyhal_sdhc_init(&sdhc_obj[self->slot_num], &sdhc_config, self->cmd->addr, self->clk->addr, self->dat0->addr, self->dat1->addr,
        self->dat2->addr, self->dat3->addr, NC, NC, NC, NC, self->cd->addr, NC, NC, NC, NC, NC, &clock);
    return result;
}

static mp_obj_t machine_sdcard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {

    mp_arg_check_num(n_args, n_kw, 0, 9, true);

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_slot,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_width,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 4} },
        { MP_QSTR_cd,       MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_wp,       MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_cmd,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_dat0,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_dat1,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_dat2,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_dat3,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_clk,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = mp_const_none} },
        { MP_QSTR_freq,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 20000000} },
    };

    // Parse the arguments.
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    machine_sdcard_obj_t *self = mp_obj_malloc_with_finaliser(machine_sdcard_obj_t, &machine_sdcard_type);

    self->slot_num = mp_obj_get_int(args[ARG_slot].u_obj);

    if (self->slot_num >= SDHC_NUM_OF_SLOTS) {
        mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("psoc6_sdcard_make_new() - SD card slot unavailable !\n"));
    }

    if (sdhc_obj[self->slot_num].sdxx.base == NULL) {
        cy_rslt_t result = sd_card_init_helper(self, args);

        if (CY_RSLT_SUCCESS == result) {
            cyhal_sdhc_get_block_count(&sdhc_obj[self->slot_num], (uint32_t *)&self->block_count);
        } else {
            mp_raise_msg(&mp_type_Exception, MP_ERROR_TEXT("psoc6_sdcard_make_new() - SD card init failed !\n"));
        }
    }
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t machine_sdcard_readblocks(size_t n_args, const mp_obj_t *args) {

    machine_sdcard_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t block_address = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);

    if (n_args == 4) {
        block_address += mp_obj_get_int(args[3]);
    }

    size_t length = bufinfo.len / SDHC_BLOCK_SIZE;
    cy_rslt_t result = cyhal_sdhc_read(&sdhc_obj[self->slot_num], block_address, bufinfo.buf, &length);

    if (CY_RSLT_SUCCESS != result) {
        mp_raise_ValueError(MP_ERROR_TEXT("machine_sdcard_readblocks() - SD Card Read failed !"));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_sdcard_readblocks_obj, 3, 4, machine_sdcard_readblocks);

static mp_obj_t machine_sdcard_writeblocks(size_t n_args, const mp_obj_t *args) {

    machine_sdcard_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t block_address = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);

    size_t length = bufinfo.len / SDHC_BLOCK_SIZE;

    if (n_args == 3) {
        cy_rslt_t result = cyhal_sdhc_erase(&sdhc_obj[self->slot_num], block_address, bufinfo.len, 0);

        if (CY_RSLT_SUCCESS != result) {
            mp_raise_ValueError(MP_ERROR_TEXT("machine_sdcard_writeblocks() - SD Card Erase failed !"));
        }
    } else {
        block_address += mp_obj_get_int(args[3]);
    }

    cy_rslt_t result = cyhal_sdhc_write(&sdhc_obj[self->slot_num], block_address, bufinfo.buf, &length);
    if (CY_RSLT_SUCCESS != result) {
        mp_raise_ValueError(MP_ERROR_TEXT("machine_sdcard_writeblocks() - SD Card Write failed!"));
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_sdcard_writeblocks_obj, 3, 4, machine_sdcard_writeblocks);

static mp_obj_t machine_sdcard_ioctl(mp_obj_t self_in, mp_obj_t cmd_in, mp_obj_t arg_in) {
    machine_sdcard_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t cmd = mp_obj_get_int(cmd_in);

    switch (cmd) {
        case MP_BLOCKDEV_IOCTL_INIT:
            return MP_OBJ_NEW_SMALL_INT(0);
        case MP_BLOCKDEV_IOCTL_DEINIT:
            mod_sdcard_deinit();
            return MP_OBJ_NEW_SMALL_INT(0);
        case MP_BLOCKDEV_IOCTL_SYNC:
            return MP_OBJ_NEW_SMALL_INT(0);
        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT:
            return MP_OBJ_NEW_SMALL_INT(self->block_count);
        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE:
            return MP_OBJ_NEW_SMALL_INT(SDHC_BLOCK_SIZE);
        case MP_BLOCKDEV_IOCTL_BLOCK_ERASE: {
            uint32_t offset = mp_obj_get_int(arg_in);
            cy_rslt_t result = cyhal_sdhc_erase(&sdhc_obj[self->slot_num], offset, 1, 0);

            if (CY_RSLT_SUCCESS != result) {
                mp_raise_ValueError(MP_ERROR_TEXT("machine_sdcard_ioctl() - SD Card erase failed !"));
            }
            return MP_OBJ_NEW_SMALL_INT(0);
        }
        default:
            return mp_const_none;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_3(machine_sdcard_ioctl_obj, machine_sdcard_ioctl);

void mod_sdcard_deinit(void) {
    for (int i = 0; i < SDHC_NUM_OF_SLOTS; i++)
    {
        if (sdhc_obj[i].sdxx.base != NULL) {
            cyhal_sdhc_free(&sdhc_obj[i]);
        }
    }
}

static const mp_rom_map_elem_t machine_sdcard_locals_dict_table[] = {
    // block device protocol
    { MP_ROM_QSTR(MP_QSTR_readblocks),  MP_ROM_PTR(&machine_sdcard_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&machine_sdcard_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl),       MP_ROM_PTR(&machine_sdcard_ioctl_obj) },
};
static MP_DEFINE_CONST_DICT(machine_sdcard_locals_dict, machine_sdcard_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    machine_sdcard_type,
    MP_QSTR_SDCard,
    MP_TYPE_FLAG_NONE,
    make_new, machine_sdcard_make_new,
    locals_dict, &machine_sdcard_locals_dict
    );
