#include <Wire.h>
#include "BQ25723.hpp"

// Define the I2C pins for the ESP32 Wrover
#define SDA_PIN 21
#define SCL_PIN 22

// Create an instance of the BQ25723 class
BQ25723 charger(BQ25723_I2C_ADDR_DEFAULT, &Wire, 100000); // Default I2C address 0x6B

void scanI2CBus() {
    Serial.println("Scanning I2C bus...");
    byte count = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.print("I2C device found at 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.println();
            count++;
        }
        delay(5);
    }

    if (count == 0) {
        Serial.println("No I2C devices found.");
    } else {
        Serial.print("Total I2C devices found: ");
        Serial.println(count);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Give Serial monitor time to open

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);

    scanI2CBus();

    // Try initializing the charger
    if (charger.begin(SDA_PIN, SCL_PIN)) {
        Serial.println("BQ25723 detected and initialized!");

        // Read the DEVICE_ID register (should return a fixed ID, e.g., 0x23 for BQ25723)
        uint16_t deviceId = charger.readRegister(BQ25723_REG_DEVICE_ID);
        if (deviceId != 0xFFFF) {
            Serial.print("DEVICE_ID (0xFF): 0x");
            Serial.println(deviceId, HEX);
        } else {
            Serial.println("Failed to read DEVICE_ID register.");
        }
    } else {
        Serial.println("BQ25723 not found at expected I2C address (0x6B or 0x6A).");
    }
}

void loop() {
    // Nothing to do here
}
