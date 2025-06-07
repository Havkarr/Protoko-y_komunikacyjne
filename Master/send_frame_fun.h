#ifndef SEND_FRAME_FUN_H
#define SEND_FRAME_FUN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "./codes.h"
#include "./crc.h"


void getRegisterAddress(uint8_t* data, uint16_t* pointer) {
    uint16_t register_address = 10000;
    while (register_address < 0 || register_address > 9998) {
        printf("Wprowadź adres rejestru (0-9998): ");
        scanf("%d", &register_address);
        if (register_address < 0 || register_address > 9998) {
            printf("Nieprawidłowy adres rejestru. Adres musi być w zakresie 0-9998.\n");
            while (getchar() != '\n' && !feof(stdin));
        }
    }
    
    data[(*pointer)++] = (register_address >> 8) & 0xFF; // Wysoki bajt adresu rejestru  
    data[(*pointer)++] = register_address & 0xFF; // Niski bajt adresu rejestru
}

uint16_t getRegisterNumber(uint8_t* data, uint16_t* pointer) {
    uint16_t register_number = 1000;
    while (register_number < 0 || register_number > 100) {
        printf("Wprowadź ilość danych (1-100): ");
        scanf("%d", &register_number);
        if (register_number < 0 || register_number > 100) {
            printf("Nieprawidłowa ilość danych. Ilość musi być w zakresie 1-100.\n");
            while (getchar() != '\n' && !feof(stdin));
        }
    }
    data[(*pointer)++] = (register_number >> 8) & 0xFF; // Wysoki bajt adresu rejestru
    data[(*pointer)++] = register_number & 0xFF; // Niski bajt adresu rejestru

    return register_number;
}

void writeSingleCoil(uint8_t* data, uint16_t* pointer) {
    int coil_value = 2;
    while (coil_value != 0 && coil_value != 1) {
        printf("Wprowadź wartość wyjścia (0 lub 1): ");
        scanf("%d", &coil_value);
        if (coil_value != 0 && coil_value != 1) {
            printf("Nieprawidłowa wartość. Wartość musi być 0 lub 1.\n");
            while (getchar() != '\n' && !feof(stdin));
        }
    }
    data[(*pointer)++] = (coil_value == 1) ? 0xFF : 0x00; // Wartość wyjścia (0xFF dla 1, 0x00 dla 0)
    data[(*pointer)++] = 0x00; // Niski bajt wartości wyjścia (zawsze 0x00 dla pojedynczego wyjścia)
}

void writeSingleRegister(uint8_t* data, uint16_t* pointer) {
    int coil_value = -1;
    while (coil_value < 0 || coil_value > 65536) {
        printf("Wprowadź wartość rejestru (0 - 65 536 lub 0x0000 - 0xFFFF): ");
        scanf("%d", &coil_value);
        if (coil_value == 0 && (getchar() == 'x' || getchar() == 'X'))
        {
            char hexstring[4] = {0};
            for (int i = 0; i < 4; i++)
            {
                hexstring[i] = getchar();
            }
            coil_value = (int)strtol(hexstring, NULL, 16);
        }
        
        if (coil_value < 0 || coil_value > 65536) {
            printf("Nieprawidłowa wartość (0 - 65 536 lub 0x0000 - 0xFFFF).\n");
            while (getchar() != '\n' && !feof(stdin));
        }
    }
    data[(*pointer)++] = (coil_value >> 8) & 0x00FF; 
    data[(*pointer)++] = coil_value & 0x00FF; 
}

void writeMultipleCoils(uint8_t* data, uint16_t* pointer, int quantity) {
    while(quantity)
    {
        printf("Wprowadź %d wartości wyjść (0 lub 1) w postaci 011010...(MSB...LSB):\n", quantity);
        char* input_str = (char*)malloc(quantity+1);
        scanf("%s", input_str);
    
        if (strlen(input_str) != quantity) {
            printf("Nieprawidłowa długość wejścia. Oczekiwano %d znaków.\n", quantity);
            continue;
        }
        
        int valid = 1;
        for (int i = 0; i < quantity; i++) {
            if (input_str[i] != '0' && input_str[i] != '1') {
                printf("Nieprawidłowy znak '%c'. Oczekiwano '0' lub '1'.\n", input_str[i]);
                valid = 0;
                break;
            }
        }
        if (valid == 0)
        {
            continue; 
        }

        int byte_count = ceil((quantity + 7) / 8); // Oblicz liczbę bajtów potrzebnych do przechowania wartości wyjść
        data[(*pointer)++] = byte_count; // Liczba bajtów do przechowania wartości wyjść
        int iterator = 0;
        for (int i = 0; i < byte_count; i++)
        {
            uint8_t coil_values = 0;
            for (int j = 0; j < 8 && iterator < quantity; j++)
            {
                if (input_str[(i*8) + j ] == '1')
                {
                    coil_values |= (1 << 7-j); 
                }
                iterator++;
            }
            
            data[(*pointer)++] = coil_values; // Wartości wyjść w postaci bajtów
        }

        free(input_str); 
    
        break;
    }
}

void writeMultipleRegisters(uint8_t* data, uint16_t* pointer, int quantity) {
    data[(*pointer)++] = quantity * 2; // Liczba bajtów do przechowania wartości rejestrów
    for (int i = 0; i < quantity; i++)
    {
        writeSingleRegister(data, pointer); // Zapisz każdy rejestr
    }
}

uint8_t buildFrame(uint8_t* data) {
    int choice = 0;
    uint16_t data_pointer = 0;
    while (choice < 1 || choice > 8)  // Poprawka: zmiana zakresu wyboru
    {
        printf("Wybierz funkcję Modbus:\n");
        printf("1. Odczyt wyjść binarnych\n");
        printf("2. Odczyt wejść binarnych\n");
        printf("3. Odczyt rejestrów\n");
        printf("4. Odczyt rejestrów wejściowych\n");
        printf("5. Zapis pojedynczego wyjścia\n");
        printf("6. Zapis pojedynczego rejestru\n");
        printf("7. Zapis wielu wyjść\n");
        printf("8. Zapis wielu rejestrów\n");
    
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                data[data_pointer++] = MODBUS_READ_COILS; // MODBUS_READ_COILS
                getRegisterAddress(data, &data_pointer);
                getRegisterNumber(data, &data_pointer);
                break;
            case 2:
                data[data_pointer++] = MODBUS_READ_DISCRETE_INPUTS; // MODBUS_READ_DISCRETE_INPUTS
                getRegisterAddress(data, &data_pointer);
                getRegisterNumber(data, &data_pointer);
                break;
            case 3:
                data[data_pointer++] = MODBUS_READ_HOLDING_REGS; // MODBUS_READ_HOLDING_REGS
                getRegisterAddress(data, &data_pointer);
                getRegisterNumber(data, &data_pointer);
                break;
            case 4:
                data[data_pointer++] = MODBUS_READ_INPUT_REGS; // MODBUS_READ_INPUT_REGS
                getRegisterAddress(data, &data_pointer);
                getRegisterNumber(data, &data_pointer);
                break;
            case 5:
                data[data_pointer++] = MODBUS_WRITE_SINGLE_COIL; // MODBUS_WRITE_SINGLE_COIL
                getRegisterAddress(data, &data_pointer);
                writeSingleCoil(data, &data_pointer);
                break;
            case 6:
                data[data_pointer++] = MODBUS_WRITE_SINGLE_REG; // MODBUS_WRITE_SINGLE_REG
                getRegisterAddress(data, &data_pointer);
                writeSingleRegister(data, &data_pointer);
                break;
            case 7:
                data[data_pointer++] = MODBUS_WRITE_MULTIPLE_COILS; // MODBUS_WRITE_MULTIPLE_COILS
                getRegisterAddress(data, &data_pointer);
                uint16_t quantity = getRegisterNumber(data, &data_pointer);
                writeMultipleCoils(data, &data_pointer, quantity); // Jest problem przy odczytywaniu nieregularnej liczby wyjść, ucina ostatni bajt
                
                break;
            case 8:
                data[data_pointer++] = MODBUS_WRITE_MULTIPLE_REGS; // MODBUS_WRITE_MULTIPLE_REGS
                getRegisterAddress(data, &data_pointer);
                uint16_t register_quantity = getRegisterNumber(data, &data_pointer);
                writeMultipleRegisters(data, &data_pointer, register_quantity); 
                break;
            default:
                printf("Nieprawidłowy wybór. Wybierz ponownie.\n");
                while (getchar() != '\n' && !feof(stdin));
        }
    }
    addCRC(data, &data_pointer);

    return data_pointer;
}

#endif