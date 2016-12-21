#include <stdio.h>
#include <stdint.h>

#include <mutex>
#include <atomic>
#include <thread>
#include <cstdint>
#include <iostream>
#include <vector>
#include <exception>
#include <queue>
#include <sstream>
#include <string>
#include <chrono>
#include <random>
#include <functional>

#include "RiceDecompression.h"

//#define PREFIX "1696_0_"
//#define PIXELS 3463

int PIXELS;
std::string PREFIX;
bool debugMode = false;

void processFile(std::string &filename, std::string &outputname) {
	//1696_0_1395.lrit
	unsigned char *input;
	FILE *f = fopen(filename.c_str(), "rb");
	if (f != NULL) {
		fseek(f, 0L, SEEK_END);
		long sz = ftell(f);
		fseek(f, 0L, SEEK_SET);
		input = new unsigned char[sz];
		fread(input, sz, 1, f);
		CRiceDecompression *Rice = new CRiceDecompression(49, 8, 16, PIXELS, 1);
		bool Worked = Rice->Decompress(input, sz);
		if (!Worked) {
			std::cerr << "Failed to decompress" << std::endl;
		}
		FILE *o = fopen(outputname.c_str(), "ab");
		fwrite(Rice->Ptr(), Rice->Size(), 1, o);
		fclose(o);
		fclose(f);
		delete input;
	} else {
		unsigned char *output = new unsigned char[PIXELS];
		memset(output, 0x00, PIXELS);
		if (debugMode) std::cout << "Inexistent file " << filename << ". Adding empty frame." << std::endl;
		FILE *o = fopen(outputname.c_str(), "ab");
		fwrite(output, PIXELS, 1, o);
		fclose(o);
		delete output;
	}
}

std::string buildFilename(std::string prefix, int n) {
	std::stringstream ss;
	ss << prefix << n << ".lrit";
	return ss.str();
}

int doOnlyOne(std::string filename, int Pixels) {
	FILE *f;

	if (debugMode) std::cout << "Filename: " << filename << " Pixels: " << Pixels << std::endl;

	f = fopen(filename.c_str(), "rb");

	if (f == NULL) {
		std::cerr << "Error Opening file" << std::endl;
		return -1;
	}

	fseek(f, 0L, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0L, SEEK_SET);

	if (debugMode) std::cout << "File size: " << sz << std::endl;

	unsigned char *data = new unsigned char[sz];

	fread(data, sz, 1, f);

	//Rice Decompression
	bool Worked;
	CRiceDecompression *Rice = new CRiceDecompression(49, 8, 16, Pixels, 1);
	Worked = Rice->Decompress(data, sz);
	if (!Worked) {
		std::cerr << "Failed to decompress" << std::endl;
	}
	if (debugMode) std::cout << "Worked: " << Worked << std::endl;

	fclose(f);
	f = fopen(std::string(filename + ".decomp").c_str(), "wb");
	fwrite(Rice->Ptr(), Rice->Size(), 1, f);
	fclose(f);
	//Frees input memory
	delete[] data;
}

int main(int argc, char *argv[]) {
	if (argc < 5) {
		int Pixels;
		std::string filename;
		if (argc >= 3) {
			std::istringstream(argv[1]) >> Pixels;
			filename = std::string(argv[2]);
		} else {
			std::cout << "Decompress Single File: " << std::endl;
			std::cout << "	Usage: Decompress.exe Pixels Filename [DebugMode]" << std::endl;
			std::cout << "Decompress Array of Files: " << std::endl;
			std::cout << "	Usage: Decompress.exe Prefix Pixels StartNumber EndNumber [DebugMode]" << std::endl;
			return 1;
		}
		debugMode = argc > 3;
		return doOnlyOne(filename, Pixels);
	}
	debugMode = argc == 7;
	PREFIX = std::string(argv[1]);
	std::istringstream(argv[2]) >> PIXELS;

	int startNumber = 10212;
	int endNumber = 10323;
	int overflowCaseLast = -1;

	std::istringstream(argv[3]) >> startNumber;
	std::istringstream(argv[4]) >> endNumber;

	std::string baseFilename = buildFilename(PREFIX + "_decomp", startNumber - 1);
	if (debugMode) std::cout << "Output file: " << baseFilename << std::endl;
	if (debugMode) std::cout << "Reading first file with ID " << startNumber - 1 << " filename: " << buildFilename(PREFIX, startNumber - 1) << std::endl;

	FILE *f = fopen(baseFilename.c_str(), "wb");
	FILE *f2 = fopen(buildFilename(PREFIX, startNumber - 1).c_str(), "rb");

	if (f == NULL || f2 == NULL) {
		std::cerr << "Error opening base files" << std::endl;
		if (f == NULL) {
			std::cerr << "Error opening: " << baseFilename.c_str() << std::endl;
		}
		if (f2 == NULL) {
			std::cerr << "Error opening: " << buildFilename(PREFIX, startNumber - 1).c_str() << std::endl;
		}
		return 1;
	}

	fseek(f2, 0L, SEEK_END);
	long sz = ftell(f2);
	fseek(f2, 0L, SEEK_SET);
	//sz -= 10;

	if (debugMode) std::cout << "Header size: " << sz << std::endl;

	char *data = new char[sz];
	fread(data, sz, 1, f2);
	fwrite(data, sz, 1, f);
	fclose(f);
	fclose(f2);

	if (debugMode) std::cout << "Header wrote. Reading sequence" << std::endl;

	if (endNumber < startNumber) {
		// Overflow case
		overflowCaseLast = endNumber;
		endNumber = 16383;
	}

	for (int i = startNumber; i <= endNumber; i++) {
		std::string infile = buildFilename(PREFIX, i);
		if (debugMode) std::cout << "Opening file " << infile << std::endl;
		processFile(infile, baseFilename);
	}

	if (overflowCaseLast != -1) {
		for (int i = 0; i < overflowCaseLast; i++) {
			std::string infile = buildFilename(PREFIX, i);
			if (debugMode) std::cout << "Opening file " << infile << std::endl;
			processFile(infile, baseFilename);
		}
	}

	if (debugMode) std::cout << "Finished!" << std::endl;
	return 0;
}
