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

 /**
 *  The Sync A signal is a 7 cycle 1040Hz sine inside each line.
 *  This array represents the 1040Hz signal in 11025 Sample Rate
 */
const float syncData[] = {
   0.000000000,  0.557489439,  0.925637660,  0.979409768,  0.700543038,
   0.183749518, -0.395451207, -0.840344072, -0.999829250, -0.819740483,
  -0.361241666,  0.219946358,  0.726433574,  0.986200747,  0.911022649,
   0.526432163, -0.036951499, -0.587785252, -0.938988361, -0.971281032,
  -0.673695644, -0.147301698,  0.429120609,  0.859799851,  0.998463604,
   0.798017227,  0.326538713, -0.255842778, -0.751331890, -0.991644696,
  -0.895163291, -0.494655843,  0.073852527,  0.617278221,  0.951056516,
   0.961825643,  0.645928062,  0.110652682, -0.462203884, -0.878081248,
  -0.995734176, -0.775203976, -0.291389747,  0.291389747,  0.775203976,
   0.995734176,  0.878081248,  0.462203884, -0.110652682, -0.645928062,
  -0.961825643, -0.951056516, -0.617278221, -0.073852527,  0.494655843,
   0.895163291,  0.991644696,  0.751331890,  0.255842778, -0.326538713,
  -0.798017227, -0.998463604, -0.859799851, -0.429120609,  0.147301698,
   0.673695644,  0.971281032,  0.938988361,  0.587785252,  0.036951499,
  -0.526432163, -0.911022649, -0.986200747, -0.726433574, -0.219946358
};

const int syncDataLength = 75;


/**
 *  Low Pass FIR Filter Coefficients.
 *  Calculated with 11025Hz Sample Rate, 1200Hz Half Amplitude Frequency, 50 taps
 */
const float lowPass[] = {
  -0.000016540625438210554, -0.00004612725751940161, -0.000003658998139144387,  0.00020216309349052608,
   0.000544912938494235300,  0.00079004385042935610,  0.000547181000001728500, -0.00048261866322718560,
  -0.002150485524907708000, -0.00363161996938288200, -0.003605136647820472700, -0.00095574476290494200,
   0.004239066969603300000,  0.01002142205834388700,  0.012828110717236996000,  0.00902324449270963700,
  -0.002744758035987615600, -0.01950957253575325000, -0.033640831708908080000, -0.034976765513420105,
  -0.015012636780738830000,  0.02854425460100174000,  0.089116714894771580000,  0.15218372642993927000,
   0.199898496270179750000,  0.21767434477806090000,  0.199898496270179750000,  0.15218372642993927000,
   0.089116714894771580000,  0.02854425460100174000, -0.015012636780738830000, -0.03497676551342010500,
  -0.033640831708908080000, -0.01950957253575325000, -0.002744758035987615600,  0.00902324449270963700,
   0.012828110717236996000,  0.01002142205834388700,  0.004239066969603300000, -0.00095574476290494200,
  -0.003605136647820472700, -0.00363161996938288200, -0.002150485524907708000, -0.00048261866322718560,
   0.000547181000001728500,  0.00079004385042935610,  0.000544912938494235300,  0.00020216309349052608,
  -0.000003658998139144387, -0.00004612725751940161, -0.000016540625438210554
};

const int lowPassLength = 51;


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
