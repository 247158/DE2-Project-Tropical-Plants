#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "twi.h"            // I2C/TWI library for AVR-GCC
#include "oled.h"
#include "ws2812.h"         // Библиотека для управления WS2812B
#include <stdio.h>          // C library for `sprintf`

// -- Defines --------------------------------------------------------
#define DHT_ADR 0x5c
#define DHT_HUM_MEM 0
#define DHT_TEMP_MEM 2

#define ADC_CHANNEL_SOIL 0      // ADC channel for the soil moisture sensor
#define ADC_CHANNEL_LIGHT 1     // ADC channel for the light sensor (fotor)

#define ADC_LIGHT_MIN 100       // Minimum ADC value in complete darkness
#define ADC_LIGHT_MAX 900       // Maximum ADC value in bright light

#define LED_PIN PB1             // Пин для управления WS2812B (пин 13 на Arduino Uno)
#define NUM_LEDS 8              // Количество светодиодов

// -- Global variables -----------------------------------------------
volatile uint8_t flag_update_oled = 0;
volatile uint8_t dht12_values[5];
struct cRGB leds[NUM_LEDS];      // Массив для управления светодиодами

// -- Function prototypes --------------------------------------------
void adc_init(void);
uint16_t adc_read(uint8_t channel);
void oled_setup(void);
void timer1_init(void);
void update_led_brightness(uint8_t brightness);

// -- Function definitions -------------------------------------------

void adc_init(void) {
    ADMUX = (1 << REFS0);          // Use AVcc as reference voltage
    ADCSRA = (1 << ADEN) |         // Enable ADC
             (1 << ADPS2) |        // Set prescaler to 64
             (1 << ADPS1);
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Select ADC channel
    ADCSRA |= (1 << ADSC);                     // Start conversion
    while (ADCSRA & (1 << ADSC));              // Wait for conversion to finish
    return ADC;
}

void oled_setup(void) {
    oled_init(OLED_DISP_ON);
    oled_clrscr();

    oled_charMode(DOUBLESIZE);
    oled_puts("OLED disp.");

    oled_charMode(NORMALSIZE);

    oled_gotoxy(0, 2);
    oled_puts("128x64, SH1106");

    oled_drawLine(0, 25, 120, 25, WHITE);

    oled_gotoxy(0, 6);
    oled_puts("Temp [C]:");
    oled_gotoxy(0, 7);
    oled_puts("Hum  [%]:");

    oled_gotoxy(0, 4);
    oled_puts("Soil [%]:");

    oled_gotoxy(0, 5);
    oled_puts("Light [%]:");

    oled_display();
}

void timer1_init(void) {
    TIM1_ovf_1sec();
    TIM1_ovf_enable();
}

void update_led_brightness(uint8_t brightness) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i].r = brightness;  // Красный компонент
        leds[i].g = brightness;  // Зеленый компонент
        leds[i].b = brightness;  // Синий компонент
    }
    ws2812_setleds(leds, NUM_LEDS);
}

/*
 * Function: Main function where the program execution begins
 * Purpose:  Collect sensor data and display it on OLED.
 */
int main(void) {
    char oled_msg[16];

    twi_init();
    oled_setup();
    timer1_init();
    adc_init();

    // Инициализация WS2812B (Пин PB5)
    DDRB |= (1 << LED_PIN);

    sei();
    //cli();

    while (1) {
        if (flag_update_oled == 1) {
            // Temperature and Humidity from DHT12
            oled_gotoxy(10, 6);
            oled_puts("    ");
            oled_gotoxy(10, 6);
            sprintf(oled_msg, "%u.%u", dht12_values[2], dht12_values[3]);
            oled_puts(oled_msg);

            oled_gotoxy(10, 7);
            oled_puts("    ");
            oled_gotoxy(10, 7);
            sprintf(oled_msg, "%u.%u", dht12_values[0], dht12_values[1]);
            oled_puts(oled_msg);

            // Soil Moisture
            uint16_t soil_adc = adc_read(ADC_CHANNEL_SOIL);
            uint8_t soil_moisture = ((1023 - soil_adc) * 100) / 1023; // Convert to percentage
            oled_gotoxy(10, 4);
            oled_puts("    ");
            oled_gotoxy(10, 4);
            sprintf(oled_msg, "%u%%", soil_moisture);
            oled_puts(oled_msg);

            // Light Sensor
            uint16_t light_adc = adc_read(ADC_CHANNEL_LIGHT);
            uint8_t light_level = 0;

            if (light_adc <= ADC_LIGHT_MIN) {
                light_level = 0; // Darkness
            } else if (light_adc >= ADC_LIGHT_MAX) {
                light_level = 255; // Bright light (max brightness)
            } else {
                light_level = ((light_adc - ADC_LIGHT_MIN) * 255) / 
                              (ADC_LIGHT_MAX - ADC_LIGHT_MIN);
            }

            oled_gotoxy(10, 5);
            oled_puts("    ");
            oled_gotoxy(10, 5);
            sprintf(oled_msg, "%u%%", (light_level * 100) / 255); // Convert to percentage
            oled_puts(oled_msg);

            // Update WS2812B LEDs brightness
            update_led_brightness(light_level);

            oled_display();
            flag_update_oled = 0;
        }
    }

    return 0;
}

// -- Interrupt service routines -------------------------------------

ISR(TIMER1_OVF_vect) {
    static uint8_t n_ovfs = 0;

    n_ovfs++;
    if (n_ovfs >= 5) {
        n_ovfs = 0;
        twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 5);
        flag_update_oled = 1;
    }
}
