#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "display.h"
#include "defs.h"

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
    display disp(
        SPI_PORT,
        PIN_MISO,
        PIN_MOSI,
        PIN_CS,
        PIN_SCK,
        PIN_DC,
        PIN_RST,
        PIN_LED,
        SCREEN_HEIGHT,
        SCREEN_WIDTH,
        ROTATION
    );

    stdio_init_all();


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
        uint8_t red = (rand() >> 7) & 0xf8;
        uint8_t green = (rand() >> 7) & 0xfc;
        uint8_t blue = (rand() >> 7) & 0xf8;

        // player[i].color = red | (green >> 5) | (blue >> 11);
        player[i].color = (red << 8) | (green << 3) | (blue >> 3);
    }

    int visible = 0;
    int delta = 1;
    int repeat = REPEAT;

    disp.clear_buffer();
    disp.write_buffer();
#ifdef USE_INTERLACE
    disp.write_buffer();
    disp.write_buffer();
#endif
    disp.backlight(true);
    sleep_ms(1000);

    while (1)
    {
        update(player, RECT_COUNT);

        disp.clear_buffer();

        for (int i = 0; i < visible; i++)
        {
            disp.draw_rectangle(player[i].x, player[i].y,
                           player[i].w, player[i].h, player[i].color);
        }
        repeat--;
        if (!repeat)
        {
            visible += delta;
            if (visible < 1)
            {
                delta = 1;
            }
            if (visible > RECT_COUNT - 1)
            {
                delta = -1;
            }
            repeat = REPEAT;
        }

        // uint32_t beforeWriteTime = time_us_32();
        disp.write_buffer();
        // uint32_t afterWriteTime = time_us_32();
        // printf("\n%ldus\n", afterWriteTime-beforeWriteTime);
    }
    return 0;
}
