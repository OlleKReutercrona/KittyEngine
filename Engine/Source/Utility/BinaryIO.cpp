#include "stdafx.h"
#include "BinaryIO.h"

namespace KE
{

	BinaryReader::BinaryReader(const char* aFileToRead)
	{
		std::ifstream file(aFileToRead, std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			size_t fileSize = static_cast<size_t>(file.tellg());
			buffer = new char[fileSize];
			bufferSize = fileSize;
			file.seekg(0, std::ios::beg);
			file.read(buffer, fileSize);
			file.close();
		}
	}

	BinaryReader::~BinaryReader()
	{
		delete[] buffer;
	}

	void BinaryReader::Read(const size_t& aByteCount, void* aDestination)
	{
		memcpy(aDestination, buffer + pointerPosition, aByteCount);
		pointerPosition += aByteCount;
	}

	void BinaryReader::Offset(const size_t& aByteCount)
	{
		pointerPosition += aByteCount;
	}

	//

	BinaryWriter::BinaryWriter(const char* aFileToWrite) : file(aFileToWrite, std::ios::binary | std::ios::ate)
	{

	}

	BinaryWriter::~BinaryWriter()
	{		
	}


	void BinaryWriter::Write(const size_t& aByteCount, void* aSource)
	{
		file.write((char*)aSource, aByteCount);
	}

}
