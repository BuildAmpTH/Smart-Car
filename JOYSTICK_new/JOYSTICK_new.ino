#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 9); // CE, CSN
const byte address[6] = "00001";
char dataBuffer[32];

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting NRF24 Transmitter..."));

  if (!radio.begin()) {
    Serial.println(F("ERROR: nRF24L01 not responding! Check wiring & power."));
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(3, 15);  // 3 retries, 15x250us delay
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println(F("nRF24L01 initialized successfully."));
}

void loop() {
  //  แกน A1 = Y, A0 = X 
  int y = analogRead(A1);  // หน้า–หลัง
  int x = analogRead(A0);  // ซ้าย–ขวา

  sprintf(dataBuffer, "%d,%d", x, y);  // Format: "X,Y"

  Serial.print(F("Sending: "));
  Serial.println(dataBuffer);

  bool success = radio.write(&dataBuffer, sizeof(dataBuffer));

  if (success) {
    Serial.println(F("Send success"));
  } else {
    Serial.println(F("Send failed!"));
    debugRadioStatus();
  } 

  delay(200);  // ปรับให้เหมาะกับความถี่ที่ต้องการส่ง
}

void debugRadioStatus() {
  Serial.println(F("=== RF24 Debug Info ==="));
  Serial.print(F("  TX FULL: "));
  Serial.println(radio.testCarrier() ? "YES (Busy)" : "NO (Free)");
  Serial.print(F("  ACK: "));
  Serial.println(radio.isAckPayloadAvailable() ? "Available" : "Not received");
  Serial.println(F("========================"));
}
