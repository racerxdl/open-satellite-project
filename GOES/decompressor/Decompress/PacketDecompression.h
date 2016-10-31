#ifndef PACKET_DECOMPRESSION_H
#define PACKET_DECOMPRESSION_H

#include <memory.h>

////////////////////////////////////////////////////////////////////////
///	Class to perform packet-level decompression
class CPacketDecompression
{
public:
	CPacketDecompression(long MaxSize);
	virtual ~CPacketDecompression();
	virtual bool Decompress(
        unsigned char *In,
		long InBytes);
	virtual unsigned char* Ptr(){return m_Out;}
	virtual long Size(){return m_Size;}
	virtual long MaxSize(){return m_MaxSize;}
    virtual unsigned long TotalCompressedBytes()const{return m_TotalCompressedBytes;}
    virtual unsigned long TotalDecompressedBytes()const{return m_TotalDecompressedBytes;}
protected:
	void Size(long Size);
private:
	unsigned char *m_Out;
	long m_MaxSize;
	long m_Size;
    unsigned long m_TotalCompressedBytes;
    unsigned long m_TotalDecompressedBytes;

    virtual bool Execute(
        unsigned char *In,
		long InBytes)=0;
};

////////////////////////////////////////////////////////////////////////
///	Class to perform no packet-level
class CNullDecompression:public CPacketDecompression
{
public:
        CNullDecompression():CPacketDecompression(8190){}
private:
	virtual bool Execute(
        unsigned char *In,
		long InBytes)
	{
		Size(InBytes);
		memcpy(Ptr(),In,Size());
		return true;
	}
};

#endif //PACKET_DECOMPRESSION_H
