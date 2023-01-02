#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "Nextion.h"
#include "SmartBMS.h"

#define NEXTION_UART_ID uart0
#define NEXTION_TX_PIN 16
#define NEXTION_RX_PIN 17

#define SMART_BMS_UART_ID uart1
#define SMART_BMS_TX_PIN 4
#define SMART_BMS_RX_PIN 5

#define INVALID_THRESHOLD 2
#define CELL_OVP_1 3650
#define CELL_OVP_2 3750
#define CELL_UVP_1 2700
#define CELL_UVP_2 2500

#define PACK_OVP_1 1460
#define PACK_OVP_2 1500
#define PACK_UVP_1 1100
#define PACK_UVP_2 1000

#define PACK_CURRENT_MAX_1 1000
#define PACK_CURRENT_MAX_2 1200

// Celcius multiples of 0.1
#define TEMP_WARN 450
#define TEMP_OVER 600

void displayBMS(Nextion nextion, SmartBMS bms) {
    nextion.setText("t8", "all is good");

    // Pack Voltage
    nextion.setText("t0", "%d.%02d", bms.getPackVoltage()/100, bms.getPackVoltage() % 100);
    if (bms.getPackVoltage()>PACK_OVP_2 || bms.getPackVoltage()<PACK_UVP_2) {
        nextion.command("t0.pco=63488"); // red
    } else if (bms.getPackVoltage()>PACK_OVP_1 || bms.getPackVoltage()<PACK_UVP_1) {
        nextion.command("t0.pco=65504"); // yellow
    } else {
        nextion.command("t0.pco=65535"); // white
    }

    // Current
    if (bms.getStatus()==BMSStatus::DISCHARGING) {
        nextion.setText("t1", "-%d.%02d", bms.getPackCurrent()/100, bms.getPackCurrent() % 100);
    } else {
        nextion.setText("t1", "%d.%02d", bms.getPackCurrent()/100, bms.getPackCurrent() % 100);
    }
    if (bms.getPackCurrent()>PACK_CURRENT_MAX_2) {
        nextion.command("t1.pco=63488"); // red
    } else if (bms.getPackCurrent()>PACK_CURRENT_MAX_1) {
        nextion.command("t1.pco=65504"); // yellow
    } else {
        nextion.command("t1.pco=65535"); // white
    }

    // Percent Capacity
    if (bms.getPercentCapacity()==-1) {
        nextion.setText("t2", "ERR");
    } else {
        nextion.setText("t2", "%01d", bms.getPercentCapacity());
        nextion.setInt("bar", "val", bms.getPercentCapacity());
    }

    // Temperature
    if (bms.getTemperature()<0) {
        nextion.setText("t3", "-%d.%01d", bms.getTemperature()/10, bms.getTemperature() % 10);
    } else {
        nextion.setText("t3", "%d.%01d", bms.getTemperature()/10, bms.getTemperature() % 10);
    }
    if (bms.getTemperature()>TEMP_OVER) {
        nextion.command("t3.pco=63488"); // red
    } else if (bms.getTemperature()>TEMP_WARN) {
        nextion.command("t3.pco=65504"); // yellow
    } else {
        nextion.command("t3.pco=65535"); // white
    }
}

int main() {
    Nextion nextion = Nextion(NEXTION_UART_ID, NEXTION_TX_PIN, NEXTION_RX_PIN);
    SmartBMS bms = SmartBMS(SMART_BMS_UART_ID, SMART_BMS_TX_PIN, SMART_BMS_RX_PIN);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    nextion.setText("t8", "initialising...");

    sleep_ms(500);
    bms.refresh();
    sleep_ms(500);

    int i = 0;
    int validCount = 0;
    int invalidCount = 0;

    while (true) {
        bms.refresh();
        if (bms.isValid()) {
            validCount++;
            displayBMS(nextion, bms);
        } else {
            invalidCount++;
            nextion.setText("t8", bms.getError());
            //nextion.setText("t8", bms.getHexResponse());
        }


        sleep_ms(1000);
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);
        gpio_put(LED_PIN, 0);

        i++;
    }
}

