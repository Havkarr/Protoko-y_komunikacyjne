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
#define BUFFER_SIZE 257 // Jeden dodatkowy bajt na przesunięcie
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

int main() {
    int slave_1;
    int slave_2;
    int ret;
    uint8_t mode = SPI_MODE;
    uint8_t bits = SPI_BITS_PER_WORD;
    uint32_t speed = SPI_SPEED;

    // Bufory danych
    uint8_t tx_buf[BUFFER_SIZE] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5,0xCA};
    uint8_t rx_buf[BUFFER_SIZE] = {0};

    slave_1 = init_spi_device(SPI_DEVICE1, mode, bits, speed);
    // slave_2 = init_spi_device(SPI_DEVICE2, mode, bits, speed);    

    // Przygotuj strukturę transferu
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = BUFFER_SIZE,
        .delay_usecs = 0,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    transfer_spi_data(tr, slave_1);

    uint8_t shifted[BUFFER_SIZE] = {0};
    for (int i = BUFFER_SIZE - 1; i > 0; i--) {
        uint8_t high = (i > 0) ? (rx_buf[i - 1] << 4) : 0x00;
        shifted[i-1] = (rx_buf[i] >> 4) | high;
    }

    // Wyświetl odebrane dane
    printf("Received data:\n");
    for (int i = 0; i < 15; i++) {
        printf("0x%02X ", rx_buf[i]);
    }
    printf("\n");

    printf("Shifted data:\n");
    for (int i = 0; i < 15; i++) {
        printf("0x%02X ", shifted[i]);
    }
    printf("\n");
    // Zamknij urządzenie
    close(slave_1);
    // close(slave_2);

  

    return EXIT_SUCCESS;
}
