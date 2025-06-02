#include <ESP32DMASPISlave.h>
#include "./Modbus.h"
#include "./Parameters.h"

ESP32DMASPI::Slave slave;

static constexpr size_t BUFFER_SIZE = BUFF_SIZE;
static constexpr size_t QUEUE_SIZE = Q_SIZE;
uint8_t *dma_tx_buf;
uint8_t *dma_rx_buf;

bool dataAvailable = false;
bool transfer = false;
bool answer = false;

void IRAM_ATTR onCsInterrupt() {
  transfer = true;
}

size_t receiveData();
size_t sendData();

void setup() {
  Serial.begin(115200);
  
  // Initialize DMA buffers
  dma_tx_buf = slave.allocDMABuffer(BUFFER_SIZE);
  dma_rx_buf = slave.allocDMABuffer(BUFFER_SIZE);

  slave.setDataMode(SPI_MODE0);
  slave.setMaxTransferSize(BUFFER_SIZE);
  slave.setQueueSize(QUEUE_SIZE);

  slave.begin(VSPI);
  
  pinMode(PIN_CS, INPUT); 
  attachInterrupt(digitalPinToInterrupt(PIN_CS), onCsInterrupt, FALLING);
  
  // Initialize some test data
  holding_registers[0] = 0x1234;
  holding_registers[1] = 0x5678;
  holding_registers[2] = 0x9ABC;
  
  input_registers[0] = 0xDEF0;
  input_registers[1] = 0x1357;
  
  setBit(coils, 0, true);
  setBit(coils, 2, true);
  setBit(discrete_inputs, 1, true);
  setBit(discrete_inputs, 3, true);
  
  Serial.println("ModBus RTU Slave initialized!");
  Serial.println("Buffer size: " + String(BUFFER_SIZE) + " bytes (4-byte aligned)");
  Serial.println("Max Coils: " + String(MAX_COILS));
  Serial.println("Max Discrete Inputs: " + String(MAX_DISCRETE_INPUTS));
  Serial.println("Max Holding Registers: " + String(MAX_HOLDING_REGS));
  Serial.println("Max Input Registers: " + String(MAX_INPUT_REGS));
}

size_t resp_len = 0;

void loop() {
  if (transfer && !answer) {
    resp_len = receiveData();
  }
  else if (transfer && answer) {
    // Clear TX buffer
    memset(dma_tx_buf, 0, BUFFER_SIZE);
    
    // Process received ModBus request
    uint16_t response_length = processModBusRequest(dma_rx_buf, dma_tx_buf, resp_len);
    sendData();
    if (response_length > 0) {
      Serial.println("Sending ModBus response (" + String(response_length) + " bytes):");
      for (int i = 0; i < response_length; i++) {
        Serial.print("0x");
        if (dma_tx_buf[i] < 0x10) Serial.print("0");
        Serial.print(dma_tx_buf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  }

  if (dataAvailable) {
    Serial.println("Received ModBus request:");
    for (int i = 0; i < 20; ++i) { // Show first 20 bytes
      Serial.print("0x");
      if (dma_rx_buf[i] < 0x10) Serial.print("0");
      Serial.print(dma_rx_buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    dataAvailable = false;
  }
}

size_t receiveData() {
  // Always transfer full buffer size (multiple of 4) for DMA compatibility
  const size_t received_bytes = slave.transfer(dma_tx_buf, dma_rx_buf, BUFFER_SIZE);
  dataAvailable = true;
  transfer = false;
  answer = true;

  return received_bytes;
}

size_t sendData() {
  // Always transfer full buffer size (multiple of 4) for DMA compatibility  
  const size_t sent_bytes = slave.transfer(dma_tx_buf, dma_rx_buf, BUFFER_SIZE);
  answer = false;
  transfer = false;  

  return sent_bytes;
}