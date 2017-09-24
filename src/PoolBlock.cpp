
#include "PoolBlock.hpp"

#include "ManagerImpl.hpp"

#include <cassert>

#include <iostream>
#include <stdexcept>

namespace memwa
{

// ----------------------------------------------------------------------------

PoolBlock::PoolBlock( std::size_t blockSize, std::size_t alignedSize, std::size_t alignment, unsigned int objectsPerPool ) :
	block_( reinterpret_cast< std::size_t * >( std::malloc( blockSize ) ) ),
	free_( block_ ),
	objectCount_( 0 )
{

	if ( nullptr == block_ )
	{
		throw std::bad_alloc();
	}
    std::size_t blockPlace = reinterpret_cast< std::size_t >( block_ );
    assert( blockPlace % alignment == 0 );

	std::size_t * place;
	const unsigned char * const end = reinterpret_cast< unsigned char * >( block_ ) + blockSize;
	unsigned char * here = reinterpret_cast< unsigned char * >( free_ );
	unsigned char * next = here + alignedSize;

	while ( next < end )
	{
		assert( here < end );
		assert( next < end );
		place = reinterpret_cast< std::size_t * >( here );
		*place = reinterpret_cast< std::size_t >( next );
/*
		std::cout << '\t' << ii << '\t' << reinterpret_cast< std::size_t >( here )
			<< '\t' << *place << '\t' << reinterpret_cast< std::size_t >( next ) << std::endl;
*/
		here = next;
		next += alignedSize;
//		++ii;
	}
	assert( here < end );
	assert( next == end );
	place = reinterpret_cast< std::size_t * >( here );
	*place = 0;
/*
	std::cout << "\t here: [" << reinterpret_cast< std::size_t >( here )
		<< "]  *here: [" << *place
		<< "]  next: [" << reinterpret_cast< std::size_t >( next ) << ']' << std::endl;
*/
}

// ----------------------------------------------------------------------------

void PoolBlock::Destroy()
{
    ::free( reinterpret_cast< void * >( block_ ) );
	block_ = nullptr;
	free_ = nullptr;
}

// ----------------------------------------------------------------------------

void * PoolBlock::Allocate()
{
	if ( IsFull() )
	{
		return nullptr;
	}
	void * p = free_;
	std::size_t * place = reinterpret_cast< std::size_t * >( *free_ );
	free_ = place;
	++objectCount_;
	return p;
}

// ----------------------------------------------------------------------------

bool PoolBlock::Release( void * place )
{
	std::size_t * p = reinterpret_cast< std::size_t * >( place );
	*p = reinterpret_cast< std::size_t >( free_ );
	free_ = p;
	--objectCount_;
	return true;
}

// ----------------------------------------------------------------------------

bool PoolBlock::HasAddress( const void * place, std::size_t blockSize ) const
{
	if ( place < block_ )
	{
		return false;
	}
	const unsigned char * const b = reinterpret_cast< const unsigned char * >( block_ );
	const unsigned char * const p = reinterpret_cast< const unsigned char * >( place );
	if ( b + blockSize <= p )
	{
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------------

bool PoolBlock::IsBelowAddress( const void * place, std::size_t blockSize ) const
{
	const unsigned char * const b = reinterpret_cast< const unsigned char * >( block_ );
	const unsigned char * const p = reinterpret_cast< const unsigned char * >( place );
	const bool isBelow = ( b + blockSize <= p );
	return isBelow;
}

// ----------------------------------------------------------------------------

bool PoolBlock::IsCorrupt( std::size_t blockSize, std::size_t alignment, std::size_t objectSize ) const
{
	assert( nullptr != this );
	assert( block_ != nullptr );
	assert( block_ != nullptr );
	const unsigned int objectsPerPool = blockSize / objectSize;
	if ( free_ == nullptr )
	{
		assert( objectCount_ != 0 );
		assert( objectsPerPool == objectCount_ );
	}
	else
	{
		assert( objectsPerPool > objectCount_ );
		assert( free_ >= block_ );
		assert( free_ < block_ + blockSize );
		std::size_t * next = free_;
		bool checkedFirst = false;
		unsigned int freeCount = 0;
		unsigned int maxFreeCount = ( objectsPerPool - objectCount_ );
		while ( next != nullptr )
		{
			assert( next >= block_ );
			assert( next < block_ + blockSize );
			if ( checkedFirst )
			{
				assert( next != free_ );
			}
			next = reinterpret_cast< std::size_t * >( *next );
			checkedFirst = true;
			++freeCount;
			assert( freeCount <= maxFreeCount );
		}
		assert( freeCount == maxFreeCount );
	}
	return false;
}

// ----------------------------------------------------------------------------

#ifdef DEBUGGING_ALLOCATORS

void PoolBlock::OutputContents() const
{
	std::cout << '\t' << this
		<< '\t' << " Block: " << block_
		<< '\t' << " Free: " << free_
		<< '\t' << " In Use: " << objectCount_
		<< std::endl;
}

#endif

// ----------------------------------------------------------------------------

}
