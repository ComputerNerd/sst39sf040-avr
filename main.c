#define F_CPU 16000000UL// 16 MHz
#include <avr/io.h> 
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define NOP asm volatile ("nop")
#define datIn DDRC&=~31; DDRD&=~224
#define datOut DDRC|=31; DDRD|=224
static void shift24(__uint24 addr){
	//this sends out the 24 bit address
	//The shift registers share clock pins but use different serial in pins
	const uint8_t*dat=(const uint8_t*)&addr;
	int8_t x;
	PORTB&=~3;
	for(x=7;x>=0;--x){
		PORTD&=~28;//clear pins
		PORTD|=((dat[0]>>x)&1)<<2;
		PORTD|=((dat[1]>>x)&1)<<3;
		PORTD|=((dat[2]>>x)&1)<<4;
		PORTB|=4;
		PORTB&=~4;//pulse clock
	}
	PORTB|=2;//send to parallel output
	PORTB&=~2;
}
static inline void serialWrB(uint8_t dat){
	UDR0=dat;
	while(!(UCSR0A&(1<<UDRE0)));//wait for byte to transmit
}
static inline uint8_t USART_Receive(void){
	/* Wait for data to be received */
	while(!(UCSR0A&(1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}
static void StringPgm(const char * str){
	do{
		serialWrB(pgm_read_byte_near(str));
	}while(pgm_read_byte_near(++str));
}
static inline uint8_t rdDat(void){
	return (PIND&224)|(PINC&31);
}
static inline void wrDat(uint8_t dat){
	PORTD&=~224;
	PORTD|=dat&224;
	PORTC&=~31;
	PORTC|=dat&31;
}
static inline void sendCmd(__uint24 addr,uint8_t dat){
	shift24(addr);
	PORTB&=~((1<<3)|(1<<5));//set CE# and WE# to low (latch address)
	wrDat(dat);
	PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high (latch data)
}
static inline void sendCmdSlowAddr(__uint24 addr,uint8_t dat){
	shift24(addr);
	PORTB&=~((1<<3)|(1<<5));//set CE# and WE# to low (latch address)
	NOP;
	NOP;
	NOP;
	wrDat(dat);
	PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high (latch data)
}
static void ReadChip(__uint24 sz){//sends entire flash content to serial
	datIn;
	PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
	PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
	__uint24 addr;
	for (addr=0;addr<sz;++addr){
		shift24(addr);
		serialWrB(rdDat());
	}
	PORTB|=(1<<3)|(1<<4);//CE# OE# to high
}
static inline uint8_t readB(__uint24 addr){
	uint8_t dat;
	datIn;
	PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
	PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
	shift24(addr);
	dat=rdDat();
	PORTB|=(1<<3)|(1<<4);//CE# OE# to high
	return dat;
}
static inline uint8_t pollB(void){
	uint8_t dat;
	PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
	PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
	NOP;
	dat=rdDat();
	PORTB|=(1<<3)|(1<<4);//CE# OE# to high
	return dat;
}
static uint8_t verifyF(__uint24 cap){
	datIn;
	PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
	PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
	__uint24 addr;
	for (addr=0;addr<cap;++addr){
	shift24(addr);
	if (rdDat()!=0xFF){
		serialWrB('E');
		PORTB|=(1<<3)|(1<<4);//CE# OE# to high
		return 1;
		}
	}
	serialWrB('S');
	PORTB|=(1<<3)|(1<<4);//CE# OE# to high
	return 0;
}
static void chipErase(void){
	datOut;
	PORTB&=~(1<<4);//set OE# to low
	PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high
	PORTB|=(1<<4);//set OE# to high
	sendCmdSlowAddr(0x5555,0xAA);
	sendCmd(0x2AAA,0x55);
	sendCmd(0x5555,0x80);
	sendCmd(0x5555,0xAA);
	sendCmd(0x2AAA,0x55);
	sendCmd(0x5555,0x10);
	_delay_ms(100);
}
static uint8_t readId(uint8_t which){
	datOut;
	PORTB&=~(1<<4);//set OE# to low
	PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high
	PORTB|=(1<<4);//set OE# to high
	sendCmd(0x5555,0xAA);
	sendCmd(0x2AAA,0x55);
	sendCmd(0x5555,0x90);
	/* From http://ww1.microchip.com/downloads/en/DeviceDoc/25022B.pdf
	 * A command is written by asserting WE# low while keeping CE# low.
	 * The address bus is latched on the falling edge of WE# or CE#, whichever  occurs last.
	 * The data bus is latched on the rising edge of WE# or CE#, whichever occurs first*/
	NOP;
	NOP;
	NOP;
	PORTB&=~((1<<4)|(1<<3));//set OE# and CE# to low
	datIn;
	shift24(which);
	uint8_t idR=rdDat();
	PORTB|=(1<<3);//set CE# to HIGH
	//exit
	datOut;
	PORTB|=(1<<4);//set OE# to high
	sendCmd(0x5555,0xAA);
	sendCmd(0x2AAA,0x55);
	sendCmd(0x5555,0xF0);
	NOP;
	NOP;
	NOP;
	PORTB&=~(1<<3);//set CE# to low
	return idR;
}
static inline void pgmB(__uint24 addr,uint8_t dat){
	datOut;
	PORTB&=~(1<<4);//set OE# to low
	PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high
	PORTB|=(1<<4);//set OE# to high
	sendCmd(0x5555,0xAA);
	sendCmd(0x2AAA,0x55);
	sendCmd(0x5555,0xA0);
	sendCmd(addr,dat);
	//This assumes a delay will follow the writing of the byte
	serialWrB('N');
}
__attribute__((noreturn)) void main(void){
	cli();
	//set-up serial communications
	DDRD=28;//3 serial pins on d.2 to d.4 d.5 to d.7 contain msbs for flash data d.0 to d.4 is in port C
	DDRB=62;//serial clocks (B.1 SHCP) (B.2 Latch) (B.3 CE#) (B.4 OE#) (B.5 WE#)
	UBRR0H=0;
	UBRR0L=3;//set to 0.5M baud
	UCSR0A|=2;//double speed aysnc
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);//Enable receiver and transmitter
	UCSR0C=6;//async 1 stop bit 8bit char no parity bits
	_delay_ms(50);
	StringPgm(PSTR("RDY"));
	USART_Receive();//wait for handshake
	char mode = USART_Receive();//wait for mode
	StringPgm(PSTR("RDY"));
	serialWrB(readId(0));
	uint8_t cap=readId(1);
	serialWrB(cap);
	__uint24 capacity=524288L;
	switch(cap){
		case 0xB5:
			capacity=131072L;
		case 0xB6:
			capacity=262144L;
		break;
	}
	if(mode=='W'){
		chipErase();
		serialWrB('D');
		verifyF(capacity);
		__uint24 x;
		for (x=0;x<capacity;++x){
			pgmB(x,USART_Receive());
			serialWrB(readB(x));
		}
	}else if(mode=='R')
		ReadChip(capacity);
	while(1);
}
