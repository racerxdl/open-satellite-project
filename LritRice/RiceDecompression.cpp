/*
 * RiceDecompression.cpp
 *
 *  Created on: 13/11/2016
 *      Author: Lucas Teske
 */

#include "RiceDecompression.h"
#include <memory.h>

CRiceDecompression::CRiceDecompression(int Mask, int BitsPerPixel, int PixelsPerBlock, int PixelsPerScanline, int ScanLinesPerPacket) {
    this->m_BitsPerPixel = BitsPerPixel;
    this->m_PixelsPerBlock = PixelsPerBlock;
    this->m_ScanLinesPerPacket = ScanLinesPerPacket;
    this->m_PixelsPerScanline = PixelsPerScanline;
    this->m_CompressionMode = (NN_OPTION_MASK & Mask) > 0;
    this->m_MsbFirst = (MSB_OPTION_MASK & Mask) > 0;
    double p = (ScanLinesPerPacket * PixelsPerScanline * BitsPerPixel) * 0.125; // Let's see where its used.
    this->m_BlocksPerScanline = 0; // ?? Maybe this ^^^

    // Defaults
    this->m_BPtr = NULL;
    this->m_InputByteData = NULL;
    this->m_InputByteCount = 0;
    this->m_XMax = 0;
    this->m_BytesPerPixel = 0;
    this->m_DefaultId = ID_DEFAULT;
    this->unkVal0 = 0;
}

void CRiceDecompression::Init() {
    this->m_BlocksPerScanline = (m_PixelsPerScanline + m_PixelsPerBlock - 1) / m_PixelsPerBlock;
    if (m_BitsPerPixel <= 16) {
        if (m_BitsPerPixel <= 8) {
            m_BytesPerPixel = 1;
            m_DefaultId = ID_DEFAULT1;
        } else {
            m_BytesPerPixel = 2;
            m_DefaultId = ID_DEFAULT2;
        }
    } else {
        m_BytesPerPixel = 4;
        m_DefaultId = ID_DEFAULT3;
    }

    m_XMax = (1 << m_BitsPerPixel) - 1;
    unkVal0 = 8;

    int i = 1, c = 7;
    while (i < 256) {
        if (i > 0) {
            CRiceDecompression::memset32((void *) &this->m_LeadingZeros[i], c, i);
        }
        i *= 2;
        c--;
    }

    int k = 0, j = 7;
    int64_t p = 0;
    while (j >= 0) {
        for (int i = 0; i <= j; m_Ext2Array1[p] = k) {
            p = (i + k) * (i + k + 1);
            int t = i + (p - (p & 0xFFFFFFFF00000000)) >> 1;
            p = (p & 0xFFFFFFFF00000000) | t;
            m_Ext2Array2[p] = i++;
        }
        k++;
        j--;
    }
}

int CRiceDecompression::RiceDecode() {
    int compressedLength, compressedLength2, dataLength;
    char workBuffer[65536];
    char buff32k[32776];

    char *buff32kPtr = buff32k;
    int buff32kPtr2 = buff32k;

    while (true) {
        if (&buff32k[8 * m_PixelsPerScanline] < buff32kPtr) {
            // goto LABEL_16
        }
        compressedLength = m_InputByteCount;
        compressedLength2 = m_InputByteCount;
        if (compressedLength > 16384) {
            compressedLength = 16384;
        }
        memcpy(workBuffer, m_InputByteData, compressedLength);

        m_InputByteCount -= compressedLength;
        m_InputByteData += compressedLength;

        if (compressedLength) {
            if (compressedLength & 1) {
                workBuffer[compressedLength] = 0;
                dataLength = compressedLength + 1;
            }

            int compressedLength2a = &buff32kPtr[-buff32kPtr2] >> 1;
            memcpy(buff32k, (const void *)buff32kPtr2, 2 * compressedLength2a);

            char *lowBuff = &buff32k[2 * compressedLength2a];
            char *highBuff = &buff32k[2 * (compressedLength2a + (dataLength >> 1))];
            if (lowBuff < highBuff) {
                int p = workBuffer - lowBuff;
                uint16_t v9 = 0;
                while (lowBuff < highBuff) {
                    v9 = (v9 & 0xFF) | (lowBuff[p] << 8);
                    lowBuff += 2;
                    v9 = (v9 & 0xFF00) | ( lowBuff[p-1]);
                    *((uint16_t*)(lowBuff-1)) = v9;
                }
                lowBuff = 0;
                // Stopped Here
            }
        }
    }

    // TODO
    return 0;
}

int CRiceDecompression::OutputDecodedData(unsigned* Sigma, unsigned char* ptr) {
    int c = 0;

    m_BPtr = ptr;
    int dataLength = (MaxSize() >= m_PixelsPerScanline) ? m_PixelsPerScanline : MaxSize();

    if (m_CompressionMode) {
        Unmap_nn(Sigma, dataLength);
    }

    if (m_BytesPerPixel == 1) { // 8bpp
        c = 0;
        while (c < dataLength) {
            *(m_BPtr++) = Sigma[c++] & 0xFF;
        }
        return m_BPtr - ptr;
    }

    if (m_BytesPerPixel == 2) { // 16 bpp
        c = 0;
        while (c < dataLength) {
            if (m_MsbFirst) {
                *(m_BPtr + 0) = (Sigma[c] & 0xFF00) >> 8;
                *(m_BPtr + 1) = (Sigma[c] & 0xFF);
            } else {
                *(m_BPtr + 0) = (Sigma[c] & 0xFF);
                *(m_BPtr + 1) = (Sigma[c] & 0xFF00) >> 8;
            }
            c++;
            m_BPtr += 2;
        }
        return m_BPtr - ptr;
    }

    // 32 bpp
    c = 0;

    while (c < dataLength) {
        if (m_MsbFirst) {
            if (m_MsbFirst) {
                *(m_BPtr + 0) = (Sigma[c] & 0xFF000000) >> 24;
                *(m_BPtr + 1) = (Sigma[c] & 0x00FF0000) >> 16;
                *(m_BPtr + 2) = (Sigma[c] & 0x0000FF00) >> 8;
                *(m_BPtr + 3) = (Sigma[c] & 0x000000FF) >> 0;
            } else {
                *(m_BPtr + 0) = (Sigma[c] & 0x000000FF) >> 0;
                *(m_BPtr + 1) = (Sigma[c] & 0x0000FF00) >> 8;
                *(m_BPtr + 2) = (Sigma[c] & 0x00FF0000) >> 16;
                *(m_BPtr + 3) = (Sigma[c] & 0xFF000000) >> 24;
            }
            c++;
            m_BPtr += 4;
        }
    }

    return m_BPtr - ptr;
}

void CRiceDecompression::Unmap_nn(unsigned *Sigma, int Pixels) {
    if (m_BytesPerPixel == 1) {
        unsigned char *s = (unsigned char *) Sigma;
        unsigned int *sigmaPtr = Sigma;
        // TODO
    }
}

bool CRiceDecompression::Execute(unsigned char *In, long InBytes) {
    this->m_InputByteData = In;
    this->m_InputByteCount = InBytes;
    int size = RiceDecode();
    Size(size);
    // TODO: Check Errors
    return true;
}
