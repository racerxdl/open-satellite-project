//============================================================================
// Name        : Decompressor.cpp
// Author      : Lucas Teske
// Version     :
// Copyright   : GPLv3
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdio>
//#include <cstdint>
#include <stdint.h>
#include <cstring>

extern "C" {
#include <jpeglib.h>
}

using namespace std;

int8_t standardQuantizationTable[64] {
	16,  11,  10,  16,  24,  40,  51,  61,
	12,  12,  14,  19,  26,  58,  60,  55,
	14,  13,  16,  24,  40,  57,  69,  56,
	14,  17,  22,  29,  51,  87,  80,  62,
	18,  22,  37,  56,  68, 109, 103,  77,
	24,  35,  55,  64,  81, 104, 113,  92,
	49,  64,  78,  87, 103, 121, 120, 101,
	72,  92,  95,  98, 112, 100, 103,  99
};

int main() {
	int8_t quantizationTable[64];

	uint8_t buff[252];
	uint8_t *data = &buff[14];

	// Read file
	FILE *f = fopen("66_0_7636.lrpt", "rb");
	fread(buff, 252, 1, f);
	fclose(f);

	uint16_t day = *((uint16_t *) buff);
	uint32_t msFromDay = *((uint32_t *) &buff[2]); // 2,3,4,5
	uint16_t iss = *((uint16_t *) &buff[6]); // 6,7

	uint8_t nmcu = buff[8];
	uint8_t qt = buff[9];
	uint8_t dc = (buff[10] & 0xF0) >> 4;
	uint8_t ac = (buff[10] & 0x0F);
	uint16_t qfm = *((uint16_t *) &buff[11]); // 11, 12
	uint8_t qFactor = buff[13]; // 13


	std::cout << "Day: " << day << std::endl;
	std::cout << "Milisseconds from Day: " << msFromDay << std::endl;
	std::cout << "ISS(?): " << iss << std::endl;

	std::cout << "NMCU: " << (uint32_t) nmcu << std::endl;
	std::cout << "QT: " << (uint32_t) qt << std::endl;
	std::cout << "DC: " << (uint32_t) dc << std::endl;
	std::cout << "AC: " << (uint32_t) ac << std::endl;
	std::cout << "QFM: " << qfm << std::endl;
	std::cout << "Q: " << (uint32_t) qFactor << std::endl;


	// Build quantization table.
	std::memcpy(quantizationTable, standardQuantizationTable, 64);

	uint8_t F = (qFactor > 20 && qFactor < 50) ? (5000 / qFactor) & 0xFF : (200 - 2 * qFactor) & 0xFF;
	for (int i=0; i<64; i++) {
		quantizationTable[i] = (F / 100) * quantizationTable[i];
	}


	// Decode
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, data, 252-14);

	cinfo.image_width = 1568;
	cinfo.image_height = 8;
	cinfo.num_components = 1;
	cinfo.out_color_space = JCS_GRAYSCALE;
	cinfo.saw_JFIF_marker = FALSE;
	cinfo.saw_Adobe_marker = FALSE;

	jpeg_start_decompress(&cinfo);

	return 0;
}
