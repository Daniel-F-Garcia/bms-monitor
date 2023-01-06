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

        std::string mPackError;
        std::string mCellsError;
        int mPackErrorCount;
        int mCellsErrorCount;
        std::string mPackHexResponse;
        std::string mCellsHexResponse;

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
        float mCellVolts[4];

        void refreshPack();
        void refreshCells();

        void uartPut(const char *data, int length);

        size_t readResponse(uint8_t *buffer, std::string &error);

        void readAndIgnore();

        size_t readByte(uint8_t *buffer, std::string &error);

        size_t readBytes(uint8_t *buffer, size_t length, std::string &error);

        std::string toHexString(const uint8_t *data, int length);

    public:
        SmartBMS(uart_inst_t *uartId, uint txPin, uint rxPin);

        void refresh();

        bool isPackValid();
        bool isCellsValid();
        std::string getPackError();
        std::string getCellsError();
        int getPackErrorCount();
        int getCellsErrorCount();

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
        float getCellVolts(int index);

        std::string getPackHexResponse();
        std::string getCellsHexResponse();

        std::string toString();
    };
}

#endif //BMS_MONITOR_SMARTBMS_H
