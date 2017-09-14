
#pragma once

#include <cstddef> // For std::size_t.

namespace memwa
{
//namespace impl
//{

// ----------------------------------------------------------------------------

class LinearBlock
{
public:

	LinearBlock() = default;

	LinearBlock( LinearBlock && ) = default;

	LinearBlock( const LinearBlock & ) = default;

	LinearBlock & operator = ( LinearBlock && ) = default;

	LinearBlock & operator = ( const LinearBlock & ) = default;

	LinearBlock( std::size_t blockSize, std::size_t alignment );

	~LinearBlock() = default;

	void * Allocate( std::size_t size, std::size_t blockSize, std::size_t alignment );

	void Destroy();

	bool HasAddress( const void * place, std::size_t blockSize ) const;

	bool IsBelowAddress( const void * place, std::size_t blockSize ) const;

	bool HasBytesAvailable( std::size_t bytes, std::size_t blockSize, std::size_t alignment ) const;

	bool operator < ( const LinearBlock & that ) const
	{
		return ( block_ < that.block_ );
	}

	bool IsEmpty( std::size_t alignment ) const;

	bool IsCorrupt( std::size_t blockSize, std::size_t alignment ) const;

private:

	/// Pointer to base of entire memory page allocated.
	unsigned char * block_;
	/// Pointer to next free spot within this block.
	unsigned char * freeSpot_;
};

// ----------------------------------------------------------------------------

// } // end internal namespace

} // end project namespace
