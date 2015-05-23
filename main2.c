#include <stdint.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <string.h> 
#include <math.h> 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/eeprom.h> 
#include <avr/portpins.h> 
#include <avr/pgmspace.h> 
 
//FreeRTOS include files 
#include "FreeRTOS.h" 
#include "task.h" 
#include "croutine.h"
 unsigned char receivedData = 63;
 void SPI_ServantInit(void) {

	 // set DDRB to have MISO line as output and MOSI, SCK, and SS as input
	 DDRB = 0b10111111;
	 // set SPCR register to enable SPI and enable SPI interrupt (pg. 168)
	 SPCR = 0b11000000;
	 
	 // make sure global interrupts are enabled on SREG register (pg. 9)
	 SREG |= 0b10000000;
 }
 ISR(SPI_STC_vect) { // this is enabled in with the SPCR register’s “SPI
	 // Interrupt Enable”
	 // SPDR contains the received data, e.g. unsigned char receivedData =
	 receivedData = SPDR;
	 // SPDR;

 }

unsigned char Ptrn = 1;
unsigned char Spd = 1;

unsigned char P1[] = { 0b00001111, 0b11110000};
unsigned char P2[] = { 0b10101010, 0b01010101};
unsigned char P3[] = {
	0b00000001,
	0b00000010,
	0b00000100,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01000000,
	0b10000000,
	0b01000000,
	0b00100000,
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
};
unsigned char P4[] = {
	0b00000001,
	0b00000011,
	0b00000111,
	0b00001111,
	0b00011111,
	0b00111111,
	0b01111111,
	0b11111111,
	0b01111111,
	0b00111111,
	0b00011111,
	0b00001111,
	0b00000111,
	0b00000011,
};

int rate = 0;

void Ptrn1(){
	static int i = 0;
	PORTA = P1[i++];
	if( i > 1) i = 0;
}
void Ptrn2(){
	static int i = 0;
	PORTA = P2[i++];
	if(i > 1) i = 0;
}
void Ptrn3(){
	static int i = 0;
	PORTA = P3[i++];
	if( i> 13 )i = 0;
}
void Ptrn4(){
	static int i = 0;
	PORTA = P4[i++];
	if( i> 13 )i = 0;
}

void Ptrnz(){
	static int i = 0;
	if( i >= rate ){
		if( Ptrn == 1 ){
			Ptrn1();
		}else if( Ptrn == 2 ){
			Ptrn2();
		}else if( Ptrn == 3 ){
			Ptrn3();
		}else if( Ptrn == 4 ){
			Ptrn4();
		}
		i=0;
	}else{
		i++;
	}
}
void Ptrns(){
	for(;;){
		if( Spd == 1) rate = 40;
		else if( Spd == 2) rate = 20;
		else if( Spd == 3) rate = 10;
		else if( Spd == 4) rate = 5;
		else if( Spd == 5) rate = 2;
		else if( Spd == 6) rate = 1;
		Ptrnz();
		vTaskDelay(50);
	}
}
void GetData(){
	for(;;){
		Ptrn = receivedData%10;
		Spd = receivedData/10;
		vTaskDelay(40);
	}
}
void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Ptrns, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(GetData, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}	
 
int main(void) 
{ 
   DDRA = 0xFF; PORTA=0x00;
   //Start Tasks 
   SPI_ServantInit();
   StartSecPulse(1);
    //RunSchedular 
   vTaskStartScheduler(); 
 
   return 0; 
}