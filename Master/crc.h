#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stdbool.h>


uint16_t calculateCRC16(uint8_t* data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void addCRC(uint8_t* data, uint16_t* length) {
    uint16_t crc = calculateCRC16(data, (*length));
    data[(*length)++] = crc & 0xFF;        // CRC Low
    data[(*length)++] = (crc >> 8) & 0xFF; // CRC High

    return;
}

// Length  to ilość bajtów danych w ramce, nie licząc CRC oraz kodu funkcji
bool validateCRC(uint8_t* data, int length) {
    uint16_t crc = calculateCRC16(data, length + 2);
    uint16_t received_crc = (data[length + 2] << 8) | data[length + 3];
    
    crc == received_crc ? : printf("Błąd CRC. Otrzymano: 0x%04X, obliczono: 0x%04X\n", received_crc, crc);

    return crc == received_crc;
}

#endif