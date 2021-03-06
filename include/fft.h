/*
 * 	File name: fft.h
 * 	Created: Jan 16, 2020 at 9:26 PM
 * 	Author: ckevar 
 * 	Reference: "Real-Time Digital Signal Processing Implementation and Application, 2nd Ed" by Sen et al.
 */
#ifndef FFT_H
#define FFT_H

#include "fcomplex.h"

// #define L 	256			// Filter length
// #define M 	769			// Data sample length
// #define N 	1024 		// number of frequencies of the FFT, N >= L + M - 1;
// #define EXP 10			// Exp = log2(N)
// twiddle N - 1 			// twiddle factor

/* ALORITHM FFT */
// 1. generate twiddle factor (createTwiddleTable)
// 2. generate or capture samples (not in the scope of this lib)
// 3. compute reverse bit (bit_rev)
// 4. compute dft (dft)

/* ALGORITHM FAST CONVOLUTION */
/*	Considering you already have the spectrum of the filter */
// 1. generate twiddle factor (createTwiddleTable)
// 2. generate or capture samples (not in the scope of this lib)
// 3. compute reverse bit (bit_rev)
// 4. compute dft (dft)
// 5. apply the filter (freqfilter)
// 6. compute reverse bit (bit_rev)
// 7. compute idft (idft)

void createTwiddleTable(complex *twiddle, short exp); 		// creates twiddle table
void bit_rev(complex *X, short exp); 			// performs bit-reverse operation
void dft(complex *X, unsigned short exp, complex *W);
void idft(complex *X, unsigned short exp, complex *W);
void freqfilter(complex *x, complex *h, unsigned short n); 		//	multiplication
void olap_add(complex *x, short *o, unsigned short l, unsigned short m, unsigned short n);

#endif


/****************

*/