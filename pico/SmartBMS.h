#include "pico/stdlib.h"
#include "string"

#ifndef BMS_MONITOR_SMARTBMS_H
#define BMS_MONITOR_SMARTBMS_H

#define READ_BUFFER_SIZE 128
#define READ_TIMEOUT 50000

#define MAGIC_START 0xDD
#define MAGIC_END 0x77
#define STATUS_OK 0x00

namespace daniel_f_garcia::bms {

    enum BMSStatus {
        DISCHARGING, NONE, CHARGING
    };

    class SmartBMS {
    private:
        uart_inst_t *mUartId;
        uint mTxPin;
        uint mRxPin;

        uint8_t mReadBuffer[READ_BUFFER_SIZE];

        std::string mError;

        float mPackVoltage;
        float mPackCurrent;
        BMSStatus mStatus;
        uint16_t mNominalCapacity;   // multiples of 10mah
        uint16_t mResidualCapacity;  // multiples of 10mah
        int8_t mPercentCapacity;   // percent, -1 if error
        uint16_t mBalanceStatus;
        float mTemperature;       // celsius
        bool mFetCharging;       // true if charging fet is on
        bool mFetDischarging;    // true if discharging fet is on

        std::string mHexResponse;

        void uartPut(char *data, int length);

        size_t readResponse(uint8_t *buffer);

        void readAndIgnore();

        size_t readByte(uint8_t *buffer);

        size_t readBytes(uint8_t *buffer, size_t length);

        std::string toHexString(const uint8_t *data, int length);

    public:
        SmartBMS(uart_inst_t *uartId, uint txPin, uint rxPin);

        void refresh();

        bool isValid();

        std::string getError();

        float getPackVoltage();

        float getPackCurrent();

        BMSStatus getStatus();

        uint16_t getNominalCapacity();

        uint16_t getResidualCapacity();

        int8_t getPercentCapacity();

        uint16_t getBalanceStatus();

        float getTemperature();

        bool getFetCharging();

        bool getFetDischarging();

        std::string getHexResponse();

        std::string toString();
    };
}

#endif //BMS_MONITOR_SMARTBMS_H
