#include "SmartBMS.h"

#define SMART_BMS_BAUD_RATE 9600
#define SMART_BMS_DATA_BITS 8
#define SMART_BMS_STOP_BITS 1
#define SMART_BMS_PARITY UART_PARITY_NONE

#define BMS_INFO_REQUEST "\xDD\xA5\x03\x00\xFF\xFD\x77"
#define BMS_INFO_REQUEST_LENGTH 7

using namespace daniel_f_garcia::bms;

SmartBMS::SmartBMS(uart_inst_t *uartId, uint txPin, uint rxPin) {
    mUartId = uartId;
    mTxPin = txPin;
    mRxPin = rxPin;

    uart_init(mUartId, SMART_BMS_BAUD_RATE);
    gpio_set_function(mTxPin, GPIO_FUNC_UART);
    gpio_set_function(mRxPin, GPIO_FUNC_UART);
    uart_set_format(mUartId, SMART_BMS_DATA_BITS, SMART_BMS_STOP_BITS, SMART_BMS_PARITY);
    uart_set_translate_crlf(mUartId, false);
}

void SmartBMS::refresh() {
    mError = "";

    uartPut(BMS_INFO_REQUEST, BMS_INFO_REQUEST_LENGTH);
    uart_tx_wait_blocking(mUartId);
    sleep_ms(100);
    size_t responseBytes = readResponse(mReadBuffer);
    mHexResponse = toHexString(mReadBuffer, responseBytes);
    if (!mError.empty()) {
        return;
    }

    if (mReadBuffer[1] != 0x03) {
        mError = "Unexpected response type";
        return;
    }

    if (responseBytes<4) {
        mError = "Fewer than 4 bytes received";
        return;
    }

    if (mReadBuffer[3] < 0x19) {
        mError = "Response length too short";
        return;
    }

    // Data addresses https://gitlab.com/bms-tools/bms-tools/-/blob/master/JBD_REGISTER_MAP.md
    // On my BMS: Fet Status @ 0x14, NTC @ 0x17

    uint8_t* data = mReadBuffer + 4;

    mPackVoltage = (float)((data[0x00] << 8) | data[0x01])/100;
    int16_t packCurrent = (data[0x02] << 8) | data[0x03];
    if (packCurrent==0) {
        mStatus = BMSStatus::NONE;
    } else if (packCurrent>0) {
        mStatus = BMSStatus::CHARGING;
    } else {
        mStatus = BMSStatus::DISCHARGING;
    }
    mPackCurrent = (float)abs(packCurrent)/100;

    mNominalCapacity = (data[0x06] << 8) | data[0x07];
    mResidualCapacity = (data[0x04] << 8) | data[0x05];

    if (mNominalCapacity==0) {
        mPercentCapacity = -1;
    } else {
        mPercentCapacity = (uint32_t) mResidualCapacity * 100 / mNominalCapacity;
    }

    mBalanceStatus = (data[0x0C] << 8) | data[0x0D];
    mTemperature = ((float)((data[0x17] << 8) | data[0x18]) - 2731)/10;
    //mTemperature = ((data[0x17] << 8) | data[0x18]);
    mFetCharging = data[0x14] & 0x01;
    mFetDischarging = (data[0x14] & 0x02) >> 1;
}

//region getters

bool SmartBMS::isValid() {
    return mError=="";
}

std::string SmartBMS::getError() {
    return mError;
}

float SmartBMS::getPackVoltage() {
    return mPackVoltage;
}

float SmartBMS::getPackCurrent() {
    return mPackCurrent;
}

BMSStatus SmartBMS::getStatus() {
    return mStatus;
}

uint16_t SmartBMS::getNominalCapacity() {
    return mNominalCapacity;
}

uint16_t SmartBMS::getResidualCapacity() {
    return mResidualCapacity;
}

int8_t SmartBMS::getPercentCapacity() {
    return mPercentCapacity;
}

uint16_t SmartBMS::getBalanceStatus() {
    return mBalanceStatus;
}

float SmartBMS::getTemperature() {
    return mTemperature;
}

bool SmartBMS::getFetCharging() {
    return mFetCharging;
}

bool SmartBMS::getFetDischarging() {
    return mFetDischarging;
}

std::string SmartBMS::getHexResponse() {
    return mHexResponse;
}

//endregion

//region other methods

std::string SmartBMS::toString() {
    return std::to_string(mPackVoltage) + ", " +
        std::to_string(mPackCurrent) + ", " +
        std::to_string(mPercentCapacity);
}

//endregion

//region private

void SmartBMS::uartPut(char *data, int length) {
    for (int i=0;i<length;i++) {
        uart_putc(mUartId, data[i]);
    }
}

size_t SmartBMS::readResponse(uint8_t *buffer) {
    // see https://blog.ja-ke.tech/2020/02/07/ltt-power-bms-chinese-protocol.html

    size_t totalBytes = 0;

    size_t bytesRead = readBytes(buffer, 3);
    totalBytes += bytesRead;
    if (bytesRead!=3) {
        return totalBytes;
    }

    if (buffer[0] != MAGIC_START) {
        mError = "unexpected value " + toHexString(buffer, 1) + " at response position 0";
        readAndIgnore();
        return bytesRead;
    }

    if (buffer[2] != STATUS_OK) {
        mError = "response returned error";
        readAndIgnore();
        return bytesRead;
    }

    size_t dataLength = buffer[3] + 4;
    if (dataLength > READ_BUFFER_SIZE - 4) {
        mError = "response exceeds buffer size";
        readAndIgnore();
        return bytesRead;
    }

    bytesRead = readBytes(buffer + totalBytes, dataLength);
    totalBytes += bytesRead;
    if (bytesRead != dataLength) {
        mError = "timeout reading response data";
        return totalBytes;
    }

    if (buffer[totalBytes - 1] != MAGIC_END) {
        mError = "unexpected value at response end";
        return totalBytes;
    }

    return totalBytes;
}

void SmartBMS::readAndIgnore() {
    while (uart_is_readable(mUartId)) {
        uart_getc(mUartId);
    }
}

size_t SmartBMS::readByte(uint8_t *buffer) {
    if (uart_is_readable_within_us(mUartId, READ_TIMEOUT)) {
        uart_read_blocking(mUartId, buffer, 1);
        return 1;
    } else {
        mError = "read timeout";
        return 0;
    }
}

// caller needs to ensure there will be no buffer overruns
size_t SmartBMS::readBytes(uint8_t *buffer, size_t length) {
    size_t totalBytes = 0;

    for (size_t i = 0; i < length; i++) {
        size_t bytes = readByte(buffer);
        if (bytes==0) {
            mError = "read timeout at byte " + std::to_string(i);
            return totalBytes;
        } else {
            buffer++;
            totalBytes++;
        }
    }

    return totalBytes;
}

std::string SmartBMS::toHexString(const uint8_t *data, int length) {
    if (length==0) {
        return "LENGTH WAS 0";
    }

    int hexStringLength = length*2 + (length-1)/4;
    char hexString[hexStringLength + 1];
    int hexStringIndex = 0;
    for (int i=0; i<length; i++) {
        if (i > 0 && i % 4 == 0) {
            hexString[hexStringIndex] = '.';
            hexStringIndex++;
        }
        sprintf(hexString + hexStringIndex, "%02x", data[i]);
        hexStringIndex += 2;
    }
    hexString[hexStringLength] = 0;

    return std::string(hexString);
}

//endregion