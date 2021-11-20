#include <Arduino.h>
#include <AltSoftSerial.h>
#include "lin-bus.h"

#define serial_mqb_clear() serial_mqb.flushInput()

class lib_bus_mqb
{
    public:
    static const uint8_t BUTTONS_ID = 0x0E;
    static const uint8_t ACC_BUTTONS_ID = 0x0F;
    static const uint8_t TEMPERATURE_ID = 0x3A;
    static const uint8_t LIGHT_ID = 0x0D;

    lib_bus_mqb(uint8_t rx, uint8_t tx, uint8_t cs)
    : rx_pin(rx), tx_pin(tx), cs_pin(cs), serial_mqb(rx, tx) {
        light_data[0] = 0x00; light_data[1] = 0xF9; light_data[2] = 0xFF; light_data[3] = 0xFF;
    }
    void setup() {
        pinMode(cs_pin, OUTPUT);
        digitalWrite(cs_pin, HIGH);
        serial_mqb_clear();
        state = IDLE;
        command = COMMANDS_COUNT - 1;
        DebugLog("\nMQB init");
    }

    int loop() {
        int result = 0;
        
        switch (state) {
            case IDLE:
                //if (DEBUG_MQB == 2) {DebugLog("\nIDLE");}
                if (millis() - loop_timer < 1) // too frequent loop breaks LIN communication
                    return result;
                request_id = 0;
                response_timer = 0;
                memset(response, 0, sizeof(response));
                response_data_index = 0;
                command = (command + 1) % COMMANDS_COUNT;
                if ((command == REQUEST_BUTTONS) || (command == REQUEST_ACC_BUTTONS) || (command == REQUEST_TEMPERATURE))
                    state = WRITE_REQUEST;
                else if (command == SEND_LIGHT_DATA)
                    state = WRITE_DATA;
                result = 0;
            break;
            case WRITE_REQUEST:
                if (command == REQUEST_BUTTONS) {
                    request_id = BUTTONS_ID;
                    response_data_length = 8;
                } else if (command == REQUEST_ACC_BUTTONS) {
                    request_id = ACC_BUTTONS_ID;
                    response_data_length = 8;
                } else {
                    request_id = TEMPERATURE_ID;
                    response_data_length = 2;
                }
                response_data_index = 0;
                if (DEBUG_MQB == 2) {DebugLog("\nMQB request "); DebugLog(request_id, HEX);}
                write_request(request_id);
                response_timer = millis();
                state = READ_DATA;
                result = 0;
            break;
            case READ_DATA:
                if (serial_mqb.available()) {
                    uint8_t c = serial_mqb.read();
                    response[response_data_index++] = c;
                    if (response_data_index < response_data_length){
                        result = 1;
                    }
                    else if (response_data_index == response_data_length) {
                        if (DEBUG_MQB == 2) {
                            DebugLog("\nMQB data:"); 
                            for (int i=0; i<response_data_index; i++) { DebugLog(" "); DebugLog(response[i], HEX); }
                        }
                        state = READ_CHECKSUM;
                    }
                    else if (response_data_index > response_data_length) {
                        state = IDLE;
                        result = -1; 
                    }
                    response_timer = millis();
                }
                else if (millis() - response_timer > 10) { // response timeout
                    if (DEBUG_MQB > 0) {
                        DebugLog("\nMQB data timeout: "); DebugLog(request_id, HEX);
                        for (int i=0; i<response_data_index; i++) { DebugLog(" "); DebugLog(response[i], HEX); }
                    }
                    state = IDLE;
                    result = -1; 
                }
            break;
            case READ_CHECKSUM:
                if (serial_mqb.available()) {
                    uint8_t c = serial_mqb.read();
                    uint8_t checksum = dataChecksum(request_id, response, response_data_index);
                    //if (DEBUG_MQB == 2) {DebugLog("\nMQB checksum: "); DebugLog(c, HEX); DebugLog(" vs. "); DebugLog(checksum, HEX);}
                    if (checksum == c) {
                        if (command == REQUEST_BUTTONS) {
                            if (DEBUG_MQB == 1) {
                                if (pressed_button != response[1]) {
                                    DebugLog("\nMQB button: "); DebugLog(response[1], HEX);
                                }
                            }
                            pressed_button = response[1];
                            pressed_gear_shifter = (response[6] & 0x0F); // gear pressed
                            pressed_horn = response[7];
                            if (DEBUG_MQB == 2) {DebugLog("\nMQB button: "); DebugLog(pressed_button, HEX);}
                            state = IDLE;
                            result = 1;
                        }
                        else if (command == REQUEST_ACC_BUTTONS) {
                            if (DEBUG_MQB == 1) {
                                if (pressed_acc_button != getPressedAccButton()) {
                                    DebugLog("\nACC button: ["); DebugLog(response[1], HEX); DebugLog(", "); DebugLog(response[2], HEX); 
                                    DebugLog("] => "); DebugLog(getPressedAccButton());
                                }
                            }
                            pressed_acc_button = getPressedAccButton();
                            if (DEBUG_MQB == 2 || DEBUG_ACC == 2) {
                                DebugLog("\nACC response: ");
                                DebugLog(request_id, HEX);
                                for (int i = 0; i < response_data_index; i++)
                                {
                                    DebugLog(" ");
                                    DebugLog(response[i], HEX);
                                }
                                DebugLog("    ACC button: "); DebugLog(pressed_acc_button, HEX); 
                            }
                            state = IDLE;
                            result = 1;
                        }
                        else if (command == REQUEST_TEMPERATURE) {
                            temperature_sensor = response[0];
                            if (DEBUG_MQB == 2) {DebugLog("\nMQB temperature: "); DebugLog(temperature_sensor, HEX);}
                            state = IDLE;
                            result = 1;
                        }
                        else {
                            state = IDLE;
                            result = -1;
                        }
                    }
                    else {
                        if (DEBUG_MQB > 0) {
                            DebugLog("\nMQB checksum mismatch: "); DebugLog(request_id, HEX); DebugLog(":");
                            for (int i=0; i<response_data_index; i++) { DebugLog(" "); DebugLog(response[i], HEX); }
                            DebugLog(" => "); DebugLog(c, HEX); DebugLog(" vs. "); DebugLog(checksum, HEX);
                        }
                        result = -1;
                        state = IDLE;
                    }
                }
                else if (millis() - response_timer > 10) { // response timeout
                    if (DEBUG_MQB > 0) {
                        DebugLog("\nMQB checksum timeout: "); DebugLog(request_id, HEX);
                        //for (int i=0; i<response_data_index; i++) { DebugLog(" "); DebugLog(response[i], HEX); }
                    }
                    state = IDLE;
                    result = -1; 
                }
                break;
            case WRITE_DATA:
                if (command == SEND_LIGHT_DATA) {
                    if (DEBUG_MQB == 2) {
                        DebugLog("\nMQB light data:");
                        for (int i=0; i<4; i++) { DebugLog(" "); DebugLog(light_data[i], HEX); }
                    }
                    write_data(LIGHT_ID, light_data, 4);
                    state = IDLE;
                    result = 1;
                }
                else {
                    state = IDLE;
                    result = -1;
                }
            break;
        }
        loop_timer = millis();
        return result;
    }

    uint8_t pressed_button = 0;
    uint8_t pressed_gear_shifter = 0;
    uint8_t pressed_horn = 0;
    uint8_t temperature_sensor = 0;
    enum {
        ACC_NONE = 0,
        ACC_SET,
        ACC_RESUME,
        ACC_OFF,
        ACC_MODE,
        ACC_MINUS,
        ACC_PLUS,
        ACC_DIST,
    } pressed_acc_button = 0;
    uint8_t light_data[4];
    void forceLightData(bool light_on) {
        light_data[0] = light_on ? 0x1A : 0x00;
    }

    private:
    enum {
        IDLE = 0,
        WRITE_REQUEST,
        READ_DATA,
        READ_CHECKSUM,
        WRITE_DATA,
    } state;
    enum {
        REQUEST_BUTTONS = 0,
        REQUEST_ACC_BUTTONS,
        SEND_LIGHT_DATA,
        REQUEST_TEMPERATURE,
        COMMANDS_COUNT
    } command;

    int write_data(uint8_t ident, uint8_t data[], uint8_t data_size) {
        uint8_t addrbyte = (ident & 0x3f) | addrParity(ident);
        uint8_t cksum = dataChecksum(ident, data, data_size);
        digitalWrite(tx_pin, LOW); delayMicroseconds(750);
        digitalWrite(tx_pin, HIGH); delayMicroseconds(100); // Write break
        serial_mqb.begin(BAUD_19200);     // Configure baudrate
        serial_mqb.write(0x55);     // write Synch Byte to serial
        serial_mqb.write(addrbyte); // write Identification Byte to serial
        for (uint8_t i = 0; i < data_size; i++)
            serial_mqb.write(data[i]); // write data to serial
        serial_mqb.write(cksum);
        serial_mqb.flush(); // Wait untill all data has transmitted
        serial_mqb.end();
        return 1;
    }
    int write_request(uint8_t ident) {
        uint8_t addrbyte = (ident & 0x3f) | addrParity(ident);
        digitalWrite(tx_pin, LOW); delayMicroseconds(750); // Write break
        digitalWrite(tx_pin, HIGH); delayMicroseconds(100);
        serial_mqb.begin(BAUD_19200);     // Configure baudrate
        serial_mqb.write(0x55);     // write Synch Byte to serial
        serial_mqb.write(addrbyte); // write Identification Byte to serial
        serial_mqb.flush(); // Wait untill all data has transmitted
        serial_mqb_clear();
        response_timer = millis();
    	return 1;
    }

    uint8_t getPressedAccButton() {
        if (response[2] & 0x01)   return ACC_SET;
        if (response[2] & 0x08)   return ACC_RESUME;
        if (response[1] & 0x10)   return ACC_OFF;
        if (response[2] & 0x04)   return ACC_MINUS;
        if (response[2] & 0x02)   return ACC_PLUS;
        if (response[1] & 0x80)   return ACC_MODE; // always pressed (on wheels with "view" button)
        if (response[2] & 0x20)   return ACC_DIST;
        if (response[2] & 0x40)   return ACC_DIST; // (on wheels with "view" button)
        return ACC_NONE;
    }

    uint8_t rx_pin;
    uint8_t tx_pin;
    uint8_t cs_pin;
    AltSoftSerial serial_mqb; // only pins 8 & 9 work
    uint8_t request_id = 0;
    unsigned long loop_timer = 0;
    unsigned long response_timer = 0;
    uint8_t response[8];
    uint8_t response_data_index = 0;
    uint8_t response_data_length = 8;
};
