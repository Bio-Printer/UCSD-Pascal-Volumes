/* pins.c -- Tiny-C library: <pins.h>, the RP2040's GPIO pins through the ucsd-pico firmware
   (and an HC-SR04's distance: TIME with 25616 and trig | echo << 8 answers its echo in us).
   The firmware answers CSP 9 TIME called with its first variable 25614 and its second a
   request word, op << 14 | option << 12 | arg << 8 | pin, with the first variable the status
   and the second the value (UCSD-Pascal-on-RP2040 port/Gpio.h).  Whether the firmware is
   there is asked once, by its machine information (25613, 0x7C00: 'RP' in the high word). */
#include "libint.h"
#include <pins.h>

int pinerr;
static int __pinfw;                     /* 0 not asked yet, 1 ucsd-pico, -1 not */

static int pinfw(void)
{
    unsigned hi, lo;
    if (__pinfw == 0) {
        hi = 25613;
        lo = 0x7C00;
        __cspv(9, &hi, &lo);
        __pinfw = hi == 0x5250 ? 1 : -1;
    }
    return __pinfw > 0;
}

/* one request; the status into pinerr, the value returned */
static int pinreq(int op, int option, int arg, int p)
{
    unsigned hi, lo;
    if (!pinfw()) {
        pinerr = PIN_NOFIRMWARE;
        return 0;
    }
    if (p < 0 || p > 255 || arg < 0 || arg > 15 || option < 0 || option > 3) {
        pinerr = p < 0 || p > 255 ? PIN_NOPIN : PIN_BADCFG;
        return 0;
    }
    hi = 25614;
    lo = (op << 14) | (option << 12) | (arg << 8) | p;
    __cspv(9, &hi, &lo);
    pinerr = hi;
    return lo;
}

int setpin(int p, int cfg, int option)
{
    pinreq(0, option, cfg, p);
    return pinerr;
}

int pinwrite(int p, int value)
{
    pinreq(1, 0, value != 0, p);
    return pinerr;
}

int pin(int p)
{
    int v;
    v = pinreq(2, 0, 0, p);
    return pinerr ? -1 : v;
}

char *pinerror(int err)
{
    switch (err) {
    case PIN_OK:         return "no error";
    case PIN_NOPIN:      return "not a GPIO pin (0..29)";
    case PIN_RESERVED:   return "GP0 and GP1 are the console";
    case PIN_BADCFG:     return "no such configuration (AIN: GP26..GP29 only)";
    case PIN_NOTSET:     return "the pin is not set for that (SETPIN)";
    case PIN_NOFIRMWARE: return "no GPIO pins here (not the ucsd-pico firmware)";
    case PIN_NOECHO:     return "no echo: nothing in range, or no sensor on those pins";
    }
    return "unknown pin error";
}

float distance(int trig, int echo)
{
    unsigned hi, lo;
    if (!pinfw()) {
        pinerr = PIN_NOFIRMWARE;
        return -1.0;
    }
    if (trig < 0 || trig > 255 || echo < 0 || echo > 255) {
        pinerr = PIN_NOPIN;
        return -1.0;
    }
    hi = 25616;                         /* the firmware: the echo's microseconds */
    lo = (echo << 8) | trig;
    __cspv(9, &hi, &lo);
    pinerr = hi == 5 ? PIN_NOECHO : hi;
    if (pinerr)
        return -1.0;
    return (int)lo / 58.0;
}

long timer(void)
{
    unsigned hi, lo;
    if (pinfw()) {
        hi = 25613;
        lo = 0x7C00 + 9;                /* the firmware's milliseconds since its start */
        __cspv(9, &hi, &lo);
        return ((long)hi << 16) | (long)lo;
    }
    __cspv(9, &hi, &lo);                /* the clock: 1/60 s */
    return (((long)hi << 16) | (long)lo) * 50L / 3L;
}

void pause(long ms)
{
    long t0;
    t0 = timer();
    while (timer() - t0 < ms)
        ;
}
