#include "pico/stdlib.h"
#include "string"

#ifndef BMS_MONITOR_SMARTBMS_H
#define BMS_MONITOR_SMARTBMS_H

#define READ_BUFFER_SIZE 64
#define READ_TIMEOUT 2000

#define MAGIC_START 0xDD
#define MAGIC_END 0xFF
#define STATUS_OK 0x00

enum BMSStatus { DISCHARGING, NONE, CHARGING};

class SmartBMS {
private:
    uart_inst_t *mUartId;
    uint mTxPin;
    uint mRxPin;

    uint8_t mReadBuffer[READ_BUFFER_SIZE];

    std::string mError;

    uint16_t mPackVoltage;       // multiples of 10 mV
    int16_t  mPackCurrent;       // multiples of 10ma. Charging is positive, discharging is negative
    BMSStatus mStatus;
    uint16_t mNominalCapacity;   // multiples of 10mah
    uint16_t mResidualCapacity;  // multiples of 10mah
    int8_t   mPercentCapacity;   // percent, -1 if error
    uint16_t mBalanceStatus;
    int16_t  mTemperature;       // celsius in multiples of 0.1 degree
    bool     mFetCharging;       // true if charging fet is on
    bool     mFetDischarging;    // true if discharging fet is on

    size_t readResponse(uint8_t *buffer);
    void readAndIgnore();
    size_t readByte(uint8_t *buffer);
    size_t readBytes(uint8_t *buffer, size_t length);

public:
    SmartBMS(uart_inst_t *uartId, uint txPin, uint rxPin);
    void refresh();

    bool isValid();
    uint16_t getPackVoltage();
    int16_t  getPackCurrent();
    BMSStatus getStatus();
    uint16_t getNominalCapacity();
    uint16_t getResidualCapacity();
    int8_t  getPercentCapacity();
    int16_t  getTemperature();
    bool     getFetCharging();
    bool     getFetDischarging();
};


#endif //BMS_MONITOR_SMARTBMS_H
