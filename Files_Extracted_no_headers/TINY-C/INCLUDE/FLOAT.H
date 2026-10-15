/* float.h -- Tiny-C: float is the 32-bit UCSD II.0 real (24-bit mantissa,
   exponent -127..127 binary); double and long double are the
   8-byte type (IEEE binary64; P-Code mode only) */
#ifndef __FLOAT_H
#define __FLOAT_H
#define FLT_RADIX 2
#define FLT_MANT_DIG 24
#define FLT_DIG 6
#define FLT_EPSILON 1.19209290e-7
#define FLT_MAX 1.7e38
#define FLT_MIN 5.9e-39
#define DBL_MANT_DIG 53
#define DBL_DIG 15
#define DBL_EPSILON 2.2204460492503131e-16L
#define DBL_MAX 1.7976931348623157e308L
#define DBL_MIN 2.2250738585072014e-308L
#define LDBL_MANT_DIG 53
#define LDBL_DIG 15
#define LDBL_EPSILON 2.2204460492503131e-16L
#define LDBL_MAX 1.7976931348623157e308L
#define LDBL_MIN 2.2250738585072014e-308L
#endif
