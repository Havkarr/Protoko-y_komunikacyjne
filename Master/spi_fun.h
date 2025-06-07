#ifndef SPI_FUN_H
#define SPI_FUN_H

#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <fcntl.h>


#define SPI_DEVICE1 "/dev/spidev0.0"
#define SPI_DEVICE2 "/dev/spidev0.1"
#define SPI_MODE SPI_MODE_0
#define SPI_BITS_PER_WORD 8
#define SPI_SPEED 50000  // 50 kHz
#define WAIT_TIME 1


int init_spi_device(const char *device, uint8_t mode, uint8_t bits, uint32_t speed) {
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
        perror("Can't set SPI mode");
        close(fd);
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
        perror("Can't set bits per word");
        close(fd);
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("Can't set max speed hz");
        close(fd);
        return -1;
    }

    return fd;
}

struct spi_ioc_transfer;

int transfer_spi_data(struct spi_ioc_transfer tr, int slave_1) {
    int ret;
    // Prześlij rządanie do slave-a
    ret = ioctl(slave_1, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("Failed to send SPI message");
        close(slave_1);
        return EXIT_FAILURE;
    }

    // Zaczekaj chwilę, aby dane mogły być przetworzone
    sleep(WAIT_TIME);

    // Odbierz dane z slave-a
    ret = ioctl(slave_1, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("Failed to send SPI message");
        close(slave_1);
        return EXIT_FAILURE;
    }

    return 0;
}

void shift_data(uint8_t *data, uint8_t *shifted, size_t length) {
    for (int i = length - 1; i > 0; i--) {
        uint8_t high = (i > 0) ? (data[i - 1] << 4) : 0x00;
        shifted[i-1] = (data[i] >> 4) | high;
    }
}

void shift_data2(uint8_t *data, uint8_t *shifted, size_t length) {
    for (int i = 0; i < length; i++) {
        uint8_t high = (i > 0) ? (data[i - 1] << 7) : 0x00;
        shifted[i] = (data[i] >> 1) | high;
    }
}

void chooseSlave(int** slave, int* slave_1, int* slave_2) {
    int choice = 10;
    while (choice < 1 || choice > 2) {
        printf("Wybierz slave'a (1 lub 2): ");
        scanf("%d", &choice);
        if (choice == 1) {
            *slave = slave_1;
        } else if (choice == 2) {
            *slave = slave_2;
        } else {
            while (getchar() != '\n' && !feof(stdin));
            printf("Nieprawidłowy wybór. Wybierz 1 lub 2.\n");
        }
    }   
}

#endif