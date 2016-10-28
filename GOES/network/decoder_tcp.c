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
#include <sys/socket.h>
#include <netinet/in.h>
#include "pn.h"

// Header, because I don't want an extra file

#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))
#define clear() printf("\033[H\033[J")

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

void display(uint8_t scid, uint8_t vcid, uint32_t packetNumber, uint16_t vitErrors, uint16_t frameBits, uint32_t *rsErrors,
             uint8_t signalQuality, uint8_t syncCorrelation, uint8_t phaseCorrection,
             uint32_t lostPackets, uint16_t averageVitCorrections, uint8_t averageRSCorrections,
             uint32_t droppedPackets, uint32_t *receivedPacketsPerChannel, uint32_t *lostPacketsPerChannel) {
  gotoxy(0, 0);
  printf("┌─────────────────────────────────────────────────────────────────────────────┐\n");
  printf("|                         LRIT DECODER - Lucas Teske                          |\n");
  printf("|─────────────────────────────────────────────────────────────────────────────|\n");
  printf("|         Current Frame Data           |               Statistics             |\n");
  printf("|──────────────────────────────────────|──────────────────────────────────────|\n");
  printf("|                                      |                                      |\n");
  printf("| SC ID: %3u                           |  Total Lost Packets:    %10u   |\n", scid, lostPackets);
  printf("| VC ID: %3u                           |  Average Viterbi Correction:  %4u   |\n", vcid, averageVitCorrections);
  printf("| Packet Number: %10u            |  Average RS Correction:         %2u   |\n", packetNumber, averageRSCorrections);
  printf("| Viterbi Errors: %4u/%4u bits       |  Total Dropped Packets: %10u   |\n", vitErrors, frameBits, droppedPackets);
  printf("| Signal Quality: %3u%%                 |                                      |\n", signalQuality);
  printf("| RS Errors: %2u %2u %2u %2u               |──────────────────────────────────────|\n", rsErrors[0], rsErrors[1], rsErrors[2], rsErrors[3]);
  printf("| Sync Correlation: %2u                 |             Channel Data             |\n", syncCorrelation);
  printf("| Phase Correction: %3u                |──────────────────────────────────────|\n", phaseCorrection);
  printf("|                                      |  Chan  |   Received   |     Lost     |\n");

  int maxChannels = 8;
  int printedChannels = 0;

  for (int i=0; i<255; i++) {
    if (receivedPacketsPerChannel[i] != -1) {
      printf("|                                      |   %2u   |  %10u  |  %10u  |\n", i, receivedPacketsPerChannel[i], lostPacketsPerChannel[i]);
      printedChannels++;
      if (printedChannels == maxChannels) {
        break;
      }
    }
  }

  for (int i=0; i<maxChannels-printedChannels;i++) {
    printf("|                                      |        |              |              |\n");
  }

  printf("└─────────────────────────────────────────────────────────────────────────────┘");
}

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

#define CSIZE 16

int readDataForChunk(uint8_t *data, uint32_t size, int sockfd) {
  int readBytes = 0;
  int chunkSize;
  int n;
  int retry = 0;
  while (readBytes < size) {
    chunkSize = (size - readBytes) > CSIZE ? CSIZE : size - readBytes;
    n = read(sockfd, data + readBytes, chunkSize);
    if (n > 0) {
      readBytes += n;
    } else {
      return -1;
    }
  }
  return readBytes;
}
#define TRUE 1

int main(int argc,char *argv[]) {

  if (argc < 2) {
    printf("Usage: ./decoder outputfile");
    return 1;
  }
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char *outputfile = argv[1];
  void *viterbi;
  uint32_t averageVitCorrections = 0;
  uint32_t averageRSCorrections = 0;
  uint32_t droppedPackets = 0;
  int32_t lostPacketsPerFrame[256];
  int32_t lastPacketCount[256];
  int32_t receivedPacketsPerFrame[256];
  uint32_t lostPackets = 0;

  struct sockaddr_in serv_addr, cli_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf("ERROR opening socket");
    exit(1);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = 5000;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("ERROR on binding");
    exit(1);
  }
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  for (int i=0; i<256;i++) {
    lostPacketsPerFrame[i] = 0;
    lastPacketCount[i] = -1;
    receivedPacketsPerFrame[i] = -1;
  }
  correlation_t corr;

  printf("Opening output file: %s\n", outputfile);
  FILE *output = fopen(outputfile, "w");
  initUW();

  printf("Initializing Viterbi\n");
  set_viterbi27_polynomial(viterbiPolynomial);
  if((viterbi = create_viterbi27(FRAMEBITS)) == NULL){
    printf("create_viterbi27 failed\n");
    exit(1);
  }

  uint64_t readsize = 0;
  uint64_t frameCount = 1;

  printf("Waiting for connection.\n");
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  printf("Client connected\n");
  clear();
  while (TRUE) {
    // Read Data
    uint32_t chunkSize = CODEDFRAMESIZE;
    memset(codedData, 0x00, chunkSize);
    int dataRead = readDataForChunk(codedData, chunkSize, newsockfd);
    if (dataRead == -1) {
      printf("Client dropped connection!\n");
      break;
    }

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
        memcpy(codedData, shiftedPosition, CODEDFRAMESIZE - p); // Copy from p to chunk size to start of codedData

        readsize += chunkSize; // Add what we processed to readsize.

        uint32_t oldChunkSize = chunkSize;
        chunkSize = p; // Read needed bytes to fill a frame.
        readDataForChunk(codedData + CODEDFRAMESIZE - p, chunkSize, newsockfd);
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
      // Calculate Errors
      convEncode(decodedData, FRAMESIZE, correctedData, CODEDFRAMESIZE);
      uint32_t errors = calculateError(codedData, correctedData, CODEDFRAMESIZE) / 2;
      float signalErrors = (100.f * errors) / FRAMEBITS; // 0 to 16
      signalErrors = 100 - (signalErrors * 10);
      uint32_t signalQuality = signalErrors < 0 ? 0 : signalErrors;
      #endif

      // De-randomization
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
        droppedPackets++;
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
      if (lastPacketCount[vcid]+1 != counter && lastPacketCount[vcid] > -1) {
        int lostCount = counter - lastPacketCount[vcid] + 1;
        lostPackets += lostCount;
        lostPacketsPerFrame[vcid] += lostCount;
      }

      lastPacketCount[vcid] = counter;
      receivedPacketsPerFrame[vcid] = receivedPacketsPerFrame[vcid] == -1 ? 1 : receivedPacketsPerFrame[vcid] + 1;
      uint8_t phaseCorr = (n == 0) || (n == 1) ? 180 : 0;
      uint16_t partialVitCorrections = (uint16_t) (averageVitCorrections / frameCount);
      uint8_t partialRSCorrections = (uint8_t) (averageRSCorrections / frameCount);

      display(scid, vcid, counter, errors, FRAMEBITS, derrors,
           signalQuality, maxCorr, phaseCorr,
           lostPackets, partialVitCorrections, partialRSCorrections,
           droppedPackets, receivedPacketsPerFrame, lostPacketsPerFrame);

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
  printf("Dropped packets: %u\n", droppedPackets);
  printf("Lost packets: %u\n", lostPackets);
  printf("Lost packets per channel: \n");
  for (int i=0;i<256;i++) {
    if (lastPacketCount[i] != -1) {
      printf("    Channel %u: %u\n", i, lostPacketsPerFrame[i]);
    }
  }

  fclose(output);
  return 0;
}