/**
 * BQ25723.hpp
 * 
 * ESP32 I2C driver for TI BQ25723 Battery Charge Controller
 * Supports register read/write operations and device configuration
 */

#ifndef BQ25723_HPP
#define BQ25723_HPP

#include <Arduino.h>
#include <Wire.h>

// Default I2C address for BQ25723
#define BQ25723_I2C_ADDR_DEFAULT 0x6B
#define BQ25723_I2C_ADDR_ALT     0x6A

// Common BQ25723 register addresses
#define BQ25723_REG_CHARGE_OPTION_0    0x00
#define BQ25723_REG_CHARGE_CURRENT     0x02
#define BQ25723_REG_CHARGE_VOLTAGE     0x04
#define BQ25723_REG_OTG_VOLTAGE        0x06
#define BQ25723_REG_OTG_CURRENT        0x08
#define BQ25723_REG_INPUT_VOLTAGE      0x0A
#define BQ25723_REG_VSYS_MIN           0x0C
#define BQ25723_REG_IIN_HOST           0x0E
#define BQ25723_REG_CHARGER_STATUS     0x20
#define BQ25723_REG_PROCHOT_STATUS     0x22
#define BQ25723_REG_IIN_DPM            0x24
#define BQ25723_REG_ADCVBUS_PSYS       0x26
#define BQ25723_REG_ADCIBAT            0x28
#define BQ25723_REG_ADCIINCMPIN        0x2A
#define BQ25723_REG_ADCVSYSVBAT        0x2C
#define BQ25723_REG_MANUFACTURER_ID    0x2E
#define BQ25723_REG_DEVICE_ID          0x2F
#define BQ25723_REG_CHARGE_OPTION_1    0x30
#define BQ25723_REG_CHARGE_OPTION_2    0x32
#define BQ25723_REG_CHARGE_OPTION_3    0x34
#define BQ25723_REG_PROCHOT_OPTION_0   0x36
#define BQ25723_REG_PROCHOT_OPTION_1   0x38
#define BQ25723_REG_ADC_OPTION         0x3A
#define BQ25723_REG_CHARGE_OPTION_4    0x3C
#define BQ25723_REG_VMIN_ACT_PROT      0x3E

class BQ25723 {
private:
    uint8_t _address;
    TwoWire* _wire;
    uint32_t _i2cSpeed;
    bool _initialized;
    
    // Helper function to check if communication is working
    bool checkCommunication() {
        _wire->beginTransmission(_address);
        return (_wire->endTransmission() == 0);
    }
    
public:
    /**
     * Constructor
     * @param address I2C address (default 0x6B)
     * @param wire TwoWire instance to use (default &Wire)
     * @param i2cSpeed I2C clock speed in Hz (default 100000)
     */
    BQ25723(uint8_t address = BQ25723_I2C_ADDR_DEFAULT, 
            TwoWire* wire = &Wire, 
            uint32_t i2cSpeed = 100000) 
        : _address(address), _wire(wire), _i2cSpeed(i2cSpeed), _initialized(false) {}
    
    /**
     * Initialize the BQ25723 communication
     * @param sdaPin SDA pin (default -1 uses default pins)
     * @param sclPin SCL pin (default -1 uses default pins)
     * @return true if device is detected, false otherwise
     */
    bool begin(int sdaPin = -1, int sclPin = -1) {
        if (sdaPin >= 0 && sclPin >= 0) {
            _wire->begin(sdaPin, sclPin);
        } else {
            _wire->begin();
        }
        
        _wire->setClock(_i2cSpeed);
        
        // Check if device is present
        if (!isConnected()) {
            _initialized = false;
            return false;
        }
        
        _initialized = true;
        return true;
    }
    
    /**
     * Check if device is connected and responding
     * @return true if device ACKs, false otherwise
     */
    bool isConnected() {
        return checkCommunication();
    }
    
    /**
     * Read a 16-bit register
     * @param regAddr Register address to read
     * @param value Pointer to store the read value
     * @return true if read successful, false otherwise
     */
    bool readRegister(uint8_t regAddr, uint16_t* value) {
        if (!_initialized || !value) return false;
        
        // Send register address
        _wire->beginTransmission(_address);
        _wire->write(regAddr);
        if (_wire->endTransmission(false) != 0) {
            return false;
        }
        
        // Read 2 bytes
        if (_wire->requestFrom(_address, (uint8_t)2) != 2) {
            return false;
        }
        
        // BQ25723 sends LSB first, then MSB
        uint8_t lsb = _wire->read();
        uint8_t msb = _wire->read();
        *value = (msb << 8) | lsb;
        
        return true;
    }
    
    /**
     * Read a 16-bit register (convenience overload)
     * @param regAddr Register address to read
     * @return Register value, or 0xFFFF on error
     */
    uint16_t readRegister(uint8_t regAddr) {
        uint16_t value;
        if (readRegister(regAddr, &value)) {
            return value;
        }
        return 0xFFFF; // Error value
    }
    
    /**
     * Write a 16-bit register
     * @param regAddr Register address to write
     * @param value 16-bit value to write
     * @return true if write successful, false otherwise
     */
    bool writeRegister(uint8_t regAddr, uint16_t value) {
        if (!_initialized) return false;
        
        _wire->beginTransmission(_address);
        _wire->write(regAddr);
        _wire->write(value & 0xFF);        // LSB first
        _wire->write((value >> 8) & 0xFF); // MSB second
        
        return (_wire->endTransmission(true) == 0);
    }
    
    /**
     * Read multiple consecutive registers
     * @param startAddr Starting register address
     * @param buffer Buffer to store values
     * @param count Number of registers to read
     * @return Number of registers successfully read
     */
    uint8_t readMultipleRegisters(uint8_t startAddr, uint16_t* buffer, uint8_t count) {
        if (!_initialized || !buffer || count == 0) return 0;
        
        uint8_t successCount = 0;
        for (uint8_t i = 0; i < count; i++) {
            if (readRegister(startAddr + i, &buffer[i])) {
                successCount++;
            } else {
                buffer[i] = 0xFFFF; // Mark as error
            }
        }
        return successCount;
    }
    
    /**
     * Get the current I2C address
     * @return Current I2C address
     */
    uint8_t getAddress() const {
        return _address;
    }
    
    /**
     * Change the I2C address (useful for testing alternate addresses)
     * @param newAddress New I2C address to use
     */
    void setAddress(uint8_t newAddress) {
        _address = newAddress;
        _initialized = false; // Force re-initialization
    }
    
    /**
     * Get initialization status
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const {
        return _initialized;
    }
    
    /**
     * Get human-readable register name
     * @param regAddr Register address
     * @return String with register name or "Unknown"
     */
    static const char* getRegisterName(uint8_t regAddr) {
        switch(regAddr) {
            case BQ25723_REG_CHARGE_OPTION_0:    return "CHARGE_OPTION_0";
            case BQ25723_REG_CHARGE_CURRENT:     return "CHARGE_CURRENT";
            case BQ25723_REG_CHARGE_VOLTAGE:     return "CHARGE_VOLTAGE";
            case BQ25723_REG_OTG_VOLTAGE:        return "OTG_VOLTAGE";
            case BQ25723_REG_OTG_CURRENT:        return "OTG_CURRENT";
            case BQ25723_REG_INPUT_VOLTAGE:      return "INPUT_VOLTAGE";
            case BQ25723_REG_VSYS_MIN:           return "VSYS_MIN";
            case BQ25723_REG_IIN_HOST:           return "IIN_HOST";
            case BQ25723_REG_CHARGER_STATUS:     return "CHARGER_STATUS";
            case BQ25723_REG_PROCHOT_STATUS:     return "PROCHOT_STATUS";
            case BQ25723_REG_IIN_DPM:            return "IIN_DPM";
            case BQ25723_REG_ADCVBUS_PSYS:       return "ADCVBUS_PSYS";
            case BQ25723_REG_ADCIBAT:            return "ADCIBAT";
            case BQ25723_REG_ADCIINCMPIN:        return "ADCIINCMPIN";
            case BQ25723_REG_ADCVSYSVBAT:        return "ADCVSYSVBAT";
            case BQ25723_REG_MANUFACTURER_ID:    return "MANUFACTURER_ID";
            case BQ25723_REG_DEVICE_ID:          return "DEVICE_ID";
            case BQ25723_REG_CHARGE_OPTION_1:    return "CHARGE_OPTION_1";
            case BQ25723_REG_CHARGE_OPTION_2:    return "CHARGE_OPTION_2";
            case BQ25723_REG_CHARGE_OPTION_3:    return "CHARGE_OPTION_3";
            case BQ25723_REG_PROCHOT_OPTION_0:   return "PROCHOT_OPTION_0";
            case BQ25723_REG_PROCHOT_OPTION_1:   return "PROCHOT_OPTION_1";
            case BQ25723_REG_ADC_OPTION:         return "ADC_OPTION";
            case BQ25723_REG_CHARGE_OPTION_4:    return "CHARGE_OPTION_4";
            case BQ25723_REG_VMIN_ACT_PROT:      return "VMIN_ACT_PROT";
            default:                             return "UNKNOWN";
        }
    }
};

#endif // BQ25723_HPP