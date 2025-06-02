#ifndef CRC_h
#define CRC_h

// CRC16 calculation for ModBus
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

bool checkCRC(uint8_t* data, uint16_t length) {
    if (length < 3) return false;

    uint16_t received_crc = (data[length - 2] << 8) | data[length - 1];
    Serial.print("Otrzymane CRC: 0x");
    Serial.println(received_crc, HEX);

    uint16_t calculated_crc = calculateCRC16(data, length - 2);
    Serial.print("Policzone CRC: 0x");
    Serial.println(calculated_crc, HEX);

    return received_crc == calculated_crc;
}

void addCRC(uint8_t* data, uint16_t& length) {
    uint16_t crc = calculateCRC16(data, length);

    data[length++] = (crc >> 8) & 0xFF; // CRC High
    data[length++] = crc & 0xFF;        // CRC Low

    while (length % 4 != 0) {
        data[length++] = 0x00;
    }
}

#endif