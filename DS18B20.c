/*
 * File:   DS18B20.c
 * Author: Matthew Winchcombem
 *
 * Created on 19 December 2013, 16:51
 */
#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>
#include <stdlib.h>
#include "DS18B20.h"

#ifndef DS18B20_PIN
#define DS18B20_PIN PORTAbits.RA1
#endif

#ifndef DS18B20_TRIS
#define DS18B20_TRIS TRISAbits.TRISA1
#endif

static void delay5ms(int multiplier)
{
    for (int i=0; i<multiplier; i++)
    {
        __delay_us(5000);
    }
}

static void _release() {
        DS18B20_TRIS = 1;
}

static void _pullLow() {
        DS18B20_TRIS = 0;
        DS18B20_PIN = 0;
}


static unsigned _initialise() {
        unsigned result = 1;
        _pullLow();
        __delay_us(500);
        _release();
        __delay_us(100);
        result = DS18B20_PIN;
        __delay_us(400);
        return result;
}

static void _sendZero() {
        _pullLow();
        __delay_us(60);
        _release();
        __delay_us(2);
}

static void _sendOne() {
        _pullLow();
        __delay_us(10);
        _release();
        __delay_us(52);
}

static void _sendByte(uint8_t byte) {
        uint8_t mask = 1;

        while (mask) {
                if ((mask & byte) != 0) {
                        _sendOne();
                } else {
                        _sendZero();
                }
                mask = mask << 1;
        }

}

static void _sendCommand(uint8_t command, uint8_t * data, uint8_t len) {
        _sendByte(command);
        while(len--) {
                _sendByte(*data);
                data++;
        }
}

static void _readData(uint8_t * result, uint8_t len) {

        while(len--) {
                for (uint8_t currentBit = 0; currentBit < 8; currentBit++) {
                        _pullLow();
                        __delay_us(2);
                        _release();
                        __delay_us(5);
                        *result = *result  | (DS18B20_PIN << currentBit);
                        __delay_us(50);
                }
                result++;
        }
}

static void _waitForProbe(void) {
        uint8_t result[] = {0};
        do {
                __delay_us(200);
                _readData(result, 1);
        } while (result[0]);
}


static void _populateResultFromData(uint8_t* data, ds18b20_Result * pResult) {

        int8_t power = 6;
        uint8_t bitmask = 0x4;

        while (bitmask) {
                if ((data[1] & bitmask) != 0) {
                        pResult->integral += 1 << power;
                }
                power--;
                bitmask >>= 1;
        }

        power = 4;
        bitmask = 0x80;
        while (power) {
                if ((data[0] & bitmask) != 0) {
                        pResult->integral  += 1 << (power - 1);
                }
                bitmask >>= 1;
                power--;
        }

        if ((data[1] & DS18B20_SIGN_MASK)) {
                pResult->integral  = -pResult->integral;
        }

        if ((data[0] & 0x8) != 0) {
                pResult->decimal  += DS18B20_DECIMAL_BIT_3;
        }
        if ((data[0] & 0x4) != 0) {
                pResult->decimal += DS18B20_DECIMAL_BIT_2;
        }
        if ((data[0] & 0x2) != 0) {
                pResult->decimal += DS18B20_DECIMAL_BIT_1;
        }
        if ((data[0] & 0x1) != 0) {
                pResult->decimal += DS18B20_DECIMAL_BIT_0;
        }
}

ds18b20_Result ds18b20_getTemperature(void) {
        ds18b20_Result result = {0,0,1};
        _release();
        __delay_ms(5);

        if (_initialise() == 0) {
                _sendCommand(SKIP_ROM, NULL, 0);
                _sendCommand(READ_TEMP, NULL, 0);
                _waitForProbe();
        } else {
                result.valid = false;
        }


        if (result.valid && _initialise() == 0) {
                _sendCommand(DS18B20_SKIP_ROM, NULL, 0);
                _sendCommand(DS18B20_GET_MEM, NULL, 0);
                uint8_t data[9] = {0,};
                _readData(data, 9);
                _populateResultFromData(data, &result);
        } else {
                result.valid = false;
        }

        return result;
}
