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
 
//1100 1550
 
//period 3413
//0.3222 0.4541 (x255)
//82 115
#define KEY_OPEN_PWM 70
#define KEY_CLOSE_PWM 100
#define KEY_TIMESTEP 30 //(1000/9)

//910 2110

//period 3413
//0.2666 0.6182 (x255)
//68 157
#define DOOR_OPEN_PWM 60
#define DOOR_CLOSE_PWM 140
#define DOOR_TIMESTEP 22 //(2000/12)

#define TCNT0_US_PER_TICK 1000/75

int door_status; 
int cmd_door_open;
int l_cmd_door_open;
int state = 0;
uint8_t pwmint = 0;

ISR(PCINT0_vect)
{
	if (((PINB & (1<<PWMCTL)))) // начался фронт ШИМа 00000000000......11
	{
		pwmint = TCNT0; //254
	}
	if (!((PINB & (1<<PWMCTL))))// кончился +pulse ШИМа 1111111111111....0
	{
		long lpwm;
		pwmint = TCNT0-pwmint;//140-254
		lpwm=(long)pwmint*TCNT0_US_PER_TICK;
		if (lpwm>1000 && lpwm<2000)
		{
			if (lpwm>1500)
			{
				PORTB&=~(1<<TESTPAD);
				cmd_door_open = 1;
			}
			else
			{
				PORTB|=(1<<TESTPAD);
				cmd_door_open = 0;
			}
		}		
	}
}
 
 
void gpio_setup (void)
{
      
}
 
int gpio_read (void)
{
    return PINB&(1<<PWMCTL);
}
 
void pwm_setup (void)
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
 
void pwm_write1 (int val)
{
    OCR0A = val;
}

void pwm_write2 (int val)
{
	OCR0B = val;
}

void setupint()
{
	 GIMSK |= (1<<PCIE); // Разрешаем внешние прерывания PCINT0.
	 PCMSK |= (1<<PCINT3); // Разрешаем по маске прерывания на ногак кнопок (PCINT3, PCINT4)
	 sei(); // Разрешаем прерывания глобально: SREG |= (1<<SREG_I)
}
 
int main (void)
{
    
	door_status = 0; //Closed;
	cmd_door_open = 0;
	l_cmd_door_open=0;
	
    // PWMOUT is an output.
    DDRB |= (1 << PWMOUT1) | (1 << PWMOUT2) | (1 << TESTPAD);
	// PWMCTL is an input  
	DDRB &= ~(1 << PWMCTL);  
	
    gpio_setup();
    pwm_setup();
  
	pwm_write1(KEY_CLOSE_PWM);
	pwm_write2(DOOR_CLOSE_PWM);
    
	setupint();
	
    while (1) {
        // Get the ADC value
        //cmd_door_open = gpio_read();
        // Open routine
		if ((cmd_door_open)&&(door_status==0))
		{
			l_cmd_door_open = cmd_door_open;
			_delay_ms(100);
			if (cmd_door_open == l_cmd_door_open)
			{
				//Lets open the key
				for (int i=KEY_CLOSE_PWM;i!=KEY_OPEN_PWM;i--)
				{
					pwm_write1(i);
					_delay_ms(KEY_TIMESTEP);
				}
				pwm_write1(KEY_OPEN_PWM);
				_delay_ms(1000);
				
				for (int i=DOOR_CLOSE_PWM;i!=DOOR_OPEN_PWM;i--)
				{
					pwm_write2(i);
					_delay_ms(DOOR_TIMESTEP);
				}
				pwm_write2(DOOR_OPEN_PWM);
				door_status = 1;
			}
		}
		//Close routine
		if ((!cmd_door_open)&&(door_status==1))
		{
			l_cmd_door_open = cmd_door_open;
			_delay_ms(100);
			if (cmd_door_open == l_cmd_door_open)
			{
				for (int i=DOOR_OPEN_PWM;i!=DOOR_CLOSE_PWM;i++)
				{
					pwm_write2(i);
					_delay_ms(DOOR_TIMESTEP);
				}
				pwm_write2(DOOR_CLOSE_PWM);
				_delay_ms(1000);
				for (int i=KEY_OPEN_PWM;i!=KEY_CLOSE_PWM;i++)
				{
					pwm_write1(i);
					_delay_ms(KEY_TIMESTEP);
				}
				pwm_write1(KEY_CLOSE_PWM);
				
				door_status = 0;
			}
			
		}
			
    }
}