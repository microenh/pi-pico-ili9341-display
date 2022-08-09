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

/*
const uint8_t NOP = 0x00;
const uint8_t SWRESET = 0x01;
const uint8_t RDDID = 0x04;
const uint8_t RDDST = 0x09;
const uint8_t SLPIN = 0x10;
const uint8_t SLPOUT = 0x11;
const uint8_t PTLON = 0x12;
const uint8_t NORON = 0x13;
const uint8_t RDMODE = 0x0a;
const uint8_t RDMADCTL = 0x0b;
const uint8_t RDPIXFMT = 0x0c;
const uint8_t RDIMGFMT = 0x0d;
const uint8_t RDSELFDIAG = 0x0f;
const uint8_t INVOFF = 0x20;
const uint8_t INVON = 0x21;
const uint8_t GAMMASET = 0x26;
const uint8_t DISPLAY_OFF = 0x28;
const uint8_t DISPLAY_ON = 0x29;
const uint8_t SET_COLUMN = 0x2a;
const uint8_t SET_PAGE = 0x2b;
const uint8_t WRITE_RAM = 0x2c;
const uint8_t READ_RAM = 0x2e;
const uint8_t PTLAR = 0x30;
const uint8_t VSCRDEF = 0x33;
const uint8_t PIXMFT = 0x3a;
const uint8_t WRITE_DISPLAY_BRIGHTNESS = 0x51;
const uint8_t READ_DISPLAY_BRIGHTNESS = 0x52;
const uint8_t WRITE_CTRL_DISPLAY = 0x53;
const uint8_t READ_CTRL_DISPLAY = 0x54;
const uint8_t WRITE_CABC = 0x55;
const uint8_t READ_CABC = 0x56;
const uint8_t WRITE_CABC_MINIMUM = 0x5e;
const uint8_t READ_CABC_MINIMUM = 0x5f;
const uint8_t FRMCTR1 = 0x81;
const uint8_t FRMCTR2 = 0x82;
const uint8_t FRMCTR3 = 0x83;
const uint8_t INVCTR = 0x84;
const uint8_t DFUNCTR = 0xb6;
const uint8_t PWCTR1 = 0xc0;
const uint8_t PWCTR2 = 0xc1;
const uint8_t PWCTRA = 0xcb;
const uint8_t PWCTRB = 0xcf;
const uint8_t VMCTR1 = 0xc5;
const uint8_t VMCTR2 = 0xc7;
const uint8_t RDID1 = 0xda;
const uint8_t RDID2 = 0xdb;
const uint8_t RDID3 = 0xdc;
const uint8_t RDID4 = 0xdd;
const uint8_t GMCTRP1 = 0xe0;
const uint8_t GMCTRN1 = 0xe1;
const uint8_t DTCA = 0xe8;
const uint8_t DTCB = 0xea;
const uint8_t POSC = 0xed;
const uint8_t ENABLE3G = 0xf2;
const uint8_t PUMPRC = 0xf7; 
*/

// display buffer of our screen size.
uint8_t buffer[BUFFER_SIZE];
uint8_t interlacePosition = 0;

uint actualBaudrate = 0;

struct Square
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int8_t xVelocity;
    int8_t yVelocity;
    uint8_t color;
};

static inline void cs_select() {
    gpio_put(PIN_CS, 0);
}

static inline void cs_deselect() {
    gpio_put(PIN_CS, 1);
}

static inline void dc_select() {
    gpio_put(PIN_DC, 0);
}

static inline void dc_deselect() {
    gpio_put(PIN_DC, 1);
}

static void inline send_short(uint16_t data)
{
    cs_select();
    dc_deselect();

    uint8_t shortBuffer[2];

    shortBuffer[0] = (uint8_t) (data >> 8);
    shortBuffer[1] = (uint8_t) data;

    spi_write_blocking(SPI_PORT, shortBuffer, 2);

    cs_deselect();
}

static void inline send_command_data(const char command, const size_t len, const char *data) {
    cs_select();
    dc_select();
    spi_write_blocking(SPI_PORT, &command, 1);
    dc_deselect();
    if (len) {
        spi_write_blocking(SPI_PORT, data, len);
    }
    cs_deselect();
}

static void inline send_command(uint8_t command)
{
    send_command_data(command, 0, (const char[]){});
}

static void backlight(bool on) {
    gpio_put(PIN_LED, on);
}

void init_display() 
{
    // [mee]
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 0);

    cs_select();

    gpio_put(PIN_RST, 0);
    sleep_ms(50);
    gpio_put(PIN_RST, 1);
    sleep_ms(50);

    send_command_data(PWCTRB,   3, (const char[]){0x00, 0xc1, 0x30});
    send_command_data(POSC,     4, (const char[]){0x64, 0x03, 0x12, 0x81});
    send_command_data(DTCA,     3, (const char[]){0x85, 0x00, 0x78});
    send_command_data(PWCTRA,   5, (const char[]){0x39, 0x2c, 0x00, 0x32, 0x02});
    send_command_data(PUMPRC,   1, (const char[]){0x20});
    send_command_data(DTCB,     2, (const char[]){0x00, 0x00});
    send_command_data(PWCTR1,   1, (const char[]){0x23});
    send_command_data(PWCTR2,   1, (const char[]){0x10});
    send_command_data(VMCTR1,   2, (const char[]){0x3e, 0x28});
    send_command_data(VMCTR2,   1, (const char[]){0x86});
    send_command_data(RDMADCTL, 1, (const char[]){0x48});
    send_command_data(PIXFMT,   1, (const char []){0x55});
    send_command_data(FRMCTR1,  2, (const char[]){0x00, 0x1f});  // 61 Hz
    // send_command_data1(FRMCTR1,  2, (const char[]){0x00, 0x0a});  // 119 Hz
    send_command_data(DFUNCTR,  3, (const char[]){0x08, 0x82, 0x27});
    send_command_data(ENABLE3G, 1, (const char[]){0x00});
    send_command_data(GAMMASET, 1, (const char[]){0x01});
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
    spi_init(SPI_PORT, 62500000);
    // actualBaudrate = spi_set_baudrate(SPI_PORT, 70000000);

    // printf("Actual Baudrate: %i\n", actualBaudrate);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_init(PIN_DC);
    gpio_init(PIN_RST);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_RST, 0);
}

void init_drawing()
{
    send_command(SET_COLUMN);
    send_short(0);
    send_short(239);

    send_command(SET_PAGE);
    send_short(0);
    send_short(319);

    sleep_ms(10);

    send_command(WRITE_RAM);

    cs_select();
    dc_deselect();
}

inline void write_buffer()
{
    spi_write_blocking(SPI_PORT, buffer, BUFFER_SIZE);
}

void write_buffer_interlaced()
{
    send_command(SET_COLUMN);
    send_short(0);
    send_short(239);

    for (int i = interlacePosition; i < SCREEN_HEIGHT; i+=2)
    {
        send_command(SET_PAGE);
        send_short(i);
        send_short(i+1);

        send_command(WRITE_RAM);

        cs_select();
        dc_deselect();

        spi_write_blocking(SPI_PORT, &buffer[i * SCREEN_WIDTH * 2],
            SCREEN_WIDTH * 2);
    }

    if (interlacePosition == 1) 
    {
        interlacePosition = 0;
    } 
    else 
    {
        interlacePosition = 1;
    }
}

inline void clear_buffer()
{
    for (uint i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = 0x00;
    }
}

void inline draw_rectangle(uint16_t x, uint16_t y, uint16_t w,
    uint16_t h, uint8_t color)
{
    for (int i = y * SCREEN_WIDTH * 2;
        i < (SCREEN_WIDTH * y * 2) + (h * SCREEN_WIDTH * 2);
        i+=SCREEN_WIDTH * 2)
    {
        for (int j = x * 2; j < (x * 2) + (w * 2); j+=2)
        {
            buffer[i+j] = color;
            buffer[i+j+1] = color;
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

    uint playerCount = 10;

    struct Square player[playerCount];

    for (int i = 0; i < playerCount; i++)
    {
        player[i].x = rand() % 209;
        player[i].y = rand() % 289;
        player[i].w = 30;
        player[i].h = 30;
        player[i].xVelocity = rand() % 4 - 2;
        player[i].yVelocity = rand() % 4 - 2;

        if (player[i].xVelocity == 0 && player[i].yVelocity == 0)
        {
            player[i].xVelocity = 3;
            player[i].yVelocity = 3;
        }

        player[i].color = rand() % 255;
    } 

    backlight(true);  

    while(1)
    {        
        update(player, playerCount);

        clear_buffer();

        for (int i = 0; i < playerCount; i++)
        {
            draw_rectangle(player[i].x, player[i].y,
                player[i].w, player[i].h, player[i].color);
        }

        uint32_t beforeWriteTime = time_us_32();
        // write_buffer();
        write_buffer_interlaced();
        write_buffer_interlaced();
        uint32_t afterWriteTime = time_us_32();
        printf("\n%ldus\n", afterWriteTime-beforeWriteTime);
        //printf("Actual Buadrate: %i\n", actualBaudrate);
    }


    return 0;
}
