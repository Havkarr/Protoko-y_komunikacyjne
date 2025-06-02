#ifndef Modbus_h
#define Modbus_h

#include "./CRC.h"
#include "./Codes.h"
#include "./Parameters.h"

uint8_t coils[MAX_COILS / 8 + 1] = { 0 };                       // Coils (Read/Write bits)
uint8_t discrete_inputs[MAX_DISCRETE_INPUTS / 8 + 1] = { 0 };   // Discrete inputs (Read-only bits)
uint16_t holding_registers[MAX_HOLDING_REGS] = { 0 };           // Holding registers (Read/Write)
uint16_t input_registers[MAX_INPUT_REGS] = { 0 };               // Input registers (Read-only)

// Set bit in array
void setBit(uint8_t* array, uint16_t bit_address, bool value) {
    uint16_t byte_index = bit_address / 8;
    uint8_t bit_index = bit_address % 8;
    if (value) {
        array[byte_index] |= (1 << bit_index);
    }
    else {
        array[byte_index] &= ~(1 << bit_index);
    }
}

// Get bit from array
bool getBit(uint8_t* array, uint16_t bit_address) {
    uint16_t byte_index = bit_address / 8;
    uint8_t bit_index = bit_address % 8;
    return (array[byte_index] >> bit_index) & 0x01;
}

// ModBus Function 01: Read Coils
uint16_t readCoils(uint8_t* request, uint8_t* response) {
    uint16_t start_address = (request[1] << 8) | request[2];
    uint16_t quantity = (request[3] << 8) | request[4];

    if (quantity < 1) {
        Serial.println("quantity < 1");
    }
    else if (quantity > 2000) {
        Serial.println("quantity > 2000");
        Serial.print("quantity:");
        Serial.println(quantity);
    }
    else if (start_address + quantity > MAX_COILS) {
        Serial.print("start_address:");
        Serial.println(start_address);
        Serial.print("quantity:");
        Serial.println(quantity);
        Serial.println("start_address + quantity > MAX_COILS");
    }

    if (quantity < 1 || quantity > 2000 || start_address + quantity > MAX_COILS) {
        response[0] = MODBUS_READ_COILS | 0x80; // Exception
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    uint8_t byte_count = (quantity + 7) / 8;
    response[0] = MODBUS_READ_COILS;
    response[1] = byte_count;

    for (uint16_t i = 0; i < byte_count; i++) {
        response[2 + i] = 0;
        for (uint8_t bit = 0; bit < 8 && (i * 8 + bit) < quantity; bit++) {
            if (getBit(coils, start_address + i * 8 + bit)) {
                response[2 + i] |= (1 << bit);
            }
        }
    }

    return 2 + byte_count;
}

// ModBus Function 02: Read Discrete Inputs
uint16_t readDiscreteInputs(uint8_t* request, uint8_t* response) {
    uint16_t start_address = (request[1] << 8) | request[2];
    uint16_t quantity = (request[3] << 8) | request[4];

    if (quantity < 1 || quantity > 2000 || start_address + quantity > MAX_DISCRETE_INPUTS) {
        response[0] = MODBUS_READ_DISCRETE_INPUTS | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    uint8_t byte_count = (quantity + 7) / 8;
    response[0] = MODBUS_READ_DISCRETE_INPUTS;
    response[1] = byte_count;

    for (uint16_t i = 0; i < byte_count; i++) {
        response[2 + i] = 0;
        for (uint8_t bit = 0; bit < 8 && (i * 8 + bit) < quantity; bit++) {
            if (getBit(discrete_inputs, start_address + i * 8 + bit)) {
                response[2 + i] |= (1 << bit);
            }
        }
    }

    return 2 + byte_count;
}

// ModBus Function 03: Read Holding Registers
uint16_t readHoldingRegisters(uint8_t* request, uint8_t* response) {
    uint16_t start_address = (request[1] << 8) | request[2];
    uint16_t quantity = (request[3] << 8) | request[4];

    if (quantity < 1 || quantity > 125 || start_address + quantity > MAX_HOLDING_REGS) {
        response[0] = MODBUS_READ_HOLDING_REGS | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    uint8_t byte_count = quantity * 2;
    response[0] = MODBUS_READ_HOLDING_REGS;
    response[1] = byte_count;

    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t reg_value = holding_registers[start_address + i];
        response[2 + i * 2] = (reg_value >> 8) & 0xFF;
        response[2 + i * 2 + 1] = reg_value & 0xFF;
    }

    return 2 + byte_count;
}

// ModBus Function 04: Read Input Registers
uint16_t readInputRegisters(uint8_t* request, uint8_t* response) {
    uint16_t start_address = (request[1] << 8) | request[2];
    uint16_t quantity = (request[3] << 8) | request[4];

    if (quantity < 1 || quantity > 125 || start_address + quantity > MAX_INPUT_REGS) {
        response[0] = MODBUS_READ_INPUT_REGS | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    uint8_t byte_count = quantity * 2;
    response[0] = MODBUS_READ_INPUT_REGS;
    response[1] = byte_count;

    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t reg_value = input_registers[start_address + i];
        response[2 + i * 2] = (reg_value >> 8) & 0xFF;
        response[2 + i * 2 + 1] = reg_value & 0xFF;
    }

    return 2 + byte_count;
}

// ModBus Function 05: Write Single Coil
uint16_t writeSingleCoil(uint8_t* request, uint8_t* response) {
    uint16_t address = (request[1] << 8) | request[2];
    uint16_t value = (request[3] << 8) | request[4];

    if (address >= MAX_COILS || (value != 0x0000 && value != 0xFF00)) {
        response[0] = MODBUS_WRITE_SINGLE_COIL | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    setBit(coils, address, value == 0xFF00);

   
    for (int i = 0; i < 5; i++) {
        response[i] = request[i];
    }

    return 5;
}

// ModBus Function 06: Write Single Register
uint16_t writeSingleRegister(uint8_t* request, uint8_t* response) {
    uint16_t address = (request[1] << 8) | request[2];
    uint16_t value = (request[3] << 8) | request[4];

    if (address >= MAX_HOLDING_REGS) {
        response[0] = MODBUS_WRITE_SINGLE_REG | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    holding_registers[address] = value;

    for (int i = 0; i < 5; i++) {
        response[i] = request[i];
    }

    return 5;
}

// ModBus Function 15: Write Multiple Coils
uint16_t writeMultipleCoils(uint8_t* request, uint8_t* response) {
    uint16_t start_address = (request[1] << 8) | request[2];
    uint16_t quantity = (request[3] << 8) | request[4];
    uint8_t byte_count = request[5];

    if (quantity < 1 || quantity > 1968 || start_address + quantity > MAX_COILS ||
        byte_count != (quantity + 7) / 8) {
        response[0] = MODBUS_WRITE_MULTIPLE_COILS | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    for (uint16_t i = 0; i < quantity; i++) {
        uint8_t byte_index = i / 8;
        uint8_t bit_index = i % 8;
        bool bit_value = (request[6 + byte_index] >> bit_index) & 0x01;
        setBit(coils, start_address + i, bit_value);
    }

    response[0] = MODBUS_WRITE_MULTIPLE_COILS;
    response[1] = request[1]; // Start address high
    response[2] = request[2]; // Start address low
    response[3] = request[3]; // Quantity high
    response[4] = request[4]; // Quantity low

    return 5;
}

// ModBus Function 16: Write Multiple Registers
uint16_t writeMultipleRegisters(uint8_t* request, uint8_t* response) {
    uint16_t start_address = (request[1] << 8) | request[2];
    uint16_t quantity = (request[3] << 8) | request[4];
    uint8_t byte_count = request[5];

    if (quantity < 1 || quantity > 123 || start_address + quantity > MAX_HOLDING_REGS ||
        byte_count != quantity * 2) {
        response[0] = MODBUS_WRITE_MULTIPLE_REGS | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDR;
        return 2;
    }

    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t reg_value = (request[6 + i * 2] << 8) | request[6 + i * 2 + 1];
        holding_registers[start_address + i] = reg_value;
    }

    response[0] = MODBUS_WRITE_MULTIPLE_REGS;
    response[1] = request[1]; // Start address high
    response[2] = request[2]; // Start address low
    response[3] = request[3]; // Quantity high
    response[4] = request[4]; // Quantity low

    return 5;
}

// Process ModBus request
uint16_t processModBusRequest(uint8_t* request, uint8_t* response, uint16_t request_length) {
    // Check minimum length and CRC
    if (request_length < 3 || !checkCRC(request, request_length)) {
        Serial.println("Invalid request: bad CRC or too short");
        return 0; // No response for invalid requests
    }

    uint8_t function_code = request[0];
    uint16_t response_length = 0;

    switch (function_code) {
    case MODBUS_READ_COILS:
        response_length = readCoils(request, response);
        break;
    case MODBUS_READ_DISCRETE_INPUTS:
        response_length = readDiscreteInputs(request, response);
        break;
    case MODBUS_READ_HOLDING_REGS:
        response_length = readHoldingRegisters(request, response);
        break;
    case MODBUS_READ_INPUT_REGS:
        response_length = readInputRegisters(request, response);
        break;
    case MODBUS_WRITE_SINGLE_COIL:
        response_length = writeSingleCoil(request, response);
        break;
    case MODBUS_WRITE_SINGLE_REG:
        response_length = writeSingleRegister(request, response);
        break;
    case MODBUS_WRITE_MULTIPLE_COILS:
        response_length = writeMultipleCoils(request, response);
        break;
    case MODBUS_WRITE_MULTIPLE_REGS:
        response_length = writeMultipleRegisters(request, response);
        break;
    default:
        // Illegal function
        response[0] = function_code | 0x80;
        response[1] = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
        response_length = 2;
        break;
    }

    // Add CRC to response
    if (response_length > 0) {
        addCRC(response, response_length);
    }

    return response_length;
}

#endif