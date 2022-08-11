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

    init_SPI();
    init_display();
    init_drawing();
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

void display::send_command(uint8_t command)
{
    cs(true);
    dc(true);
    spi_write_blocking(spi, &command, 1);
    dc(false);
    cs(false);
}

void display::send_command(const uint8_t command, const uint8_t param)
{
    cs(true);
    dc(true);
    spi_write_blocking(spi, &command, 1);
    dc(false);
    spi_write_blocking(spi, &param, 1);
    cs(false);
}

void display::send_command(const uint8_t command, const size_t len, const uint8_t *data)
{
    cs(true);
    dc(true);
    spi_write_blocking(spi, &command, 1);
    dc(false);
    spi_write_blocking(spi, data, len);
    cs(false);
}

void display::send_short(uint16_t data)
{
    uint8_t shortBuffer[] = {(uint8_t)(data >> 8), (uint8_t)data};
    cs(true);
    spi_write_blocking(spi, shortBuffer, 2);
    cs(false);
}

void display::init_display()
{
    // [mee]
    gpio_init(pin_led);
    gpio_set_dir(pin_led, GPIO_OUT);
    backlight(false);

    gpio_put(pin_rst, 0);
    sleep_ms(50);
    gpio_put(pin_rst, 1);
    sleep_ms(50);

    send_command(SWRESET);
    send_command(PWCTRLB, 3, (const uint8_t[]){0x00, 0xc1, 0x30});
    send_command(POSCTRL, 4, (const uint8_t[]){0x64, 0x03, 0x12, 0x81});
    send_command(DTIMCTRLA, 3, (const uint8_t[]){0x85, 0x00, 0x78});
    send_command(PWCTRLA, 5, (const uint8_t[]){0x39, 0x2c, 0x00, 0x32, 0x02});
    send_command(PUMPRATIOCTRL, 0x20);
    send_command(DTIMCTRLB, 2, (const uint8_t[]){0x00, 0x00});
    send_command(PWCTRL1, 0x23);
    send_command(PWCTRL2, 0x10);
    send_command(VMCTRL1, 2, (const uint8_t[]){0x3e, 0x28});
    send_command(VMCTRL2, 0x86);
    send_command(MADCTL, rotation); // 0x48 (R180)
    send_command(PIXSET, 0x55);     // 16-bits / pixel
    // send_command(FRMCTR1, 2, (const uint8_t[]){0x00, 0x1f});  // 61 Hz
    // send_command(FRMCTR1, 2, (const uint8_t[]){0x00, 0x10});  // 119 Hz (per docs)
    // send_command(FRMCTR1, 2, (const uint8_t[]){0x00, 0x0a});  // 119 Hz
    send_command(DISCTRL, 3, (const uint8_t[]){0x08, 0x82, 0x27});
    send_command(ENABLE_3G, 0x00);
    send_command(GAMSET, 0x01);
    send_command(PGAMCTRL, 15, (const uint8_t[]){0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00});
    send_command(NGAMCTRL, 15, (const uint8_t[]){0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f});
    send_command(SLPOUT);
    sleep_ms(120);
    send_command(DISPON);
    // sleep_ms(120);
    send_command(NORON);
}

void display::init_SPI()
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

void display::init_drawing()
{
    send_command(CASET);
    send_short(0);
    send_short(screen_width - 1);

    send_command(PASET);
    send_short(0);
    send_short(screen_height - 1);

    sleep_ms(10);
}

void display::write_buffer()
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
    send_command(RAMWR, buffer_size, buffer);
#endif
}

void display::clear_buffer()
{
    memset(buffer, 0, buffer_size);
}

void display::draw_rectangle(uint16_t x, uint16_t y, uint16_t w,
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