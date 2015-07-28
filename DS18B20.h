/*
 * File:   DS18B20.h
 * Defines methods for getting values from a DS18B20 temperature sensor.  This
 * intial version works for a signel sensor on the one wire bus.
 *
 * Author: Matthew Winchcombe
 *
 * Created on 19 December 2013, 16:47
 */

#include <stdbool.h>

#define DS18B20_SKIP_ROM 0xCC
#define DS18B20_READ_TEMP  0x44
#define DS18B20_GET_MEM  0xBE
#define DS18B20_SIGN_MASK  0x80

#define DS18B20_DECIMAL_BIT_3 50
#define DS18B20_DECIMAL_BIT_2 25
#define DS18B20_DECIMAL_BIT_1 13
#define DS18B20_DECIMAL_BIT_0 6


typedef struct {
        int8_t integral;
        uint8_t decimal;
        bool valid;
} ds18b20_Result;

ds18b20_Result ds18b20_getTemperature(void);
