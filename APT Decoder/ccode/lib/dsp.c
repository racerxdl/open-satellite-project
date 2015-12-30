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

#include "dsp.h"

void fir(float *coeffs, int coeffsLen, float *input, int inputLen) {
  for (int i = 0; i < inputLen; i++) {
    float acc = 0;
    for ( int x = 0; x < coeffsLen; x++ ) {
      if ( i+x >= inputLen )
        break;
      acc += input[i+x] * coeffs[x];
    }
    input[i] = acc;
  }
}

int resample(float *input, int inputSize, int interpolation, int decimation, float *coeffs, int coeffsLen, float **output) {
  float factor = interpolation / decimation;
  int outputLength = factor * inputSize;

  *output = malloc( outputLength * sizeof(float) );

  float nInput[inputSize];
  memcpy ( nInput, input, inputSize * sizeof(float) );

  fir(coeffs, coeffsLen, nInput, inputSize);

  for (int i = 0; i < outputLength; i++) {
    int idx = floor(i / factor);
    *output[i] = nInput[idx];
  }

  return outputLength;
}

void normalize(float *input, int inputSize) {
  float max = -10000;
  float min =  10000;

  for (int i = 0; i < inputSize; i++) {
    max = input[i] > max ? input[i] : max;
    min = input[i] < min ? input[i] : min;
  }

  if (max != 0 && max != min) {
    for (int i = 0; i < inputSize; i++)
      input[i] = (input[i] - min) / (max - min);
  }
}

float mean(float *input, int inputSize) {
  float sMean = 0;
  for (int i = 0; i < inputSize; i++) {
    sMean += input[i];
  }

  return sMean / inputSize;
}

syncPosition_t *getSync(int start, int range, float dataMean, float *data, int dataLen) {
  syncPosition_t *position = malloc(sizeof(syncPosition_t));
  float sum;

  position->score = 0;
  position->index = 0;

  for (int i = start; i < start + range; i++) {
    sum = 0;

    for(int c = 0; c < syncDataLength; c++) {
      sum += ( data[i+c] - dataMean ) * syncData[c];
    }

    if( sum > position->score ){
      position->score = sum;
      position->index = i;
    }
  }

  return position;
}
