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


#define PWMOUT1 PORTB1 
#define PWMOUT2 PORTB0 
#define PWMCTL PORTB4
 
 
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

int door_status; 
 
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
 
int main (void)
{
    int cmd_door_open;
	door_status = 0; //Closed;
    // PWMOUT is an output.
    DDRB |= (1 << PWMOUT1) | (1 << PWMOUT2);
	// PWMCTL is an input  
	DDRB &= ~(1 << PWMCTL);  
	
    gpio_setup();
    pwm_setup();
  
	pwm_write1(KEY_CLOSE_PWM);
	pwm_write2(DOOR_CLOSE_PWM);
  
    while (1) {
        // Get the ADC value
        cmd_door_open = gpio_read();
        // Open routine
		if ((cmd_door_open)&&(door_status==0))
		{
			_delay_ms(100);
			if (cmd_door_open == gpio_read())
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
			_delay_ms(100);
			if (cmd_door_open == gpio_read())
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