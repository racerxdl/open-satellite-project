/*
 * PacketDecompression.cpp
 *
 *  Created on: 13/11/2016
 *      Author: Lucas Teske
 */

#include "PacketDecompression.h"

#include <stdio.h>

//-----------------------------------------------------------------------------
CPacketDecompression::CPacketDecompression(long MaxSize) : m_MaxSize(MaxSize), m_TotalCompressedBytes(0), m_TotalDecompressedBytes(0), m_Size(0) {
    m_Out = new unsigned char[MaxSize * 4];
}

CPacketDecompression::~CPacketDecompression() {
    delete[] m_Out;
}

void CPacketDecompression::Size(long Size) {
    if (Size > m_MaxSize)
        m_Size = m_MaxSize;
    else if (Size < 0)
        m_Size = 0;
    else
        m_Size = Size;
}

bool CPacketDecompression::Decompress(unsigned char *In, long InBytes) {
    if (!Execute(In, InBytes))
        return false;
    m_TotalCompressedBytes += InBytes;
    m_TotalDecompressedBytes += Size();
    return true;
}
