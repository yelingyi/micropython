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

#include <stdlib.h>
#include <assert.h>

#include "misc.h"
#include "mpconfig.h"
#include "qstr.h"
#include "obj.h"
#include "runtime.h"

typedef struct _mp_obj_enumerate_t {
    mp_obj_base_t base;
    mp_obj_t iter;
    machine_int_t cur;
} mp_obj_enumerate_t;

STATIC mp_obj_t enumerate_iternext(mp_obj_t self_in);

/* TODO: enumerate is one of the ones that can take args or kwargs.
   Sticking to args for now */
STATIC mp_obj_t enumerate_make_new(mp_obj_t type_in, uint n_args, uint n_kw, const mp_obj_t *args) {
    assert(n_args > 0);
    mp_obj_enumerate_t *o = m_new_obj(mp_obj_enumerate_t);
    o->base.type = &mp_type_enumerate;
    o->iter = mp_getiter(args[0]);
    o->cur = n_args > 1 ? mp_obj_get_int(args[1]) : 0;
    return o;
}

const mp_obj_type_t mp_type_enumerate = {
    { &mp_type_type },
    .name = MP_QSTR_enumerate,
    .make_new = enumerate_make_new,
    .iternext = enumerate_iternext,
    .getiter = mp_identity,
};

STATIC mp_obj_t enumerate_iternext(mp_obj_t self_in) {
    assert(MP_OBJ_IS_TYPE(self_in, &mp_type_enumerate));
    mp_obj_enumerate_t *self = self_in;
    mp_obj_t next = mp_iternext(self->iter);
    if (next == MP_OBJ_STOP_ITERATION) {
        return MP_OBJ_STOP_ITERATION;
    } else {
        mp_obj_t items[] = {MP_OBJ_NEW_SMALL_INT(self->cur++), next};
        return mp_obj_new_tuple(2, items);
    }
}
