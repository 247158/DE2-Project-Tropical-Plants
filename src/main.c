#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "twi.h"            // I2C/TWI library for AVR-GCC
#include "oled.h"
#include <util/delay.h>      // Для функций задержки _delay_ms и _delay_us
#include <stdio.h>          // C library for `sprintf`

// -- Defines --------------------------------------------------------
#define DHT_ADR 0x5c
#define DHT_HUM_MEM 0
#define DHT_TEMP_MEM 2

#define ADC_CHANNEL_SOIL 0      // ADC channel for the soil moisture sensor
#define ADC_CHANNEL_LIGHT 1     // ADC channel for the light sensor (fotor)

#define ADC_LIGHT_MIN 100       // Minimum ADC value in complete darkness
#define ADC_LIGHT_MAX 900       // Maximum ADC value in bright light

#define LED_PIN PB5             // Pin for LED strip control
#define BUTTON_PIN PD2          // Pin for the button
#define DEBOUNCE_DELAY 50       // Debounce delay in milliseconds

// -- Global variables -----------------------------------------------
volatile uint8_t flag_update_oled = 0;
volatile uint8_t dht12_values[5];
volatile uint8_t led_on = 0;     // State of the LED strip

// -- Function prototypes --------------------------------------------
void adc_init(void);
uint16_t adc_read(uint8_t channel);
void oled_setup(void);
void timer1_init(void);
void led_strip_control(uint8_t state);
void button_init(void);

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

    oled_drawLine(0, 25, 120, 25, WHITE);

    oled_gotoxy(0, 6);
    oled_puts("Temp [C]:");
    oled_gotoxy(0, 7);
    oled_puts("Hum  [%]:");

    oled_gotoxy(0, 4);
    oled_puts("Soil [%]:");

    oled_gotoxy(0, 5);
    oled_puts("Light [%]:");

    oled_gotoxy(0, 2);
    oled_puts("LED: OFF");

    oled_display();
}

void timer1_init(void) {
    TIM1_ovf_1sec();
    TIM1_ovf_enable();
}

void led_strip_control(uint8_t state) {
    if (state) {
        PORTB |= (1 << LED_PIN);  // Turn on LED strip
    } else {
        PORTB &= ~(1 << LED_PIN); // Turn off LED strip
    }
    led_on = state;
}

void button_init(void) {
    DDRD &= ~(1 << BUTTON_PIN);  // Configure BUTTON_PIN as input
    PORTD |= (1 << BUTTON_PIN);  // Enable internal pull-up resistor
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
    button_init();

    // Set LED pin as output
    DDRB |= (1 << LED_PIN);
    led_strip_control(0); // Ensure LED strip is off initially

    sei();

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


            oled_display();
            flag_update_oled = 0;
        }

        // Check for button press
        if (!(PIND & (1 << BUTTON_PIN))) { // If button is pressed (active LOW)
            _delay_ms(DEBOUNCE_DELAY);
            if (!(PIND & (1 << BUTTON_PIN))) { // Confirm button press
                led_strip_control(!led_on); // Toggle LED state
                while (!(PIND & (1 << BUTTON_PIN))); // Wait for button release
            }
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
