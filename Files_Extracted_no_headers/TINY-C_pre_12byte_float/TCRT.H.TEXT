/* tcrt.h -- Tiny-C runtime helpers.
 *
 * The compiler turns operations the II.0 P-machine has no instruction
 * for into calls to these functions, and appends this file to every
 * program; the linker keeps only the ones a program uses.  They are
 * written with operations that map directly onto P-code, and __dvi()
 * (the raw DVI instruction, only ever given non-negative operands).
 */
#ifndef __TCRT_H
#define __TCRT_H
#pragma segment MAIN

unsigned __udiv(unsigned a, unsigned b);

unsigned __umod(unsigned a, unsigned b);

int __divi(int a, int b);

int __modi(int a, int b);

int __shl(int a, int n);

unsigned __ushr(unsigned a, int n);

int __shr(int a, int n);

int __xor(int a, int b);

int __sx(int c);

double __utof(unsigned u);

unsigned __ftou(double f);

/* ---- 32-bit long: word 0 is the low half ---- */

long __ladd(long a, long b);

long __lsub(long a, long b);

long __lneg(long a);

long __lnot(long a);

long __land(long a, long b);

long __lor(long a, long b);

long __lxor(long a, long b);

/* 16 x 16 -> 32 unsigned multiply into r[0] (low), r[1] */
void __mul32(unsigned u, unsigned v, unsigned *r);

long __lmul(long a, long b);

int __ulcmp(unsigned long a, unsigned long b);

int __lcmp(long a, long b);

/* unsigned 32-bit divide: q = n / d, r = n % d (word pairs) */
void __udiv32(unsigned *n, unsigned *d, unsigned *q, unsigned *r);

unsigned long __uldiv(unsigned long a, unsigned long b);

unsigned long __ulmod(unsigned long a, unsigned long b);

long __ldiv(long a, long b);

long __lmod(long a, long b);

long __lshl(long a, int n);

unsigned long __ulshr(unsigned long a, int n);

long __lshr(long a, int n);

long __itol(int i);

long __utol(unsigned u);

int __ltoi(long a);

double __ltof(long a);

double __ultof(unsigned long a);

unsigned long __ftoul(double f);

long __ftol(double f);

#endif
