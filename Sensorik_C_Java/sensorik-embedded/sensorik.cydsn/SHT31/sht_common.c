/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This module provides functionality that is common to all SHT drivers
 */

#include "sht_common.h"
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "sht.h"

int8_t sht_common_read_ticks(uint8_t address, int32_t* temperature_ticks,
                             int32_t* humidity_ticks) {
    uint8_t data[6];
    int8_t ret = sensirion_i2c_read(address, data, sizeof(data));
    if(ret){
        return ret;
    }
    if(sensirion_common_check_crc(data, 2, data[2]) ||
       sensirion_common_check_crc(data + 3, 2, data[5])){
        return STATUS_CRC_FAIL;
    }

    *temperature_ticks = (data[1] & 0xff) | ((int32_t)data[0] << 8);
    *humidity_ticks = (data[4] & 0xff) | ((int32_t)data[3] << 8);

    return STATUS_OK;
}

int8_t sht_common_read_measurement(uint8_t address, int32_t* temperature,
                                   int32_t* humidity) {
    int8_t ret = sht_common_read_ticks(address, temperature, humidity);
    /**
     * formulas for conversion of the sensor signals, optimized for fixed point
     * algebra: Temperature       = 175 * S_T / 2^16 - 45 Relative Humidity =
     * 100 * S_RH / 2^16
     */
    *temperature = ((21875 * *temperature) >> 13) - 45000;
    *humidity = ((12500 * *humidity) >> 13);

    return ret;
}