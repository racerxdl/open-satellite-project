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


#include "lib/dsp.h"
#include "lib/wav.h"

#define BUFF_SIZE 11025

void printHelp() {
  printf("Usage: \n");
  printf("./aptdec data.wav\n");
}


int main(int argc, char **argv) {
  if (argc != 2) {
    printHelp();
    return 1;
  }

  wave_t wav;
  loadWave(argv[1], &wav);

  if (!checkAPTCompatible(&wav)) {
    printf("WAV not compatible to APT Decoding. It should be 11025Hz Sample Rate and this file is %u\n", wav.wavefile.format.sampleRate);
    return 1;
  }

  rectified(&wav, 1);

  FILE *out = fopen("out.bin", "wb");
  uint8_t lineData[2080];
  float buffer[BUFF_SIZE];
  float *resampledLine;
  int resampledLength;
  int lineCount = 0;
  int buffPos = 0;
  int read = 0;

  while(1) {
    int read = getNextFloatBufferSamples(&wav, buffer, BUFF_SIZE);
    if (read == 0)
      break;
    fir(lowPass, lowPassLength, buffer, BUFF_SIZE);
    normalize(buffer, BUFF_SIZE);
    resampledLength = resample(buffer, BUFF_SIZE, 4160, 11025, lowPass, lowPassLength, &resampledLine);
    for(int i=0;i<resampledLength;i++) {
      if (buffPos == 2080) {
        fwrite(lineData, 2080, 1, out);
        buffPos = 0;
        lineCount++;
      }
      lineData[buffPos] = (uint8_t)(resampledLine[i] * 256);
      buffPos++;
    }
  }

  printf("Read %u lines.\n", lineCount);

  fclose(out);
  closeWave(&wav);

  return 0;
}