#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "commands.h"

#define SPI_PORT spi0
#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_DC 15
#define PIN_RST 14
#define PIN_LED 2

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define SCREEN_TOTAL_PIXELS SCREEN_WIDTH * SCREEN_HEIGHT
#define BUFFER_SIZE SCREEN_TOTAL_PIXELS * 2

#define RECT_SIZE 2   // default 30
#define RECT_COUNT 1000  // default 50
#define REPEAT 4
#define USE_INTERLACE


// display buffer of our screen size.
uint8_t buffer[BUFFER_SIZE];

struct Square
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int8_t xVelocity;
    int8_t yVelocity;
    uint16_t color;
};

static void backlight(bool on) {
    gpio_put(PIN_LED, on);
}

static void cs(bool select) {
    gpio_put(PIN_CS, !select);  // note: selected is low!
}

static void dc(bool select) {
    gpio_put(PIN_DC, !select);  // note: selected is low!
}

static void inline send_short(uint16_t data)
{
    uint8_t shortBuffer[] = {data >> 8, data};
    cs(true);
    spi_write_blocking(SPI_PORT, shortBuffer, 2);
    cs(false);
}

static void inline send_command_data(const char command, const size_t len, const char *data) {
    cs(true);
    dc(true);
    spi_write_blocking(SPI_PORT, &command, 1);
    dc(false);
    spi_write_blocking(SPI_PORT, data, len);
    cs(false);
}

static void inline send_command_param(const char command, const char param) {
    cs(true);
    dc(true);
    spi_write_blocking(SPI_PORT, &command, 1);
    dc(false);
    spi_write_blocking(SPI_PORT, &param, 1);
    cs(false);
}

static void inline send_command(uint8_t command)
{
    cs(true);
    dc(true);
    spi_write_blocking(SPI_PORT, &command, 1);
    dc(false);
    cs(false);
}

void init_display() 
{
    // [mee]
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    backlight(false);

    gpio_put(PIN_RST, 0);
    sleep_ms(50);
    gpio_put(PIN_RST, 1);
    sleep_ms(50);

    send_command_data(PWCTRB,   3, (const char[]){0x00, 0xc1, 0x30});
    send_command_data(POSC,     4, (const char[]){0x64, 0x03, 0x12, 0x81});
    send_command_data(DTCA,     3, (const char[]){0x85, 0x00, 0x78});
    send_command_data(PWCTRA,   5, (const char[]){0x39, 0x2c, 0x00, 0x32, 0x02});
    send_command_param(PUMPRC, 0x20);
    send_command_data(DTCB,     2, (const char[]){0x00, 0x00});
    send_command_param(PWCTR1, 0x23);
    send_command_param(PWCTR2, 0x10);
    send_command_data(VMCTR1,   2, (const char[]){0x3e, 0x28});
    send_command_param(VMCTR2, 0x86);
    send_command_param(RDMADCTL, 0x48);
    send_command_param(PIXFMT, 0x55);
    send_command_data(FRMCTR1,  2, (const char[]){0x00, 0x1f});  // 61 Hz
    // send_command_data(FRMCTR1,  2, (const char[]){0x00, 0x0a});  // 119 Hz
    send_command_data(DFUNCTR,  3, (const char[]){0x08, 0x82, 0x27});
    send_command_param(ENABLE3G, 0x00);
    send_command_param(GAMMASET, 0x01);
    send_command_data(GMCTRP1, 15, (const char[]){0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00});
    send_command_data(GMCTRN1, 15, (const char[]){0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f});
    send_command(SLPOUT);
    sleep_ms(120);
    send_command(DISPLAY_ON);
    sleep_ms(120);
    send_command(NORON);
}

void init_SPI()
{
    // set up the SPI interface.
    uint actualBaudrate = spi_init(SPI_PORT, 100000000);  //62,500,000
    // printf("Actual Baudrate: %i\n", actualBaudrate);  //62,500,000

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_init(PIN_DC);
    gpio_init(PIN_RST);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_RST, GPIO_OUT);
}

void init_drawing()
{
    send_command(SET_COLUMN);
    send_short(0);
    send_short(SCREEN_WIDTH - 1);

    send_command(SET_PAGE);
    send_short(0);
    send_short(SCREEN_HEIGHT - 1);

    sleep_ms(10);
}

static inline void write_buffer()
{
    #ifdef USE_INTERLACE
    static uint8_t interlacePosition = 0;

    send_command(SET_COLUMN);
    send_short(0);
    send_short(239);

    for (int i = interlacePosition; i < SCREEN_HEIGHT; i+=2)
    {
        send_command(SET_PAGE);
        send_short(i);
        send_short(i+1);

        send_command_data(WRITE_RAM, SCREEN_WIDTH * 2, &buffer[i * SCREEN_WIDTH * 2]);
    }
    interlacePosition ^= 1;
    #else
    send_command_data(WRITE_RAM, BUFFER_SIZE, buffer);
    #endif
}

inline void clear_buffer()
{
    for (uint i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = 0x00;
    }
}

void inline draw_rectangle(uint16_t x, uint16_t y, uint16_t w,
    uint16_t h, uint16_t color)
{
    for (int i = y * SCREEN_WIDTH * 2;
        i < (SCREEN_WIDTH * y * 2) + (h * SCREEN_WIDTH * 2);
        i+=SCREEN_WIDTH * 2)
    {
        for (int j = x * 2; j < (x * 2) + (w * 2); j+=2)
        {
            buffer[i+j] = color >> 8;
            buffer[i+j+1] = color & 0xff;
        }
    }    
}

void update(struct Square player[], uint playerCount)
{
    for (int i = 0; i < playerCount; i++)
        {
            player[i].x += player[i].xVelocity;
            player[i].y += player[i].yVelocity;
            
            if (player[i].x <= 0)
            {
                player[i].x = 0;
                player[i].xVelocity = -player[i].xVelocity;
            }

            if (player[i].x >= SCREEN_WIDTH - player[i].w)
            {
                player[i].x = SCREEN_WIDTH - player[i].w;
                player[i].xVelocity = -player[i].xVelocity;
            }

            if (player[i].y <= 0)
            {
                player[i].y = 0;
                player[i].yVelocity = -player[i].yVelocity;
            }

            if (player[i].y >= SCREEN_HEIGHT - player[i].h)
            {
                player[i].y = SCREEN_HEIGHT - player[i].h;
                player[i].yVelocity = -player[i].yVelocity;
            }
            // printf("Player[%i]: (%i, %i) (%i, %i)\n",
            //    i, player[i].x, player[i].y,
            //    player[i].xVelocity, player[i].yVelocity);
        }
}

int main()
{
    stdio_init_all();

    printf("Starting up.\n");

    init_SPI();

    printf("SPI initialized.\n");

    init_display();

    printf("Display initialized.\n");

    init_drawing();

    printf("Drawing initialized.\n");

    // uint playerCount = RECT_COUNT;

    struct Square player[RECT_COUNT];

    for (int i = 0; i < RECT_COUNT; i++)
    {
        player[i].x = rand() % (SCREEN_WIDTH - 1) - RECT_SIZE;
        player[i].y = rand() % (SCREEN_HEIGHT - 1) - RECT_SIZE;
        player[i].w = RECT_SIZE;
        player[i].h = RECT_SIZE;
        player[i].xVelocity = rand() % 4 - 2;
        player[i].yVelocity = rand() % 4 - 2;

        if (player[i].xVelocity == 0 && player[i].yVelocity == 0)
        {
            player[i].xVelocity = 3;
            player[i].yVelocity = 3;
        }
        uint16_t red = rand() & 0xf800;
        uint16_t green = rand() & 0xfc00;
        uint16_t blue = rand() & 0xf800;

        player[i].color = red | (green >> 5) | (blue >> 11);
    } 

    backlight(true); 
    int visible = 0;
    int delta = 1;
    int repeat = REPEAT;

    
    while(1)
    {        
        update(player, RECT_COUNT);

        clear_buffer();

        for (int i = 0; i < visible; i++)
        {
            draw_rectangle(player[i].x, player[i].y,
                player[i].w, player[i].h, player[i].color);
        }
        repeat--;
        if (!repeat) {
            visible += delta;
            if (visible < 1) {
                delta = 1;
            }
            if (visible > RECT_COUNT - 1) {
                delta = -1;
            }
            repeat = REPEAT;
        }

        // uint32_t beforeWriteTime = time_us_32();
        write_buffer();
        // uint32_t afterWriteTime = time_us_32();
        // printf("\n%ldus\n", afterWriteTime-beforeWriteTime);
    }
    return 0;
}
