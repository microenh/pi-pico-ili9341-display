#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <stdio.h>
#include "hardware/spi.h"

class display {
    private:
        spi_inst_t *spi;
        uint8_t pin_miso;
        uint8_t pin_mosi;
        uint8_t pin_cs;
        uint8_t pin_sck;
        uint8_t pin_dc;
        uint8_t pin_rst;
        uint8_t pin_led;
        uint16_t screen_height;
        uint16_t screen_width;
        uint8_t rotation;
        uint8_t interlacePosition;
        uint8_t *buffer;
        int buffer_size;

        void cs(bool select);
        void dc(bool select);
        void sendCommand(uint8_t command);
        void sendCommand(const uint8_t command, const uint8_t param);
        void sendCommand(const uint8_t command, const size_t len, const uint8_t *data);
        void sendShort(const uint16_t data);
        void initDisplay();
        void initSPI();
        void initDrawing();

    public:
        display(
            spi_inst_t *p_spi,
            const uint8_t p_pin_miso,
            const uint8_t p_pin_mosi,
            const uint8_t p_pin_cs,
            const uint8_t p_pin_sck,
            const uint8_t p_pin_dc,
            const uint8_t p_pin_rst,
            const uint8_t p_pin_led,
            const uint16_t p_screen_height,
            const uint16_t p_screen_width,
            const uint8_t p_rotation
        );
        ~display();
        void backlight(bool on);
        void writeBuffer();
        void clearBuffer();
        void drawRectangleToBuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
        void clearScreen(uint16_t color);



};

#endif