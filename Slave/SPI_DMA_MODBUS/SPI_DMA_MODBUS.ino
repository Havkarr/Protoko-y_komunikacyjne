#include <ESP32DMASPISlave.h>

#define PIN_CS   5

ESP32DMASPI::Slave slave;

static constexpr size_t BUFFER_SIZE = 256;
static constexpr size_t QUEUE_SIZE = 1;
uint8_t *dma_tx_buf;
uint8_t *dma_rx_buf;

bool dataAvailable = false;
bool transfer = false;
bool answear = false;

void IRAM_ATTR onCsInterrupt() {
  transfer = true;
}

void receiveData(){
    const size_t received_bytes = slave.transfer(dma_tx_buf, dma_rx_buf, BUFFER_SIZE);
    dataAvailable = true;
    transfer = false;
    answear = true;
}

void sendData(){
    const size_t received_bytes = slave.transfer(dma_tx_buf, dma_rx_buf, BUFFER_SIZE);
    answear = false;
    transfer = false;  
}

void setup()
{
    Serial.begin(115200);
    // to use DMA buffer, use these methods to allocate buffer
    dma_tx_buf = slave.allocDMABuffer(BUFFER_SIZE);
    dma_rx_buf = slave.allocDMABuffer(BUFFER_SIZE);

    slave.setDataMode(SPI_MODE0);           // default: SPI_MODE0
    slave.setMaxTransferSize(BUFFER_SIZE);  // default: 4092 bytes
    slave.setQueueSize(QUEUE_SIZE);         // default: 1

    // begin() after setting
    slave.begin(VSPI);  // default: HSPI (please refer README for pin assignments)
    
    pinMode(PIN_CS, INPUT); 
    attachInterrupt(digitalPinToInterrupt(PIN_CS), onCsInterrupt, FALLING);
    Serial.println("init done!");
}

void loop()
{
  if(transfer && !answear){
    receiveData();
  }
  else if (transfer && answear){
    dma_tx_buf[0] = 0xFF; //0F
    dma_tx_buf[1] = 2;
    dma_tx_buf[2] = 3;
    dma_tx_buf[3] = 4;
    dma_tx_buf[4] = 5;
    dma_tx_buf[5] = 6;
    dma_tx_buf[6] = 7;
    dma_tx_buf[7] = 8;
    sendData();
  }

  if(dataAvailable)
  {
    for (int i = 0; i < 8; ++i)
    {
      Serial.println(dma_rx_buf[i]);
      
    }
    dataAvailable = false;
  }
}