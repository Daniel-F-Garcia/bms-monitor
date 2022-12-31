#ifndef BMS_MONITOR_NEXTION_H
#define BMS_MONITOR_NEXTION_H

#include <string>

class Nextion {
private:
    uart_inst_t *mUartId;
    uint mBaudRate;
    uint mTxPin;
    uint mRxPin;
public:
    Nextion(uart_inst_t *uartId, uint baudRate, uint txPin, uint rxPin);

    void setText(std::string variable, std::string format, ...);
    void setInt(std::string, std::string, int value);
    void command(std::string);
};


#endif //BMS_MONITOR_NEXTION_H