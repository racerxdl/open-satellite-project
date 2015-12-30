/**
 *   ___  ____  ____
 *  / _ \/ ___||  _ \
 * | | | \___ \| |_) |
 * | |_| |___) |  __/
 *  \___/|____/|_|
 *
 * OpenSatelliteProject - A OpenSource Satellite Study
 * Copyright (C) 2015  Lucas Teske < lucas at teske dot br dot net >
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef _DSP_H_
#define _DSP_H_

/**
 *  The Sync A signal is a 7 cycle 1040Hz sine inside each line.
 *  This array represents the 1040Hz signal in 11025 Sample Rate
 */
extern const float syncData[];

extern const int syncDataLength;


/**
 *  Low Pass FIR Filter Coefficients.
 *  Calculated with 11025Hz Sample Rate, 1200Hz Half Amplitude Frequency, 50 taps
 */
extern const float lowPass[];

extern const int lowPassLength;

typedef struct syncPosition {
  int index;
  float score;
} syncPosition_t;

void fir(const float *coeffs, const int coeffsLen, float *input, int inputLen);
int resample(float *input, int inputSize, int interpolation, int decimation, const float *coeffs, const int coeffsLen, float **output);
void normalize(float *input, int inputSize);
float mean(float *input, int inputSize);
syncPosition_t *getSync(int start, int range, float dataMean, float *data, int dataLen);

#endif