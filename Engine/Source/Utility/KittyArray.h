#pragma once
#include <bitset>
#include <stack>

struct KitField
{
	char byte;
};

class KitSet
{
public:
	inline static size_t GetFieldSize(size_t aElementCount) { return sizeof(size_t) + sizeof(KitField) * (aElementCount / sizeof(KitField) + 1); }

	inline bool Get(size_t aIndex)
	{
		return *reinterpret_cast<bool*>(this+aIndex);
	}

	inline void Set(size_t aIndex, bool aVal)
	{
		*(bool*)(this + aIndex) = aVal;
	}
};

template <typename T>
class KittyArray;
template <typename T>
class Kitterator;

template <typename T>
class LaserPtr
{
	friend class KittyArray<T>;
private:
	KittyArray<T>* dataSource;
	size_t index;
public:
	LaserPtr(KittyArray<T>* aDataSource, size_t aIndex)
		: dataSource(aDataSource)
		, index(aIndex)
	{
	}

	T* operator->()
	{
		return dataSource->Get(index);
	}

	T* Get()
	{
		return dataSource->Get(index);
	}

	T& operator*()
	{
		return *dataSource->Get(index);
	}

	const T& operator*() const
	{
		return *dataSource->Get(index);
	}
};



template <typename T>
class KittyArray
{
	friend class Kitterator<T>;
	friend class LaserPtr<T>;
private:
	size_t count;
	size_t capacity;
	
	KitSet* kitSet;
	T* data;

	std::stack<size_t> backFillIndices;

	void Allocate(size_t aSize)
	{
		const size_t fieldSize = KitSet::GetFieldSize(aSize);
		const size_t allocSize = fieldSize + sizeof(T) * aSize;
		void* block = malloc(
			allocSize
		);
		memset(block, 0, allocSize);

		capacity = aSize;
		kitSet = (KitSet*)block;
		data = (T*)((char*)block + fieldSize);

	}

	void Reallocate(size_t aNewCapacity)
	{
		const size_t newFieldSize = KitSet::GetFieldSize(aNewCapacity);
		const size_t reAllocSize = newFieldSize + sizeof(T) * aNewCapacity;

		void* newBlock = realloc(kitSet, reAllocSize);
		assert(newBlock && "Failed to reallocate data block. That sucks :(");

		const size_t oldFieldSize = KitSet::GetFieldSize(capacity);
		memmove((char*)newBlock + newFieldSize, (char*)newBlock + oldFieldSize, sizeof(T) * capacity);
		memset((char*)newBlock + oldFieldSize, 0, newFieldSize - oldFieldSize);

		capacity = aNewCapacity;
		kitSet = (KitSet*)newBlock;
		data = (T*)((char*)newBlock + newFieldSize);
	}

	size_t GetNextFreeIndex()
	{
		if (backFillIndices.size() > 0)
		{
			size_t index = backFillIndices.top();
			backFillIndices.pop();
			return index;
		}

		return count;
	}


	T* Get(size_t aIndex)
	{
		return &data[aIndex];
	}

public:

	KittyArray(size_t aCapacity = 1)
		: count(0)
		, capacity(aCapacity)
		, backFillIndices{}
	{
		Allocate(aCapacity);
	}

	~KittyArray()
	{
		Clear();
		free(kitSet); // This will also free the data block
	}

	template <typename ...Args>
	LaserPtr<T> Emplace(Args&&... args)
	{
		if (count == capacity)
		{
			Reallocate(capacity * 2);
		}

		size_t index = GetNextFreeIndex();
		count++;
		new (&data[index]) T(std::forward<Args>(args)...);
		kitSet->Set(index, true);

		return LaserPtr<T>{ this, index};
	}

	LaserPtr<T> Append(const T& inputData)
	{
		if (count == capacity)
		{
			Reallocate(capacity * 2);
		}

		size_t index = GetNextFreeIndex();
		memcpy(&data[index], &inputData, sizeof(T));
		count++;
		kitSet->Set(index, true);

		return LaserPtr<T>{ this, index};
	}

	Kitterator<T> begin();
	Kitterator<T> end();
	Kitterator<const T> begin() const;
	Kitterator<const T> end() const;

	LaserPtr<T> operator[](size_t aIndex)
	{
		return LaserPtr<T>{ this, aIndex };
	}

	LaserPtr<T> At(size_t aIndex)
	{
		return LaserPtr<T>{ this, aIndex };
	}



	void Erase(size_t aIndex)
	{
		data[aIndex].~T();

		kitSet->Set(aIndex, false);
		count--;
		backFillIndices.push(aIndex);
	}

	void Erase(const LaserPtr<T>& aPtr)
	{
		Erase(aPtr.index);
	}

	void Clear()
	{
		for (size_t i = 0; i < count; i++)
		{
			if (kitSet->Get(i))
			{
				data[i].~T();
			}
		}

		memset(kitSet, 0, KitSet::GetFieldSize(capacity));

		count = 0;
		backFillIndices = {};

	}

	void Reserve(size_t aSize)
	{
		if (capacity < aSize)
		{
			Reallocate(aSize);
		}
	}

	size_t Size() const
	{
		return count;
	}
};

template<typename T>
class Kitterator
{
private:
	KittyArray<T>* dataLocation;
	size_t index;
	size_t end;
public:
	Kitterator(KittyArray<T>* aDataLocation, size_t aIndex)
		: dataLocation(aDataLocation)
		, index(aIndex)
		, end(aDataLocation->count + aDataLocation->backFillIndices.size())
	{
	}

	bool IndexContainsData(size_t aIndex)
	{
		return dataLocation->kitSet->Get(aIndex);
	}

	Kitterator& operator++()
	{
		do
		{
			++index;
		} while (index < end && !IndexContainsData(index));
		
		return *this;
	}

	bool operator==(const Kitterator& aOther) const
	{
		return index == aOther.index;
	}

	bool operator!=(const Kitterator& aOther) const
	{
		return index != aOther.index;
	}

	T& operator*()
	{
		return *dataLocation->At(index);
	}

	const T& operator*() const
	{
		return *dataLocation->At(index);
	}
};



template<typename T>
Kitterator<T> KittyArray<T>::begin()
{
	size_t index = 0;
	return Kitterator<T>{ this, index };
}

template<typename T>
Kitterator<T> KittyArray<T>::end()
{
	return Kitterator<T>{ this, count + backFillIndices.size() };
}

template<typename T>
Kitterator<const T> KittyArray<T>::begin() const
{
	size_t index = 0;

	return Kitterator<const T>{ this, index };
}

template<typename T>
Kitterator<const T> KittyArray<T>::end() const
{
	return Kitterator<const T>{ this, count + backFillIndices.size() };
}