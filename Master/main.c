#include <stdint.h>
#include <stdio.h>
#include "./codes.h"
#include "./crc.h"
#include "./send_frame_fun.h"
#include "./spi_fun.h"
#include "./receive_frame_fun.h"


#define BUFFER_SIZE 257 // Jeden dodatkowy bajt na przesunięcie

int slave_1 = 0;
int slave_2 = 0;

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

    int menu = -5;
    int retry = 0;
    while (menu != 0) {
        if (retry == 0) {
            chooseSlave(&current_slave, &slave_1, &slave_2);
            menu = buildFrame(tx_buf);
        }    
    
        if (!menu)
        {
            break;
        }

        transfer_spi_data(tr, current_slave ? *current_slave : slave_1);
        printf("Wysłane dane:\n");
        for (int i = 0; i < 16; i++) {
            printf("0x%02X ", tx_buf[i]);
        }
        printf("\n");

        uint8_t shifted_a[BUFFER_SIZE] = {0};
        shift_data(rx_buf, shifted_a, BUFFER_SIZE);
        int result = processFrame(tx_buf, shifted_a);
        if (result == 0) {
            printf("Otrzymane dane:\n");
            for (int i = 0; i < 16; i++) {
                printf("0x%02X ", shifted_a[i]);
            }
            printf("\n");
            retry = 0;
            while (getchar() != '\n' && !feof(stdin));
            continue; 
        }
        else if (result == 1) {
            retry = 0;
            printf("Proszę przesłąć rządanie ponownie.\n");
            continue; 
        }

        uint8_t shifted_b[BUFFER_SIZE] = {0};
        shift_data2(shifted_a, shifted_b, BUFFER_SIZE);
        int result2 = processFrame(tx_buf, shifted_b);
        if (result2 == 0) {
            printf("Otrzymane dane:\n");
            for (int i = 0; i < 16; i++) {
                printf("0x%02X ", shifted_b[i]);
            }
            printf("\n");
            retry = 0;
            while (getchar() != '\n' && !feof(stdin));
            continue; 
        }
        else if (result2 == 1) {
            retry = 0;
            printf("Proszę przesłąć rządanie ponownie.\n");
            continue; 
        }

        if (result == 2 || result2 == 2) {
            printf("Odpowiedź niezgodna z żądaniem. Ponawiam transmisję.\n");
            retry++;
            if (retry > 3) {
                printf("Przekroczono maksymalną liczbę prób. Proszę podać rozkaz ponownie.\n");
                retry = 0; 
                continue;
            }
        } 
        else if (result == 3 || result2 == 3) {
            printf("Błędne CRC. Ponawiam transmisję.\n");
            retry++;
            if (retry > 3) {
                printf("Przekroczono maksymalną liczbę prób. Proszę podać rozkaz ponownie.\n");
                retry = 0; 
                continue;
            }
        } 
    }

    close(slave_1);
    close(slave_2);
    return EXIT_SUCCESS;
}