/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include <stdio.h>
#include <string.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "pin.h"
#include "i2c.h"
#include "accel.h"

#if MICROPY_HW_HAS_MMA7660 || MICROPY_HW_HAS_KXTJ3 || MICROPY_HW_HAS_LIS2DH12TR

/// \moduleref pyb
/// \class Accel - accelerometer control
///
/// Accel is an object that controls the MMA7660 or the KXTJ3 accelerometer
/// depending on one/two constant in mpconfigboard.h file of board project :
/// #define MICROPY_HW_HAS_MMA7660      (1)
/// #define MICROPY_HW_HAS_KXTJ3        (0) // not mandatory if equal to 0
///
/// Example usage:
///
///     accel = pyb.Accel()
///     for i in range(10):
///         print(accel.x(), accel.y(), accel.z())
///
/// Raw values are between -32 and 31 for -/+ 1.5G acceleration for MMA7660.
/// Raw values are between -128 and 127 for -/+ 8G acceleration for KXTJ3.


#define I2C_TIMEOUT_MS (50)

#if MICROPY_HW_HAS_MMA7660

#define ACCEL_ADDR                  (76)
#define ACCEL_REG_X                 (0)
#define ACCEL_REG_Y                 (1)
#define ACCEL_REG_Z                 (2)
#define ACCEL_REG_TILT              (3)
#define ACCEL_REG_MODE              (7)
#define ACCEL_AXIS_SIGNED_VALUE(i)  (((i) & 0x3f) | ((i) & 0x20 ? (~0x1f) : 0))

#elif MICROPY_HW_HAS_KXTJ3

#define ACCEL_ADDR                  (0x0f)
#define ACCEL_REG_DCST_RESP         (0x0c)
#define ACCEL_REG_WHO_AM_I          (0x0f)
#define ACCEL_REG_X                 (0x07) // XOUT_H
#define ACCEL_REG_Y                 (0x09) // YOUT_H
#define ACCEL_REG_Z                 (0x0B) // ZOUT_H
#define ACCEL_REG_CTRL_REG1         (0x1B)
#define ACCEL_REG_CTRL_REG2         (0x1d)
#define ACCEL_REG_CTRL_REG2         (0x1d)
#define ACCEL_REG_DATA_CTRL_REG     (0x21)
#define ACCEL_AXIS_SIGNED_VALUE(i)  (((i) & 0x7f) | ((i) & 0x80 ? (~0x7f) : 0))
#elif MICROPY_HW_HAS_LIS2DH12TR
#define ACCEL_ADDR                  (0x19)
#define ACCEL_ADDR_SUB              (0x80)
#define ACCEL_REG_STATUS_REG_AUX    (0x07)
#define ACCEL_REG_OUT_TEMP_L        (0x0C)
#define ACCEL_REG_OUT_TEMP_H        (0x0D)
#define ACCEL_REG_WHO_AM_I          (0x0f)

#define ACCEL_REG_CTRL_REG0         (0x1E)
#define ACCEL_REG_TEMP_CFG_REG      (0x1F)
#define ACCEL_REG_CTRL_REG1         (0x20)
#define ACCEL_REG_CTRL_REG2         (0x21)
#define ACCEL_REG_CTRL_REG3         (0x22)
#define ACCEL_REG_CTRL_REG4         (0x23)
#define ACCEL_REG_CTRL_REG5         (0x24)
#define ACCEL_REG_CTRL_REG6         (0x25)
#define ACCEL_REG_REFERENCE         (0x26)

#define ACCEL_REG_STATUS_REG        (0x27)

#define ACCEL_REG_X                 (0x28) // XOUT_L
#define ACCEL_REG_Y                 (0x2A) // YOUT_L
#define ACCEL_REG_Z                 (0x2C) // ZOUT_L

#define ACCEL_REG_FIFO_CTRL_REG     (0x2E)
#define ACCEL_REG_FIFO_SRC_REG      (0x2F)

#define ACCEL_REG_INT1_CFG          (0x30)
#define ACCEL_REG_INT1_SRC          (0x31)
#define ACCEL_REG_INT1_THS          (0x32)
#define ACCEL_REG_INT1_DURATION     (0x33)

#define ACCEL_REG_INT2_CFG          (0x34)
#define ACCEL_REG_INT2_SRC          (0x35)
#define ACCEL_REG_INT2_THS          (0x36)
#define ACCEL_REG_INT2_DURATION     (0x37)

#define ACCEL_REG_CLICK_CFG         (0x38)
#define ACCEL_REG_CLICK_SRC         (0x39)
#define ACCEL_REG_CLICK_THS         (0x3A)

#define ACCEL_REG_TIME_LIMIT        (0x3B)
#define ACCEL_REG_TIME_LATENCY      (0x3C)
#define ACCEL_REG_TIME_WINDOW       (0x3D)

#define ACCEL_REG_ACT_THS           (0x3E)
#define ACCEL_REG_ACT_DUR           (0x3F)


#define ACCEL_AXIS_SIGNED_VALUE(lsb)  (lsb>>4)*1
// #define LIS2DH12_FROM_FS_2g_HR_TO_mg(lsb)  (float)((int16_t)lsb>>4)* 1.0f
// #define LIS2DH12_FROM_FS_4g_HR_TO_mg(lsb)  (float)((int16_t)lsb>>4)* 2.0f
// #define LIS2DH12_FROM_FS_8g_HR_TO_mg(lsb)  (float)((int16_t)lsb>>4)* 4.0f
// #define LIS2DH12_FROM_FS_16g_HR_TO_mg(lsb) (float)((int16_t)lsb>>4)*12.0f
// #define LIS2DH12_FROM_LSB_TO_degC_HR(lsb) (float)((int16_t)lsb>>6)/4.0f+25.0f

// #define LIS2DH12_FROM_FS_2g_NM_TO_mg(lsb)  (float)((int16_t)lsb>>6)*  4.0f
// #define LIS2DH12_FROM_FS_4g_NM_TO_mg(lsb)  (float)((int16_t)lsb>>6)*  8.0f
// #define LIS2DH12_FROM_FS_8g_NM_TO_mg(lsb)  (float)((int16_t)lsb>>6)* 16.0f
// #define LIS2DH12_FROM_FS_16g_NM_TO_mg(lsb) (float)((int16_t)lsb>>6)* 48.0f
// #define LIS2DH12_FROM_LSB_TO_degC_NM(lsb) (float)((int16_t)lsb>>6)/4.0f+25.0f

// #define LIS2DH12_FROM_FS_2g_LP_TO_mg(lsb)  (float)((int16_t)lsb>>8)*16.0f
// #define LIS2DH12_FROM_FS_4g_LP_TO_mg(lsb)  (float)((int16_t)lsb>>8)*32.0f
// #define LIS2DH12_FROM_FS_8g_LP_TO_mg(lsb)  (float)((int16_t)lsb>>8)*64.0f
// #define LIS2DH12_FROM_FS_16g_LP_TO_mg(lsb) (float)((int16_t)lsb>>8)*192.0f
// #define LIS2DH12_FROM_LSB_TO_degC_LP(lsb) (float)((int16_t)lsb>>8)*1.0f + 25.0f
#endif

void accel_init(void) {
    #if MICROPY_HW_HAS_MMA7660 || MICROPY_HW_HAS_LIS2DH12TR
    // PB5 is connected to AVDD; pull high to enable MMA accel device
    mp_hal_pin_low(MICROPY_HW_MMA_AVDD_PIN); // turn off AVDD
    mp_hal_pin_output(MICROPY_HW_MMA_AVDD_PIN);
    #endif
}

static void accel_start(void) {
    // start the I2C bus in master mode
    i2c_init(I2C1, MICROPY_HW_I2C1_SCL, MICROPY_HW_I2C1_SDA, 400000, I2C_TIMEOUT_MS);

    #if MICROPY_HW_HAS_MMA7660

    // turn off AVDD, wait 30ms, turn on AVDD, wait 30ms again
    mp_hal_pin_low(MICROPY_HW_MMA_AVDD_PIN); // turn off
    mp_hal_delay_ms(30);
    mp_hal_pin_high(MICROPY_HW_MMA_AVDD_PIN); // turn on
    mp_hal_delay_ms(30);

    int ret;
    for (int i = 0; i < 4; i++) {
        ret = i2c_writeto(I2C1, ACCEL_ADDR, NULL, 0, true);
        if (ret == 0) {
            break;
        }
    }

    if (ret != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("accelerometer not found"));
    }

    // set MMA to active mode
    uint8_t data[2] = {ACCEL_REG_MODE, 1}; // active mode
    i2c_writeto(I2C1, ACCEL_ADDR, data, 2, true);

    // wait for MMA to become active
    mp_hal_delay_ms(30);

    #elif MICROPY_HW_HAS_KXTJ3

    // readout WHO_AM_I register to check KXTJ3 device presence
    uint8_t data[2] = { ACCEL_REG_WHO_AM_I };
    i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
    i2c_readfrom(I2C1, ACCEL_ADDR, data, 1, true);
    if (data[0] != 0x35) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("accelerometer not found"));
    }

    // set operating mode (default: 8 bits, range +/-8G)
    data[0] = ACCEL_REG_CTRL_REG1;
    data[1] = 0x90;
    i2c_writeto(I2C1, ACCEL_ADDR, data, 2, true);
    // set dat output rates to 200Hz (LPF roll-over 10ms), idd=35uA
    data[0] = ACCEL_REG_DATA_CTRL_REG;
    data[1] = 0x04;
    i2c_writeto(I2C1, ACCEL_ADDR, data, 2, true);

    #elif MICROPY_HW_HAS_LIS2DH12TR
        // turn off AVDD, wait 30ms, turn on AVDD, wait 30ms again
    mp_hal_pin_low(MICROPY_HW_MMA_AVDD_PIN); // turn off
    mp_hal_delay_ms(30);
    mp_hal_pin_high(MICROPY_HW_MMA_AVDD_PIN); // turn on
    mp_hal_delay_ms(30);

    int ret;
    for (int i = 0; i < 4; i++) {
        ret = i2c_writeto(I2C1, ACCEL_ADDR, NULL, 0, true);
        if (ret == 0) {
            break;
        }
    }
    if (ret != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("accelerometer not found"));
    }

    uint8_t data[2] = { ACCEL_REG_WHO_AM_I };
    i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
    i2c_readfrom(I2C1, ACCEL_ADDR, data, 1, true);
    if (data[0] != 0x33) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("accelerometer is not LIS2DH12TR"));
    }
    data[0] = ACCEL_REG_CTRL_REG1;
    data[1] = 0x17;//1Hz,xyz enable
    i2c_writeto(I2C1, ACCEL_ADDR, data, 2, true);

    data[0] = ACCEL_REG_CTRL_REG4;
    data[1] = 0x08;//BDU enable,±2g,12bit
    i2c_writeto(I2C1, ACCEL_ADDR, data, 2, true);

    #endif
}

/******************************************************************************/
/* MicroPython bindings                                                      */

#define NUM_AXIS (3)
#define FILT_DEPTH (4)

typedef struct _pyb_accel_obj_t {
    mp_obj_base_t base;
    int16_t buf[NUM_AXIS * FILT_DEPTH];
} pyb_accel_obj_t;

static pyb_accel_obj_t pyb_accel_obj;

/// \classmethod \constructor()
/// Create and return an accelerometer object.
///
/// Note: if you read accelerometer values immediately after creating this object
/// you will get 0.  It takes around 20ms for the first sample to be ready, so,
/// unless you have some other code between creating this object and reading its
/// values, you should put a `pyb.delay(20)` after creating it.  For example:
///
///     accel = pyb.Accel()
///     pyb.delay(20)
///     print(accel.x())
static mp_obj_t pyb_accel_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // init accel object
    pyb_accel_obj.base.type = &pyb_accel_type;
    accel_start();

    return MP_OBJ_FROM_PTR(&pyb_accel_obj);
}

static mp_obj_t read_axis(int axis) {

    #if MICROPY_HW_HAS_MMA7660 || MICROPY_HW_HAS_KXTJ3
    uint8_t data[1] = { axis };
    i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
    i2c_readfrom(I2C1, ACCEL_ADDR, data, 1, true);
    return mp_obj_new_int(ACCEL_AXIS_SIGNED_VALUE(data[0]));
    #elif MICROPY_HW_HAS_LIS2DH12TR
    uint8_t data[2] = { ACCEL_REG_STATUS_REG };
    do
    {
        data[0] = ACCEL_REG_STATUS_REG;
        i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
        i2c_readfrom(I2C1, ACCEL_ADDR, data, 1, true);
    } while (!(data[0]&0x80));
    data[0] = axis|ACCEL_ADDR_SUB;//set SUB
    i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
    i2c_readfrom(I2C1, ACCEL_ADDR, data, 2, true);
    int16_t acc = data[1];
    acc = (acc<<8)|data[0];
    return mp_obj_new_int(ACCEL_AXIS_SIGNED_VALUE(acc));
    #endif
}

/// \method x()
/// Get the x-axis value.
static mp_obj_t pyb_accel_x(mp_obj_t self_in) {
    return read_axis(ACCEL_REG_X);
}
static MP_DEFINE_CONST_FUN_OBJ_1(pyb_accel_x_obj, pyb_accel_x);

/// \method y()
/// Get the y-axis value.
static mp_obj_t pyb_accel_y(mp_obj_t self_in) {
    return read_axis(ACCEL_REG_Y);
}
static MP_DEFINE_CONST_FUN_OBJ_1(pyb_accel_y_obj, pyb_accel_y);

/// \method z()
/// Get the z-axis value.
static mp_obj_t pyb_accel_z(mp_obj_t self_in) {
    return read_axis(ACCEL_REG_Z);
}
static MP_DEFINE_CONST_FUN_OBJ_1(pyb_accel_z_obj, pyb_accel_z);

/// \method tilt()
/// Get the tilt register.
static mp_obj_t pyb_accel_tilt(mp_obj_t self_in) {
    #if MICROPY_HW_HAS_MMA7660
    uint8_t data[1] = { ACCEL_REG_TILT };
    i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
    i2c_readfrom(I2C1, ACCEL_ADDR, data, 1, true);
    return mp_obj_new_int(data[0]);
    #elif MICROPY_HW_HAS_KXTJ3 || MICROPY_HW_HAS_LIS2DH12TR
    /// No tilt like register with KXTJ3 accelerometer
    return 0;
    #endif
}
static MP_DEFINE_CONST_FUN_OBJ_1(pyb_accel_tilt_obj, pyb_accel_tilt);

/// \method filtered_xyz()
/// Get a 3-tuple of filtered x, y and z values.
static mp_obj_t pyb_accel_filtered_xyz(mp_obj_t self_in) {
    pyb_accel_obj_t *self = MP_OBJ_TO_PTR(self_in);
RETRY:
    memmove(self->buf, self->buf + NUM_AXIS, NUM_AXIS * (FILT_DEPTH - 1) * sizeof(int16_t));

    #if MICROPY_HW_HAS_MMA7660
    const size_t DATA_SIZE = NUM_AXIS;
    const size_t DATA_STRIDE = 1;
    #elif MICROPY_HW_HAS_KXTJ3
    const size_t DATA_SIZE = 5;
    const size_t DATA_STRIDE = 2;
    #elif MICROPY_HW_HAS_LIS2DH12TR
    const size_t DATA_SIZE = 6;
    const size_t DATA_STRIDE = 2;
    #endif
    uint8_t data[DATA_SIZE];
    data[0] = ACCEL_REG_X;
    #if MICROPY_HW_HAS_LIS2DH12TR
    data[0] = ACCEL_REG_X|ACCEL_ADDR_SUB;
    #endif
    i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
    i2c_readfrom(I2C1, ACCEL_ADDR, data, DATA_SIZE, true);

    mp_obj_t tuple[NUM_AXIS];
    for (int i = 0; i < NUM_AXIS; i++) {
        #if MICROPY_HW_HAS_MMA7660 || MICROPY_HW_HAS_KXTJ3
        self->buf[NUM_AXIS * (FILT_DEPTH - 1) + i] = ACCEL_AXIS_SIGNED_VALUE(data[i * DATA_STRIDE]);
        #elif MICROPY_HW_HAS_LIS2DH12TR
        int16_t acc = data[i * DATA_STRIDE+1];
        acc = (acc<<8)|data[i * DATA_STRIDE];
        self->buf[NUM_AXIS * (FILT_DEPTH - 1) + i] = ACCEL_AXIS_SIGNED_VALUE(acc);//9 10 11 = x y z 
        #endif

        int32_t val = 0;
        for (int j = 0; j < FILT_DEPTH; j++) {
            val += self->buf[i + NUM_AXIS * j];
        }
        tuple[i] = mp_obj_new_int(val/FILT_DEPTH);
    }
    if(self->buf[ 0]==0&&self->buf[1]==0&&self->buf[2]==0)
    {
        goto RETRY;
    }

    return mp_obj_new_tuple(3, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_1(pyb_accel_filtered_xyz_obj, pyb_accel_filtered_xyz);

static mp_obj_t pyb_accel_read(mp_obj_t self_in, mp_obj_t reg) {
    uint8_t data[1] = { mp_obj_get_int(reg) };
    i2c_writeto(I2C1, ACCEL_ADDR, data, 1, false);
    i2c_readfrom(I2C1, ACCEL_ADDR, data, 1, true);
    return mp_obj_new_int(data[0]);
}
MP_DEFINE_CONST_FUN_OBJ_2(pyb_accel_read_obj, pyb_accel_read);

static mp_obj_t pyb_accel_write(mp_obj_t self_in, mp_obj_t reg, mp_obj_t val) {
    uint8_t data[2] = { mp_obj_get_int(reg), mp_obj_get_int(val) };
    i2c_writeto(I2C1, ACCEL_ADDR, data, 2, true);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(pyb_accel_write_obj, pyb_accel_write);

static const mp_rom_map_elem_t pyb_accel_locals_dict_table[] = {
    // TODO add init, deinit, and perhaps reset methods
    { MP_ROM_QSTR(MP_QSTR_x), MP_ROM_PTR(&pyb_accel_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_y), MP_ROM_PTR(&pyb_accel_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_z), MP_ROM_PTR(&pyb_accel_z_obj) },
    { MP_ROM_QSTR(MP_QSTR_tilt), MP_ROM_PTR(&pyb_accel_tilt_obj) },
    { MP_ROM_QSTR(MP_QSTR_filtered_xyz), MP_ROM_PTR(&pyb_accel_filtered_xyz_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&pyb_accel_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&pyb_accel_write_obj) },
};

static MP_DEFINE_CONST_DICT(pyb_accel_locals_dict, pyb_accel_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    pyb_accel_type,
    MP_QSTR_Accel,
    MP_TYPE_FLAG_NONE,
    make_new, pyb_accel_make_new,
    locals_dict, &pyb_accel_locals_dict
    );

#endif // MICROPY_HW_HAS_MMA7660 || MICROPY_HW_HAS_KXTJ3
