#ifndef SPTK_H
#define SPTK_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

char *getmem(const size_t leng, const size_t size);

short *sgetmem(const int leng);

long *lgetmem(const int leng);

double *dgetmem(const int leng);

float *fgetmem(const int leng);

float **ffgetmem(const int leng);

double **ddgetmem(const int leng1, const int leng2);

void movem(void *a, void *b, const size_t size, const int nitem);

void ignorm(double *c1, double *c2, int m, const double g);

void gnorm(double *c1, double *c2, int m, const double g);

void fillz(void *ptr, const size_t size, const int nitem);

void freqt(double *c1, const int m1, double *c2, const int m2, const double a);

int checkm(const int m);

int fft(double *x, double *y, const int m, int *maxfftsize_, double *_sintbl);

int fftr(double *x, double *y, const int m);

void gc2gc(double *c1, const int m1, const double g1, double *c2, const int m2, const double g2);

void c2sp(double *c, const int m, double *x, double *y, const int l);

void mgc2mgc(double *c1, const int m1, const double a1, const double g1,
             double *c2, const int m2, const double a2, const double g2);

void mgc2sp(double *mgc, const int m, const double a, const double g, double *x,
            double *y, const int flng);

#endif // SPTK_H

