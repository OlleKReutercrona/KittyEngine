#pragma once
namespace KE_SER
{
	struct BaseData
	{
		virtual ~BaseData() = default;
		virtual size_t GetSize() = 0;
		virtual void* GetData() = 0;
	};
	
	template<typename T>
	struct TypedData : public BaseData
	{
		TypedData(T& aData) : data(aData) {}
		T data;
	
		size_t GetSize() override { return sizeof(data); }
		void* GetData() override { return &data; }
	};

	template <typename T>
	struct ReflectionPackage
	{
		T& member;
		size_t size;
		const std::string& varName;
		const std::string& typeName;
		size_t offset;
	};

	struct Data
	{

#ifndef KITTYENGINE_NO_EDITOR
		std::string name;
		std::string typeName;
		size_t offset;

#endif
		char* data = nullptr;
		size_t size;

		template<typename T>
#ifndef KITTYENGINE_NO_EDITOR
		Data(T& aData, const std::string& aName, const std::string& aType, size_t anOffset)
		{
			data = new char[sizeof(T)];
			memcpy(data, &aData, sizeof(T));
			size = sizeof(T);
			name = aName;
			typeName = aType;
			offset = anOffset;
		}
#else
		Data(T& aData)
		{
			data = new char[sizeof(T)];
			memcpy(data, &aData, sizeof(T));
			size = sizeof(T);
		}
#endif

		//move operator
		Data(Data&& aData) noexcept
		{
			data = aData.data;
			size = aData.size;

#ifndef KITTYENGINE_NO_EDITOR
			name = aData.name;
			typeName = aData.typeName;
			offset = aData.offset;
#endif

			aData.data = nullptr;
		}

		//copy operator
		Data(const Data& aData)
		{
			data = new char[aData.size];
			memcpy(data, aData.data, aData.size);
			size = aData.size;

#ifndef KITTYENGINE_NO_EDITOR
			name = aData.name;
			typeName = aData.typeName;
			offset = aData.offset;
#endif

		}

		//copy assignment
		Data& operator=(const Data& aData)
		{
			if (this != &aData)
			{
				delete[] data;
				data = new char[aData.size];
				memcpy(data, aData.data, aData.size);
				size = aData.size;

#ifndef KITTYENGINE_NO_EDITOR
				name = aData.name;
				typeName = aData.typeName;
				offset = aData.offset;
#endif
			}
			return *this;
		}

		~Data()
		{
			delete[] data;
		}
	};

	struct SerializerData
	{
		std::vector<Data> myData;


		template <typename T>
#ifndef KITTYENGINE_NO_EDITOR
		SerializerData& operator &(const ReflectionPackage<T>& aRefPKG)
		{
			myData.push_back(Data(aRefPKG.member, aRefPKG.varName, aRefPKG.typeName, aRefPKG.offset));
			return *this;
		}
#else
		SerializerData& operator &(T& aData)
		{
			myData.push_back(Data(aData));
			return *this;
		}
#endif

	};

	struct DeserializerData
	{
		std::vector<Data> myData;

		size_t index = 0;

		template <typename T>
#ifndef KITTYENGINE_NO_EDITOR
		DeserializerData& operator &(const ReflectionPackage<T>& aRefPKG)
		{
			aRefPKG.member = *(T*)(myData[index].data);
			return *this;
		}
#else
		DeserializerData& operator &(T& aData)
		{
			aData = *(T*)(myData[index].data);
			return *this;
		}
#endif

	};

	class Serializer
	{
	private:

	public:
		template<typename T>
		static SerializerData Serialize(T& anObject)
		{
			SerializerData serIn;
			anObject.Serialize(serIn);
			return serIn;
		}

		template<typename T>
		static void Deserialize(T& anObject, DeserializerData& aDeserializerData)
		{
			anObject.Serialize(aDeserializerData);
		}

	};


	inline SerializerData MakeSerializerData(const DeserializerData& aDeserializerData)
	{
		SerializerData serIn;
		serIn.myData = aDeserializerData.myData;
		return serIn;
	}

	inline DeserializerData MakeDeserializerData(const SerializerData& aSerializerData)
	{
		DeserializerData serOut;
		serOut.myData = aSerializerData.myData;
		return serOut;
	}

	template<typename T, typename U> constexpr size_t offsetOf(U T::* member)
	{
		return (char*)&((T*)nullptr->*member) - (char*)nullptr;
	}
}

#ifndef KITTYENGINE_NO_EDITOR
#define SET_REFLECTION(class) using ReflectedType = class;
#define PARAM(member) KE_SER::ReflectionPackage((member), sizeof(member), #member, typeid(member).name(), KE_SER::offsetOf(&ReflectedType::member))
#else
#define SET_REFLECTION(class)
#define PARAM(name) name
#endif


