# La-Fortuna-Keyboard-Input
An on screen keyboard that returns a string that fits on the screen.

###Dependencies
* avr-gcc

###How to Build
1. Run the make file that is supplied, which should work fine on Linux and will work on Windows if run through Cygwin or another bash like terminal

###Usage
As shown in main.c, all oyu have to do is set some basic variables and then call os_open_keyboard() which will open the keyboard onto the screen. This makes use the library that I created for scrolling on the LCD screen. This means that when you open the keyboard it will shift the lines already on the screen up if they are in the way, and then it will put them back when it closes.

To implement the keyboard in another application, you only have to call at least one of the left or right moving methods, either for the rotary switch or for the arrow keys. In the example I make use of the ruota library for the input of the buttons.

To implement the keyboard library in a stand alone application have a look at the example in standalone._c. This shows how to implement the keyboard without having to use the ruota library.

This library requires you to use the supplied LCD library.

The keyboard is fully customizable up to an extent. By this I mean that you can edit the keyboard characters at the top of keyboard.c . There is a limit on the number of characters that will be displayed, however if you add more than can be displayed, it will not break the code and it should still work completely. The alternate character sets are for when the 'Shift' button is selected, where it will show these characters instead.

You have to supply a pointer to a character array which will only be returned containing up to 54 characters which is the total amount that can be displayed on the La Fortuna when it is in landscape orientation.
