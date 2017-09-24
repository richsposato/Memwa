
#pragma once

#include <cstddef> // For std::size_t.

namespace memwa
{

// ----------------------------------------------------------------------------

class PoolBlock
{
public:

    PoolBlock( std::size_t blockSize, std::size_t alignedSize, std::size_t alignment, unsigned int objectsPerPool );

	void Destroy();

	void * Allocate();

	bool Release( void * place );

	bool HasAddress( const void * place, std::size_t blockSize ) const;

	bool IsBelowAddress( const void * place, std::size_t blockSize ) const;

	bool operator < ( const PoolBlock & that ) const
	{
		return ( block_ < that.block_ );
	}

	bool IsEmpty() const
	{
		return ( 0 == objectCount_ );
	}

	bool IsFull() const
	{
		return ( free_ == nullptr );
	}

	unsigned int GetInUseCount() const
	{
		return objectCount_;
	}

#ifdef DEBUGGING_ALLOCATORS

	void OutputContents() const;

#endif

	std::size_t * GetAddress() const
	{
		return block_;
	}

	bool IsCorrupt( std::size_t blockSize, std::size_t alignment, std::size_t objectSize ) const;

private:

	/// Pointer to block allocated to hold from 1 to N objects.
	std::size_t * block_;
	/// Pointer to next free chunk in pool.
	std::size_t * free_;
	/// Number of objects allocated so far.
	unsigned int objectCount_;
};

// ----------------------------------------------------------------------------

}
