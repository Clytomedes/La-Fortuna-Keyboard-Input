/*  keyboard.c

    Author: Conor Keegan(cpk1g13 / Clytomedes)
*/

#include <avr/interrupt.h>
#include "lcd.h"
#include "ruota.h"

void display_keyboard(void);
void update_display(void);
void colour_tile(int8_t position, uint8_t row, uint16_t col);
void colour_string(char *str, uint16_t backwardsX, uint16_t forwardsY, uint16_t col);
void move_screen(void);
void move_screen_back(void);

int8_t cursorPosition;
uint8_t firstRowLength, secondRowLength, cursorRow, arrayPosition, shiftOn;
uint8_t *done;
uint16_t printingX;
char *outputArray;
lcdState savedState;

/* Can only display the display width / 12 characters on the screen (rounding down), with La Fortuna in landscape this is 26 */
const char row0[]  = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
/* Alternative version to row0 */
const char row0A[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
/* The second row must no more than 17 characters to allow for Enter, Shift, backspace and space. Or the same as above but taking away 108 before dividing */
const char row1[]  = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', ',', '!', '?', '-', '_', '/' };
/* Alternative version to row1 */
const char row1A[] = { '(', ')', '[', ']', '$', '#', ':', ';', '@', '%', '&', '^', '=', '+', '*', '\"', '\'' };

void os_open_keyboard(char *array, uint8_t *flag)
{
    savedState = get_lcd_state();
    cursorPosition = cursorRow = arrayPosition = printingX = shiftOn = 0;
    outputArray = array;
    done = flag;

    /* Turn off all forms of scrolling */
    set_clearing(0);

    cli();
    display_keyboard();
    sei();
}

void keyboard_centre_button(void)
{
    if (cursorRow == 1) {
        if (cursorPosition == secondRowLength) {
            /* Input a space, which is key "__" */
            outputArray[arrayPosition] = ' ';
            display_char_xy(' ', printingX, 216);
        } else if (cursorPosition == (secondRowLength + 1)) {
            /* Remove the last inputted letter */
            if (arrayPosition > 0) {
                outputArray[--arrayPosition] = 0;
                rectangle r = {(printingX - 8), (printingX + 1), 216, 223};
                fill_rectangle(r, BLACK);
                printingX -= 8;
            }
            return;
        } else if (cursorPosition == (secondRowLength + 2)) {
            /* Show and use the alternate keyboard */
            uint16_t i, j;
            /* Covers the rows in black */
            rectangle r0 = {0, (display.width - 1), (display.height - 16), (display.height - 9)};
            fill_rectangle(r0, BLACK);
            rectangle r1 = {0, (display.width - 109), (display.height - 8), (display.height - 1)};
            fill_rectangle(r1, BLACK);

            /* Change the variables for the alternate keyboard */
            if (shiftOn) {
                shiftOn = 0;
                firstRowLength = (sizeof row0) / (sizeof row0[0]);
                secondRowLength = (sizeof row1) / (sizeof row1[0]);
                if (firstRowLength > (display.width / 12)) firstRowLength = display.width / 12; /* Can only display this many characters */
                if (secondRowLength > ((display.width - 108) / 12)) secondRowLength = (display.width - 108) / 12;
            } else {
                shiftOn = 1;
                firstRowLength = (sizeof row0A) / (sizeof row0A[0]);
                secondRowLength = (sizeof row1A) / (sizeof row1A[0]);
                if (firstRowLength > (display.width / 12)) firstRowLength = display.width / 12; /* Can only display this many characters */
                if (secondRowLength > ((display.width - 108) / 12)) secondRowLength = (display.width - 108) / 12;
            }
            set_background(BLACK);

            /* Redraw the alternate keyboard */
            j = 3;
            for (i = 0; i < firstRowLength; i++) {
                if (shiftOn) {
                    display_char_xy(row0A[i], j, 224);
                    if (i < secondRowLength) {
                        display_char_xy(row1A[i], j, 232);
                    }
                } else {
                    display_char_xy(row0[i], j, 224);
                    if (i < secondRowLength) {
                        display_char_xy(row1[i], j, 232);
                    }
                }
                j += 12;
            }
            return;
        } else if (cursorPosition == (secondRowLength + 3)) {
            /* Close the keyboard and set the done flag */
            rectangle r = {0, (display.width - 1), (display.height - 24), (display.height - 1)};
            fill_rectangle(r, BLACK);
            arrayPosition = printingX = 0;
            savedState.keepState = 1;
            savedState.clearingState = 1;
            set_lcd_state(savedState);
            move_screen_back();
            *done = 1;
            return;
        } else {
            if (shiftOn) {
                outputArray[arrayPosition] = row1A[cursorPosition];
                display_char_xy(row1A[cursorPosition], printingX, 216);
            } else {
                outputArray[arrayPosition] = row1[cursorPosition];
                display_char_xy(row1[cursorPosition], printingX, 216);
            }
        }
    } else {
        if (shiftOn) {
            outputArray[arrayPosition] = row0A[cursorPosition];
            display_char_xy(row0A[cursorPosition], printingX, 216);
        } else {
            outputArray[arrayPosition] = row0[cursorPosition];
            display_char_xy(row0[cursorPosition], printingX, 216);
        }
    }

    printingX += 8;
    arrayPosition++;
}

void keyboard_rotary_left(void)
{
    /* Move the keyboard cursor to the left */
    if (cursorRow == 0) {
        if (cursorPosition >= 0 && cursorPosition < firstRowLength) {
            colour_tile(cursorPosition, cursorRow, BLACK);
        }
    } else {
        if (cursorPosition >= 0 && cursorPosition < secondRowLength) {
            colour_tile(cursorPosition, cursorRow, BLACK);
        }
    }
    cursorPosition--;
    update_display();
}

void keyboard_rotary_right(void)
{
    /* Move the keyboard cursor to the right */
    if (cursorRow == 0) {
        if (cursorPosition >= 0 && cursorPosition < firstRowLength) {
            colour_tile(cursorPosition, cursorRow, BLACK);
        }
    } else {
        if (cursorPosition >= 0 && cursorPosition < secondRowLength) {
            colour_tile(cursorPosition, cursorRow, BLACK);
        }
    }
    cursorPosition++;
    update_display();
}

void keyboard_arrow_north(void)
{
    /* Move the keyboard cursor up */
    if (cursorRow == 0) {
        if (cursorPosition >= 0 && cursorPosition < firstRowLength) {
            colour_tile(cursorPosition, cursorRow, BLACK);
            if (cursorPosition >= 24) {
                cursorPosition = secondRowLength + 3;
            } else if (cursorPosition >= 21) {
                cursorPosition = secondRowLength + 2;
            } else if (cursorPosition >= 19) {
                cursorPosition = secondRowLength + 1;
            } else if (cursorPosition >= 17) {
                cursorPosition = secondRowLength;
            }
            cursorRow = 1;
        }
    } else {
        if (cursorPosition >= 0 && cursorPosition < secondRowLength) {
            colour_tile(cursorPosition, cursorRow, BLACK);
        } else if (cursorPosition == secondRowLength) {
            colour_string("__", 105, 232, BLACK);
            if (firstRowLength >= 17) {
                cursorPosition = 17;
            } else {
                cursorPosition = firstRowLength;
            }
        } else if (cursorPosition == secondRowLength + 1) {
            colour_string("<-", 87, 232, BLACK);
            if (firstRowLength >= 19) {
                cursorPosition = 19;
            } else {
                cursorPosition = firstRowLength;
            }
        } else if (cursorPosition == secondRowLength + 2) {
            colour_string("Shift", 69, 232, BLACK);
            if (firstRowLength >= 21) {
                cursorPosition = 21;
            } else {
                cursorPosition = firstRowLength;
            }
        } else if (cursorPosition == secondRowLength + 3) {
            colour_string("Enter", 33, 232, BLACK);
            if (firstRowLength >= 24) {
                cursorPosition = 24;
            } else {
                cursorPosition = firstRowLength;
            }
        }
        cursorRow = 0;
    }
    update_display();
}

void keyboard_arrow_south(void)
{
    /* Move the keyboard cursor down, but since there is only two rows it works the same as going up */
    keyboard_arrow_north();
}

void keyboard_arrow_west(void)
{
    /* Move the keyboard cursor to the left */
    keyboard_rotary_left();
}

void keyboard_arrow_east(void)
{
    /* Move the keyboard cursor to the right */
    keyboard_rotary_right();
}

void display_keyboard(void)
{
    uint8_t i;
    uint16_t j;

    /* Shift the screen out of the way of the keyboard */
    move_screen();

    /* Ensure that there is nothing left on the screen in the way */
    set_foreground(WHITE);
    set_background(BLUE);
    rectangle r = {0, 10, 224, 230};
    fill_rectangle(r, BLUE);
    display_char_xy(row0[0], 3, 224);

    firstRowLength = (sizeof row0) / (sizeof row0[0]);
    secondRowLength = (sizeof row1) / (sizeof row1[0]);
    if (firstRowLength > (display.width / 12)) firstRowLength = display.width / 12; /* Can only display this many characters */
    if (secondRowLength > ((display.width - 108) / 12)) secondRowLength = (display.width - 108) / 12;

    set_background(BLACK);
    display_char_xy(row1[0], 3, 232);

    /* Print the keyboard to the screen */
    j = 15;
    for (i = 1; i < firstRowLength; i++) {
        display_char_xy(row0[i], j, 224);

        if (i < secondRowLength) {
            display_char_xy(row1[i], j, 232);
        }
        j += 12;
    }

    /* Add the additional buttons to the screen */
    display_string_xy("__", (display.width - 105), 232);
    display_string_xy("<-", (display.width - 87), 232);
    display_string_xy("Shift", (display.width - 69), 232);
    display_string_xy("Enter", (display.width - 33), 232);
}

void update_display(void)
{
    /* Update the cursor with the new position */
    if (cursorRow == 0) {
        if (cursorPosition > (firstRowLength - 1)) {
            cursorPosition = 0;
            cursorRow = 1;
        } else if (cursorPosition < 0) {
            cursorPosition = (secondRowLength + 3);
            cursorRow = 1;
            colour_string("Enter", 33, 232, BLUE);
            return;
        }
    } else {
        if (cursorPosition == (secondRowLength - 1)) {
            colour_string("__", 105, 232, BLACK);
        } else if (cursorPosition == secondRowLength) {
            colour_string("<-", 87, 232, BLACK);
            colour_string("__", 105, 232, BLUE);
            return;
        } else if (cursorPosition == (secondRowLength + 1)) {
            colour_string("Shift", 69, 232, BLACK);
            colour_string("__", 105, 232, BLACK);
            colour_string("<-", 87, 232, BLUE);
            return;
        } else if (cursorPosition == (secondRowLength + 2)) {
            colour_string("Enter", 33, 232, BLACK);
            colour_string("<-", 87, 232, BLACK);
            colour_string("Shift", 69, 232, BLUE);
            return;
        } else if (cursorPosition == (secondRowLength + 3)) {
            colour_string("Shift", 69, 232, BLACK);
            colour_string("<-", 87, 232, BLACK);
            colour_string("__", 105, 232, BLACK);
            colour_string("Enter", 33, 232, BLUE);
            return;
        } else if (cursorPosition > (secondRowLength + 3)) {
            colour_string("Shift", 69, 232, BLACK);
            colour_string("Enter", 33, 232, BLACK);
            cursorPosition = 0;
            cursorRow = 0;
        } else if (cursorPosition < 0) {
            cursorPosition = (firstRowLength - 1);
            cursorRow = 0;
        }
    }

    colour_tile(cursorPosition, cursorRow, BLUE);
}

void colour_tile(int8_t position, uint8_t row, uint16_t col)
{
    /* Colour a location on the keyboard with a new background colour */
    uint16_t oldCol = display.background;

    uint16_t x, y;
    if (row == 0) {
        y = 224;
    } else {
        y = 232;
    }
    x = (position * 12) + 3;

    /* Colour the background of the letter */
    rectangle r = {(x - 3), (x + 8), (y - 1), (y + 7)};
    fill_rectangle(r, col);

    /* Redraw the character with the correct background */
    set_background(col);
    if (row == 0) {
        if (shiftOn) {
            display_char_xy(row0A[position], x, y);
        } else {
            display_char_xy(row0[position], x, y);
        }
    } else {
        if (shiftOn) {
            display_char_xy(row1A[position], x, y);
        } else {
            display_char_xy(row1[position], x, y);
        }
    }
    set_background(oldCol);
}

void colour_string(char *str, uint16_t backwardsX, uint16_t forwardsY, uint16_t col)
{
    /* Same as recolouring a tile, however for the special buttons */
    uint16_t oldCol = display.background;

    uint16_t x, y;
    x = display.width - backwardsX;
    y = forwardsY;

    rectangle r = {(x - 3), (x + (strlen(str) + 2)), (y - 1), (y + 7)};
    fill_rectangle(r, col);

    set_background(col);
    display_string_xy(str, x, y);
    set_background(oldCol);
}

void move_screen(void)
{
    /* Shift the screen up if it is within three lines of the bottom of the screen to make a place for the keyboard */
    uint8_t linesFromBottom = lines_from_bottom();
    if (linesFromBottom < 3) {
        if (linesFromBottom > 0) {
            shift_up_lines(3 - linesFromBottom);
        } else {
            shift_up_lines(3);
        }
    }
}

void move_screen_back(void)
{
    /* Return the screen to the location before it was shifted */
    shift_back_down();
}
