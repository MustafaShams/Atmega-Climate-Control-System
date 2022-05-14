/*	Author: Khuaja Shams
 *  Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Final Project
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *	Demo Link: https://www.youtube.com/watch?v=0gW-LnDFG9E
 */



#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "io.h"
#include <math.h>
#include <string.h>
#endif


typedef struct task {
	unsigned char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct) (int);
} task;

unsigned char text[32] = "                                       ";
unsigned char tmp_msg[32] = "                                       ";

float temperature;
float desired_temp = 24; 
unsigned char vert, horz, sel, motion, up, down, tempmenu_on, fan_on, motion_on, tmpt0, tmpt1, tmpD = 0x00;

void MainMenuMSG() {
        strcpy( (unsigned char*) tmp_msg, " Set Temp");
        for (int i = 0; i < 9; i++)
                text[i] = tmp_msg[i];

        strcpy( (unsigned char*) tmp_msg, " Set Fan/Motion ");
        for (int i = 0; i < 16; i++)
                text[i + 16] = tmp_msg[i];
        return;
}

void TempMenuMSG() {
        strcpy( (unsigned char*) tmp_msg, "CURRENT TMP:");
        for (int i = 0; i < 12; i++)
                text[i] = tmp_msg[i];

        strcpy( (unsigned char*) tmp_msg, "DESIRED TMP:");
        for (int i = 0; i < 12; i++)
                text[i + 16] = tmp_msg[i];
        return;
}

void SensorMenuMSG() {
        strcpy( (unsigned char*) tmp_msg, " FAN STATUS:    ");
	if (fan_on) {tmp_msg[13]='O';tmp_msg[14]='N';}
	else {tmp_msg[13]='O';tmp_msg[14]='F';tmp_msg[15]='F';}
        
	for (int i = 0; i < 16; i++)
                text[i] = tmp_msg[i];

        strcpy( (unsigned char*) tmp_msg, " MOTION:        ");
	if (motion_on == 0x01) {tmp_msg[13]='O';tmp_msg[14]='N';}
        else {tmp_msg[13]='O';tmp_msg[14]='F';tmp_msg[15]='F';}

        for (int i = 0; i < 16; i++)
                text[i + 16] = tmp_msg[i];
        
	return;
}


enum MenuStates {mainmenu, mainmenu2, temp_menu, temp_set, sensor_menu, setfan, setfan_wait, vert_pause, setmotion, setmotion_wait, wait};
int MenuTick(int state) {
        switch(state) {
                case mainmenu:
                        MainMenuMSG();
			text[0] = '>'; text[16] = ' ';
                        if (!sel && !horz) {
                                state = mainmenu;
                        }
                        else if (sel)
                              state = temp_menu;
                        else if (horz)
                                state = mainmenu2;
                        break;
                case mainmenu2:
			MainMenuMSG();
			text[16] = '>'; text[0] = ' ';
                        if (vert) {
                                state = mainmenu;
                        }
			else if (sel) {
				state = sensor_menu;
			}
                        else {
                                state = mainmenu2;
                        }
                        break;
                case temp_menu:
			TempMenuMSG();
                        if (sel) 
                                state = temp_menu;
                        else { 
				state = temp_set;
				tempmenu_on = 0x01;
			}
                        break;
		case sensor_menu:
			SensorMenuMSG();
			if (sel)
				state = sensor_menu;
			else 
				state = setfan;
			break;
                case temp_set:
			TempMenuMSG();
			tmpt0 = desired_temp/10  + '0';
                        tmpt1 = (int)desired_temp % 10 + '0';
                        text[31] = 'C';
                        text[28] = tmpt0;
                        text[29] = tmpt1;
			up = 0x00;
			down = 0x00;
                        if (vert) 
                                up = 0x01; 
                        else if (horz) 
                                down = 0x01; 
                        
			if (!sel)
                                state = temp_set;
                        else if (sel) {
                                state = wait;   
				tempmenu_on = 0x00;
			}			
                        break;
		case setfan:
			SensorMenuMSG();
			text[0] = '>'; 
			if (vert) 
				state = vert_pause;
			else if (sel) {
				if(fan_on)
					fan_on = 0x00;
				else 
					fan_on = 0x01;
				state = setfan_wait;	
			}
			else if (horz) 
				state = wait;	
			else 
				state = setfan;
			break;
		case setfan_wait:
			SensorMenuMSG();
			text[0] = '>';
			if (sel) 
				state = setfan_wait; 
			else if (vert) 
				state = setfan_wait; 
			else
				state = setfan;
			break;
		case vert_pause:
			SensorMenuMSG();
			text[16] = '>';
			if (vert)
				state = vert_pause;
			else 
				state = setmotion;	
			break;
		case setmotion:
			SensorMenuMSG();
			text[16] = '>';
			if (sel) {
				if (motion_on)
					motion_on = 0x00;
				else 
					motion_on = 0x01;
				state = setmotion_wait;
			}
			else if (vert)
				state = setfan_wait;
			else if (horz)
				state = wait;
			else 
				state = setmotion;
			break;
		case setmotion_wait:
			SensorMenuMSG();
			text[16] = '>';
			if (sel) 
				state = setmotion_wait;
			else
				state = setmotion;
			break;
                case wait:
                        if (sel || horz)
                                state = wait;
                        else 
                                state = mainmenu;
                        break;
                default:
                        state = mainmenu;
                        break;

        }
        return state;
}

enum TempStates {tempinit};
int TempSensorTick(int state) {
        switch (state) {
                case tempinit:
                        state = tempinit;
                        temperature = (30 + ((10.888 - sqrt(118.548544 + 0.01388*(1777.3 - ADC)))/-0.00694)) - 129 ;

                        tmpt0 = temperature/10  + '0';
                        tmpt1 = (int)temperature % 10 + '0';
                        text[15] = 'C';
                        text[12] = tmpt0;
                        text[13] = tmpt1;
                        break;
                default:
                        state = tempinit;
                        break;
        }
        return state;
}

enum MotionStates {motioninit};
int MotionTick(int state) {
        switch (state) {
                case motioninit:
                        state = motioninit;
                        unsigned char tmpC = PINC & 0x80;
                        if (tmpC == 0x80)
                                motion = 0x01;
                        else
                                motion = 0x00;
                        break;
                default:
                        state = motioninit;
                        break;

        }
        return state;
}


enum States {joywait, joyvert, joyhorz, joysel};
int JoyStickTick(int state) {
	unsigned char tmpC = ~PINC & 0x07;

	switch(state) {
		case joywait:
			if (tmpC == 0x02)  //horz
				state = joyhorz;
			else if (tmpC == 0x04)   //vert
				state = joyvert;
			else if (tmpC == 0x01)   //selc 
				state = joysel; 
			else 	
				state = joywait;
			break;
		case joyhorz: 
			if (tmpC == 0x00) {
				state = joywait;
				horz = 0x00;
			}
			else 
				state = joyhorz;
			break;
		case joyvert:
                        if (tmpC == 0x00) {
                                state = joywait;
                                vert = 0x00;
                        }
                        else
                                state = joyvert;
                        break;
		case joysel:
                        if (tmpC == 0x00) {
                                state = joywait;
                                sel = 0x00;
                        }
                        else
                                state = joysel;
                        break;
		default:
			state = joywait;
			break;
	}

	switch(state) {
		case joyhorz: 
		        horz = 0x02;
	       		break;
 		case joyvert:
			vert = 0x04;
			break;
		case joysel:
			sel = 0x01;
			break;
		default:
			break;			
	}

	return state;
	
}

enum DTempStates {dtemploop};
int DesiredTempTick(int state) {
	switch (state) {
		case dtemploop:
			if (up && desired_temp < 100 && tempmenu_on)
				desired_temp++;
			else if (down && desired_temp > 0 && tempmenu_on) 
				desired_temp--;

			state = dtemploop;
			break;
		default:
			state = dtemploop;
			break;
	}
	return state;
}

enum OutputStates {outputloop};
int OutputTick(int state) {
	switch (state) {
		case outputloop: 
			tmpD = 0x00;

			if (motion)
        	        	tmpD = 0x08;

        		tmpD = tmpD | horz | sel | vert;

			if (motion && motion_on) {
				if (desired_temp <= temperature && fan_on)
					tmpD = tmpD | 0x10;
			}
			else if (!motion_on) {
				if (desired_temp <= temperature && fan_on)
					tmpD = tmpD | 0x10;
			}

        		PORTD = tmpD;

			state = outputloop;
			break;
		default: 
			state = outputloop;
			break;
	}
	return state;
}

void ADC_init() {
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADATE);
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PORTC = 0x7F;
	DDRD = 0xFF; PORTD = 0x00;

	static task task1, task2, task3, task4, task5, task6;

	task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

	task1.state = start; task1.period = 200; task1.elapsedTime = task1.period; task1.TickFct = &JoyStickTick;
	task2.state = start; task2.period = 200; task2.elapsedTime = task2.period; task2.TickFct = &MotionTick;
	task3.state = start; task3.period = 100; task3.elapsedTime = task3.period; task3.TickFct = &TempSensorTick;
	task4.state = start; task4.period = 100; task4.elapsedTime = task4.period; task4.TickFct = &MenuTick;
	task5.state = start; task5.period = 500; task5.elapsedTime = task5.period; task5.TickFct = &DesiredTempTick;
	task6.state = start; task6.period = 100; task6.elapsedTime = task6.period; task6.TickFct = &OutputTick;

	TimerSet(100);
	TimerOn();
	LCD_init();
	ADC_init();

	unsigned char last_text[32] = "                                ";
	unsigned char i, j;

    while (1) {
	
	for (unsigned char k = 0; k < 32; k++) { last_text[k] = text[k];}
	strcpy(text,  "                                      ");	
	    
	
	for (i = 0; i < numTasks; i++) {
		if (tasks[i]->elapsedTime == tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 100;
	}

	for (j = 0; j < 32; j++) { LCD_Cursor(j+1); if (last_text[j] != text[j]) {LCD_WriteData(text[j]);}}
	
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
