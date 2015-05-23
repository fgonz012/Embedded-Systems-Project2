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
#include "io.c"
#include "keypad.h"
#include "usart_ATmega1284.h"
void A2D_init() {

	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);

	// ADEN: Enables analog-to-digital conversion

	// ADSC: Starts analog-to-digital conversion

	// ADATE: Enables auto-triggering, allowing for constant

	// analog to digital conversions.

}


void Set_A2D_Pin(unsigned char pinNum) {

	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;

	// Allow channel to stabilize

	static unsigned char i = 0;

	for ( i=0; i<15; i++ ) { asm("nop"); }

}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}
unsigned char EEPROM_read(unsigned int uiAddress)
{
	SREG &= 0x7F;
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	SREG |= 0x80;
	return EEDR;
}

enum menus{main_menu, lock_menu, temp_menu, lock_password_menu, success_unlock, unsuccess_unlock, change_pw_menu, change_pw_success_menu, successfully_changed_pw_menu, main_menu_unlocked, Unlocked_success} menu;
	
/*DATA VARIABLES*/
unsigned char unlocked = 0;
int temp_joystick = 0;
double duty_cycle = 0;
	
/*GLOBAL VARIABLES*/
int pw = 1234;
int usrPw = 0;
int newpw = 0;
int password_n = 17;
int change_pw_n = 17;
int eeprom_mem = 1;
int manual = 1;
int automatic = 0;
unsigned char temperature = 85;
unsigned char data0 = 0x00;
unsigned char data1 = 0x00;

unsigned char TopLeft = 0;
unsigned char TopRight = 0;
unsigned char BotRight = 0;
unsigned char BotLeft = 0;

unsigned char electric_motor_data_to_send = 0;//control
unsigned char heater_motor_data_to_send = 0;//control


void Menu_Control(){
	for(;;){
		
		
		if( menu == main_menu ){//MAIN MENU
			if(TopLeft){
				menu = lock_menu; 
				LCD_DisplayString(1,"EnterPw    Enter");
				TopLeft = 0;
			}
		}
		
		
		
		
		else if( menu == lock_menu){//LOCK MENU
			if(TopRight){
				if( pw == usrPw ){
					menu = success_unlock;
					unlocked = 1;
					LCD_DisplayString(1,"Sucess!         Back    ChangePw");
				}else{
					menu = unsuccess_unlock;
					LCD_DisplayString(1,"Wrong Pw        TryAgain Return"); 
				}
				TopRight = 0;
			}
			
		}
		
		
		else if( menu == success_unlock ){//SUCESS_UNLOCK
			if(BotLeft){
				if(!unlocked){
					menu = main_menu;
					LCD_DisplayString(1,"Unlock");
				}else{
					menu = main_menu_unlocked;
					LCD_DisplayString(1,"Lock            Manual Automatic");
					LCD_Cursor(14);
					LCD_WriteData(temperature%10 + '0'   );
					
					LCD_Cursor(13);
					LCD_WriteData((temperature/10)%10 + '0'   );
					
					LCD_Cursor(16);
					LCD_WriteData('F');
					LCD_Cursor(0);
				}
				BotLeft = 0;
			}
			else if(BotRight){
				menu = change_pw_menu;
				BotRight=0;
				LCD_DisplayString(1,"New Pw      Done");
			}
		}
		
		
		
		else if( menu == unsuccess_unlock ){//FAILED UNLOCK
			if(BotLeft){
				menu = lock_menu;
				LCD_DisplayString(1,"EnterPw    Enter");
				BotLeft = 0;
			}else if(BotRight){
				menu = main_menu;
				LCD_DisplayString(1,"Unlock");
				LCD_Cursor(14);
				LCD_WriteData(temperature%10 + '0'   );
				
				LCD_Cursor(13);
				LCD_WriteData((temperature/10)%10 + '0'   );
				
				LCD_Cursor(16);
				LCD_WriteData('F');
				LCD_Cursor(0);
				BotRight = 0;
			}
			
		}
		
		
		else if( menu == change_pw_menu ){//MENU TO CHANGE PW
			if(TopRight){
				pw = newpw;
				EEPROM_write(0,1);
				newpw = 0;
				TopRight = 0;
				menu = successfully_changed_pw_menu;
				LCD_DisplayString(1,"Success!        MainMenu");
			}
		}
		
		
		else if( menu == successfully_changed_pw_menu ){//SUCCESS AFTER CHANGING PW
			if( BotLeft ){
				if(!unlocked){
					menu = main_menu;
					LCD_DisplayString(1,"Unlock");
					LCD_Cursor(14);
					LCD_WriteData(temperature%10 + '0'   );
					
					LCD_Cursor(13);
					LCD_WriteData((temperature/10)%10 + '0'   );
					
					LCD_Cursor(16);
					LCD_WriteData('F');
					LCD_Cursor(0);
					}else{
					menu = main_menu_unlocked;
					LCD_DisplayString(1,"Lock            Manual Automatic");
					LCD_Cursor(14);
					LCD_WriteData(temperature%10 + '0'   );
					
					LCD_Cursor(13);
					LCD_WriteData((temperature/10)%10 + '0'   );
					
					LCD_Cursor(16);
					LCD_WriteData('F');
					LCD_Cursor(0);
				}
				BotLeft = 0;
			}
		}
		
		else if( menu == main_menu_unlocked ){
			if(TopLeft){
				unlocked = 0;
				menu = Unlocked_success;
				LCD_DisplayString(1,"Success!                  Return");
				TopLeft = 0;
			}else if( BotRight){
				automatic = 1;
				manual = 0;
				BotRight = 0;
				
			}else if( BotLeft){
				automatic = 0;
				manual = 1;
				BotLeft = 0;
			}
		}
		
		else if( menu == Unlocked_success ){
			if(BotRight){
				menu = main_menu;
				LCD_DisplayString(1,"Unlock");
				LCD_Cursor(14);
				LCD_WriteData(temperature%10 + '0'   );
				
				LCD_Cursor(13);
				LCD_WriteData((temperature/10)%10 + '0'   );
				
				LCD_Cursor(16);
				LCD_WriteData('F');
				LCD_Cursor(0);
			}
			BotRight = 0;
		}
		vTaskDelay(100);
	}
}
enum tlStates{tlWaitPress,tlWaitRelease}tlState;// // // //
enum blStates{blWaitPress,blWaitRelease}blState;// // // //
enum trStates{trWaitPress,trWaitRelease}trState;// // // //
enum brStates{brWaitPress,brWaitRelease}brState;// // // //
void Buttons_Control(){//
	tlState = tlWaitPress;
	blState = blWaitPress;
	trState = trWaitPress;
	for(;;){
		unsigned char topLeft = !(PIND & 0b00100000);
		unsigned char topRight = !(PIND & 0b00001000);
		unsigned char botLeft = !(PIND & 0b00010000);
		unsigned char botRight = !(PIND & 0b00000100);
		
		
		switch(blState){//
			case blWaitPress://
				if(botLeft){
					blState = blWaitRelease;
					BotLeft = 1;
				}
			break;
			case blWaitRelease://
				if(!botLeft)blState = blWaitPress;
			break;
		}
		
		
		switch(tlState){//
			case tlWaitPress://
			if(topLeft){
				tlState = tlWaitRelease;
				TopLeft = 1;
			}
			break;
			case tlWaitRelease://
			if(!topLeft)tlState = tlWaitPress;
			break;
		}
		
		switch(trState){//
			case trWaitPress://
			if(topRight){
				trState = trWaitRelease;
				TopRight = 1;
			}
			break;
			case trWaitRelease://
			if(!topRight)trState = trWaitPress;
			break;
		}
		switch(brState){//
			case brWaitPress://
			if(botRight){
				brState = brWaitRelease;
				BotRight = 1;
			}
			break;
			case brWaitRelease://
			if(!BotRight)brState = brWaitPress;
			break;
		}
		vTaskDelay(100);
	}
}

enum password_states{waitPress,waitRelease}pw_states;
void Pw(){
	static int tempPw = 0;
	unsigned char key = GetKeypadKey();
	
	switch(pw_states){
		case waitPress:
			if( key != '\0' ){
				pw_states = waitRelease;
				if( key != 'A' && key != 'B' && key != 'C' && key != 'D' && key != '#' && key != '*'){
					LCD_Cursor(password_n++);
					LCD_WriteData(key);
				}
				usrPw = usrPw * 10 + key - '0';
			}
		break;
		case waitRelease:
			if( key == '\0'){
				pw_states = waitPress;
			}
		break;
	}
}
void Password_Control(){
	pw_states = waitPress;
	for(;;){
		if( menu == lock_menu){
			Pw();
		}else{
			password_n = 17;
			usrPw = 0;
		}
		vTaskDelay(50);
	}
}


enum password_states2{waitPress2,waitRelease2}pw_states2;
void Change_PwTask(){
	unsigned char key = GetKeypadKey();
	
	switch(pw_states2){
		case waitPress2:
		if( key != '\0' ){
			pw_states2 = waitRelease2;
			if( key != 'A' && key != 'B' && key != 'C' && key != 'D' && key != '#' && key != '*'){
				LCD_Cursor(change_pw_n++);
				LCD_WriteData(key);
			}
			newpw = newpw * 10 + key - '0';
			EEPROM_write(eeprom_mem++,key);
		}
		break;
		case waitRelease2:
		if( key == '\0'){
			pw_states2 = waitPress2;
		}
		break;
	}
}
void Change_Pw(){
	pw_states2 = waitPress2;
	for(;;){
		if( menu == change_pw_menu){
			Change_PwTask();
		}else{
			change_pw_n = 17;
		}
		vTaskDelay(100);
	}
}


void SendData(){
	for(;;){
		if(USART_IsSendReady(0) ){
			unsigned char data0 = 0x00;
			if( ADC > 600 ){
				data0 |= 0b00001100;
			}else if( ADC < 400 ){
				data0 |= 0b00001000;
				data0 &= 0b11111011;
			}else{
				data0 &= 0b11110011;
			}
			
			switch(electric_motor_data_to_send){
				case 0: data0 &= 0b11111100; break;
				case 2: data0 &= 0b11111101; data0 |= 0b00000010; break;
				case 1: data0 &= 0b11111110; data0 |= 0b00000001; break;
				case 3: data0 |= 0b00000011; break;
			}
			switch(heater_motor_data_to_send){
				case 0: data0 &= 0b11001111; break;
				case 2: data0 &= 0b11101111; data0 |= 0b00100000; break;
				case 1: data0 &= 0b11011111; data0 |= 0b00010000; break;
				case 3: data0 |= 0b00110000; break;
			}
			
			USART_Send(data0,0);

		}
		
		if(USART_IsSendReady(0)){
			unsigned char data1 = 0x80 + temperature - 50;
			
			if(automatic)data1 |= 0b01000000;
			else data1 &= 0b10110000;
			
			USART_Send(data1 ,0);
		}
		
		vTaskDelay(5);
	}
}

unsigned char LeftButton = 0;
unsigned char RightButton = 0;
enum rButtonTasks{waitPushR, waitReleaseR}rButtonState;
enum lButtonTasks{waitPushL, waitReleaseL}lButtonState;
void TakeButtonInput(){
	for(;;){
		
		unsigned char leftbutton = !(PINA & 0x80);
		unsigned char rightbutton = !(PINA & 0x40);
		
		switch(rButtonState){
			case waitPushR:
				if(rightbutton){
					rButtonState = waitReleaseR;
					RightButton = 1;
				}
			break;
			case waitReleaseR:
				if(!rightbutton){
					rButtonState = waitPushR;
				}
			break;
		}
		
		
		switch(lButtonState){
			case waitPushL:
				if(leftbutton){
					lButtonState = waitReleaseL;
					LeftButton = 1;
				}
			break;
			case waitReleaseL:
				if(!leftbutton){
					lButtonState = waitPushL;
				}
			break;
		}
		vTaskDelay(50);
	}
}
void Manual_Control(){
	for(;;){
		//unsigned char leftbutton = !(PINA & 0x80);
		
		if( manual && unlocked){//lights the LED lights to indicate manual or automatic
			PORTA |= 0b00100000;
			PORTA &= 0b11101111;
		}else if(automatic && unlocked){
			PORTA |= 0b00010000;
			PORTA &= 0b11011111;
		}else if(!unlocked){
			PORTA &= 0b11001111;
		}
		
		if(LeftButton && unlocked && manual){//controls electric motor
			if( (++electric_motor_data_to_send) > 3   )electric_motor_data_to_send = 0;
			LeftButton = 0;
		}else{
			LeftButton = 0;
		}
		if(RightButton && unlocked && manual){//controls electric motor
			if( (++heater_motor_data_to_send) > 3   )heater_motor_data_to_send = 0;
			RightButton = 0;
			}else{
			RightButton = 0;
		}
		vTaskDelay(50);
	}
}



void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Menu_Control, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(Password_Control, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(Change_Pw, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(Buttons_Control, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(SendData, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(Manual_Control, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(TakeButtonInput, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}	
 
int main(void) 
{ 
   DDRA = 0x3E; PORTA = ~0x3E;
   DDRD = 0xC3; PORTD = ~0xC3;
   DDRC = 0xFF; PORTC = 0x00;
   DDRB = 0xF0; PORTB = 0x0F;
   LCD_init();
   initUSART(0);
   
   LCD_DisplayString(1,"Unlock");
   LCD_Cursor(14);
   LCD_WriteData(temperature%10 + '0'   );
   
   LCD_Cursor(13);
   LCD_WriteData((temperature/10)%10 + '0'   );
   
   LCD_Cursor(16);
   LCD_WriteData('F');
   LCD_Cursor(0);
   menu = main_menu;
	if( EEPROM_read(0) == 1 ){
		pw = 0;
		for(int i=1; i<5; i++){
			pw = pw*10 + EEPROM_read(i) - '0';
		}
	}else{
		pw = 1234;
	}
	A2D_init();
	Set_A2D_Pin(0);
	
   //Start Tasks  
	StartSecPulse(1);
    //RunSchedular 
	vTaskStartScheduler(); 
 
   return 0; 
}