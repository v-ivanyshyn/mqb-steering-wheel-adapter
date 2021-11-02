#define ACC_ON_BTN lib_bus_mqb::ACC_OFF

class AccHandle {
#if defined(__AVR_ATmega328P__)
    const uint8_t ACC_ON_PIN = 10;
    const uint8_t ACC_SPEED_PLUS_PIN = 11;
    const uint8_t ACC_SPEED_MINUS_PIN = 12;
    const uint8_t ACC_RESUME_PIN = A0;
#elif defined(__AVR_ATmega32U4__)
    const uint8_t ACC_ON_PIN = 10;
    const uint8_t ACC_SPEED_PLUS_PIN = 16;
    const uint8_t ACC_SPEED_MINUS_PIN = 14;
    const uint8_t ACC_RESUME_PIN = 15;
#endif
    const uint8_t ACC_CANCEL_PIN = 13;
    const uint8_t ACC_DIST_PLUS_PIN = A1;
    const uint8_t ACC_DIST_MINUS_PIN = A2;
    const uint8_t ACC_SET_PIN = A3;
public:
    bool accOn = false;
    unsigned long distPressedTimer;
    void setup() {
        pinMode(ACC_ON_PIN, OUTPUT);
        pinMode(ACC_RESUME_PIN, OUTPUT);
        pinMode(ACC_CANCEL_PIN, OUTPUT);
        pinMode(ACC_SPEED_PLUS_PIN, OUTPUT);
        pinMode(ACC_SPEED_MINUS_PIN, OUTPUT);
        pinMode(ACC_DIST_PLUS_PIN, OUTPUT);
        pinMode(ACC_DIST_MINUS_PIN, OUTPUT);
        pinMode(ACC_SET_PIN, OUTPUT);
        accOn = EEPROM.read(EEPROM_ACC_ON_ADDRESS);
        distPressedTimer = 0;
    }

    void loop(uint8_t pressedAccButton) {
#if LIN_MQB
        static unsigned long time = millis();
        time = millis();
        static uint8_t prevAccOnPressed = false;
        if ((pressedAccButton == ACC_ON_BTN) && !prevAccOnPressed) {
            prevAccOnPressed = true;
            accOn = !accOn;
            EEPROM.write(EEPROM_ACC_ON_ADDRESS, accOn);
            if (DEBUG_ACC == 1) DebugLog(accOn ? "ON  " : "OFF ");
        }
        else if (pressedAccButton != ACC_ON_BTN) {
            prevAccOnPressed = false;
        }
        if (pressedAccButton == lib_bus_mqb::ACC_DIST) {
            distPressedTimer = millis() + 1000;
        }
        if ((distPressedTimer > time) && ((pressedAccButton == lib_bus_mqb::ACC_PLUS) || (pressedAccButton == lib_bus_mqb::ACC_MINUS))) {
            distPressedTimer = millis() + 1000;
        }
        if (pressedAccButton != 0 && pressedAccButton != ACC_ON_BTN && !accOn) {
            accOn = true;   // automatically turn ACC on if any button pressed
            EEPROM.write(EEPROM_ACC_ON_ADDRESS, accOn);
            if (DEBUG_ACC == 1) DebugLog(accOn ? "ACC ON" : "ACC OFF");
        }
        uint8_t ACC_ON = accOn ? HIGH : LOW;
        uint8_t ACC_RESUME = (pressedAccButton == lib_bus_mqb::ACC_RESUME) ? HIGH : LOW;
        uint8_t ACC_CANCEL = HIGH;//!accOn || (pressedAccButton == lib_bus_mqb::ACC_DIST) ? LOW : HIGH;
        uint8_t ACC_SPEED_PLUS = (distPressedTimer < time) && (pressedAccButton == lib_bus_mqb::ACC_PLUS) ? HIGH : LOW;
        uint8_t ACC_SPEED_MINUS = (distPressedTimer < time) && (pressedAccButton == lib_bus_mqb::ACC_MINUS) ? HIGH : LOW;
        uint8_t ACC_DIST_PLUS = (distPressedTimer > time) && (pressedAccButton == lib_bus_mqb::ACC_PLUS) ? HIGH : LOW;
        uint8_t ACC_DIST_MINUS = (distPressedTimer > time) && (pressedAccButton == lib_bus_mqb::ACC_MINUS) ? HIGH : LOW;
        uint8_t ACC_SET = (pressedAccButton == lib_bus_mqb::ACC_SET) ? HIGH : LOW;
        digitalWrite(ACC_ON_PIN, ACC_ON);
        digitalWrite(ACC_RESUME_PIN, ACC_RESUME);
        digitalWrite(ACC_CANCEL_PIN, ACC_CANCEL);
        digitalWrite(ACC_SPEED_PLUS_PIN, ACC_SPEED_PLUS);
        digitalWrite(ACC_SPEED_MINUS_PIN, ACC_SPEED_MINUS);
        digitalWrite(ACC_DIST_PLUS_PIN, ACC_DIST_PLUS);
        digitalWrite(ACC_DIST_MINUS_PIN, ACC_DIST_MINUS);
        digitalWrite(ACC_SET_PIN, ACC_SET);
        // if (DEBUG_ACC == 2) {
        //     DebugLog("\nACC handle: ");
        //     DebugLog("BTN: "); DebugLog(pressedAccButton, HEX); DebugLog(" "); 
        //     DebugLog(ACC_ON ? "ON  " : "OFF ");
        //     DebugLog(ACC_RESUME ? "RESUME " : "");
        //     DebugLog(ACC_CANCEL ? "CANCEL " : "");
        //     DebugLog(ACC_SPEED_PLUS ? "SPD+ " : "");
        //     DebugLog(ACC_SPEED_MINUS ? "SPD- " : "");
        //     DebugLog(ACC_DIST_PLUS ? "DIST+ " : "");
        //     DebugLog(ACC_DIST_MINUS ? "DIST- " : "");
        //     DebugLog(ACC_SET ? "SET " : "");
        // }
#endif
    }
} accHandle;
