/* pins.h -- the RP2040's GPIO pins, as MMBasic's SETPIN and PIN() (the ucsd-pico firmware:
   UCSD-Pascal-on-RP2040; its simulator psys-sim simulates them).  Elsewhere every call
   says PIN_NOFIRMWARE.

     setpin(25, DOUT, 0);          GP25 an output (the LED of a Pico or a YD-RP2040), low
     pinwrite(25, 1);              ... high
     setpin(24, DIN, PULLUP);      GP24 an input, pulled up (the YD-RP2040's USR button)
     if (pin(24) == 0) ...         its level: 0 or 1 (a pressed button pulls it to 0)
     setpin(26, AIN, 0);           GP26 analog (GP26..GP29 only)
     mv = pin(26);                 its voltage in millivolts, 0..3300
     setpin(25, OFF, 0);           not used again: as after a reset

   GP0 and GP1 are the console's UART: refused.  Every pin is OFF again when the P-System
   starts (and a "restart"); a program's pins stay as they are after it ends.  The monitor's
   "pins" (Ctrl-] pins) lists them. */
#ifndef __PINS_H
#define __PINS_H

/* setpin's configurations (cfg) */
#define OFF       0             /* not used: no output, no pulls */
#define AIN       1             /* an analog input: pin() is millivolts (GP26..GP29) */
#define DIN       2             /* a digital input: pin() is 0 or 1 */
#define DOUT      3             /* a digital output: pinwrite() sets it, pin() what it drives */
/* setpin's options, for DIN */
#define PULLUP    1             /* a weak pull to 3.3 V: an open pin (a button let go) reads 1 */
#define PULLDOWN  2             /* a weak pull to 0 V: an open pin reads 0 */

/* what setpin and pinwrite return, and pinerr after a pin() of -1 */
#define PIN_OK          0
#define PIN_NOPIN       1       /* not a GPIO: 0..29 */
#define PIN_RESERVED    2       /* GP0, GP1: the console */
#define PIN_BADCFG      3       /* no such cfg or option, or AIN on GP2..GP25 */
#define PIN_NOTSET      4       /* pin() of an OFF pin, pinwrite() of a pin not DOUT */
#define PIN_NOFIRMWARE  5       /* not the ucsd-pico firmware: no pins here */
#define PIN_NOECHO      6       /* distance(): no echo -- nothing in range, or no sensor there */

int setpin(int pin, int cfg, int option);       /* PIN_OK or an error */
int pin(int pin);                               /* the pin's value; -1: an error (pinerr) */
int pinwrite(int pin, int value);               /* a DOUT pin's level (0, or 1 for any other) */
extern int pinerr;                              /* the last call's PIN_OK or error */
char *pinerror(int err);                        /* the error's text */

/* an HC-SR04 ultrasonic sensor's distance in centimetres (MMBasic's DISTANCE): a 10 us pulse
   on trig, then the echo pin's high time, measured by the firmware (58 us a centimetre; 2 cm to
   about 400 cm); -1.0 on an error (pinerr: PIN_NOECHO when nothing came back).  The pins become
   DOUT and DIN.  The sensor wants 60 ms between measurements.  Its ECHO is 5 V: through a
   divider (1k / 2k) to the RP2040's 3.3 V pin, or a 3.3 V sensor (HC-SR04P) */
float distance(int trig, int echo);

long timer(void);                               /* milliseconds since the start (ucsd-pico;
                                                   elsewhere the P-System's clock, 1/60 s) */
void pause(long ms);                            /* wait ms milliseconds */

#endif
