#include "BMSDisplay.h"
#include <iomanip>
#include <sstream>

#define INVALID_THRESHOLD 2
#define CELL_OVP_1 3.65f
#define CELL_OVP_2 3.75f
#define CELL_UVP_1 2.70f
#define CELL_UVP_2 2.50f

#define PACK_OVP_1 14.60f
#define PACK_OVP_2 15.00f
#define PACK_UVP_1 11.00f
#define PACK_UVP_2 10.00f

#define PACK_CURRENT_MAX_1 1000
#define PACK_CURRENT_MAX_2 1200

// Celsius multiples of 0.1
#define TEMP_WARN 450
#define TEMP_OVER 600

using namespace daniel_f_garcia::bms;

BMSDisplay::BMSDisplay(Nextion &nextion, SmartBMS &bms) : mNextion(nextion), mBMS(bms) {
}

void BMSDisplay::refresh() {
    //clear any errors
    //mNextion.setText("t8", "");
    mNextion.setText("t8", mBMS.getHexResponse());

    // Pack Voltage
    mNextion.setText("t0", format(mBMS.getPackVoltage(),2).c_str());

    if (mBMS.getPackVoltage()>PACK_OVP_2 || mBMS.getPackVoltage()<PACK_UVP_2) {
        mNextion.command("t0.pco=63488"); // red
    } else if (mBMS.getPackVoltage()>PACK_OVP_1 || mBMS.getPackVoltage()<PACK_UVP_1) {
        mNextion.command("t0.pco=65504"); // yellow
    } else {
        mNextion.command("t0.pco=65535"); // white
    }

    // Current
    if (mBMS.getStatus()==BMSStatus::DISCHARGING) {
        mNextion.setText("t1", ("-" + format(mBMS.getPackCurrent(),2)).c_str());
    } else {
        mNextion.setText("t1", format(mBMS.getPackCurrent(),2).c_str());
    }

    if (mBMS.getPackCurrent()>PACK_CURRENT_MAX_2) {
        mNextion.command("t1.pco=63488"); // red
    } else if (mBMS.getPackCurrent()>PACK_CURRENT_MAX_1) {
        mNextion.command("t1.pco=65504"); // yellow
    } else {
        mNextion.command("t1.pco=65535"); // white
    }

    // Percent Capacity
    if (mBMS.getPercentCapacity()==-1) {
        mNextion.setText("t2", "ERR");
    } else {
        mNextion.setText("t2", std::to_string(mBMS.getPercentCapacity()).c_str());
        mNextion.setInt("bar", "val", mBMS.getPercentCapacity());
    }

    // Temperature
    mNextion.setText("t3", format(mBMS.getTemperature(), 2).c_str());

    if (mBMS.getTemperature()>TEMP_OVER) {
        mNextion.command("t3.pco=63488"); // red
    } else if (mBMS.getTemperature()>TEMP_WARN) {
        mNextion.command("t3.pco=65504"); // yellow
    } else {
        mNextion.command("t3.pco=65535"); // white
    }

    // Balance Status
    if (mBMS.getBalanceStatus() & 0b00000001) {
        mNextion.command("vis p0,1");
    } else {
        mNextion.command("vis p0,0");
    }

    if (mBMS.getBalanceStatus() & 0b00000010) {
        mNextion.command("vis p1,1");
    } else {
        mNextion.command("vis p1,0");
    }

    if (mBMS.getBalanceStatus() & 0b00000100) {
        mNextion.command("vis p2,1");
    } else {
        mNextion.command("vis p2,0");
    }

    if (mBMS.getBalanceStatus() & 0b00001000) {
        mNextion.command("vis p3,1");
    } else {
        mNextion.command("vis p3,0");
    }
}

std::string BMSDisplay::format(float value, int decimalPlaces) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(decimalPlaces) << value;
    return stream.str();
}