/*
 * This file is part of the Micro Python project, http://micropython.org/
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

#include "py/runtime.h"
#include "py/mphal.h"
#include "extint.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "usrsw.h"

#if MICROPY_HW_HAS_SWITCH

/// \moduleref pyb
/// \class Switch - switch object
///
/// A Switch object is used to control a push-button switch.
///
/// Usage:
///
///      sw = pyb.Switch()       # create a switch object
///      sw()                    # get state (True if pressed, False otherwise)
///      sw.callback(f)          # register a callback to be called when the
///                              #   switch is pressed down
///      sw.callback(None)       # remove the callback
///
/// Example:
///
///      pyb.Switch().callback(lambda: pyb.LED(1).toggle())

// this function inits the switch GPIO so that it can be used
void switch_init0(void) {
    mp_hal_pin_config(&MICROPY_HW_USRSW_PIN, MP_HAL_PIN_MODE_INPUT, MICROPY_HW_USRSW_PULL, 0);
}

int switch_get(void) {
    int val = ((MICROPY_HW_USRSW_PIN.gpio->IDR & MICROPY_HW_USRSW_PIN.pin_mask) != 0);
    return val == MICROPY_HW_USRSW_PRESSED;
}

/******************************************************************************/
// Micro Python bindings

typedef struct _pyb_switch_obj_t {
    mp_obj_base_t base;
} pyb_switch_obj_t;

STATIC const pyb_switch_obj_t pyb_switch_obj = {{&pyb_switch_type}};

void pyb_switch_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_print_str(print, "Switch()");
}

/// \classmethod \constructor()
/// Create and return a switch object.
STATIC mp_obj_t pyb_switch_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // No need to clear the callback member: if it's already been set and registered
    // with extint then we don't want to reset that behaviour.  If it hasn't been set,
    // then no extint will be called until it is set via the callback method.

    // return static switch object
    return (mp_obj_t)&pyb_switch_obj;
}

/// \method \call()
/// Return the switch state: `True` if pressed down, `False` otherwise.
mp_obj_t pyb_switch_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // get switch state
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    return switch_get() ? mp_const_true : mp_const_false;
}

STATIC mp_obj_t switch_callback(mp_obj_t line) {
    if (MP_STATE_PORT(pyb_switch_callback) != mp_const_none) {
        mp_call_function_0(MP_STATE_PORT(pyb_switch_callback));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(switch_callback_obj, switch_callback);

/// \method callback(fun)
/// Register the given function to be called when the switch is pressed down.
/// If `fun` is `None`, then it disables the callback.
//mp_obj_t pyb_switch_callback(mp_obj_t self_in, mp_obj_t callback) {
    //MP_STATE_PORT(pyb_switch_callback) = callback;
    //// Init the EXTI each time this function is called, since the EXTI
    //// may have been disabled by an exception in the interrupt, or the
    //// user disabling the line explicitly.
    //extint_register((mp_obj_t)&MICROPY_HW_USRSW_PIN,
                    //MICROPY_HW_USRSW_EXTI_MODE,
                    //MICROPY_HW_USRSW_PULL,
                    //callback == mp_const_none ? mp_const_none : (mp_obj_t)&switch_callback_obj,
                    //false, // TODO is_soft argument
                    //true);
    //return mp_const_none;
//}
//STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_switch_callback_obj, pyb_switch_callback);


STATIC mp_obj_t pyb_switch_callback(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_soft,     MP_ARG_KW_ONLY  | MP_ARG_BOOL, {.u_bool = false} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    MP_STATE_PORT(pyb_switch_callback) = args[0].u_obj;
    extint_register((mp_obj_t)&MICROPY_HW_USRSW_PIN,
                    MICROPY_HW_USRSW_EXTI_MODE,
                    MICROPY_HW_USRSW_PULL,
                    args[0].u_obj == mp_const_none ? mp_const_none : (mp_obj_t)&switch_callback_obj,
                    args[1].u_bool,
                    true);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_switch_callback_obj, 1, pyb_switch_callback);



STATIC const mp_map_elem_t pyb_switch_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_callback), (mp_obj_t)&pyb_switch_callback_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_switch_locals_dict, pyb_switch_locals_dict_table);

const mp_obj_type_t pyb_switch_type = {
    { &mp_type_type },
    .name = MP_QSTR_Switch,
    .print = pyb_switch_print,
    .make_new = pyb_switch_make_new,
    .call = pyb_switch_call,
    .locals_dict = (mp_obj_t)&pyb_switch_locals_dict,
};

#endif // MICROPY_HW_HAS_SWITCH
