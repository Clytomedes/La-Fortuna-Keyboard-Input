/* COMP2215 Task 6  Skeleton */

#include <util/delay.h>
#include "os.h"


int blink(int);
int update_dial(int);
int collect_delta(int);
int check_switches(int);
void openKeyboard(void);

volatile uint8_t flag;
uint8_t keyboardOpen;
char array[54];



FIL File;  						/* FAT File */

int encoderPosition = 0;

void main(void)
{
    os_init();

    os_add_task( blink,            20, 1);
    os_add_task( collect_delta,    20, 1);
    os_add_task( check_switches,  100, 1);

    sei();
    for(;;){
        if (flag) {
            flag = keyboardOpen = 0;
            display_string(array);
            display_string("\n");
        }
    }

}

int collect_delta(int state)
{
    int8_t val = os_enc_delta();
    if (keyboardOpen) {
        if (val == -1) {
            keyboard_rotary_left();
        } else if (val == 1) {
            keyboard_rotary_right();
        }
    } else {
        encoderPosition += val;
    }
	return state;
}

int check_switches(int state)
{

	if (get_switch_press(_BV(SWN))) {
        if (!keyboardOpen) {
			display_string("North\n");
        } else {
            keyboard_arrow_north();
        }
	}

	if (get_switch_press(_BV(SWE))) {
        if (!keyboardOpen) {
			display_string("East\n");
        } else {
            keyboard_arrow_east();
        }
	}

	if (get_switch_press(_BV(SWS))) {
        if (!keyboardOpen) {
			display_string("South\n");
        } else {
            keyboard_arrow_south();
        }
	}

	if (get_switch_press(_BV(SWW))) {
        if (!keyboardOpen) {
			display_string("West\n");
        } else {
            keyboard_arrow_west();
        }
	}

    if(get_switch_press(_BV(SWC))) {
        if (keyboardOpen) {
            keyboard_centre_button();
        } else {
            /* Call method at bottom of this file to set the variables */
            open_keyboard();
        }
    }

	if (get_switch_long(_BV(SWC))) {
        if (!keyboardOpen) {
    		f_mount(&FatFs, "", 0);
    		if (f_open(&File, "myfile.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
    			f_lseek(&File, f_size(&File));
    			f_printf(&File, "Encoder position is: %d \r\n", encoderPosition);
    			f_close(&File);
    			display_string("Wrote position\n");
    		} else {
    			display_string("Can't write file! \n");
    		}
        }
	}

	if (get_switch_short(_BV(SWC))) {
        if (!keyboardOpen) {
			display_string("[S] Centre\n");
        }
	}

	if (get_switch_rpt(_BV(SWN))) {
        if (!keyboardOpen) {
			display_string("[R] North\n");
        } else {
            keyboard_arrow_north();
        }
	}

	if (get_switch_rpt(_BV(SWE))) {
        if (!keyboardOpen) {
			display_string("[R] East\n");
        } else {
            keyboard_arrow_east();
        }
	}

	if (get_switch_rpt(_BV(SWS))) {
        if (!keyboardOpen) {
			display_string("[R] South\n");
        } else {
            keyboard_arrow_south();
        }
	}

	if (get_switch_rpt(_BV(SWW))) {
        if (!keyboardOpen) {
			display_string("[R] West\n");
        }
        else {
           keyboard_arrow_west();
       }
	}


	if (get_switch_long(_BV(OS_CD))) {
        if (!keyboardOpen) {
		          display_string("Detected SD card.\n");
        }
	}

	return state;
}

int blink(int state)
{
	static int light = 0;
	uint8_t level;

	if (light < -120) {
		state = 1;
	} else if (light > 254) {
		state = -20;
	}


	/* Compensate somewhat for nonlinear LED
       output and eye sensitivity:
    */
	if (state > 0) {
		if (light > 40) {
			state = 2;
		}
		if (light > 100) {
			state = 5;
		}
	} else {
		if (light < 180) {
			state = -10;
		}
		if (light < 30) {
			state = -5;
		}
	}
	light += state;

	if (light < 0) {
		level = 0;
	} else if (light > 255) {
		level = 255;
	} else {
		level = light;
	}

	os_led_brightness(level);
	return state;
}

void open_keyboard(void)
{
    /* Set the variables and call the method in the library that opens the keyboard */
    flag = 0;
    keyboardOpen = 1;
    os_open_keyboard(array, &flag);
}
