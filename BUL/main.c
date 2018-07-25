/* ---------------------------------------------------------------------
 * PWM LED Brightness control for ATtiny13.
 * Datasheet for ATtiny13: http://www.atmel.com/images/doc2535.pdf
 * 
 * Pin configuration -
 * PB1/OC0B: LED output (Pin 6)
 * PB2/ADC1: Potentiometer input (Pin 7)
 *
 * ~100 bytes.
 * 
 * Find out more: http://bit.ly/1eBhqHc
 * -------------------------------------------------------------------*/
 
// 9.6 MHz, built in resonator
#define F_CPU 9600000
#define LED PB1 
 
 
#include <avr/io.h>
 
void gpio_setup (void)
{
    // Set the ADC input to PB2/ADC1
    
}
 
int gpio_read (void)
{
    return 1;
}
 
void pwm_setup (void)
{
    // Set Timer 0 prescaler to clock/8.
    // At 9.6 MHz this is 1.2 MHz.
    // See ATtiny13 datasheet, Table 11.9.
    TCCR0B |= (1 << CS01);
 
    // Set to 'Fast PWM' mode
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
 
    // Clear OC0B output on compare match, upwards counting.
    TCCR0A |= (1 << COM0B1);
}
 
void pwm_write (int val)
{
    OCR0B = val;
}
 
int main (void)
{
    int gpio_in;
 
    // LED is an output.
    DDRB |= (1 << LED);  
 
    gpio_setup();
    pwm_setup();
  
    while (1) {
        // Get the ADC value
        gpio_in = gpio_read();
        // Now write it to the PWM counter
        pwm_write(gpio_in);
    }
}