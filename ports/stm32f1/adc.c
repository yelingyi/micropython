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

#include "py/runtime.h"
#include "py/binary.h"
#include "py/mphal.h"
#include "adc.h"
#include "pin.h"
#include "timer.h"

#if MICROPY_HW_ENABLE_ADC

/// \moduleref pyb
/// \class ADC - analog to digital conversion: read analog values on a pin
///
/// Usage:
///
///     adc = pyb.ADC(pin)              # create an analog object from a pin
///     val = adc.read()                # read an analog value
///
///     adc = pyb.ADCAll(resolution)    # creale an ADCAll object
///     val = adc.read_channel(channel) # read the given channel
///     val = adc.read_core_temp()      # read MCU temperature
///     val = adc.read_core_vbat()      # read MCU VBAT
///     val = adc.read_core_vref()      # read MCU VREF

/* ADC defintions */


#define ADCx                    (ADC1)
#define PIN_ADC_MASK            PIN_ADC1
#define pin_adc_table           pin_adc1

#define ADCx_CLK_ENABLE         __HAL_RCC_ADC1_CLK_ENABLE
#define ADC_NUM_CHANNELS        (19)

#define ADC_FIRST_GPIO_CHANNEL  (0)
#define ADC_LAST_GPIO_CHANNEL   (15)
#define ADC_SCALE_V             (3.3f)
#define ADC_CAL_ADDRESS         (0x1ffff7ba)
#define ADC_CAL1                ((uint16_t*)0x1ffff7b8)
#define ADC_CAL2                ((uint16_t*)0x1ffff7c2)
#define ADC_CAL_BITS            (12)

#ifndef ADC_RESOLUTION_6B
#define ADC_RESOLUTION_6B 6
#endif
#ifndef ADC_RESOLUTION_8B
#define ADC_RESOLUTION_8B 8
#endif
#ifndef ADC_RESOLUTION_10B
#define ADC_RESOLUTION_10B 10
#endif
#ifndef ADC_RESOLUTION_12B
#define ADC_RESOLUTION_12B 12
#endif

#define VBAT_DIV (2) /* TODO: fixme */

// Timeout for waiting for end-of-conversion, in ms
#define EOC_TIMEOUT (10)

/* Core temperature sensor definitions */
#define CORE_TEMP_V25          (943)  /* (0.76v/3.3v)*(2^ADC resoultion) */
#define CORE_TEMP_AVG_SLOPE    (3)    /* (2.5mv/3.3v)*(2^ADC resoultion) */

// scale and calibration values for VBAT and VREF
#define ADC_SCALE (ADC_SCALE_V / ((1 << ADC_CAL_BITS) - 1))
#define VREFIN_CAL ((uint16_t *)ADC_CAL_ADDRESS)

typedef struct _pyb_obj_adc_t {
    mp_obj_base_t base;
    mp_obj_t pin_name;
    int channel;
    ADC_HandleTypeDef handle;
} pyb_obj_adc_t;

// convert user-facing channel number into internal channel number
static inline uint32_t adc_get_internal_channel(uint32_t channel) {
    // on F1 MCUs we want channel 16 to always be the TEMPSENSOR
    // (on some MCUs ADC_CHANNEL_TEMPSENSOR=16, on others it doesn't)
    if (channel == 16) {
        channel = ADC_CHANNEL_TEMPSENSOR;
    }
    return channel;
}

STATIC bool is_adcx_channel(int channel) {
    return IS_ADC_CHANNEL(channel);
}

STATIC void adc_wait_for_eoc_or_timeout(int32_t timeout) {
    uint32_t tickstart = HAL_GetTick();
    while (READ_BIT(ADCx->SR, ADC_FLAG_EOC) != ADC_FLAG_EOC) {
        if (((HAL_GetTick() - tickstart ) > timeout)) {
            break; // timeout
        }
    }
}

STATIC void adcx_clock_enable(void) {
	/* TODO: fixme */
    ADCx_CLK_ENABLE();
}

STATIC void adcx_init_periph(ADC_HandleTypeDef *adch, uint32_t resolution) {
    adcx_clock_enable();

    adch->Instance                   = ADCx;
    // adch->Init.Resolution            = resolution; F1 为固定12bit分辨率
    adch->Init.ContinuousConvMode    = DISABLE;             // 指定是否连续转换
    adch->Init.DiscontinuousConvMode = DISABLE;             // 没看懂的一个属性, 只有上一个禁止的情况才可开启
    // adch->Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
    adch->Init.ScanConvMode          = DISABLE;             // 常规组和注入组的序列器
    adch->Init.DataAlign             = ADC_DATAALIGN_RIGHT; // 数据对齐方式
    // adch->Init.DMAContinuousRequests = DISABLE;

    HAL_ADC_Init(adch);
}

STATIC void adc_init_single(pyb_obj_adc_t *adc_obj) {
    if (!is_adcx_channel(adc_obj->channel)) {
        return;
    }

    if (ADC_FIRST_GPIO_CHANNEL <= adc_obj->channel && adc_obj->channel <= ADC_LAST_GPIO_CHANNEL) {
        // Channels 0-16 correspond to real pins. Configure the GPIO pin in ADC mode.
        const pin_obj_t *pin = pin_adc_table[adc_obj->channel];
        mp_hal_pin_config(pin, MP_HAL_PIN_MODE_ADC, MP_HAL_PIN_PULL_NONE, 0);
    }

    adcx_init_periph(&adc_obj->handle, 0 /*ADC_RESOLUTION_12B  */);
}

STATIC void adc_config_channel(ADC_HandleTypeDef *adc_handle, uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig;

    sConfig.Channel = channel;
    sConfig.Rank = 1;
	
	/* TODO: fixme */
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;

    HAL_ADC_ConfigChannel(adc_handle, &sConfig);
}

STATIC uint32_t adc_read_channel(ADC_HandleTypeDef *adcHandle) {
    HAL_ADC_Start(adcHandle);
    adc_wait_for_eoc_or_timeout(EOC_TIMEOUT);
    uint32_t value = ADCx->DR;
    HAL_ADC_Stop(adcHandle);
    return value;
}

STATIC uint32_t adc_config_and_read_channel(ADC_HandleTypeDef *adcHandle, uint32_t channel) {
    adc_config_channel(adcHandle, channel);
    uint32_t raw_value = adc_read_channel(adcHandle);
    return raw_value;
}

/******************************************************************************/
/* MicroPython bindings : adc object (single channel)                         */

STATIC void adc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_obj_adc_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, "<ADC on ");
    mp_obj_print_helper(print, self->pin_name, PRINT_STR);
    mp_printf(print, " channel=%u>", self->channel);
}

/// \classmethod \constructor(pin)
/// Create an ADC object associated with the given pin.
/// This allows you to then read analog values on that pin.
STATIC mp_obj_t adc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check number of arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    // 1st argument is the pin name
    mp_obj_t pin_obj = args[0];

    uint32_t channel;

    if (mp_obj_is_int(pin_obj)) {
        channel = adc_get_internal_channel(mp_obj_get_int(pin_obj));
    } else {
        const pin_obj_t *pin = pin_find(pin_obj);
        if ((pin->adc_num & PIN_ADC_MASK) == 0) {
            // No ADC1 function on that pin
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pin %q does not have ADC capabilities", pin->name));
        }
        channel = pin->adc_channel;
    }

    if (!is_adcx_channel(channel)) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "not a valid ADC Channel: %d", channel));
    }


    if (ADC_FIRST_GPIO_CHANNEL <= channel && channel <= ADC_LAST_GPIO_CHANNEL) {
        // these channels correspond to physical GPIO ports so make sure they exist
        if (pin_adc_table[channel] == NULL) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "channel %d not available on this board", channel));
        }
    }

    pyb_obj_adc_t *o = m_new_obj(pyb_obj_adc_t);
    memset(o, 0, sizeof(*o));
    o->base.type = &pyb_adc_type;
    o->pin_name = pin_obj;
    o->channel = channel;
    adc_init_single(o);

    return MP_OBJ_FROM_PTR(o);
}

/// \method read()
/// Read the value on the analog pin and return it.  The returned value
/// will be between 0 and 4095.
STATIC mp_obj_t adc_read(mp_obj_t self_in) {
    pyb_obj_adc_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(adc_config_and_read_channel(&self->handle, self->channel));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(adc_read_obj, adc_read);

/// \method read_timed(buf, timer)
///
/// Read analog values into `buf` at a rate set by the `timer` object.
///
/// `buf` can be bytearray or array.array for example.  The ADC values have
/// 12-bit resolution and are stored directly into `buf` if its element size is
/// 16 bits or greater.  If `buf` has only 8-bit elements (eg a bytearray) then
/// the sample resolution will be reduced to 8 bits.
///
/// `timer` should be a Timer object, and a sample is read each time the timer
/// triggers.  The timer must already be initialised and running at the desired
/// sampling frequency.
///
/// To support previous behaviour of this function, `timer` can also be an
/// integer which specifies the frequency (in Hz) to sample at.  In this case
/// Timer(6) will be automatically configured to run at the given frequency.
///
/// Example using a Timer object (preferred way):
///
///     adc = pyb.ADC(pyb.Pin.board.X19)    # create an ADC on pin X19
///     tim = pyb.Timer(6, freq=10)         # create a timer running at 10Hz
///     buf = bytearray(100)                # creat a buffer to store the samples
///     adc.read_timed(buf, tim)            # sample 100 values, taking 10s
///
/// Example using an integer for the frequency:
///
///     adc = pyb.ADC(pyb.Pin.board.X19)    # create an ADC on pin X19
///     buf = bytearray(100)                # create a buffer of 100 bytes
///     adc.read_timed(buf, 10)             # read analog values into buf at 10Hz
///                                         #   this will take 10 seconds to finish
///     for val in buf:                     # loop over all values
///         print(val)                      # print the value out
///
/// This function does not allocate any memory.
STATIC mp_obj_t adc_read_timed(mp_obj_t self_in, mp_obj_t buf_in, mp_obj_t freq_in) {
    pyb_obj_adc_t *self = MP_OBJ_TO_PTR(self_in);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);
    size_t typesize = mp_binary_get_size('@', bufinfo.typecode, NULL);

    TIM_HandleTypeDef *tim;
    #if defined(TIM6)
    if (mp_obj_is_integer(freq_in)) {
        // freq in Hz given so init TIM6 (legacy behaviour)
        tim = timer_tim6_init(mp_obj_get_int(freq_in));
        HAL_TIM_Base_Start(tim);
    } else
    #endif
    {
        // use the supplied timer object as the sampling time base
        tim = pyb_timer_get_handle(freq_in);
    }

    // configure the ADC channel
    adc_config_channel(&self->handle, self->channel);

    // This uses the timer in polling mode to do the sampling
    // TODO use DMA

    uint nelems = bufinfo.len / typesize;
    for (uint index = 0; index < nelems; index++) {
        // Wait for the timer to trigger so we sample at the correct frequency
        while (__HAL_TIM_GET_FLAG(tim, TIM_FLAG_UPDATE) == RESET) {
        }
        __HAL_TIM_CLEAR_FLAG(tim, TIM_FLAG_UPDATE);

        if (index == 0) {
            // for the first sample we need to turn the ADC on
            HAL_ADC_Start(&self->handle);
        } else {
            // for subsequent samples we can just set the "start sample" bit
			/*TODO: fixme */
            SET_BIT(ADCx->CR2, ADC_CR2_EXTTRIG);
        }

        // wait for sample to complete
        adc_wait_for_eoc_or_timeout(EOC_TIMEOUT);

        // read value
        uint value = ADCx->DR;

        // store value in buffer
        if (typesize == 1) {
            value >>= 4;
        }
        mp_binary_set_val_array_from_int(bufinfo.typecode, bufinfo.buf, index, value);
    }

    // turn the ADC off
    HAL_ADC_Stop(&self->handle);

    #if defined(TIM6)
    if (mp_obj_is_integer(freq_in)) {
        // stop timer if we initialised TIM6 in this function (legacy behaviour)
        HAL_TIM_Base_Stop(tim);
    }
    #endif

    return mp_obj_new_int(bufinfo.len);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(adc_read_timed_obj, adc_read_timed);

// read_timed_multi((adcx, adcy, ...), (bufx, bufy, ...), timer)
//
// Read analog values from multiple ADC's into buffers at a rate set by the
// timer.  The ADC values have 12-bit resolution and are stored directly into
// the corresponding buffer if its element size is 16 bits or greater, otherwise
// the sample resolution will be reduced to 8 bits.
//
// This function should not allocate any heap memory.
STATIC mp_obj_t adc_read_timed_multi(mp_obj_t adc_array_in, mp_obj_t buf_array_in, mp_obj_t tim_in) {
    size_t nadcs, nbufs;
    mp_obj_t *adc_array, *buf_array;
    mp_obj_get_array(adc_array_in, &nadcs, &adc_array);
    mp_obj_get_array(buf_array_in, &nbufs, &buf_array);

    if (nadcs < 1) {
        mp_raise_ValueError("need at least 1 ADC");
    }
    if (nadcs != nbufs) {
        mp_raise_ValueError("length of ADC and buffer lists differ");
    }

    // Get buf for first ADC, get word size, check other buffers match in type
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_array[0], &bufinfo, MP_BUFFER_WRITE);
    size_t typesize = mp_binary_get_size('@', bufinfo.typecode, NULL);
    void *bufptrs[nbufs];
    for (uint array_index = 0; array_index < nbufs; array_index++) {
        mp_buffer_info_t bufinfo_curr;
        mp_get_buffer_raise(buf_array[array_index], &bufinfo_curr, MP_BUFFER_WRITE);
        if ((bufinfo.len != bufinfo_curr.len) || (bufinfo.typecode != bufinfo_curr.typecode)) {
            mp_raise_ValueError("size and type of buffers must match");
        }
        bufptrs[array_index] = bufinfo_curr.buf;
    }

    // Use the supplied timer object as the sampling time base
    TIM_HandleTypeDef *tim;
    tim = pyb_timer_get_handle(tim_in);

    // Start adc; this is slow so wait for it to start
    pyb_obj_adc_t *adc0 = MP_OBJ_TO_PTR(adc_array[0]);
    adc_config_channel(&adc0->handle, adc0->channel);
    HAL_ADC_Start(&adc0->handle);
    // Wait for sample to complete and discard
    adc_wait_for_eoc_or_timeout(EOC_TIMEOUT);
    // Read (and discard) value
    uint value = ADCx->DR;

    // Ensure first sample is on a timer tick
    __HAL_TIM_CLEAR_FLAG(tim, TIM_FLAG_UPDATE);
    while (__HAL_TIM_GET_FLAG(tim, TIM_FLAG_UPDATE) == RESET) {
    }
    __HAL_TIM_CLEAR_FLAG(tim, TIM_FLAG_UPDATE);

    // Overrun check: assume success
    bool success = true;
    size_t nelems = bufinfo.len / typesize;
    for (size_t elem_index = 0; elem_index < nelems; elem_index++) {
        if (__HAL_TIM_GET_FLAG(tim, TIM_FLAG_UPDATE) != RESET) {
            // Timer has already triggered
            success = false;
        } else {
            // Wait for the timer to trigger so we sample at the correct frequency
            while (__HAL_TIM_GET_FLAG(tim, TIM_FLAG_UPDATE) == RESET) {
            }
        }
        __HAL_TIM_CLEAR_FLAG(tim, TIM_FLAG_UPDATE);

        for (size_t array_index = 0; array_index < nadcs; array_index++) {
            pyb_obj_adc_t *adc = MP_OBJ_TO_PTR(adc_array[array_index]);
            // configure the ADC channel
            adc_config_channel(&adc->handle, adc->channel);
            // for the first sample we need to turn the ADC on
            // ADC is started: set the "start sample" bit
            /*TODO: fixme */
            SET_BIT(ADCx->CR2, ADC_CR2_EXTTRIG);

            // wait for sample to complete
            adc_wait_for_eoc_or_timeout(EOC_TIMEOUT);

            // read value
            value = ADCx->DR;

            // store values in buffer
            if (typesize == 1) {
                value >>= 4;
            }
            mp_binary_set_val_array_from_int(bufinfo.typecode, bufptrs[array_index], elem_index, value);
        }
    }

    // Turn the ADC off
    adc0 = MP_OBJ_TO_PTR(adc_array[0]);
    HAL_ADC_Stop(&adc0->handle);

    return mp_obj_new_bool(success);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(adc_read_timed_multi_fun_obj, adc_read_timed_multi);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(adc_read_timed_multi_obj, MP_ROM_PTR(&adc_read_timed_multi_fun_obj));

STATIC const mp_rom_map_elem_t adc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&adc_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_timed), MP_ROM_PTR(&adc_read_timed_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_timed_multi), MP_ROM_PTR(&adc_read_timed_multi_obj) },
};

STATIC MP_DEFINE_CONST_DICT(adc_locals_dict, adc_locals_dict_table);

const mp_obj_type_t pyb_adc_type = {
    { &mp_type_type },
    .name = MP_QSTR_ADC,
    .print = adc_print,
    .make_new = adc_make_new,
    .locals_dict = (mp_obj_dict_t*)&adc_locals_dict,
};

/******************************************************************************/
/* adc all object                                                             */

typedef struct _pyb_adc_all_obj_t {
    mp_obj_base_t base;
    ADC_HandleTypeDef handle;
} pyb_adc_all_obj_t;

void adc_init_all(pyb_adc_all_obj_t *adc_all, uint32_t resolution, uint32_t en_mask) {

    switch (resolution) {
        case 6:  resolution = ADC_RESOLUTION_6B;  break;
        case 8:  resolution = ADC_RESOLUTION_8B;  break;
        case 10: resolution = ADC_RESOLUTION_10B; break;
        case 12: resolution = ADC_RESOLUTION_12B; break;
        default:
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "resolution %d not supported", resolution));
    }

    for (uint32_t channel = ADC_FIRST_GPIO_CHANNEL; channel <= ADC_LAST_GPIO_CHANNEL; ++channel) {
        // only initialise those channels that are selected with the en_mask
        if (en_mask & (1 << channel)) {
            // Channels 0-16 correspond to real pins. Configure the GPIO pin in
            // ADC mode.
            const pin_obj_t *pin = pin_adc_table[channel];
            if (pin) {
                mp_hal_pin_config(pin, MP_HAL_PIN_MODE_ADC, MP_HAL_PIN_PULL_NONE, 0);
            }
        }
    }

    adcx_init_periph(&adc_all->handle, resolution);
}

int adc_get_resolution(ADC_HandleTypeDef *adcHandle) {
	/* 
    uint32_t res_reg = ADC_GET_RESOLUTION(adcHandle);

    switch (res_reg) {
        case ADC_RESOLUTION_6B:  return 6;
        case ADC_RESOLUTION_8B:  return 8;
        case ADC_RESOLUTION_10B: return 10;
    }
	*/
    return 12;
}

STATIC uint32_t adc_config_and_read_ref(ADC_HandleTypeDef *adcHandle, uint32_t channel) {
    uint32_t raw_value = adc_config_and_read_channel(adcHandle, channel);
    // Scale raw reading to the number of bits used by the calibration constants
    return raw_value << (ADC_CAL_BITS - adc_get_resolution(adcHandle));
}

int adc_read_core_temp(ADC_HandleTypeDef *adcHandle) {
    int32_t raw_value = adc_config_and_read_ref(adcHandle, ADC_CHANNEL_TEMPSENSOR);
    return ((raw_value - CORE_TEMP_V25) / CORE_TEMP_AVG_SLOPE) + 25;
}

#if MICROPY_PY_BUILTINS_FLOAT
// correction factor for reference value
STATIC volatile float adc_refcor = 1.0f;

float adc_read_core_temp_float(ADC_HandleTypeDef *adcHandle) {
    int32_t raw_value = adc_config_and_read_ref(adcHandle, ADC_CHANNEL_TEMPSENSOR);
    float core_temp_avg_slope = (*ADC_CAL2 - *ADC_CAL1) / 80.0;
    return (((float)raw_value * adc_refcor - *ADC_CAL1) / core_temp_avg_slope) + 30.0f;
}

float adc_read_core_vbat(ADC_HandleTypeDef *adcHandle) {
    uint32_t raw_value = adc_config_and_read_ref(adcHandle, ADC_CHANNEL_VREFINT);
    return raw_value * VBAT_DIV * ADC_SCALE * adc_refcor;
}

float adc_read_core_vref(ADC_HandleTypeDef *adcHandle) {
    uint32_t raw_value = adc_config_and_read_ref(adcHandle, ADC_CHANNEL_VREFINT);

    // update the reference correction factor
    adc_refcor = ((float)(*VREFIN_CAL)) / ((float)raw_value);

    return (*VREFIN_CAL) * ADC_SCALE;
}
#endif

/******************************************************************************/
/* MicroPython bindings : adc_all object                                      */

STATIC mp_obj_t adc_all_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check number of arguments
    mp_arg_check_num(n_args, n_kw, 1, 2, false);

    // make ADCAll object
    pyb_adc_all_obj_t *o = m_new_obj(pyb_adc_all_obj_t);
    o->base.type = &pyb_adc_all_type;
    mp_int_t res = mp_obj_get_int(args[0]);
    uint32_t en_mask = 0xffffffff;
    if (n_args > 1) {
        en_mask =  mp_obj_get_int(args[1]);
    }
    adc_init_all(o, res, en_mask);

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t adc_all_read_channel(mp_obj_t self_in, mp_obj_t channel) {
    pyb_adc_all_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t chan = adc_get_internal_channel(mp_obj_get_int(channel));
    uint32_t data = adc_config_and_read_channel(&self->handle, chan);
    return mp_obj_new_int(data);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(adc_all_read_channel_obj, adc_all_read_channel);

STATIC mp_obj_t adc_all_read_core_temp(mp_obj_t self_in) {
    pyb_adc_all_obj_t *self = MP_OBJ_TO_PTR(self_in);
    #if MICROPY_PY_BUILTINS_FLOAT
    float data = adc_read_core_temp_float(&self->handle);
    return mp_obj_new_float(data);
    #else
    int data  = adc_read_core_temp(&self->handle);
    return mp_obj_new_int(data);
    #endif
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(adc_all_read_core_temp_obj, adc_all_read_core_temp);

#if MICROPY_PY_BUILTINS_FLOAT
STATIC mp_obj_t adc_all_read_core_vbat(mp_obj_t self_in) {
    pyb_adc_all_obj_t *self = MP_OBJ_TO_PTR(self_in);
    float data = adc_read_core_vbat(&self->handle);
    return mp_obj_new_float(data);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(adc_all_read_core_vbat_obj, adc_all_read_core_vbat);

STATIC mp_obj_t adc_all_read_core_vref(mp_obj_t self_in) {
    pyb_adc_all_obj_t *self = MP_OBJ_TO_PTR(self_in);
    float data  = adc_read_core_vref(&self->handle);
    return mp_obj_new_float(data);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(adc_all_read_core_vref_obj, adc_all_read_core_vref);

STATIC mp_obj_t adc_all_read_vref(mp_obj_t self_in) {
    pyb_adc_all_obj_t *self = MP_OBJ_TO_PTR(self_in);
    adc_read_core_vref(&self->handle);
    return mp_obj_new_float(ADC_SCALE_V * adc_refcor);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(adc_all_read_vref_obj, adc_all_read_vref);
#endif

STATIC const mp_rom_map_elem_t adc_all_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_read_channel), MP_ROM_PTR(&adc_all_read_channel_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_core_temp), MP_ROM_PTR(&adc_all_read_core_temp_obj) },
#if MICROPY_PY_BUILTINS_FLOAT
    { MP_ROM_QSTR(MP_QSTR_read_core_vbat), MP_ROM_PTR(&adc_all_read_core_vbat_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_core_vref), MP_ROM_PTR(&adc_all_read_core_vref_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_vref), MP_ROM_PTR(&adc_all_read_vref_obj) },
#endif
};

STATIC MP_DEFINE_CONST_DICT(adc_all_locals_dict, adc_all_locals_dict_table);

const mp_obj_type_t pyb_adc_all_type = {
    { &mp_type_type },
    .name = MP_QSTR_ADCAll,
    .make_new = adc_all_make_new,
    .locals_dict = (mp_obj_dict_t*)&adc_all_locals_dict,
};

#endif // MICROPY_HW_ENABLE_ADC
