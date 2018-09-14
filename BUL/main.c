/* ---------------------------------------------------------------------
* PWM control for ATtiny13.
* Datasheet for ATtiny13: http://www.atmel.com/images/doc2535.pdf
*
* Pin configuration -
* PB1/OC0B:
* PB4: input
*
* -------------------------------------------------------------------*/

// 4.8 MHz, built in resonator
#define F_CPU 4800000
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define PWMOUT1 PORTB1
#define PWMOUT2 PORTB0
#define PWMCTL PORTB3
#define TESTPAD PORTB2

//#define TCNT0_US_PER_TICK 1000 / 56
#define PWM_SCALE 56L / 1000

#define KEY_OPEN_PWM (1070 * PWM_SCALE)
#define KEY_CLOSE_PWM (1950 * PWM_SCALE)
#define KEY_TIMESTEP 30 //(1000/9)

#define DOOR_OPEN_PWM (930 * PWM_SCALE)
#define DOOR_CLOSE_PWM (2200 * PWM_SCALE)
#define DOOR_FINAL_PWM (2050 * PWM_SCALE)
#define DOOR_TIMESTEP 22 //(2000/12)

#define PWM_MIN (1000 * PWM_SCALE)
#define PWM_MAX (2000 * PWM_SCALE)
#define PWM_MID (1500 * PWM_SCALE)

volatile uint8_t cmd_door_open;

ISR(PCINT0_vect)
{
	static uint8_t pwmint = 0;
	static uint16_t pwmf = PWM_MIN;

	if (!(PINB & (1<<PWMCTL))) // inverted PWM start
		pwmint = TCNT0;
	else { // inverted PWM end
		pwmint = TCNT0 - pwmint;
		if (pwmint >= PWM_MIN && pwmint <= PWM_MAX) {
			pwmf = (pwmf * 15 + pwmint) / 16;
			if (pwmf > PWM_MID) {
				PORTB &= ~(1<<TESTPAD);
				cmd_door_open = 1;
			}
			else {
				PORTB |= (1<<TESTPAD);
				cmd_door_open = 0;
			}
		}
	}
}

inline void pwm_setup (void)
{
	// Set Timer 0 prescaler to clock/64.
	// At 4.8 MHz this is 75 kHz.
	// See ATtiny13 datasheet, Table 11.9.
	TCCR0B |= (1 << CS01) | (1 << CS00);
	
	// Set to 'Fast PWM' mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	
	// Clear OC0B output on compare match, upwards counting.
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	// F~~293
}

inline void pwm_write1 (int val)
{
	OCR0A = 255 - val;
}

inline void pwm_write2 (int val)
{
	OCR0B = 255 - val;
}

inline void setupint()
{
	GIMSK |= (1<<PCIE); // Разрешаем внешние прерывания PCINT0.
	PCMSK |= (1<<PCINT3); // Разрешаем по маске прерывания на ногак кнопок (PCINT3, PCINT4)
	sei(); // Разрешаем прерывания глобально: SREG |= (1<<SREG_I)
}

int main (void)
{
	uint8_t door_status = 0; //Closed;
	cmd_door_open = 0;
	
	// PWMOUT is an output.
	DDRB |= (1 << PWMOUT1) | (1 << PWMOUT2) | (1 << TESTPAD);
	// PWMCTL is an input
	DDRB &= ~(1 << PWMCTL);
	
	pwm_setup();
	
	pwm_write1(KEY_CLOSE_PWM);
	pwm_write2(DOOR_FINAL_PWM);
	
	setupint();
	
	while (1) {
		if (cmd_door_open && !door_status) // open
		{
			//Lets open the key
			for (int i = KEY_CLOSE_PWM;i >= KEY_OPEN_PWM; i--)
			{
				pwm_write1(i);
				_delay_ms(KEY_TIMESTEP);
			}
			
			_delay_ms(400);
			
			for (int i = DOOR_FINAL_PWM; i >= DOOR_OPEN_PWM; i--)
			{
				pwm_write2(i);
				_delay_ms(DOOR_TIMESTEP);
			}
			
			door_status = 1;
		}
		else if (!cmd_door_open && door_status) // close
		{
			for (int i = DOOR_OPEN_PWM;i <= DOOR_CLOSE_PWM; i++)
			{
				pwm_write2(i);
				_delay_ms(DOOR_TIMESTEP);
			}
			
			_delay_ms(400);
			
			for (int i = KEY_OPEN_PWM; i <= KEY_CLOSE_PWM; i++)
			{
				pwm_write1(i);
				_delay_ms(KEY_TIMESTEP);
			}
			
			_delay_ms(100);
			
			pwm_write2(DOOR_FINAL_PWM);		
			door_status = 0;
		}	
	}
}