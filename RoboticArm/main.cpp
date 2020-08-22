/*
 * RoboticArm.cpp
 *
 * Created: 1.8.2020. 23:41:14
 * Author : volaric
 */ 

#define F_CPU 16000000
#define BAUD 9600
#define UBR_9600 103



//#include <stdlib.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include <stdio.h>
//#include <string.h>




char distance[8];
int x;
void USART_Init(void);
void USART_Transmit(unsigned char data);
//void USART_Transmit_Int(int data);
void timer_init(void);
void start_timer(int timer, int prescaler);
void USART_putstring(char* StringPtr);
void sonarTrigger(void);
//void USART_sendInt(int a);
//void gcvt(float f, int ndigits, char * buf);

using namespace std;

bool TCNT_reset = true;
bool pinChangeEnabled = true;
bool inputCaptureEnabled = false;
bool startServo = false;
float timeDelay;
char String[]="distanca ";
int tenthou;
int thou;
int hunds;
int tents;
int ones;
int ICR_first_value;
int ICR_last_value;
bool _ICR_first_value = true;
bool _ICR_last_value = false;
int finalPosition;

// INTERRUPTS

ISR(TIMER4_CAPT_vect){
	
	
	TCCR5B |= 1 << CS51;
	//PORTB = 0xff;
	
}

ISR(TIMER5_CAPT_vect){
	
	//convert float to char array
	//timeDelay = ICR5*0.0000005;
	//timeDelay = ICR5;
	x = ICR5;
	tenthou = x / 10000;
	thou = (x % 10000) / 1000;
	hunds = (x % 1000) / 100;
	tents = (x % 100) / 10;
	ones = (x % 100) % 10;
	
	distance[0]= tenthou + 48;
	distance[1]= thou + 48;
	
	distance[2]= hunds + 48;
	distance[3]= tents + 48;
	distance[4]= ones + 48;
	
	distance[5]= 0x00;
	
	//convert float to char array
	//timeDelay = ICR5*0.0000005;
	/*timeDelay = ICR5;
	x = timeDelay *1000 * (345/2);
	tenthou = x / 10000;
	thou = (x % 10000) / 1000;
	hunds = (x % 1000) / 100;
	tents = (x % 100) / 10;
	ones = (x % 100) % 10;
	
	distance[0]= tenthou + 48;
	distance[1]= thou + 48;
	distance[2]= '.';
	distance[3]= hunds + 48;
	distance[4]= tents + 48;
	distance[5]= ones + 48;
	distance[6]= 'm';
	distance[7]= 0x00;*/
	
	
	
	//PORTB = 0x00;
	TCCR5B &= ~(1 << CS51);
	TCNT5 = 0x00;
}


void USART_putstring(char* StringPtr){
	
	while(*StringPtr != 0x00){    
		USART_Transmit(*StringPtr);    
		StringPtr++;}        
}

/*void USART_sendInt(int a){
	
	
		USART_Transmit_Int(a);
	
}
*/
void USART_Init(unsigned int ubrr){
	
	// set baud rate
	UBRR2H = (ubrr >> 8);
	UBRR2L = ubrr;
	
	// enable transmitter receiver
	UCSR2B |= (1 << TXEN2);
	
	// set frame format, no parity 1 stop bit
	//UCSR2C |= (1 << UCSZ20) | (1 << UCSZ21); // 8 bit data format
	
}

void USART_Transmit(unsigned char data){
	
		while(!(UCSR2A & (1 << UDRE2)))
		_delay_ms(0);
		UDR2 = data;
		
		
}

/*void USART_Transmit_Int(int data){
	
	while(!(UCSR2A & (1 << UDRE2)))
	_delay_ms(0);
	UDR2 = 49;
	
	
}*/

void sonarTrigger(){
	
	start_timer(0, 256);
	while(TCNT0 < 1)			// delay 12micro
	{
		PORTL |= 1 << 6;	// start trigger
	}
	PORTL &=~(1 << 6);		// shut trigger
}

void timer_init(){
	
	TCCR1A |= 1 << WGM11 | 1 << COM1A1;
	TCCR1B |= 1 << WGM13 | 1 << WGM12 | 1 << CS11; 
	ICR1 = 6700;
	OCR1A = 3080;
}

void start_timer(int timer, int prescaler){
	
	switch(timer){															// reset counters and prescaler
		
	  case 0:	TCNT0 = 0x00;
				TCCR0B &= ~(1 << CS00) & ~(1 << CS01) & ~(1 << CS02);
				break;
				
	  case 5:	TCNT5 = 0x00;
				TCCR5B &= ~(1 << CS50) & ~(1 << CS51) & ~(1 << CS52);
				break;
	}
	
												
	
	switch(prescaler){														// set prescaler
		
	  case   0: TCCR0B |= 1 << CS00;
				break;
	  case 256: TCCR0B |= 1 << CS02;
				break;
				
	}
	
}




int main(void)

{	
	// global interrupt enable
	sei();	
				
	DDRB = 0xff;
	DDRL |= 1 << 6 | 1 << 2;		 //set trigger pin PL6 as output
	
	PORTB = 0x00;
	PORTL |= (1 << 5) | (1 << 7);
	timer_init();
	DDRL &=~(1 << DDL7) & ~(1 << DDL5);
	//PORTL = 0x00;
	MCUCR &=~ (1 << PUD);
	USART_Init(UBR_9600);
	TCCR3B |= (1 << CS32) | (1 << CS30);
	TCNT3 = 0;
	
	//set PJ1 as input
	DDRJ &=~(1 << PJ1);
	
	//Pin on change interrupt Reg:PCMSK1, Pin:PCINT10(PJ1), bit 2
	PCMSK1 |= (1 << PCINT10);
	
	//PCICR – Pin Change Interrupt Control Register
	//Set PCIE1, bit 1
	//PCICR |= (1 << PCIE1);
	
	//Input Capture pin ( ICP4, PL0, AMega pin 49 ) edge select setup
	//TCCR4B ICES4 bit 6 - input capture edge select - 1 rising
	TCCR4B |= (1 << ICES4);
	
	//Input Capture pin ( ICP5, PL1, AMega pin 48 ) edge select setup
	//TCCR5B ICES5 bit 6 - input capture edge select - 0 falling
	
	//Input Capture interrupt enable - TIMSK4 (input capture mask register), ICIE4, bit 5
	TIMSK4 |= (1 << ICIE4);
	
	//Input Capture interrupt enable - TIMSK5 (input capture mask register), ICIE5, bit 5
	TIMSK5 |= (1 << ICIE5);
	
	
	/*char tab2[1024];
	strncpy(tab2, to_string(123.45).strcat(), sizeof(tab2));
	tab2[sizeof(tab2) - 1] = 0;*/
	
	ICR_first_value = 0;
	ICR_last_value = 0;
	
	
	while (1)
	{
		/*PORTB = 0xFF;
		_delay_ms(500);
		PORTB = 0x00;
		_delay_ms(500);*/
		
		sonarTrigger();
		
		//_delay_ms(1000);
		if(TCNT3 > 15000){
		//USART_putstring(String);
		//USART_putstring(distance);
		TCNT3 = 0;
		}
		    //Pass the string to the USART_putstring function and sends it over the serial
		//_delay_ms(5000);
		/*if(!(UCSR2A & (1 << UDRE2))){
			
			
		}*/
		
		//PINL |= (1 << PINL2);
		
		//OCR1A = 5100;
		
		
		
		if(!(PINL & (1 << PINL7))) startServo = true;
			
			
		if(startServo){
			
			OCR1A = 1240;
			sonarTrigger();
			_delay_ms(500);
			//PORTB = 0xFF;
		for(int i = OCR1A; i < 5101; i+=7){
			
			if(!(PINL & (1 << PINL5))){
				
				 startServo = false; 
				 OCR1A = 3080;
				 break;
			}
			
			
			sonarTrigger();
			_delay_ms(5);
			
			
			
			if(ICR5 < 3600){
				//PORTB = 0x00;
				//break;
				
				/*ICR_current_value = ICR5;
				
				if((ICR_last_value - ICR_current_value) < -120){
					
					break;
				}
				
				
				ICR_last_value = ICR_current_value;*/
				_delay_ms(20);
				_ICR_last_value = true;
				
				if(_ICR_first_value){
					
				ICR_first_value = OCR1A;
				_ICR_first_value = false;
				
				}
			}
			else {
				
				if(_ICR_last_value){
					
					ICR_last_value = OCR1A;
					while(1){
						
						OCR1A = ((ICR_last_value - ICR_first_value) / 2) + ICR_first_value;
						if(!(PINL & (1 << PINL5))){
							
							//PORTB = 0x00;
							
							OCR1A = 3080;
							startServo = false;
							break;
							
						}
					}
					_ICR_last_value = false;
					
					
				}
				
				_ICR_first_value = true;
			}
			
			
			
			OCR1A = i;
			
			
			
		
			
		}
		startServo = false;
		
		}
		
		
		//_delay_ms(50);
		
		/*for(int i = OCR1A; i > 3079; i--){
			
			if(!(PINL & (1 << PINL5))){
				
				 startServo = false; 
				 OCR1A = 3080;
				 break;
			}
			
			OCR1A = i;
			
			_delay_us(60);
			
		
			
		}
		
		_delay_ms(50);
		
		}*/
		
		
		
		
		
		
		
		if(!(PINL & (1 << PINL5))){
			
			//PORTB = 0x00;
		
		OCR1A = 3080;
		startServo = false;
		
		
		}
		/*TCCR0B |= 1 << CS00;*/
		/*ICR5 = 0x00;*/				//set input compare register 0
		
		/*TCNT5 = 0x00;				//reset timer 5
		TCNT0 = 0x00;				//reset timer 0*/
		/*PORTL |= 1 << 6;*/			// fire trigger
		
		
		
		
		
		//_delay_ms(100)	;
		/*TCCR1B |= 1 << CS50;		// start echo timer
		start_timer(0, 256);		
		while(!(TIFR5 & (1 << 5)))
		{
			PORTL |= 1 << 6;
			
			if(TCNT0 >= 190){			
				
				break;
			}
		}
		PORTL &=~(1 << 6);*/		// shut trigger		
		/*PORTB = 0xFF;*/
		//_delay_ms(300)	;
		}
}

