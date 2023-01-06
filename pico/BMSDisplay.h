#ifndef BMS_MONITOR_BMSDISPLAY_H
#define BMS_MONITOR_BMSDISPLAY_H

#include "Nextion.h"
#include "SmartBMS.h"


namespace daniel_f_garcia::bms {
    class BMSDisplay {
    private:
        Nextion& mNextion;
        SmartBMS& mBMS;
        std::string format(float value, int decimalPlaces);
        void refreshPack();
        void clearPack();
        void refreshCells();
        void clearCells();
    public:
        BMSDisplay(Nextion &nextion, SmartBMS &bms);
        void refresh();
    };
}

#endif //BMS_MONITOR_BMSDISPLAY_H
