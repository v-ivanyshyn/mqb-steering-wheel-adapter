#define ACC_ON_BTN lib_bus_mqb::ACC_OFF
#define ACC_MODE_BTN lib_bus_mqb::ACC_MODE

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
    const uint8_t WHEEL_HEAT_PIN = 5;
    // Отключение подогрева при достижении рулем 30 град. цельсия
    // [Температура в град. цельсия] = [значения с датчика] - 68.
    const uint8_t WHEEL_TEMPERATUR_OFF = 0x62;
public:
    bool accOn = false;
    bool wheelHeatOn = false;
    unsigned long modePressedTimer;
    unsigned long distPressedTimer;
    unsigned long wheelHeatTimer;
    void setup() {
        pinMode(ACC_ON_PIN, OUTPUT);
        pinMode(ACC_RESUME_PIN, OUTPUT);
        pinMode(ACC_CANCEL_PIN, OUTPUT);
        pinMode(ACC_SPEED_PLUS_PIN, OUTPUT);
        pinMode(ACC_SPEED_MINUS_PIN, OUTPUT);
        pinMode(ACC_DIST_PLUS_PIN, OUTPUT);
        pinMode(ACC_DIST_MINUS_PIN, OUTPUT);
        pinMode(ACC_SET_PIN, OUTPUT);
        pinMode(WHEEL_HEAT_PIN, OUTPUT);
        accOn = EEPROM.read(EEPROM_ACC_ON_ADDRESS);
        distPressedTimer = 0;
        modePressedTimer = 0;
        wheelHeatTimer = 0;
    }

    void loop(uint8_t pressedAccButton, uint8_t temperatureSensor) {
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
        if (pressedAccButton != 0 && pressedAccButton != ACC_ON_BTN && pressedAccButton != ACC_MODE_BTN && !accOn) {
            accOn = true;   // automatically turn ACC on if any button pressed
            EEPROM.write(EEPROM_ACC_ON_ADDRESS, accOn);
            if (DEBUG_ACC == 1) DebugLog(accOn ? "ACC ON" : "ACC OFF");
        }
        // Track MODE button long pressed for switching wheel heater
        if (pressedAccButton == ACC_MODE_BTN) {
            if (modePressedTimer == 0) {
                // Mode button must be pressed for 2 seconds for sweetching wheel heater
                modePressedTimer = millis() + 2000;
            }
            else if ((modePressedTimer > 1) && (modePressedTimer < time)) {
                modePressedTimer = 1;
                wheelHeatOn = !wheelHeatOn;
                if (wheelHeatOn) {
                    // set wheel heater timer for 5 minutes. Automatically turn wheel heater off after that time.
                    wheelHeatTimer = millis() + 300000;
                } else {
                    wheelHeatTimer = 0;
                }
                if (DEBUG_ACC == 3) {
                    DebugLog(wheelHeatOn ? "Wheel heater ON \n" : "Wheel heater OFF \n");
                }
            }
        } else if (pressedAccButton != ACC_MODE_BTN) {
            modePressedTimer = 0;
        }
        // Отключение подогрева руля по температуре
        if (wheelHeatOn && (temperatureSensor >= WHEEL_TEMPERATUR_OFF)) {
            wheelHeatTimer = 0;
            wheelHeatOn = 0;
            if (DEBUG_ACC == 3) {
                DebugLog("Wheel heater OFF by temperature \n");
            }
        }
        // Automatically turn of wheel heater after 5 minutes
        if (wheelHeatOn && (wheelHeatTimer < time)) {
            wheelHeatTimer = 0;
            wheelHeatOn = 0;
            if (DEBUG_ACC == 3) {
                DebugLog("Wheel heater OFF by timer \n");
            }
        }
        uint8_t ACC_ON = accOn ? HIGH : LOW;
        uint8_t ACC_RESUME = (pressedAccButton == lib_bus_mqb::ACC_RESUME) ? HIGH : LOW;
        uint8_t ACC_CANCEL = HIGH;//!accOn || (pressedAccButton == lib_bus_mqb::ACC_DIST) ? LOW : HIGH;
        uint8_t ACC_SPEED_PLUS = (distPressedTimer < time) && (pressedAccButton == lib_bus_mqb::ACC_PLUS) ? HIGH : LOW;
        uint8_t ACC_SPEED_MINUS = (distPressedTimer < time) && (pressedAccButton == lib_bus_mqb::ACC_MINUS) ? HIGH : LOW;
        uint8_t ACC_DIST_PLUS = (distPressedTimer > time) && (pressedAccButton == lib_bus_mqb::ACC_PLUS) ? HIGH : LOW;
        uint8_t ACC_DIST_MINUS = (distPressedTimer > time) && (pressedAccButton == lib_bus_mqb::ACC_MINUS) ? HIGH : LOW;
        uint8_t ACC_SET = (pressedAccButton == lib_bus_mqb::ACC_SET) ? HIGH : LOW;
        uint8_t WHEEL_HEAT = wheelHeatOn ? HIGH : LOW;
        digitalWrite(ACC_ON_PIN, ACC_ON);
        digitalWrite(ACC_RESUME_PIN, ACC_RESUME);
        digitalWrite(ACC_CANCEL_PIN, ACC_CANCEL);
        digitalWrite(ACC_SPEED_PLUS_PIN, ACC_SPEED_PLUS);
        digitalWrite(ACC_SPEED_MINUS_PIN, ACC_SPEED_MINUS);
        digitalWrite(ACC_DIST_PLUS_PIN, ACC_DIST_PLUS);
        digitalWrite(ACC_DIST_MINUS_PIN, ACC_DIST_MINUS);
        digitalWrite(ACC_SET_PIN, ACC_SET);
        digitalWrite(WHEEL_HEAT_PIN, WHEEL_HEAT);
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
