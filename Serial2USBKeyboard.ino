/*Courtesy of andrewintw: https://github.com/andrewintw/arduino-serial-to-keyboard?tab=readme-ov-file
Console colouring: https://stackoverflow.com/questions/5762491/how-to-print-color-in-console-using-system-out-println

List of keyboard input start with 27 (screen /dev/ttyUSB0 in Debian 12.5):
Sequence - Key (identifier after 27)
79 80 - F1 (80)
79 81 - F2 (81)
79 82 - F3 (82)
79 83 - F4 (83) 
91 49 53 126 - F5 (49 + 53 + ignore 1 suffix)
91 49 55 126 - F6 (55 + ignore 1 suffix) 
91 49 56 126 - F7 (56 + ignore 1 suffix) 
91 49 57 126 - F8 (57 + ignore 1 suffix) 
91 49 126 - HOME (126)
91 50 48 126 - F9 (50 + 48 + ignore 1 suffix)
91 50 49 126 - F10 (50 + 49 + ignore 1 suffix)
91 50 51 126 - F11 (50 + 51 + ignore 1 suffix)
91 50 51 126 - F12 (50 + 52 + ignore 1 suffix)
91 50 126 - INSERT (50 + 126)
91 51 126 - DELETE (51 + ignore 1 suffix)
91 52 126 - END (52 + ignore 1 suffix)
91 53 126 - PGUP (53 + ignore 1 suffix) 
91 54 126 - PGDN (54 + ignore 1 suffix
91 65 - UP (65)
91 66 - DOWN (66)
91 67 - RIGHT (67)
91 68 - LEFT (68)
(F10-F12 binded by system in Debian, but available in Windows Putty 0.81)

Handled Ctrl + key combinations:
Ctrl + c
Ctrl + k
Ctrl + u
Ctrl + v
Ctrl + w
Ctrl + x
Ctrl + z

Special Ctrl + key combinations:
Ctrl + g	Disable Flood protection for 30 seconds
Ctrl + h	Hide non special characters feedback
Ctrl + q	OS logo, Command (⌘) on Mac
Ctrl + n	Mouse Mode

To prevent Serial buffer full, please refer to 
https://hackaday.com/2020/07/13/surgery-on-the-arduino-ide-makes-bigger-serial-buffers/
C:\Users\<username>\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.6\cores\arduino
Test:
#if SERIAL_RX_BUFFER_SIZE==256
#error 256
#endif
*/
#include "Keyboard.h"
#include <Mouse.h>
#define DEBUG 0 //0 = No debug 1 = Notice 2 = ALL

int inCh;
int RXLED = 17;	// The RX LED has a defined Arduino pin
bool identified = false;
bool Fkeys = false;
bool F50 = false;
bool F49 = false;
bool reducedfeedback = false;
bool MouseMode = false;
bool flooddisabled = false;
int tick =0; /*garbled input (e.g. floating pin) check*/
int verpos=0;
int ignorechar = 0; //Number of char to be ignored. Useful for function keys
unsigned long Fkeytime=0;
unsigned long flooddisabletime=0;

void initSerials()
{
	#if DEBUG
	Serial.begin(9600);
	print2serial(1);
	#endif
	
	Serial1.begin(9600);
	while (!Serial1);
	Serial1.println(F("Serial1 ready!"));
	Serial1.println(F("Welcome to use Serial to USB keyboard v1.2 developed by Hazuki. To prevent floating pin messing the input, please type \"qwerty\" and press Enter for enabling the keyboard."));
}

void print2serial(int msg)
{
	#if DEBUG
	switch (msg)
	{
		case 1:
		Serial.println(F("Serial ready!"));
		break;
		case 2:
		Serial.println(F("Flood detected!"));
		break;
		case 3:
		Serial.println(F("Host identifed"));
		break;
		case 4:
		Serial.println(F("Inform host to identify"));
		break;
	}
	#endif
}

void setup()
{
	pinMode(RXLED, OUTPUT);
	initSerials();
	Keyboard.begin();
	Mouse.begin();
}

void loop() {
	
	/*Exit Fkeys status if overtime - ESC is assumed*/
	if (Fkeys && millis()-Fkeytime> 100)
	{
		Fkeys = F49 = F50 = false;
		Serial1.println(F("\u001B[32m[ESC]\u001B[0m"));
		Keyboard.write(KEY_ESC);
	}
	
	if(flooddisabled)
	{
		if(millis()-flooddisabletime>30000)
		{
			flooddisabled=false;
			Serial1.println(F("\u001B[35mFlood detection enabled\u001B[0m"));
		}
	}
	
	if (Serial1.available() > 0)
	{
		
		if(tick >30 && !flooddisabled)
		{
			identified=Fkeys=false;
			verpos=ignorechar=tick=0;
			print2serial(2);
			Serial1.println(F("Flood detected! Press \"Enter\" for details"));
		}
		
		digitalWrite(RXLED, LOW);   // set the RX LED ON
		inCh = Serial1.read();
		#if DEBUG == 2
		Serial.print(inCh, DEC);
		Serial.print(" - ");
		Serial.println((char)inCh);
		#endif
		if(identified)
		{
			TXLED0; //TX LED is not tied to a normally controlled pin so a macro is needed, turn LED OFF

			if(Fkeys)
			{
				switch (inCh) 
				{
					case 27: /*Double ESC keys*/
					Keyboard.write(KEY_ESC);
					Keyboard.write(KEY_ESC);
					Serial1.println(F("\u001B[32m[ESC][ESC]\u001B[0m"));
					Fkeys=false;
					break;
					
					case 48: /*F9 - need preceding 50*/
					if(F50)
					{
						Keyboard.write(KEY_F9);
						Serial1.println(F("\u001B[32m[F9]\u001B[0m"));
						Fkeys=false;
						ignorechar=1;
					}
					break;

					case 49: 
					if(F50) /*F10*/
					{
						Keyboard.write(KEY_F10);
						Serial1.println(F("\u001B[32m[F10]\u001B[0m"));						
						Fkeys=false;
						ignorechar=1;	
					} else /*flag in some sequence*/
					F49=true;
					break;
					
					case 50: /*flag in some sequence*/
					F50=true;
					break;
					
					case 51: 
					if(F50) /*F11*/
					{
						Keyboard.write(KEY_F11);
						Serial1.println(F("\u001B[32m[F11]\u001B[0m"));						
					}else/*DELETE*/
					{
						if(MouseMode)
						{	if(!Mouse.isPressed()) Mouse.click();}
						else
						{
							Keyboard.write(KEY_DELETE);
							Serial1.println(F("\u001B[32m[DEL]\u001B[0m"));
						}
					}
					Fkeys=false;
					ignorechar=1;
					break;
					
					case 52: 
					if(F50) /*F12*/
					{
						Keyboard.write(KEY_F12);
						Serial1.println(F("\u001B[32m[F12]\u001B[0m"));						
					}else{/*END*/
						if(MouseMode)
							Mouse.click(MOUSE_RIGHT);
						else
						{
							Keyboard.write(KEY_END);
							Serial1.println(F("\u001B[32m[END]\u001B[0m"));
						}
					}
					Fkeys=false;
					ignorechar=1;
					break;
					
					case 53: 
					if(F49)/*F5*/
					{
						Keyboard.write(KEY_F5);
						Serial1.println(F("\u001B[32m[F5]\u001B[0m"));
					}else
					{/*PGUP*/
						if(MouseMode)
							Mouse.move(0, 0, 1);
						else
						{
							Keyboard.write(KEY_PAGE_UP);
							Serial1.println(F("\u001B[32m[PGUP]\u001B[0m"));
						}
					}
					Fkeys=false;
					ignorechar=1;
					break;
		
					case 54: /*PGDN*/
					if(MouseMode)
						Mouse.move(0, 0, -1);
					else
					{
						Keyboard.write(KEY_PAGE_DOWN);
						Serial1.println(F("\u001B[32m[PGDN]\u001B[0m"));
					}
					Fkeys=false;
					ignorechar=1;
					break;
					
					case 55: /*F6*/
					Keyboard.write(KEY_F6);
					Serial1.println(F("\u001B[32m[F6]\u001B[0m"));
					Fkeys=false;
					ignorechar=1;
					break;
					
					case 56: /*F7*/
					Keyboard.write(KEY_F7);
					Serial1.println(F("\u001B[32m[F7]\u001B[0m"));
					Fkeys=false;
					ignorechar=1;
					break;

					case 57: /*F8*/
					Keyboard.write(KEY_F8);
					Serial1.println(F("\u001B[32m[F8]\u001B[0m"));
					Fkeys=false;
					ignorechar=1;
					break;
					
					case 65: /*UP*/
					if(MouseMode)
						Mouse.move(0, -10, 0);
					else
					{
						Keyboard.write(KEY_UP_ARROW);
						Serial1.println(F("\u001B[32m[↑]\u001B[0m"));
					}
					Fkeys=false;
					break;
					
					case 66: /*DOWN*/
					if(MouseMode)
						Mouse.move(0, 10, 0);
					else
					{
						Keyboard.write(KEY_DOWN_ARROW);
						Serial1.println(F("\u001B[32m[↓]\u001B[0m"));
					}
					Fkeys=false;
					break;
					
					case 67: /*RIGHT*/
					if(MouseMode)
						Mouse.move(10, 0, 0);
					else
					{
						Keyboard.write(KEY_RIGHT_ARROW);
						Serial1.print(F("\u001B[32m[→]\u001B[0m"));
					}
					Fkeys=false;
					break;
					
					case 68: /*LEFT*/
					if(MouseMode)
						Mouse.move(-10, 0, 0);
					else
					{
						Keyboard.write(KEY_LEFT_ARROW);
						Serial1.print(F("\u001B[32m[←]\u001B[0m"));
					}
					Fkeys=false;
					break;
					
					case 80: /*F1*/
					Keyboard.write(KEY_F1);
					Serial1.println(F("\u001B[32m[F1]\u001B[0m"));
					Fkeys=false;
					break;
					
					case 81: /*F2*/
					Keyboard.write(KEY_F2);
					Serial1.println(F("\u001B[32m[F2]\u001B[0m"));
					Fkeys=false;
					break;
					
					case 82: /*F3*/
					Keyboard.write(KEY_F3);
					Serial1.println(F("\u001B[32m[F3]\u001B[0m"));
					Fkeys=false;
					break;
					
					case 83: /*F4*/
					Keyboard.write(KEY_F4);
					Serial1.println(F("\u001B[32m[F4]\u001B[0m"));
					Fkeys=false;
					break;
					
					case 126: 
					if(F50)/*INSERT*/
					{	
						if(MouseMode)
						{
							if(Mouse.isPressed())
							{
								Mouse.release();
								Serial1.println(F("\u001B[35mMouse Left button released\u001B[0m"));
							}else
							{
								Mouse.press();
								Serial1.println(F("\u001B[35mMouse Left button pressed\u001B[0m"));						
							}
						}else
						{
							Keyboard.write(KEY_INSERT);
							Serial1.println(F("\u001B[32m[INS]\u001B[0m"));
						}
					}else{/*HOME*/
						Keyboard.write(KEY_HOME);
						Serial1.println(F("\u001B[32m[HOME]\u001B[0m"));
					}
					Fkeys=false;
					break;
					/*Default ignore e.g. 79/91 received*/
				}
				if(Fkeys==false)
				{
					F49 = F50 = false;
				}
			}else if(ignorechar>0)
			{
				ignorechar--;
			}
			else switch (inCh) 
			{
				case 3: /*[Ctrl+c]*/
				Keyboard.press(KEY_LEFT_CTRL);
				Keyboard.press('c');
				Keyboard.releaseAll(); 
				Serial1.println(F("\u001B[32m[Ctrl+c]\u001B[0m"));
				break;
				case 7:
				flooddisabletime=millis();
				flooddisabled=true;
				Serial1.print(F("\u001B[35mFlood detection disabled for 30 seconds. RX buffer size (Bytes): \u001B[0m"));
				Serial1.println(SERIAL_RX_BUFFER_SIZE);
				Serial1.println(F("\u001B[33mWARNING!!! Buffer may full and data will lost if you paste over RX buffer size.\u001B[0m"));
				break;
				case 8: /*[Ctrl+h]*/
				if(reducedfeedback)
				{
					reducedfeedback=false;
					Serial1.println(F("\u001B[35mFeedback enabled\u001B[0m"));
				}else
				{
					reducedfeedback=true;
					Serial1.println(F("\u001B[35mFeedback disabled\u001B[0m"));						
				}
				break;
				case 11: /*[Ctrl+k]*/				
				Keyboard.press(KEY_LEFT_CTRL);
				Keyboard.press('k');
				Keyboard.releaseAll(); 
				Serial1.println(F("\u001B[32m[Ctrl+k]\u001B[0m"));
				break;
				case 13:
				Keyboard.write(KEY_RETURN);
				Serial1.println(F("\u001B[32m[Enter]\u001B[0m"));
				break;
				case 14: /*Ctrl+n Mouse mode*/
				if(MouseMode)
				{
					MouseMode=false;
					Serial1.println(F("\u001B[35mMouse Mode disabled\u001B[0m"));
				}else
				{
					Serial1.println(F("\u001B[35mMouse Mode enabled.\u001B[0m"));
					Serial1.println(F("\u001B[32m[↑][↓][→][←]\u001B[0mfor mouse position"));
					Serial1.println(F("\u001B[32m[DELETE][END]\u001B[0m for left and right click respectively"));
					Serial1.println(F("\u001B[32m[PGUP][PGDN]\u001B[0m for scrolling up and down"));
					Serial1.println(F("\u001B[32m[INSERT]\u001B[0m for pressing/releasing left mouse button"));
					MouseMode=true;
				}
				break;
				case 17:
				Keyboard.write(KEY_LEFT_GUI);
				Serial1.println(F("\u001B[32m[OS]\u001B[0m"));
				break;
				case 21: /*[Ctrl+u]*/
				Keyboard.press(KEY_LEFT_CTRL);
				Keyboard.press('u');
				Keyboard.releaseAll(); 
				Serial1.println(F("\u001B[32m[Ctrl+u]\u001B[0m"));
				break;
				case 22: /*[Ctrl+v]*/
				Keyboard.press(KEY_LEFT_CTRL);
				Keyboard.press('v');
				Keyboard.releaseAll(); 
				Serial1.println(F("\u001B[32m[Ctrl+v]\u001B[0m"));
				break;
				case 23: /*[Ctrl+w]*/
				Keyboard.press(KEY_LEFT_CTRL);
				Keyboard.press('w');
				Keyboard.releaseAll(); 
				Serial1.println(F("\u001B[32m[Ctrl+w]\u001B[0m"));
				break;
				case 24: /*[Ctrl+x]*/
				Keyboard.press(KEY_LEFT_CTRL);
				Keyboard.press('x');
				Keyboard.releaseAll(); 
				Serial1.println(F("\u001B[32m[Ctrl+x]\u001B[0m"));
				break;
				case 26: /*[Ctrl+z]*/
				Keyboard.press(KEY_LEFT_CTRL);
				Keyboard.press('z');
				Keyboard.releaseAll(); 
				Serial1.println(F("\u001B[32m[Ctrl+z]\u001B[0m"));
				break;					
				case 27:/*special keys - need to reset flood detector*/
				tick=0;
				Fkeys=true;
				Fkeytime = millis();
				break;
				case 127:
				Keyboard.write(KEY_BACKSPACE);
				Serial1.print(F("\u001B[32m[BKSP]\u001B[0m"));
				break;

				default:
				Keyboard.write((char)inCh);
				if(!reducedfeedback)
				{
					if(inCh >127 || inCh <32)
						Serial1.print(F("\u001B[32m[SC]\u001B[0m"));
					else Serial1.print((char)inCh);
				}
			}
			TXLED1; //TX LED macro to turn LED ON
		} else
		{	/*Not identified*/
			Serial1.print((char)inCh);
			switch (inCh) 
			{ /*qwerty - 113 119 101 114 116 121*/
			
				case 13:
					if(verpos==6)
					{
						identified=true;
						print2serial(3);
						Serial1.println(F("Enabled remote keyboard."));
						Serial1.println(F("\u001B[35mCtrl+h\u001B[0m to toggle feeding back non special character"));
						Serial1.println(F("\u001B[35mCtrl+g\u001B[0m to disable flood detection for 30 seconds"));
						Serial1.println(F("\u001B[35mCtrl+q\u001B[0m for OS logo key"));
						Serial1.println(F("\u001B[35mCtrl+n\u001B[0m for toggle Mouse Mode"));
					}
					else
					{
						print2serial(4);
						Serial1.println(F("\nTo prevent floating pin messing the input, please type \"qwerty\" and press Enter for enabling remote keyboard."));
					}
					verpos=0;
					break;
				case 113: /*q*/
					if (verpos==0)
						verpos=1;
					break;
				case 119: /*w*/
					if (verpos==1)
						verpos=2;
					else verpos=0;
					break;
				case 101: /*e*/
					if (verpos==2)
						verpos=3;
					else verpos=0;
					break;				
				case 114: /*r*/
					if (verpos==3)
						verpos=4;
					else verpos=0;
					break;
				case 116: /*t*/
					if (verpos==4)
						verpos=5;
					else verpos=0;
					break;
				case 121: /*y*/
					if (verpos==5)
						verpos=6;
					else verpos=0;
					break;
					
				default:
					;
			}
		}
		tick=tick+10;
		digitalWrite(RXLED, HIGH);
	} else if(tick >0) tick--;
}