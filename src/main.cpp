#define LIN_PQ 1
#define LIN_MQB 1
#define DEBUG_SERIAL 1
const int DEBUG_PQ = 0;
const int DEBUG_MQB = 0;
const int DEBUG_ACC = 3;

#include <Arduino.h>
#include <EEPROM.h>
#define EEPROM_ACC_ON_ADDRESS 0
#if DEBUG_SERIAL
#include <SoftwareSerial.h>
SoftwareSerial debug_serial(/*rx*/3, /*tx*/2);
#endif
#if DEBUG_SERIAL
#define DebugLog debug_serial.print
#else
#define DebugLog(...)
#endif
#if LIN_PQ
#include "lin-bus-pq.h"
lib_bus_pq linPq;
#endif
#if LIN_MQB
#include "lin-bus-mqb.h"
#if defined(__AVR_ATmega328P__)
lib_bus_mqb linMqb(8, 9, 7);
#elif defined(__AVR_ATmega32U4__)
lib_bus_mqb linMqb(4, 9, 5);
#endif
#endif
#include "acc-handle.h"

struct {
    uint8_t pq_id;
    uint8_t mqb_id;
} buttons[] = {
// MQB right side:
    {0x00, 0x00},
    {0x03, 0x16}, // prev
    {0x02, 0x15}, // next
    {0x1A, 0x19}, // phone <- voice/mic
    {0x1A, 0x1C}, // phone
    {0x29, 0x23}, // return <- view (on wheels with "view" button)
    {0x22, 0x04}, // up
    {0x23, 0x05}, // down
    {0x09, 0x03}, // src-
    {0x0A, 0x02}, // src+
    {0x28, 0x07}, // ok
    // 'return' on PQ not assigned to MQB: 0x29
// MQB left side:
    {0x06, 0x10}, // vol+
    {0x07, 0x11}, // vol-
    {0x2B, 0x0C}, // voice/mic <- ACC mode (on wheels with "view" button)
};

void setup() {
#if DEBUG_SERIAL
    debug_serial.begin(115200);
    DebugLog("\nSetup...");
#endif
    accHandle.setup();
#if LIN_PQ
    linPq.setup();
#endif
#if LIN_MQB
    linMqb.setup();
#endif
}

void loop() {
    static unsigned long time = millis();
#if LIN_PQ
    int linPqStatus = linPq.loop();
#elif LIN_MQB
    bool light_on = (time / 1000) % 2;
    linMqb.forceLightData(light_on);
#endif
#if LIN_MQB
    int linMqbStatus = linMqb.loop();
#endif
#if LIN_PQ && LIN_MQB
    memcpy(linMqb.light_data, linPq.light_data, 4);
    linPq.pressed_button = linMqb.pressed_button;
    linPq.pressed_gear_shifter = linMqb.pressed_gear_shifter;
    linPq.pressed_horn = linMqb.pressed_horn;
    for (unsigned int i=0; i<sizeof(buttons) / sizeof(buttons[0]); i++) {
        if (buttons[i].mqb_id == linMqb.pressed_button) {
            linPq.pressed_button = buttons[i].pq_id;
            break;
        }
    }

#endif
#if LIN_MQB
    accHandle.loop(linMqb.pressed_acc_button, linMqb.temperature_sensor);
#endif
    time = millis();
}