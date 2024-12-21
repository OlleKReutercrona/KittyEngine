#pragma once

namespace KE
{
	class BinaryReader
	{
	private:
		char* buffer = nullptr;
		size_t bufferSize = 0;
		size_t pointerPosition = 0;

	public:
		BinaryReader(const char* aFileToRead);
		~BinaryReader();


		void Read(const size_t& aByteCount, void* aDestination);

		template <typename T>
		void Read(T& aDestination)
		{
			T temp;
			memcpy(&temp, buffer + pointerPosition, sizeof(T));
			pointerPosition += sizeof(T);
			aDestination = temp;

		}

		void Offset(const size_t& aByteCount);
	};

	class BinaryWriter
	{
	private:
		std::ofstream file;
	public:
		BinaryWriter(const char* aFileToWrite);
		~BinaryWriter();

		void Write(const size_t& aByteCount, void* aSource);

		template <typename T>
		void Write(const T& aSource)
		{
			file.write((char*)&aSource, sizeof(T));
		}
	};


}