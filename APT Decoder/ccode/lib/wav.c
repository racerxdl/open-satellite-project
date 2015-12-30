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


#include "wav.h"


void loadWave(const char *filename, wave_t *wave) {
  wave->file = fopen(filename, "rb");
  int count = fread(&wave->wavefile, WAVEFILE_SIZE, 1, wave->file);

  if (count != WAVEFILE_SIZE) {
    printf("Error reading file %s\n", filename);
    exit(1);
  }

  if (strncmp (wave->wavefile.riff.chunkId, RIFF_CHUNK_ID, RIFF_CHUNK_ID_SIZE) != 0) {
    printf("Error: RIFF Chunk ID not match!\n");
    exit(1);
  }

  if (strncmp (wave->wavefile.format.chunkId, FORMAT_CHUNK_ID, FORMAT_CHUNK_ID_SIZE) != 0) {
    printf("Error: FORMAT Chunk ID not match!\n");
    exit(1);
  }

  if (strncmp (wave->wavefile.data.chunkId, DATA_CHUNK_ID, DATA_CHUNK_ID_SIZE) != 0) {
    printf("Error: DATA Chunk ID not match!\n");
    exit(1);
  }
  wave->currentPosition = 0;
}

int16_t getNextSample(wave_t *wave) {
  const int dataPosition = WAVEFILE_SIZE;
  int16_t sample = 0;
  fseek(wave->file, wave->currentPosition * sizeof(int16_t) + dataPosition, SEEK_SET);
  fread(&sample, sizeof(int16_t), 1, wave->file);
  wave->currentPosition += 1;
  return wave->rectified ? abs(sample) : sample;
}

int getNextFloatBufferSamples(wave_t *wave, float *buffer, int bufferSize) {
  int16_t iBuffer[bufferSize];
  int count = getNextBufferSamples(wave, iBuffer, bufferSize);

  for (int i = 0; i < count; i++) {
    buffer[i] = iBuffer[i] / 32768.0f;
  }

  return count;
}

int getNextBufferSamples(wave_t *wave, int16_t *buffer, int bufferSize) {
  const int dataPosition = WAVEFILE_SIZE;
  fseek(wave->file, wave->currentPosition * sizeof(int16_t) + dataPosition, SEEK_SET);

  int count = fread(buffer, sizeof(int16_t), bufferSize, wave->file);
  wave->currentPosition += count / sizeof(int16_t);

  if (wave->rectified) {
    for (int i = 0; i < count; i++) {
      buffer[i] = abs(buffer[i]);
    }
  }

  return count;
}

void closeWave(wave_t *wave) {
  fclose(wave->file);
  free(wave);
}
