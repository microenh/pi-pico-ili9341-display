#include "display.h"
#include "pico/stdlib.h"
#include "commands.h"
#include "defs.h"
#include <string.h>

display::display(
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
    ) {
    spi = p_spi;
    pin_miso = p_pin_miso;
    pin_mosi = p_pin_mosi;
    pin_cs = p_pin_cs;
    pin_sck = p_pin_sck;
    pin_dc = p_pin_dc;
    pin_rst = p_pin_rst;
    pin_led = p_pin_led;
    screen_height = p_screen_height;
    screen_width = p_screen_width;
    rotation = p_rotation;

    buffer_size = screen_height * screen_width * 2;
    buffer = new uint8_t[buffer_size];

    initSPI();
    initDisplay();
    initDrawing();
}

display::~display() {
    backlight(false);
    delete[] buffer;
}

void display::backlight(bool on) {
    gpio_put(pin_led, on);
}

void display::cs(bool select) {
    gpio_put(pin_cs, !select);
}

void display::dc(bool select) {
    gpio_put(pin_dc, !select);
}

void display::sendCommand(uint8_t command)
{
    cs(true);
    dc(true);
    spi_write_blocking(spi, &command, 1);
    dc(false);
    cs(false);
}

void display::sendCommand(const uint8_t command, const uint8_t param)
{
    cs(true);
    dc(true);
    spi_write_blocking(spi, &command, 1);
    dc(false);
    spi_write_blocking(spi, &param, 1);
    cs(false);
}

void display::sendCommand(const uint8_t command, const size_t len, const uint8_t *data)
{
    cs(true);
    dc(true);
    spi_write_blocking(spi, &command, 1);
    dc(false);
    spi_write_blocking(spi, data, len);
    cs(false);
}

void display::sendShort(uint16_t data)
{
    uint8_t shortBuffer[] = {(uint8_t)(data >> 8), (uint8_t)data};
    cs(true);
    spi_write_blocking(spi, shortBuffer, 2);
    cs(false);
}

void display::initDisplay()
{
    // [mee]
    gpio_init(pin_led);
    gpio_set_dir(pin_led, GPIO_OUT);
    backlight(false);

    gpio_put(pin_rst, 0);
    sleep_ms(50);
    gpio_put(pin_rst, 1);
    sleep_ms(50);

    sendCommand(SWRESET);
    sendCommand(PWCTRLB, 3, (const uint8_t[]){0x00, 0xc1, 0x30});
    sendCommand(POSCTRL, 4, (const uint8_t[]){0x64, 0x03, 0x12, 0x81});
    sendCommand(DTIMCTRLA, 3, (const uint8_t[]){0x85, 0x00, 0x78});
    sendCommand(PWCTRLA, 5, (const uint8_t[]){0x39, 0x2c, 0x00, 0x32, 0x02});
    sendCommand(PUMPRATIOCTRL, 0x20);
    sendCommand(DTIMCTRLB, 2, (const uint8_t[]){0x00, 0x00});
    sendCommand(PWCTRL1, 0x23);
    sendCommand(PWCTRL2, 0x10);
    sendCommand(VMCTRL1, 2, (const uint8_t[]){0x3e, 0x28});
    sendCommand(VMCTRL2, 0x86);
    sendCommand(MADCTL, rotation); // 0x48 (R180)
    sendCommand(PIXSET, 0x55);     // 16-bits / pixel
    // send_command(FRMCTR1, 2, (const uint8_t[]){0x00, 0x1f});  // 61 Hz
    // send_command(FRMCTR1, 2, (const uint8_t[]){0x00, 0x10});  // 119 Hz (per docs)
    // send_command(FRMCTR1, 2, (const uint8_t[]){0x00, 0x0a});  // 119 Hz
    sendCommand(DISCTRL, 3, (const uint8_t[]){0x08, 0x82, 0x27});
    sendCommand(ENABLE_3G, 0x00);
    sendCommand(GAMSET, 0x01);
    sendCommand(PGAMCTRL, 15, (const uint8_t[]){0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00});
    sendCommand(NGAMCTRL, 15, (const uint8_t[]){0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f});
    sendCommand(SLPOUT);
    sleep_ms(120);
    sendCommand(DISPON);
    // sleep_ms(120);
    sendCommand(NORON);
}

void display::initSPI()
{
    // set up the SPI interface.
    uint actualBaudrate = spi_init(spi, 100000000); // 62,500,000
    // printf("Actual Baudrate: %i\n", actualBaudrate);  //62,500,000

    gpio_set_function(pin_miso, GPIO_FUNC_SPI);
    gpio_set_function(pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(pin_mosi, GPIO_FUNC_SPI);

    gpio_init(pin_cs);
    gpio_init(pin_dc);
    gpio_init(pin_rst);

    gpio_set_dir(pin_cs, GPIO_OUT);
    gpio_set_dir(pin_dc, GPIO_OUT);
    gpio_set_dir(pin_rst, GPIO_OUT);
}

void display::initDrawing()
{
    sendCommand(CASET);
    sendShort(0);
    sendShort(screen_width - 1);

    sendCommand(PASET);
    sendShort(0);
    sendShort(screen_height - 1);

    sleep_ms(10);
}

void display::writeBuffer()
{
#ifdef USE_INTERLACE
    send_command(CASET);
    send_short(0);
    send_short(screen_width - 1);

    for (int i = interlacePosition; i < screen_height; i += 2)
    {
        send_command(PASET);
        send_short(i);
        send_short(i + 1);

        send_command(RAMWR, screen_width * 2, &buffer[i * screen_width * 2]);
    }
    interlacePosition ^= 1;
#else
    sendCommand(RAMWR, buffer_size, buffer);
#endif
}

void display::clearBuffer()
{
    memset(buffer, 0, buffer_size);
}

void display::drawRectangleToBuffer(uint16_t x, uint16_t y, uint16_t w,
                           uint16_t h, uint16_t color)
{
    for (int i = y * screen_width * 2;
         i < (screen_width * y * 2) + (h * screen_width * 2);
         i += screen_width * 2)
    {
        for (int j = x * 2; j < (x * 2) + (w * 2); j += 2)
        {
            buffer[i + j] = color >> 8;
            buffer[i + j + 1] = color & 0xff;
        }
    }
}

void display::clearScreen(uint16_t color = 0) {
    uint8_t shortBuffer[] = {(uint8_t)(color >> 8), (uint8_t)color};

    cs(true);
    dc(true);
    spi_write_blocking(spi, &RAMWR, 1);
    dc(false);
    for (int i=0; i<buffer_size/2; i++) {
        spi_write_blocking(spi, shortBuffer, 2);
    }
    cs(false);
    
}