#ifndef BMS_MONITOR_NEXTION_H
#define BMS_MONITOR_NEXTION_H

#include <string>
#include "pico/stdlib.h"

namespace daniel_f_garcia::bms {
    class Nextion {
    private:
        uart_inst_t *mUartId;
        uint mTxPin;
        uint mRxPin;
    public:
        Nextion(uart_inst_t *uartId, uint txPin, uint rxPin);

        void setText(std::string variable, std::string text);

        void setInt(std::string, std::string, int value);

        void command(std::string);
    };
}

#endif //BMS_MONITOR_NEXTION_H
