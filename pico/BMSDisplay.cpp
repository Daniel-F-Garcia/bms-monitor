#include "BMSDisplay.h"
#include <iomanip>
#include <sstream>

#define ERROR_THRESHOLD 2

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
    if (mBMS.isPackValid()) {
        mNextion.setText("t8", "");
        refreshPack();
    } else if (mBMS.getPackErrorCount()>ERROR_THRESHOLD) {
        mNextion.setText("t8", mBMS.getPackError());
    }

    if (mBMS.isCellsValid()) {
        mNextion.setText("t8", "");
        refreshCells();
    } else if (mBMS.getCellsErrorCount()>ERROR_THRESHOLD) {
        mNextion.setText("t8", mBMS.getCellsError());
    }
}

//region private

void BMSDisplay::refreshPack() {
    // Pack Voltage
    mNextion.setText("t0", format(mBMS.getPackVoltage(),2));

    if (mBMS.getPackVoltage()>PACK_OVP_2 || mBMS.getPackVoltage()<PACK_UVP_2) {
        mNextion.command("t0.pco=63488"); // red
    } else if (mBMS.getPackVoltage()>PACK_OVP_1 || mBMS.getPackVoltage()<PACK_UVP_1) {
        mNextion.command("t0.pco=65504"); // yellow
    } else {
        mNextion.command("t0.pco=65535"); // white
    }

    // Current
    if (mBMS.getStatus()==BMSStatus::DISCHARGING) {
        mNextion.setText("t1", "-" + format(mBMS.getPackCurrent(),2));
    } else {
        mNextion.setText("t1", format(mBMS.getPackCurrent(),2));
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
        mNextion.setText("t2", std::to_string(mBMS.getPercentCapacity()));
        mNextion.setInt("bar", "val", mBMS.getPercentCapacity());
    }

    // Temperature
    mNextion.setText("t3", format(mBMS.getTemperature(), 2));

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

    // Charge/Discharge FETs
    if (mBMS.getFetCharging()) {
        mNextion.command("vis p4,1");
    } else {
        mNextion.command("vis p4,0");
    }
    if (mBMS.getFetDischarging()) {
        mNextion.command("vis p5,1");
    } else {
        mNextion.command("vis p5,0");
    }
}

void BMSDisplay::clearPack() {
    // Pack Voltage
    mNextion.setText("t0", ". . . .");
    mNextion.command("t0.pco=65535"); // white

    // Current
    mNextion.setText("t1", ". . . .");
    mNextion.command("t1.pco=65535"); // white

    // Percent Capacity
    mNextion.setText("t2", ". . . .");

    // Temperature
    mNextion.setText("t3", ". . . .");
    mNextion.command("t3.pco=65535"); // white

    // Balance Status
    mNextion.command("vis p0,0");
    mNextion.command("vis p1,0");
    mNextion.command("vis p2,0");
    mNextion.command("vis p3,0");

    // Charge/Discharge FETs
    mNextion.command("vis p4,0");
    mNextion.command("vis p5,0");

}

void BMSDisplay::refreshCells() {
    for (int i=0; i<4; i++) {
        std::string field = "t" + std::to_string(i+4);
        mNextion.setText(field, format(mBMS.getCellVolts(i),2));
        if (mBMS.getCellVolts(i)>CELL_OVP_2 || mBMS.getCellVolts(i)<CELL_UVP_2) {
            mNextion.command(field + ".pco=63488"); // red
        } else if (mBMS.getCellVolts(i)>CELL_OVP_1 || mBMS.getCellVolts(i)<CELL_UVP_1) {
            mNextion.command(field + ".pco=65504"); // yellow
        } else {
            mNextion.command(field + ".pco=65535"); // white
        }
    }
}

void BMSDisplay::clearCells() {
    mNextion.setText("t4", ". . . .");
    mNextion.command("t4.pco=65535"); // white

    mNextion.setText("t5", ". . . .");
    mNextion.command("t5.pco=65535"); // white

    mNextion.setText("t6", ". . . .");
    mNextion.command("t6.pco=65535"); // white

    mNextion.setText("t7", ". . . .");
    mNextion.command("t7.pco=65535"); // white
}

std::string BMSDisplay::format(float value, int decimalPlaces) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(decimalPlaces) << value;
    return stream.str();
}

//endregion