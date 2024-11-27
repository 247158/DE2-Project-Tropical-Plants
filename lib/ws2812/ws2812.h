#ifndef WS2812_H
#define WS2812_H

#include <avr/io.h>

struct cRGB {
    uint8_t r;  // Красный компонент
    uint8_t g;  // Зеленый компонент
    uint8_t b;  // Синий компонент
};

void ws2812_setleds(struct cRGB *ledarray, uint16_t number_of_leds);

#endif
