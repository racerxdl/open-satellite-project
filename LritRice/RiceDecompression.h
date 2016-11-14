/*
 * RiceDecompression.h
 *
 *  Created on: 13/11/2016
 *      Author: Lucas Teske
 */

#ifndef RICEDECOMPRESSION_H_
#define RICEDECOMPRESSION_H_

#include <stdint.h>
#include "PacketDecompression.h"

class CRiceDecompression: public CPacketDecompression {
public:
	CRiceDecompression(int Mask, int BitsPerPixel, int PixelsPerBlock,
			int PixelsPerScanline, int ScanLinesPerPacket);
private:
	enum {
		EC_MODE = 0, NN_MODE = 1
	};
	enum {
		MAX_EXT2 = 7, MAX_EXT2_SUM = MAX_EXT2 * (MAX_EXT2 + 1) / 2 + MAX_EXT2
	};
	enum {
		MAX_BLOCKS_PER_SCANLINE = 2048,
		MAX_PIXELS_PER_BLOCK = 32,
		MAX_PIXELS_PER_SCANLINE = MAX_BLOCKS_PER_SCANLINE * MAX_PIXELS_PER_BLOCK
	};
	enum {
		ID_ZERO = -1,
		ID_LOW = 0,
		ID_FS = 1,
		ID_K1 = 2,
		ID_K2 = 3,
		ID_K3 = 4,
		ID_K4 = 5,
		ID_K5 = 6,
		ID_K6 = 7,
		ID_K7 = 8,
		ID_K8 = 9,
		ID_K9 = 10,
		ID_K10 = 11,
		ID_K11 = 12,
		ID_K12 = 13,
		ID_K13 = 14,
		ID_K14 = 15,
		ID_K15 = 16,
		ID_K16 = 17,
		ID_K17 = 18,
		ID_K18 = 19,
		ID_K19 = 20,
		ID_K20 = 21,
		ID_K21 = 22,
		ID_K22 = 23,
		ID_K23 = 24
	};
	enum {
		ID_DEFAULT = 31, ID_DEFAULT1 = 7, ID_DEFAULT2 = 15, ID_DEFAULT3 = 31
	};
	enum {
		K_FACTOR = 1
	};
	enum {
		INPUT_BUFFER_SIZE = 16 * 1024
	};
	enum {
		OUTPUT_BUFFER_SIZE = 16 * 1024
	};
	enum {
		MSB_OPTION_MASK = 16, NN_OPTION_MASK = 32
	};

	const bool m_MsbFirst;
	long m_InputByteCount;
	unsigned char *m_InputByteData;

	//int bits_per_block;
	const int m_BitsPerPixel;
	const int m_PixelsPerBlock;
	const int m_PixelsPerScanline;
	const int m_ScanLinesPerPacket;
	const int m_CompressionMode;
	int m_BlocksPerScanline;
	int m_BytesPerPixel;

	int m_DefaultId;
	int m_XMax;

	unsigned char* m_BPtr;
	unsigned char m_Ext2Array1[MAX_EXT2_SUM + 1];
	unsigned char m_Ext2Array2[MAX_EXT2_SUM + 1];
	int unkVal0;
	int m_LeadingZeros[256];

	void Init();
	int RiceDecode();
	int OutputDecodedData(unsigned* Sigma, unsigned char* ptr);
	void Unmap_nn(unsigned *Sigma, int Pixels);
	virtual bool Execute(unsigned char *In, long InBytes);

	static inline void memset32(void * dest, int32_t value, uint32_t size) {
		for (uint32_t i = 0; i < size; i++) {
			((uint32_t *) dest)[i] = value;
		}
	}
};

#endif /* RICEDECOMPRESSION_H_ */
