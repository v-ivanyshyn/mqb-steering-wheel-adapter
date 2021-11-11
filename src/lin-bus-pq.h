#include <Arduino.h>
#include "lin-bus.h"

#if defined(__AVR_ATmega328P__)
#define serial_pq Serial
#elif defined(__AVR_ATmega32U4__)
#define serial_pq Serial1
#endif

class lib_bus_pq
{
    public:
    static const uint8_t BUTTONS_ID = 0x0E;
    static const uint8_t LIGHT_ID = 0x0D;

    lib_bus_pq() {
        buttons_response[0] = 0xFF; buttons_response[1] = 0x00; buttons_response[2] = 0xFF; buttons_response[3] = 0xF0; buttons_response[4] = 0x60;buttons_response[5] = 0x00; buttons_response[6] = 0x30; buttons_response[7] = 0x00;
        light_data[0] = 0x00; light_data[1] = 0xF9; light_data[2] = 0xFF; light_data[3] = 0xFF;
        request_time = millis();
    }
    void setup() {
        serial_pq.begin(BAUD_19200);
        DebugLog("\nPQ init");
    }
    int loop() {
        int result = 0;
        if (millis() - request_time > 100) {
            request_time = millis();
            if (DEBUG_PQ > 0) {DebugLog("\nPQ request timeout");}
        }
        while(serial_pq.available()) {
            request_time = millis();
            uint8_t c = serial_pq.read();
            //if (DEBUG_PQ == 2) {DebugLog("\nPQ in: "); DebugLog(c, HEX);}
            switch (state) {
                case IDLE:
                    if (c == 0x55) {
                        //if (DEBUG_PQ == 2) {DebugLog("\nPQ IDLE 55");}
                        request[0] = c;
                        request_data_index = 0;
                        state = READ_REQUEST;
                        result = 1;
                    }
                break;
                case READ_REQUEST:
                    request_id = c & 0b00111111;
                    if (DEBUG_PQ == 2) {DebugLog("\nPQ request "); DebugLog(request_id, HEX);}
                    if (request_id == BUTTONS_ID) {
                        if (serial_pq.available()) {
                            if (DEBUG_PQ > 0) {DebugLog("\nPQ error: income data after request");}
                            state = IDLE;
                            result = -1;
                        }
                        else {
                            state = WRITE_RESPONSE;
                            buttons_response[0] = 0xF0 | ((buttons_response[0]+1) % 0x0F);
                            if ((DEBUG_PQ == 1) && (buttons_response[1] != pressed_button)) {
                                DebugLog("\nPQ button "); DebugLog(buttons_response[1], HEX);
                            }
                            buttons_response[1] = pressed_button;
                            buttons_response[6] = pressed_gear_shifter | 0x30;
                            buttons_response[7] = pressed_horn;
                            if (DEBUG_PQ == 2) {
                                DebugLog("\nPQ response "); DebugLog(request_id, HEX); DebugLog(":");
                                for (int i=0; i<8; i++) { DebugLog(" "); DebugLog(buttons_response[i], HEX); }
                            }
                            write_response(BUTTONS_ID, buttons_response, 8);
                            state = IDLE;
                            result = 1;
                        }
                    }
                    else if (request_id == LIGHT_ID) {
                        state = READ_DATA;
                        result = 1;
                    }
                    else {
                        while(serial_pq.available())
                            serial_pq.read();
                        state = IDLE;
                        result = -1;
                    }
                break;
                case READ_DATA:
                    request[request_data_index++] = c;
                    if (request_id == LIGHT_ID && (request_data_index == 4)) {
                        state = READ_CHECKSUM;
                    }
                    result = 1;
                break;
                case READ_CHECKSUM:
                    uint8_t checksum = dataChecksum(request_id, request, request_data_index);
                    if (checksum == c) {
                        if (DEBUG_PQ == 2) {
                            DebugLog("\nPQ data "); DebugLog(request_id, HEX); DebugLog(":"); 
                            for (int i=0; i<request_data_index; i++) { DebugLog(" "); DebugLog(request[i], HEX); }
                        }
                        if (request_id == LIGHT_ID) {
                            memcpy(light_data, request, 4);
                        }
                        state = IDLE;
                        result = 1;
                    }
                    else {
                        if (DEBUG_PQ > 0) {
                            DebugLog("\nPQ checksum mismatch "); DebugLog(request_id, HEX); DebugLog(": ");
                            for (int i=0; i<request_data_index; i++) { DebugLog(" "); DebugLog(request[i], HEX); }
                            DebugLog(" => "); DebugLog(checksum, HEX); DebugLog("vs "); DebugLog(c, HEX);
                        }
                        state = IDLE;
                        result = -1;
                    }
                break;
            }
        }
        return result;
    }

    uint8_t pressed_button = 0;
    uint8_t pressed_horn = 0;
    uint8_t pressed_gear_shifter = 0;
    uint8_t light_data[4];

    private:
    enum {
        IDLE,
        READ_REQUEST,
        READ_DATA,
        READ_CHECKSUM,
        WRITE_RESPONSE,
    } state;

    int write_response(uint8_t ident, uint8_t data[], uint8_t data_size) {
    	uint8_t cksum = dataChecksum(ident, data, data_size);
        for (uint8_t i = 0; i < data_size; i++)
            serial_pq.write(data[i]);
        serial_pq.write(cksum);
        return 1;
    }
    uint8_t request_id = 0;
    uint8_t request[4];
    uint8_t request_data_index = 0;
    uint8_t buttons_response[8];
    unsigned long request_time = 0;
};
#undef serial_pq