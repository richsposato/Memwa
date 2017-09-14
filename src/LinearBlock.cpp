
#include "LinearBlock.hpp"

#include "BlockInfo.hpp"
#include "ManagerImpl.hpp"

#include <cassert>
#include <cstdlib>
#include <new>

namespace memwa
{
//namespace impl
//{

// ----------------------------------------------------------------------------

LinearBlock::LinearBlock( std::size_t blockSize, std::size_t alignment ) :
	block_( nullptr ),
	freeSpot_( nullptr )
{
	const std::size_t remainder = reinterpret_cast< std::size_t >( block_ ) % alignment;
	blockSize += remainder;
	block_ = reinterpret_cast< unsigned char * >( std::malloc( blockSize ) );
	if ( nullptr == block_ )
	{
		throw std::bad_alloc();
	}
	freeSpot_ = block_ + remainder;
}

// ----------------------------------------------------------------------------

void LinearBlock::Destroy()
{
	std::free( block_ );
	block_ = nullptr;
}

// ----------------------------------------------------------------------------

void * LinearBlock::Allocate( std::size_t bytes, std::size_t blockSize, std::size_t alignment )
{
	const std::size_t bytesNeeded = memwa::CalculateBytesNeeded( bytes, alignment );
	const std::size_t bytesAvailable = ( block_ + blockSize ) - freeSpot_;
	const bool hasEnough = ( bytesNeeded <= bytesAvailable );
	if ( !hasEnough )
	{
		return nullptr;
	}

	void * p = freeSpot_;
	freeSpot_ += bytesNeeded;
	return p;
}

// ----------------------------------------------------------------------------

bool LinearBlock::HasAddress( const void * place, std::size_t blockSize ) const
{
	if ( place < block_ )
	{
		return false;
	}
	if ( place >= block_ + blockSize )
	{
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------------

bool LinearBlock::IsBelowAddress( const void * place, std::size_t blockSize ) const
{
	const bool below = ( block_ + blockSize <= place );
	return below;
}

// ----------------------------------------------------------------------------

bool LinearBlock::HasBytesAvailable( std::size_t bytes, std::size_t blockSize, std::size_t alignment ) const
{
	const std::size_t bytesNeeded = memwa::CalculateBytesNeeded( bytes, alignment );
	const std::size_t bytesAvailable = ( block_ + blockSize ) - freeSpot_;
	const bool hasEnough = ( bytesNeeded <= bytesAvailable );
	return hasEnough;
}

// ----------------------------------------------------------------------------

bool LinearBlock::IsEmpty( std::size_t alignment ) const
{
	const std::size_t remainder = reinterpret_cast< std::size_t >( block_ ) % alignment;
	const unsigned char * place = block_ + remainder;
	const bool empty = ( place == freeSpot_ );
	return empty;
}

// ----------------------------------------------------------------------------

bool LinearBlock::IsCorrupt( std::size_t blockSize, std::size_t alignment ) const
{
	assert( this != nullptr );
	assert( block_ != nullptr );
	assert( block_ <= freeSpot_ );
	assert( block_ + blockSize >= freeSpot_ );
	return false;
}

// ----------------------------------------------------------------------------

// } // end internal namespace

} // end project namespace
