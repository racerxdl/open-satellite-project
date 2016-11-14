/*
 * PacketDecompression.h
 *
 *  Created on: 13/11/2016
 *      Author: Lucas Teske
 */

#ifndef PACKETDECOMPRESSION_H_
#define PACKETDECOMPRESSION_H_

#include <memory.h>

class CPacketDecompression {
public:
	CPacketDecompression(long MaxSize);
	virtual ~CPacketDecompression();
	virtual bool Decompress(unsigned char *In, long InBytes);
	virtual unsigned char* Ptr() {
		return m_Out;
	}
	virtual long Size() {
		return m_Size;
	}
	virtual long MaxSize() {
		return m_MaxSize;
	}
	virtual unsigned long TotalCompressedBytes() const {
		return m_TotalCompressedBytes;
	}
	virtual unsigned long TotalDecompressedBytes() const {
		return m_TotalDecompressedBytes;
	}
protected:
	void Size(long Size);
private:
	unsigned char *m_Out;
	long m_MaxSize;
	long m_Size;
	unsigned long m_TotalCompressedBytes;
	unsigned long m_TotalDecompressedBytes;

	virtual bool Execute(unsigned char *In, long InBytes)=0;
};

class CNullDecompression: public CPacketDecompression {
public:
	CNullDecompression() :
			CPacketDecompression(8190) {
	}
private:
	virtual bool Execute(unsigned char *In, long InBytes) {
		Size(InBytes);
		memcpy(Ptr(), In, Size());
		return true;
	}
};

#endif /* PACKETDECOMPRESSION_H_ */
