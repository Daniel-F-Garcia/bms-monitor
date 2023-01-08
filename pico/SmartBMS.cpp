#include "SmartBMS.h"

#define SMART_BMS_BAUD_RATE 9600
#define SMART_BMS_DATA_BITS 8
#define SMART_BMS_STOP_BITS 1
#define SMART_BMS_PARITY UART_PARITY_NONE

#define REQUEST_WAIT_MS 100

#define BMS_INFO_REQUEST "\xDD\xA5\x03\x00\xFF\xFD\x77"
#define BMS_INFO_REQUEST_LENGTH 7
#define BMS_CELL_REQUEST "\xDD\xA5\x04\x00\xFF\xFC\x77"
#define BMS_CELL_REQUEST_LENGTH 7

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
    refreshPack();
    if (isPackValid()) {
        mPackErrorCount = 0;
    } else {
        mPackErrorCount++;
    }

    refreshCells();
    if (isCellsValid()) {
        mCellsErrorCount = 0;
    } else {
        mCellsErrorCount++;
    }
}

//region getters

bool SmartBMS::isPackValid() {
    return mPackError=="";
}

bool SmartBMS::isCellsValid() {
    return mCellsError=="";
}

std::string SmartBMS::getPackError() {
    return mPackError;
}

std::string SmartBMS::getCellsError() {
    return mCellsError;
}

int SmartBMS::getPackErrorCount() {
    return mPackErrorCount;
}

int SmartBMS::getCellsErrorCount() {
    return mCellsErrorCount;
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

float SmartBMS::getCellVolts(int index) {
    return mCellVolts[index];
}

std::string SmartBMS::getPackHexResponse() {
    return mPackHexResponse;
}

std::string SmartBMS::getCellsHexResponse() {
    return mCellsHexResponse;
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

void SmartBMS::refreshPack() {
    mPackError = "";
    readAndIgnore();
    uartPut(BMS_INFO_REQUEST, BMS_INFO_REQUEST_LENGTH);
    uart_tx_wait_blocking(mUartId);
    sleep_ms(REQUEST_WAIT_MS);
    size_t responseBytes = readResponse(mReadBuffer, mPackError);
    mPackHexResponse = toHexString(mReadBuffer, responseBytes);
    if (!mPackError.empty()) {
        return;
    }

    if (mReadBuffer[1] != 0x03) {
        mPackError = "Unexpected response type";
        return;
    }

    if (responseBytes<4) {
        mPackError = "Fewer than 4 bytes received";
        return;
    }

    if (mReadBuffer[3] < 0x19) {
        mPackError = "Response length too short";
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
    mFetCharging = data[0x14] & 0x01;
    mFetDischarging = (data[0x14] & 0x02) >> 1;
}

void SmartBMS::refreshCells() {
    mCellsError = "";
    for (int i=0;i<4;i++) {
        mCellVolts[i] = 0;
    }

    readAndIgnore();
    uartPut(BMS_CELL_REQUEST, BMS_CELL_REQUEST_LENGTH);
    uart_tx_wait_blocking(mUartId);
    sleep_ms(REQUEST_WAIT_MS);
    size_t responseBytes = readResponse(mReadBuffer, mCellsError);
    mCellsHexResponse = toHexString(mReadBuffer, responseBytes);
    if (!mCellsError.empty()) {
        return;
    }

    if (mReadBuffer[1] != 0x04) {
        mCellsError = "Unexpected response type";
        return;
    }

    if (responseBytes<4) {
        mCellsError = "Fewer than 4 bytes received";
        return;
    }

    if (mReadBuffer[3] < 0x08) {
        mCellsError = "Response length too short";
        return;
    }

    uint8_t* data = mReadBuffer + 4;

    mCellVolts[0] = (float)((data[0] << 8) | data[1])/1000;
    mCellVolts[1] = (float)((data[2] << 8) | data[3])/1000;
    mCellVolts[2] = (float)((data[4] << 8) | data[5])/1000;
    mCellVolts[3] = (float)((data[6] << 8) | data[7])/1000;
}


void SmartBMS::uartPut(const char *data, int length) {
    for (int i=0;i<length;i++) {
        uart_putc(mUartId, data[i]);
    }
}

size_t SmartBMS::readResponse(uint8_t *buffer, std::string &error) {
    // see https://blog.ja-ke.tech/2020/02/07/ltt-power-bms-chinese-protocol.html

    size_t totalBytes = 0;

    size_t bytesRead = readBytes(buffer, 4, error);
    totalBytes += bytesRead;
    if (bytesRead!=4) {
        return totalBytes;
    }

    if (buffer[0] != MAGIC_START) {
        error = "unexpected value " + toHexString(buffer, 1) + " at response position 0";
        readAndIgnore();
        return bytesRead;
    }

    if (buffer[2] != STATUS_OK) {
        error = "response returned error";
        readAndIgnore();
        return bytesRead;
    }

    size_t dataLength = buffer[3] + 3;
    if (dataLength > READ_BUFFER_SIZE - 3) {
        error = "response exceeds buffer size (" + std::to_string(dataLength) + ")";
        readAndIgnore();
        return bytesRead;
    }

    bytesRead = readBytes(buffer + totalBytes, dataLength, error);
    totalBytes += bytesRead;
    if (bytesRead != dataLength) {
        error = "timeout reading response data";
        return totalBytes;
    }

    if (buffer[totalBytes - 1] != MAGIC_END) {
        error = "unexpected value at response end";
        return totalBytes;
    }

    return totalBytes;
}

void SmartBMS::readAndIgnore() {
    while (uart_is_readable(mUartId)) {
        uart_getc(mUartId);
    }
}

size_t SmartBMS::readByte(uint8_t *buffer, std::string &error) {
    if (uart_is_readable_within_us(mUartId, READ_TIMEOUT)) {
        uart_read_blocking(mUartId, buffer, 1);
        return 1;
    } else {
        error = "read timeout";
        return 0;
    }
}

// caller needs to ensure there will be no buffer overruns
size_t SmartBMS::readBytes(uint8_t *buffer, size_t length, std::string &error) {
    size_t totalBytes = 0;

    for (size_t i = 0; i < length; i++) {
        size_t bytes = readByte(buffer, error);
        if (bytes==0) {
            error = "read timeout at byte " + std::to_string(i);
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