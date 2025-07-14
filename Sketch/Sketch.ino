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
        Serial.println("\nDumping BQ25723 Registers:");
        uint8_t knownRegisters[] = {
            BQ25723_REG_CHARGE_OPTION_0,
            BQ25723_REG_CHARGE_CURRENT,
            BQ25723_REG_CHARGE_VOLTAGE,
            BQ25723_REG_OTG_VOLTAGE,
            BQ25723_REG_OTG_CURRENT,
            BQ25723_REG_INPUT_VOLTAGE,
            BQ25723_REG_VSYS_MIN,
            BQ25723_REG_IIN_HOST,
            BQ25723_REG_CHARGER_STATUS,
            BQ25723_REG_PROCHOT_STATUS,
            BQ25723_REG_IIN_DPM,
            BQ25723_REG_ADCVBUS_PSYS,
            BQ25723_REG_ADCIBAT,
            BQ25723_REG_ADCIINCMPIN,
            BQ25723_REG_ADCVSYSVBAT,
            BQ25723_REG_MANUFACTURER_ID,
            BQ25723_REG_DEVICE_ID,
            BQ25723_REG_CHARGE_OPTION_1,
            BQ25723_REG_CHARGE_OPTION_2,
            BQ25723_REG_CHARGE_OPTION_3,
            BQ25723_REG_PROCHOT_OPTION_0,
            BQ25723_REG_PROCHOT_OPTION_1,
            BQ25723_REG_ADC_OPTION,
            BQ25723_REG_CHARGE_OPTION_4,
            BQ25723_REG_VMIN_ACT_PROT
        };

        for (uint8_t i = 0; i < sizeof(knownRegisters); i++) {
            uint8_t reg = knownRegisters[i];
            uint16_t value = charger.readRegister(reg);
            Serial.print("0x");
            if (reg < 0x10) Serial.print("0");
            Serial.print(reg, HEX);
            Serial.print(" (");
            Serial.print(BQ25723::getRegisterName(reg));
            Serial.print("): 0x");
            Serial.println(value, HEX);
        }
    } else {
        Serial.println("BQ25723 not found at expected I2C address (0x6B or 0x6A).");
    }
}

void loop() {
    // Nothing to do here
}
