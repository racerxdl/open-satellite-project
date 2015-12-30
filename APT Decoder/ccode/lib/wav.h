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
#include <stdio.h>
#include <inttypes.h>

#ifndef _WAV_H_
#define _WAV_H_

#define APT_SAMPLE_RATE 11025

#define RIFF_CHUNK_ID_SIZE 4
extern const char RIFF_CHUNK_ID[RIFF_CHUNK_ID_SIZE];

#define FORMAT_CHUNK_ID_SIZE 3
extern const char FORMAT_CHUNK_ID[FORMAT_CHUNK_ID_SIZE];

#define DATA_CHUNK_ID_SIZE 4
extern const char DATA_CHUNK_ID[DATA_CHUNK_ID_SIZE];

typedef struct {
  char chunkId[4];
  uint32_t chunkSize;
  char format[4];
} wav_riff_t;

typedef struct {
  char chunkId[4];
  uint32_t chunkSize;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
} wav_format_t;

typedef struct {
  char chunkId[4];
  uint32_t chunkSize;
} wav_data16_t;

typedef struct {
  wav_riff_t riff;
  wav_format_t format;
  wav_data16_t data;
} wavefile_t;

#define WAVEFILE_SIZE sizeof(wavefile_t)

typedef struct {
  FILE *file;
  uint32_t currentPosition;
  uint8_t rectified;

  wavefile_t wavefile;
} wave_t;

void loadWave(const char *filename, wave_t *wave);
int16_t getNextSample(wave_t *wave);
int getNextFloatBufferSamples(wave_t *wave, float *buffer, int bufferSize);
int getNextBufferSamples(wave_t *wave, int16_t *buffer, int bufferSize);
void closeWave(wave_t *wave);


inline float getNextFloatSample(wave_t *wave) { return getNextSample(wave) / 32768.0f; }
inline uint8_t checkAPTCompatible(wave_t *wave) { return wave->wavefile.format.sampleRate == APT_SAMPLE_RATE; };
inline void rectified(wave_t *wave, uint8_t rectified) { wave->rectified = rectified; };
inline void setPosition(wave_t *wave, uint32_t position) { wave->currentPosition = position; };

#endif