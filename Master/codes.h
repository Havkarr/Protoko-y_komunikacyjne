#ifndef CODES_H
#define CODES_H

#include <stdint.h>

#define MODBUS_READ_COILS           0x01
#define MODBUS_READ_DISCRETE_INPUTS 0x02
#define MODBUS_READ_HOLDING_REGS    0x03
#define MODBUS_READ_INPUT_REGS      0x04
#define MODBUS_WRITE_SINGLE_COIL    0x05
#define MODBUS_WRITE_SINGLE_REG     0x06
#define MODBUS_WRITE_MULTIPLE_COILS 0x0F
#define MODBUS_WRITE_MULTIPLE_REGS  0x10

#define MESSAGE_SIZE 26

const char* decode(uint8_t code) {
    switch (code) {
        case 0x00: return "COMMUNICATION_ERROR";
        case 0x01: return "READ_COILS";
        case 0x02: return "READ_DISCRETE_INPUTS";
        case 0x03: return "READ_HOLDING_REGISTERS";
        case 0x04: return "READ_INPUT_REGISTERS";
        case 0x05: return "WRITE_SINGLE_COIL";
        case 0x06: return "WRITE_SINGLE_REGISTER";
        case 0x0F: return "WRITE_MULTIPLE_COILS";
        case 0x10: return "WRITE_MULTIPLE_REGISTERS";
        default:   return "UNKNOWN CODE";
    }
}

const char* decodeError(uint8_t code) {
    switch (code) {
        case 0x00: return "COMMUNICATION_ERROR";
        case 0x01: return "ILLEGAL_FUNCTION";
        case 0x02: return "ILLEGAL_DATA_ADDRESS";
        case 0x03: return "ILLEGAL_DATA_VALUE";
        case 0x04: return "SLAVE_DEVICE_FAILURE";
        default:   return "UNKNOWN ERROR CODE";
    }
}

#endif