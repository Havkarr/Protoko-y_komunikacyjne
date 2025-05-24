#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <errno.h>

#define SPI_DEVICE1 "/dev/spidev0.0"
#define SPI_DEVICE2 "/dev/spidev0.1"
#define SPI_MODE SPI_MODE_0
#define SPI_BITS_PER_WORD 8
#define SPI_SPEED 50000  // 50 kHz
#define BUFFER_SIZE 8

int main() {
    int fd;
    int ret;
    uint8_t mode = SPI_MODE;
    uint8_t bits = SPI_BITS_PER_WORD;
    uint32_t speed = SPI_SPEED;

    // Bufory danych
    uint8_t tx_buf[BUFFER_SIZE] = {10, 20, 30, 40, 50, 60, 70, 80};
    uint8_t rx_buf[BUFFER_SIZE] = {0};

    // Otwórz urządzenie SPI
    fd = open(SPI_DEVICE1, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return EXIT_FAILURE;
    }

    // Ustaw tryb SPI
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
        perror("Can't set SPI mode");
        close(fd);
        return EXIT_FAILURE;
    }

    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
        perror("Can't set bits per word");
        close(fd);
        return EXIT_FAILURE;
    }

    // Ustaw prędkość transmisji
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        perror("Can't set max speed hz");
        close(fd);
        return EXIT_FAILURE;
    }

    // Przygotuj strukturę transferu
    struct spi_ioc_transfer tr2 = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = BUFFER_SIZE,
        .delay_usecs = 0,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    // Wykonaj transfer SPI (wysyłanie i odbiór)
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);
    if (ret < 1) {
        perror("Failed to send SPI message");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Received data:\n");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("0x%02X ", rx_buf[i]);
    }
    printf("\n");

    sleep(1);

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr2);
    if (ret < 1) {
        perror("Failed to send SPI message");
        close(fd);
        return EXIT_FAILURE;
    }

    // Wyświetl odebrane dane
    printf("Received data 2:\n");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("0x%02X ", rx_buf[i]);
    }
    printf("\n");

    // Zamknij urządzenie
    close(fd);

    fd = open(SPI_DEVICE2, O_RDWR);
ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = BUFFER_SIZE,
        .delay_usecs = 0,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

 ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("Failed to send SPI message");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Received data:\n");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("0x%02X ", rx_buf[i]);
    }
    printf("\n");

    sleep(1);

   ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("Failed to send SPI message");
        close(fd);
        return EXIT_FAILURE;
    }

    // Wyświetl odebrane dane
    printf("Received data 2:\n");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("0x%02X ", rx_buf[i]);
    }
    printf("\n");

    // Zamknij urządzenie
    close(fd);

    return EXIT_SUCCESS;
}
