#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define enA 2
#define in1 3
#define in2 4
#define enB 7
#define in3 5
#define in4 6

RF24 radio(8, 9); // CE, CSN
const byte address[6] = "00001";

char dataBuffer[32];
int xAxis = 512, yAxis = 512;
unsigned long lastReceiveTime = 0;
unsigned long lastDebugTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  if (!radio.begin()) {
    Serial.println(F("ERROR: nRF24L01 not responding! Check wiring & power."));
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, address);
  radio.startListening();
  Serial.println("Receiver ready...");
}

void loop() {
  // ตรวจสอบข้อมูลที่ได้รับจาก transmitter
  if (radio.available()) {
    radio.read(&dataBuffer, sizeof(dataBuffer));

    if (strchr(dataBuffer, ',')) {
      char* token = strtok(dataBuffer, ",");
      if (token) {
        int tempX = atoi(token);
        token = strtok(NULL, ",");
        if (token) {
          int tempY = atoi(token);
          if (tempX >= 0 && tempX <= 1023 && tempY >= 0 && tempY <= 1023) {
            xAxis = tempX;
            yAxis = tempY;
            lastReceiveTime = millis();
            Serial.print("Received X: "); Serial.print(xAxis);
            Serial.print(" Y: "); Serial.println(yAxis);
          }
        }
      }
    } else {
      Serial.println("Invalid data, skipping...");
    }
  }

  // หยุดมอเตอร์หากไม่ได้รับข้อมูล
  if (millis() - lastReceiveTime > 1000) {
    analogWrite(enA, 0);
    analogWrite(enB, 0);
    digitalWrite(in1, LOW); digitalWrite(in2, LOW);
    digitalWrite(in3, LOW); digitalWrite(in4, LOW);
    Serial.println("No data timeout - Motors stopped");
  }

  // ควบคุมความเร็ว
  int motorSpeedA = 0;
  int motorSpeedB = 0;

  if (yAxis < 490) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
    motorSpeedA = map(yAxis, 490, 0, 0, 255);
    motorSpeedB = map(yAxis, 490, 0, 0, 255);
  } else if (yAxis > 530) {
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
    motorSpeedA = map(yAxis, 530, 1023, 0, 255);
    motorSpeedB = map(yAxis, 530, 1023, 0, 255);
  }

  if (xAxis < 490) {
    int offset = map(xAxis, 490, 0, 0, 255);
    motorSpeedA += offset;
    motorSpeedB -= offset;
  } else if (xAxis > 530) {
    int offset = map(xAxis, 530, 1023, 0, 255);
    motorSpeedA -= offset;
    motorSpeedB += offset;
  }

  motorSpeedA = constrain(motorSpeedA, 0, 255);
  motorSpeedB = constrain(motorSpeedB, 0, 255);
  if (motorSpeedA < 70) motorSpeedA = 0;
  if (motorSpeedB < 70) motorSpeedB = 0;

  analogWrite(enA, motorSpeedA);
  analogWrite(enB, motorSpeedB);

  // === แสดงสถานะ NRF ทุก 2 วินาที ===
  if (millis() - lastDebugTime > 2000) {
    lastDebugTime = millis();
    debugNRF24();
  }
}

void debugNRF24() {
  Serial.println(F("=== NRF24L01 Status ==="));
  Serial.print(F("  isChipConnected: "));
  Serial.println(radio.isChipConnected() ? "YES" : "NO");

  Serial.print(F("  isAckPayloadAvailable: "));
  Serial.println(radio.isAckPayloadAvailable() ? "YES" : "NO");

  Serial.print(F("  testCarrier (channel noise): "));
  Serial.println(radio.testCarrier() ? "BUSY" : "CLEAR");

  Serial.println(F("========================"));
}
