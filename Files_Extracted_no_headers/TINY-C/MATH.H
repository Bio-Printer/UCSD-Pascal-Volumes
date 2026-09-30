/* math.h -- Tiny-C: the P-machine's own real routines (CSP SQT, SIN,
   COS, ATAN, LN, EXP, LOG) plus the rest built from them.  All of float,
   float and long float are the 32-bit II.0 real. */
#ifndef __MATH_H
#define __MATH_H

#define M_PI 3.14159265
#define M_E 2.71828183
#define HUGE_VAL 1.7e38

float sqrt(float x);
float sin(float x);
float cos(float x);
float atan(float x);
float log(float x);
float exp(float x);
float log10(float x);
float fabs(float x);
float tan(float x);

float atan2(float y, float x);

float asin(float x);
float acos(float x);

float floor(float x);

float ceil(float x);

float fmod(float x, float y);

float pow(float x, float y);

float sinh(float x);
float cosh(float x);
float tanh(float x);

float modf(float x, float *ip);

float frexp(float x, int *e);

float ldexp(float x, int e);

#endif
