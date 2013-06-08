#define F_CPU 16000000UL  // 16 MHz
#include <avr/io.h> 
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "shifttab.h"
#define NOP asm volatile ("nop")
#define datIn DDRC&=~31; DDRD&=~224
#define datOut DDRC|=31; DDRD|=224
char buf[128];
void shift24(uint32_t addr)
{
	//this sends out the 24 bit address
	//The shift registers share clock pins but use different serial in pins
	uint8_t * dat=(uint8_t *)&addr;
	int8_t x;
	PORTB&=~3;
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab27+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab37+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab47+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab26+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab36+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab46+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab25+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab35+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab45+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab24+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab34+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab44+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab23+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab33+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab43+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab22+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab32+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab42+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab21+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab31+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab41+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	PORTD&=~28;//clear pins
	PORTD|=pgm_read_byte_near(shiftab20+dat[0]);
	PORTD|=pgm_read_byte_near(shiftab30+dat[1]);
	PORTD|=pgm_read_byte_near(shiftab40+dat[2]);
	PORTB|=4;
	PORTB&=~4;//pulse clock
	/*for (x=7;x>=0;x--)
	{
		PORTD&=~28;//clear pins
		PORTD|=((dat[0]>>x)&1)<<2;
		PORTD|=((dat[1]>>x)&1)<<3;
		PORTD|=((dat[2]>>x)&1)<<4;
		PORTB|=4;
		PORTB&=~4;//pulse clock
	}*/
	PORTB|=2;//send to parrell output
	PORTB&=~2;
}
inline void serialWrB(uint8_t dat)
{
	UDR0=dat;
	while ( !( UCSR0A & (1<<UDRE0)) ) {} //wait for byte to transmit
}
inline uint8_t USART_Receive(void)
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) ) {}
	/* Get and return received data from buffer */
	return UDR0;
}
void StringPgm(char * str)
{
	do {
		serialWrB(pgm_read_byte_near(str));
	} while(pgm_read_byte_near(++str));
}
void StringRam(char * str)
{
	do {
		serialWrB(*str);
	} while(*++str);
}
inline uint8_t rdDat(void)
{
	return (PIND&224)|(PINC&31);
}
inline uint8_t wrDat(uint8_t dat)
{
	PORTD&=~224;
	PORTD|=dat&224;
	PORTC&=~31;
	PORTC|=dat&31;
}
inline void sendCmd(uint32_t addr,uint8_t dat)
{
  shift24(addr);
  PORTB&=~((1<<3)|(1<<5));//set CE# and WE# to low (latch address)
  wrDat(dat);
  PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high (latch data)
}
inline void sendCmdSlowAddr(uint32_t addr,uint8_t dat)
{
  shift24(addr);
  PORTB&=~((1<<3)|(1<<5));//set CE# and WE# to low (latch address)
  NOP;
  NOP;
  NOP;
  wrDat(dat);
  PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high (latch data)
}
void ReadChip(void)
{//sends entire flash content to serial
  datIn;
  PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
  PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
  uint32_t addr;
  for (addr=0;addr<524288;addr++)
  {
   shift24(addr);
   serialWrB(rdDat());
  }
  PORTB|=(1<<3)|(1<<4);//CE# OE# to high
}
inline uint8_t readB(uint32_t addr)
{
  uint8_t dat;
  datIn;
  PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
  PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
   shift24(addr);
   dat=rdDat();
  PORTB|=(1<<3)|(1<<4);//CE# OE# to high
  return dat;
}
inline uint8_t pollB(void)
{
	uint8_t dat;
 // datIn;
  PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
  PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
  NOP;
   //shift24(addr);
   dat=rdDat();
  PORTB|=(1<<3)|(1<<4);//CE# OE# to high
  return dat;
}
uint8_t verifyF(void)
{
  datIn;
  PORTB|=(1<<3)|(1<<4)|(1<<5);//CE# OE# WE# all high
  PORTB&=~((1<<3)|(1<<4));//CE# OE# to low
  uint32_t addr;
  for (addr=0;addr<524288;addr++)
  {
   shift24(addr);
   if (rdDat()!=0xFF)
   {
     serialWrB('E');
     PORTB|=(1<<3)|(1<<4);//CE# OE# to high
     return 1;
   }
  }
  serialWrB('S');
  PORTB|=(1<<3)|(1<<4);//CE# OE# to high
  return 0;
}
void chipErase(void)
{
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
	//while ((pollB()&64)!=(pollB()&64)){}
	_delay_ms(100);
	//PORTB&=~(1<<3);//set WE# to low
}
uint8_t readId(uint8_t which)
{
  datOut;
  PORTB&=~(1<<4);//set OE# to low
  PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high
  PORTB|=(1<<4);//set OE# to high
  sendCmd(0x5555,0xAA);
  sendCmd(0x2AAA,0x55);
  sendCmd(0x5555,0x90);
/*From http://ww1.microchip.com/downloads/en/DeviceDoc/25022B.pdf
A command is written by asserting WE# low while keeping CE# low.
The address bus is latched on the falling edge of WE# or CE#, whichever  occurs last.
The data bus is latched on the rising edge of WE# or CE#, whichever occurs first*/
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
inline void pgmB(uint32_t addr,uint8_t dat)
{
  datOut;
  PORTB&=~(1<<4);//set OE# to low
  PORTB|=(1<<3)|(1<<5);//set CE# and WE# to high
  PORTB|=(1<<4);//set OE# to high
  sendCmd(0x5555,0xAA);
  sendCmd(0x2AAA,0x55);
  sendCmd(0x5555,0xA0);
  sendCmd(addr,dat);
  //delayMicroseconds(20);
 // datIn;
 //while ((pollB()&64)!=(pollB()&64)) {}
 //
   /*_delay_us(22);
  PORTB&=~(1<<5);//set WE# to low*/
  //This assumes a delay will follow the writting of the byte
  serialWrB('N');
}
uint8_t verifyB(void)
{
	uint8_t dat;
again:
	dat=USART_Receive();
  serialWrB(dat);
  if (USART_Receive() == 'C')
    return dat;
  else
    goto again;
}
void main(void)
{
	cli();
	//set-up serial communications
	DDRD=28;//3 serial pins on d.2 to d.4 d.5 to d.7 contain msbs for flash data d.0 to d.4 is in port C
	DDRB=62;//serial clocks (B.1 SHCP) (B.2 Latch) (B.3 CE#) (B.4 OE#) (B.5 WE#)
	UBRR0H=0;
	UBRR0L=1;//set to 1M baud
	UCSR0A|=2;//double speed aysnc
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);//Enable receiver and transmitter
	UCSR0C=6;//async 1 stop bit 8bit char no parity bits
/*
//The code commented below is just for fast testing before I wrote the computer-side program
StringPgm(PSTR("Manufacturer ID: 0x"));
  uint8_t x=readId(0);
   StringRam(utoa(x,buf,16));
  serialWrB('\n');
  switch (x)
  {
   case 0xBF:
  StringPgm(PSTR("Identifed as SST flash"));
  break;
  default:
   StringPgm(PSTR("Unknown manufacturer or bad connection"));
  break;
  }
 serialWrB('\n');
  StringPgm(PSTR("Device ID: 0x"));
  x=readId(1);
  StringRam(utoa(x,buf,16));
  serialWrB('\n');
  switch (x)
  {
   case 0xB5:
  StringPgm(PSTR("SST39SF010A"));
  break;
  case 0xB6:
  StringPgm(PSTR("SST39SF020A"));
  break;
  case 0xB7:
  StringPgm(PSTR("SST39SF040"));
  break;
  default:
  StringPgm(PSTR("Unknown flash"));
  break;
  }*/
	_delay_ms(100);
	StringPgm(PSTR("RDY"));
	USART_Receive();//wait for handshake
	char mode = USART_Receive();//wait for mode
	StringPgm(PSTR("RDY"));
	serialWrB(readId(0));
	serialWrB(readId(1));
	if(mode=='W'){
		chipErase();
		serialWrB('D');
		verifyF();
		uint32_t x;
		for (x=0;x<524288UL;x++){
			pgmB(x,USART_Receive());
			serialWrB(readB(x));
		}
	}else if(mode=='R')
		ReadChip();
	else
		while(1);//an error has occured
}
