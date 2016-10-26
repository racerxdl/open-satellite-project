/*
    GOES LRIT Decoder
    Copyright (C) 2016 Lucas Teske <lucas {at} teske {dot] net [dot} br>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    For compiling this program you need libfec: https://github.com/quiet/libfec
    Compile:
      gcc decoder.c -lfec -lm -o decoder
    Run:
      ./decoder demodout.bin test.bin
    TODO: More detailed instructions
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fec.h>
#include "pn.h"

// Header, because I don't want an extra file

#define FRAMESIZE 1024
#define SYNCWORDSIZE 32
#define MINCORRELATIONBITS 46
#define FRAMEBITS (FRAMESIZE * 8)

#define RSBLOCKS 4
#define RSPARITYSIZE 32
#define RSPARITYBLOCK (RSPARITYSIZE * RSBLOCKS)

#define VITPOLYA 0x4F
#define VITPOLYB 0x6D

#define CHECK_VITERBI_CORRECTIONS

#define CODEDFRAMESIZE (FRAMEBITS * 2)
#define SYNCWORDSIZEDOUBLE (SYNCWORDSIZE * 2)
#define PARITY_OFFSET 892

const uint64_t UW0 = 0xfca2b63db00d9794;
const uint64_t UW2 = 0x035d49c24ff2686b;
const uint64_t REVUW0 = 0xfc51793e700e6b68;
const uint64_t REVUW2 = 0x03ae86c18ff19497;

typedef struct {
  uint32_t uw0mc;
  uint32_t uw0p;
  uint32_t uw2mc;
  uint32_t uw2p;
  uint32_t ruw0mc;
  uint32_t ruw0p;
  uint32_t ruw2mc;
  uint32_t ruw2p;
} correlation_t ;

uint8_t UW0b[SYNCWORDSIZE * 2];
uint8_t UW2b[SYNCWORDSIZE * 2];
uint8_t REVUW0b[SYNCWORDSIZE * 2];
uint8_t REVUW2b[SYNCWORDSIZE * 2];

uint8_t codedData[CODEDFRAMESIZE];
uint8_t decodedData[FRAMESIZE];
uint8_t correctedData[CODEDFRAMESIZE];
uint8_t rsCorrectedData[FRAMESIZE];
uint8_t rsWorkBuffer[255];

uint8_t M_PDU[886];

int viterbiPolynomial[2] = {VITPOLYA, VITPOLYB};


void initUW();
void checkCorrelation(uint8_t *buffer, int buffLength, correlation_t *corr);
void resetCorrelation(correlation_t * corr);
uint32_t hardCorrelate(uint8_t dataByte, uint8_t wordByte);
void fixPacket(uint8_t *buffer, int buffLength, uint8_t n);
void convEncode(uint8_t *data, int dataLength, uint8_t *output, int outputLen);
uint32_t calculateError(uint8_t *original, uint8_t *corrected, int length);
void deinterleaveRS(uint8_t *data, uint8_t *rsbuff, uint8_t pos, uint8_t I);
void interleaveRS(uint8_t *idata, uint8_t *outbuff, uint8_t pos, uint8_t I);
void writeChannel(uint8_t *data, int size, uint16_t vcid);
// Code

uint32_t swapEndianess(uint32_t num) {
  return  ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000);
}

uint32_t maxCorrelation(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t f = (a > b ? a : b);
  uint32_t g = (c > d ? c : d);
  return f > g ? f : g;
}


void initUW() {
  printf("Converting Sync Words to Soft Data\n");
  for (int i = 0; i < SYNCWORDSIZEDOUBLE; i++) {
    UW0b[i] = (UW0 >> (SYNCWORDSIZEDOUBLE-i-1)) & 1 ? 0xFF : 0x00;
    UW2b[i] = (UW2 >> (SYNCWORDSIZEDOUBLE-i-1)) & 1 ? 0xFF : 0x00;
    REVUW0b[i] = (REVUW0 >> (SYNCWORDSIZEDOUBLE-i-1)) & 1 ? 0xFF : 0x00;
    REVUW2b[i] = (REVUW2 >> (SYNCWORDSIZEDOUBLE-i-1)) & 1 ? 0xFF : 0x00;
  }
}

uint32_t hardCorrelate(uint8_t dataByte, uint8_t wordByte) {
  //1 if (a        > 127 and       b == 255) or (a        < 127 and       b == 0) else 0
  return (dataByte >= 127 & wordByte == 0) | (dataByte < 127 & wordByte == 255);
}

void checkCorrelation(uint8_t *buffer, int buffLength, correlation_t *corr) {
  resetCorrelation(corr);
  for (int i = 0; i < buffLength - SYNCWORDSIZEDOUBLE; i++) {
    uint32_t uw0c = 0;
    uint32_t uw2c = 0;
    uint32_t ruw0c = 0;
    uint32_t ruw2c = 0;

    for (int k = 0; k < SYNCWORDSIZEDOUBLE; k++) {
      uw0c += hardCorrelate(buffer[i+k], UW0b[k]);
      uw2c += hardCorrelate(buffer[i+k], UW2b[k]);
      ruw0c += hardCorrelate(buffer[i+k], REVUW0b[k]);
      ruw2c += hardCorrelate(buffer[i+k], REVUW2b[k]);
    }

    corr->uw0p = uw0c > corr->uw0mc ? i : corr->uw0p;
    corr->uw2p = uw2c > corr->uw2mc ? i : corr->uw2p;
    corr->ruw0p = ruw0c > corr->ruw0mc ? i : corr->ruw0p;
    corr->ruw2p = ruw2c > corr->ruw2mc ? i : corr->ruw2p;

    corr->uw0mc = uw0c > corr->uw0mc ? uw0c : corr->uw0mc;
    corr->uw2mc = uw2c > corr->uw2mc ? uw2c : corr->uw2mc;
    corr->ruw0mc = ruw0c > corr->ruw0mc ? ruw0c : corr->ruw0mc;
    corr->ruw2mc = ruw2c > corr->ruw2mc ? ruw2c : corr->ruw2mc;
  }
}

void resetCorrelation(correlation_t * corr) {
  memset(corr, 0x00, sizeof(correlation_t));
}

void fixPacket(uint8_t *buffer, int buffLength, uint8_t n) {
  if (n != 0) {
    for (int i=0; i < buffLength; i+=2) {
      if (n % 2) {  // Process IQ Inversion
        char a = buffer[i];
        buffer[i] = buffer[i+1];
        buffer[i+1] = a;
      }

      if (n >= 4) { // Process 180 phase shift, aka inverted bits
        buffer[i] ^= 0xFF;
        buffer[i+1] ^= 0xFF;
      }
    }
  }
}

uint32_t calculateError(uint8_t *original, uint8_t *corrected, int length) {
  uint32_t errors = 0;
  for (int i=0; i<length; i++) {
    errors += hardCorrelate(original[i], ~corrected[i]);
  }

  return errors;
}

void convEncode(uint8_t *data, int dataLength, uint8_t *output, int outputLen) {
  unsigned int encstate = 0;
  uint8_t c;
  uint32_t pos = 0;
  uint32_t opos = 0;

  memset(output, 0x00, outputLen);
  while (pos < dataLength && (pos * 16) < outputLen) {
    c = data[pos];
    for(int i=7;i>=0;i--){
      encstate = (encstate << 1) | ((c >> 7) & 1);
      c <<= 1;
      output[opos]   = ~(0 - parity(encstate & viterbiPolynomial[0]));
      output[opos+1] = ~(0 - parity(encstate & viterbiPolynomial[1]));

      opos += 2;
    }
    pos++;
  }
}

void printBuff(uint8_t *buff, int length) {
  int countlen = (length > 40 ? 40 : length);
  for (int i=0; i<countlen; i++) {
    if (i % 8 == 0 && i != 0) {
      printf("\n");
    }
    printf("%d ", buff[i]);
  }
  printf("\n");
}

void deinterleaveRS(uint8_t *data, uint8_t *rsbuff, uint8_t pos, uint8_t I) {
  // Copy data
  for (int i=0; i<223; i++) {
    rsbuff[i] = data[i*I + pos];
  }
  // Copy parity
  for (int i=0; i<32; i++) {
    rsbuff[i+223] = data[PARITY_OFFSET + i*I + pos];
  }
}

void interleaveRS(uint8_t *idata, uint8_t *outbuff, uint8_t pos, uint8_t I) {
  // Copy data
  for (int i=0; i<223; i++) {
    outbuff[i*I + pos] = idata[i];
  }
  // Copy parity - Not needed here, but I do.
  for (int i=0; i<32; i++) {
    outbuff[PARITY_OFFSET + i*I + pos] = idata[i+223];
  }
}

void writeChannel(uint8_t *data, int size, uint16_t vcid) {
  char filename[256];
  sprintf(filename, "channels/channel_%d.bin", vcid);
  FILE *f = fopen(filename, "a+");
  fwrite(data, size, 1, f);
  fclose(f);
}

int main(int argc,char *argv[]) {

  if (argc < 3) {
    printf("Usage: ./decoder inputfile outputfile");
    return 1;
  }

  char *inputfile = argv[1];
  char *outputfile = argv[2];
  void *viterbi;
  uint32_t averageVitCorrections = 0;
  uint32_t averageRSCorrections = 0;
  correlation_t corr;

  printf("Opening files\n");
  FILE *input = fopen(inputfile, "r");
  FILE *output = fopen(outputfile, "w");

  initUW();

  printf("Initializing Viterbi\n");
  set_viterbi27_polynomial(viterbiPolynomial);
  if((viterbi = create_viterbi27(FRAMEBITS)) == NULL){
    printf("create_viterbi27 failed\n");
    exit(1);
  }

  fseek(input, 0L, SEEK_END);
  uint64_t sz = ftell(input);
  fseek(input, 0L, SEEK_SET);
  printf("Input size is %lu\n", sz);

  uint64_t readsize = 0;
  uint64_t frameCount = 1;

  while (readsize < sz) {
    // Read Data
    uint32_t chunkSize = sz - readsize > CODEDFRAMESIZE ? CODEDFRAMESIZE : sz - readsize;
    memset(codedData, 0x00, CODEDFRAMESIZE);
    fread(codedData, chunkSize, 1, input);

    //printBuff(codedData, chunkSize);
    // Check Correlation
    checkCorrelation(codedData, chunkSize, &corr);
    // Get Max Correlation
    uint32_t maxCorr = maxCorrelation(corr.uw0mc, corr.uw2mc, corr.ruw0mc, corr.ruw2mc);

    if (maxCorr < MINCORRELATIONBITS) {
      printf("  Skipping read. Correlation %d less than required %d.\n", maxCorr, MINCORRELATIONBITS);
    } else {
      // Check Phase Shifting and Position
      uint8_t n;
      uint32_t p;

      if (maxCorr == corr.uw0mc) {
        n = 0;
        p = corr.uw0p;
      } else if (maxCorr == corr.uw2mc) {
        n = 4;
        p = corr.uw2p;
      } else if (maxCorr == corr.ruw0mc) {
        n = 1;
        p = corr.ruw0p;
      } else if (maxCorr == corr.ruw2mc) {
        n = 5;
        p = corr.ruw2p;
      }

      if (p != 0) {
        // Shift position
        char *shiftedPosition = codedData + p;
        //printf("  Missing bytes for frame: %d\n", p);
        memcpy(codedData, shiftedPosition, CODEDFRAMESIZE - p); // Copy from p to chunk size to start of codedData

        readsize += chunkSize; // Add what we processed to readsize.

        uint32_t oldChunkSize = chunkSize;
        chunkSize = (sz - readsize) > p ? p : (sz - readsize); // Read needed bytes to fill a frame.
        //printf("  Reading for frame missing bytes: %d\n", chunkSize);
        fread(codedData + CODEDFRAMESIZE - p, chunkSize, 1, input);
      }

      // Correct Frame Phase
      //printf("  Fixing packet.\n");
      fixPacket(codedData, CODEDFRAMESIZE, n);

      // Viterbi
      //printf("  Decoding using viterbi.\n");
      init_viterbi27(viterbi, 0);
      update_viterbi27_blk(viterbi, codedData, FRAMEBITS + 6);
      chainback_viterbi27(viterbi, decodedData, FRAMEBITS, 0);

      #ifdef CHECK_VITERBI_CORRECTIONS
      //printf("  Re-encoding.\n");
      // Calculate Errors
      convEncode(decodedData, FRAMESIZE, correctedData, CODEDFRAMESIZE);
      //printf("  Calculating errors.\n");
      uint32_t errors = calculateError(codedData, correctedData, CODEDFRAMESIZE) / 2;
      float signalErrors = (100.f * errors) / FRAMEBITS; // 0 to 16
      signalErrors = 100 - (signalErrors * 10);
      uint32_t signalQuality = signalErrors < 0 ? 0 : signalErrors;
      #endif

      // De-randomization
      //printf("  De-randomizing data.\n");
      uint8_t skipsize = (SYNCWORDSIZE/8);
      memcpy(decodedData, decodedData + skipsize, FRAMESIZE-skipsize);
      for (int i=0; i<FRAMESIZE-skipsize; i++) {
        decodedData[i] ^= pn[i];
      }

      // Reed-solomon
      int derrlocs[255];
      int derrors[4] = { 0, 0, 0, 0 };

      for (int i=0; i<RSBLOCKS; i++) {
        deinterleaveRS(decodedData, rsWorkBuffer, i, RSBLOCKS);
        derrors[i] = decode_rs_ccsds(rsWorkBuffer, derrlocs, 0, 0);
        interleaveRS(rsWorkBuffer, rsCorrectedData, i, RSBLOCKS);
      }

      // If RS returns -1 for all 4 RS Blocks,
      if (derrors[0] == -1 && derrors[1] == -1 && derrors[2] == -1 && derrors[3] == -1) {
        #ifdef CHECK_VITERBI_CORRECTIONS
        printf("  Viterbi Errors: %u/%u bits\n", errors, FRAMEBITS);
        printf("  Signal Quality: %u%%\n", signalQuality);
        printf("  Sync Correlation: %d\n", maxCorr);
        #endif
        printf("  Corrupted packet, RS Cannot correct. Dropping.\n");
        goto packet_process_end;
      } else {
        averageRSCorrections += derrors[0] != -1 ? derrors[0] : 0;
        averageRSCorrections += derrors[1] != -1 ? derrors[1] : 0;
        averageRSCorrections += derrors[2] != -1 ? derrors[2] : 0;
        averageRSCorrections += derrors[3] != -1 ? derrors[3] : 0;
      }

      // Packet Header Filtering
      uint8_t versionNumber = (*rsCorrectedData) & 0xC0 >> 6;
      uint8_t scid = ((*rsCorrectedData) & 0x3F) << 2 | (*(rsCorrectedData+1) & 0xC0) >> 6;
      uint8_t vcid = (*(rsCorrectedData+1)) & 0x3F;

      // Packet Counter from Packet
      uint32_t counter = *((uint32_t *) (rsCorrectedData+2));
      counter = swapEndianess(counter);
      counter &= 0xFFFFFF00;
      counter = counter >> 8;
      writeChannel(rsCorrectedData, FRAMESIZE - RSPARITYBLOCK - (SYNCWORDSIZE/8), vcid);

      if (vcid == 63) {
        //printf("Empty Frame. Discarding.\n");
        //fwrite(rsCorrectedData, FRAMESIZE - skipsize, 1, fill);
      } else { //if (vcid == 42) {
        fwrite(rsCorrectedData, FRAMESIZE - skipsize, 1, output);
        printf("Frame %lu.\n", frameCount);
        printf("  Version Number: %u\n", versionNumber);
        printf("  S/C ID: %u\n", scid);
        printf("  VC ID: %u\n", vcid);
        printf("  Packet Number: %u\n", counter);
        #ifdef CHECK_VITERBI_CORRECTIONS
        printf("  Viterbi Errors: %u/%u bits\n", errors, FRAMEBITS);
        printf("  Signal Quality: %u%%\n", signalQuality);
        #endif
        printf("  RS Errors: %d %d %d %d\n", derrors[0], derrors[1], derrors[2], derrors[3]);
        printf("  Sync Correlation: %d\n", maxCorr);
        switch(n) {
          case 0: printf("  Max Correlation with 0   degrees Word at %u\n", p); break;
          case 4: printf("  Max Correlation with 180 degrees Word at %u\n", p); break;
          case 1: printf("  Max Correlation with 0   degrees Word at %u\n with IQ Reversal", p); break;
          case 5: printf("  Max Correlation with 180 degrees Word at %u\n with IQ Reversal", p); break;
        }
      }
packet_process_end:
      averageVitCorrections += errors;
      frameCount++;
    }

    readsize += chunkSize;
  }

  averageVitCorrections /= frameCount;
  averageRSCorrections /= frameCount * 4;

  printf("Average Viterbi Correction: %u\n", averageVitCorrections);
  printf("Average ReedSolomon Correction: %u\n", averageRSCorrections);

  fclose(input);
  fclose(output);
  return 0;
}