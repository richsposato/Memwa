
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
	block_( reinterpret_cast< unsigned char * >( std::malloc( blockSize ) ) ),
	freeSpot_( block_ )
{
	if ( nullptr == block_ )
	{
		throw std::bad_alloc();
	}
	const std::size_t blockPlace = reinterpret_cast< std::size_t >( block_ );
	const std::size_t remainder = reinterpret_cast< std::size_t >( blockPlace ) % alignment;
	const std::size_t padding = ( remainder == 0 ) ? 0 : alignment - remainder;
	freeSpot_ += padding;
	assert( reinterpret_cast< std::size_t >( freeSpot_ ) % alignment == 0 );
}

// ----------------------------------------------------------------------------

void LinearBlock::Destroy()
{
	std::free( block_ );
	block_ = nullptr;
}

// ----------------------------------------------------------------------------

std::size_t LinearBlock::CalculatePadding( std::size_t alignment ) const
{
	const std::size_t freePlace = reinterpret_cast< const std::size_t >( freeSpot_ );
	const std::size_t remainder = ( freePlace % alignment );
	const std::size_t padding = ( remainder == 0 ) ? 0 : alignment - remainder;
	return padding;
}

// ----------------------------------------------------------------------------

void * LinearBlock::Allocate( std::size_t bytes, std::size_t blockSize, std::size_t alignment )
{
	const unsigned char * const end = block_ + blockSize;
	const std::size_t freePlace = reinterpret_cast< const std::size_t >( freeSpot_ );
	const std::size_t padding = CalculatePadding( alignment );
	const std::size_t blockPlace = reinterpret_cast< const std::size_t >( block_ );
	const std::size_t bytesAvailable = ( blockPlace + blockSize ) - ( freePlace + padding );
	const bool hasEnough = ( bytes <= bytesAvailable );
	if ( !hasEnough )
	{
		return nullptr;
	}

	unsigned char * p = freeSpot_ + padding;
	freeSpot_ = p + bytes;
	assert( p < freeSpot_ );
	assert( freeSpot_ <= end );
	assert( reinterpret_cast< const std::size_t >( p ) % alignment == 0 );
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
	const std::size_t freePlace = reinterpret_cast< const std::size_t >( freeSpot_ );
	const std::size_t padding = CalculatePadding( alignment );
	const std::size_t blockPlace = reinterpret_cast< const std::size_t >( block_ );
	const std::size_t bytesAvailable = ( blockPlace + blockSize ) - ( freePlace + padding );
	const bool hasEnough = ( bytes <= bytesAvailable );
	return hasEnough;
}

// ----------------------------------------------------------------------------

bool LinearBlock::IsCorrupt( std::size_t blockSize, std::size_t alignment ) const
{
	assert( this != nullptr );
	assert( block_ != nullptr );
	assert( freeSpot_ != nullptr );
	assert( block_ <= freeSpot_ );
	assert( block_ + blockSize >= freeSpot_ );
	return false;
}

// ----------------------------------------------------------------------------

// } // end internal namespace

} // end project namespace
