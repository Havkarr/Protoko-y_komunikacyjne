#include <stdint.h>
#include <stdio.h>
#include "./codes.h"
#include "./crc.h"
#include "./send_frame_fun.h"
#include "./spi_fun.h"

#define BUFFER_SIZE 257 // Jeden dodatkowy bajt na przesunięcie

int slave_1 = 0;
int slave_2 = 0;

int pReadBinaryIO(uint8_t* rx_data, uint8_t* tx_data, char* io_type) {
    int byte_count = rx_data[1]; // Liczba bajtów z danymi
    uint16_t start_address = (tx_data[1] << 8) | tx_data[2]; // Adres startowy rejestru
    uint16_t num_coils = (tx_data[3] << 8) | tx_data[4]; // Ile cewek ma zostać odczytanych

    if (!validateCRC(rx_data, byte_count)) {
        return 3;
    }

    uint16_t iterator = 0;
    for (int byte = 0; byte < byte_count; byte++)
    {
        uint16_t coil_value = rx_data[2 + byte]; // Odczytaj wartość wyjścia
        for (int bit = 7; bit >= 0 && iterator < num_coils; bit--)
        {
            if (coil_value & (1 << (7 - bit))) {
                printf("%s %d (%d:%d): ON\n", io_type, iterator+start_address, byte, iterator+start_address);
            } else {
                printf("%s %d (%d:%d): OFF\n", io_type, iterator+start_address, byte, iterator+start_address);
            }
            iterator++;
        }   
    }
    return 0; 
}

int pReadAnalogIO(uint8_t* rx_data, uint8_t* tx_data, char* io_type) {
    int byte_count = rx_data[1]; // Liczba bajtów z danymi
    uint16_t start_address = (tx_data[1] << 8) | tx_data[2]; // Adres startowy rejestru

    if (!validateCRC(rx_data, byte_count)) {
        return 3;
    }

    for (int i = 0; i < byte_count / 2; i++) {
        uint16_t reg_value = (rx_data[2 + 2 * i] << 8) | rx_data[3 + 2 * i];
        printf("%s %d: 0x%04X\n", io_type, start_address+i, reg_value);
    }

    return 0;
}

int pWriteCoil(uint8_t* rx_data) {
    // Znamy ramkę odpowiedzi, (bez CRC) ma 5 bajtów:
    // Do funkcji musimy podać długość ramki pomniejszoną o 2
    if (!validateCRC(rx_data, 3)) {
        return 3; 
    }

    uint16_t coil_address = (rx_data[1] << 8) | rx_data[2]; 
    uint16_t coil_value = (rx_data[3] << 8) | rx_data[4]; 
    printf("Zapisano cewkę %d: %s\n", coil_address, (coil_value == 0xFF00) ? "ON" : "OFF");
    
    if (coil_value != 0xFF00 && coil_value != 0x0000) {
        return 4; 
    }

    return 0; 
}

int processFrame(uint8_t* tx_data, uint8_t* rx_data) {
    // Błąd polecenia, podaj rozkaz od nowa
    if (rx_data[0] & 0x80) { 
        // Error message ma zawsze 4 bajty: funkcja + kod błędu + CRC
        // funkcja i kod błedu są już uwzględniane w funkcji validateCRC a CRC pomijamy
        // Dlatego jako długość ramki podajemy 0
        if (!validateCRC(rx_data, 0)) {
            return 3;
        }
        
        char error_name[MESSAGE_SIZE];
        strcpy (error_name, decodeError(rx_data[1])); 
        printf("Błąd: %s (kod: 0x%02X)\n", error_name, rx_data[1]);
        return 1;
    }

    // Błąd komunikacji, retransmituj dane
    if (rx_data[0] != tx_data[0]) { 
        printf("Odpowiedź niezgodna z żądaniem. Oczekiwano: 0x%02X, otrzymano: 0x%02X\n", tx_data[0], rx_data[0]);
        return 2;
    }

    int ret = 0;
    switch (rx_data[0]) {
        case MODBUS_READ_COILS:
            ret = pReadBinaryIO(rx_data, tx_data, "Cewka");
            break;
        case MODBUS_READ_DISCRETE_INPUTS:
            ret = pReadBinaryIO(rx_data, tx_data, "Wejście");
            break;
        case MODBUS_READ_HOLDING_REGS:
            ret = pReadAnalogIO(rx_data, tx_data, "Rejestr");
            break;
        case MODBUS_READ_INPUT_REGS:
            ret = pReadAnalogIO(rx_data, tx_data, "Rejestr wejściowy");    
            break;
        case MODBUS_WRITE_SINGLE_COIL:
            ret = pWriteCoil(rx_data);
            break;
        case MODBUS_WRITE_SINGLE_REG:
            
            break;
        case MODBUS_WRITE_MULTIPLE_COILS:
            
            break;
        case MODBUS_WRITE_MULTIPLE_REGS:
        
            break;
        default:
            return 10; // Nieznana funkcja
    }
    return ret;
}

int main() {
    int* current_slave = NULL; // Zmienna do przechowywania aktualnego slave'a
    int ret;
    uint8_t mode = SPI_MODE;
    uint8_t bits = SPI_BITS_PER_WORD;
    uint32_t speed = SPI_SPEED;

    // Bufory danych
    uint8_t tx_buf[BUFFER_SIZE] = {0};
    uint8_t rx_buf[BUFFER_SIZE] = {0};

    slave_1 = init_spi_device(SPI_DEVICE1, mode, bits, speed);
    slave_2 = init_spi_device(SPI_DEVICE2, mode, bits, speed);  

    // Przygotuj strukturę transferu
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = BUFFER_SIZE,
        .delay_usecs = 0,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    chooseSlave(&current_slave, &slave_1, &slave_2);
    buildFrame(tx_buf);
    
    transfer_spi_data(tr, current_slave ? *current_slave : slave_1);
    printf("Wysłane dane:\n");
    for (int i = 0; i < 16; i++) {
        printf("0x%02X ", tx_buf[i]);
    }
    printf("\n");

    uint8_t shifted[BUFFER_SIZE] = {0};
    shift_data(rx_buf, shifted, BUFFER_SIZE);

    printf("Otrzymane dane:\n");
    for (int i = 0; i < 16; i++) {
        printf("0x%02X ", shifted[i]);
    }
    printf("\n");

    uint8_t shifted2[BUFFER_SIZE] = {0};
    shift_data2(shifted, shifted2, BUFFER_SIZE);
    printf("Otrzymane dane2:\n");
    for (int i = 0; i < 16; i++) {
        printf("0x%02X ", shifted2[i]);
    }
    printf("\n");

    int result = processFrame(tx_buf, shifted);
    close(slave_1);
    close(slave_2);

    return EXIT_SUCCESS;
}
