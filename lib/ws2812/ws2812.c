#include "ws2812.h"
#include <util/delay.h>
#include <avr/interrupt.h> 

// Выводной пин
#define WS2812_PORT PORTD
#define WS2812_PIN 3

// Определения временных интервалов (зависят от процессора, здесь для 16 МГц)
#define WS2812_T0H  350  // 0,35 мкс
#define WS2812_T1H  900  // 0,9 мкс
#define WS2812_T0L  900  // 0,9 мкс
#define WS2812_T1L  350  // 0,35 мкс
#define WS2812_RESET 50000  // 50 мкс

static void ws2812_sendbyte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & (1 << (7 - i))) {  // Отправляем бит 1
            WS2812_PORT |= (1 << WS2812_PIN);
            _delay_us(WS2812_T1H / 1000.0);
            WS2812_PORT &= ~(1 << WS2812_PIN);
            _delay_us(WS2812_T1L / 1000.0);
        } else {  // Отправляем бит 0
            WS2812_PORT |= (1 << WS2812_PIN);
            _delay_us(WS2812_T0H / 1000.0);
            WS2812_PORT &= ~(1 << WS2812_PIN);
            _delay_us(WS2812_T0L / 1000.0);
        }
    }
}

void ws2812_setleds(struct cRGB *ledarray, uint16_t number_of_leds) {
    cli();  // Отключаем прерывания
    for (uint16_t i = 0; i < number_of_leds; i++) {
        ws2812_sendbyte(ledarray[i].g);  // Отправляем зеленый компонент
        ws2812_sendbyte(ledarray[i].r);  // Отправляем красный компонент
        ws2812_sendbyte(ledarray[i].b);  // Отправляем синий компонент
    }
    sei();  // Включаем прерывания
    _delay_us(WS2812_RESET / 1000.0);  // Обеспечиваем сигнал сброса
}
