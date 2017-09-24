
#include "StackBlock.hpp"

#include "ManagerImpl.hpp"

#include <cassert>
#include <cstdlib>

#include <iostream>

namespace memwa
{

// ----------------------------------------------------------------------------

StackBlock::StackBlock( const std::size_t blockSize, const std::size_t alignment ) :
	block_( reinterpret_cast< unsigned char * >( std::malloc( blockSize ) ) ),
	freeSpot_( block_ )
{
	if ( nullptr == block_ )
	{
		throw std::bad_alloc();
	}
	assert( alignment != 0 );
/*
	const std::size_t blockPlace = reinterpret_cast< std::size_t >( block_ );
	// This if statement is in case the caller wants this allocator's alignment to be bigger than operating system alignment.
	const std::size_t remainder = blockPlace % alignment;
	if ( remainder != 0 )
	{
		const std::size_t padding = alignment - remainder;
		freeSpot_ += padding;
	}
*/
	assert( reinterpret_cast< std::size_t >( freeSpot_ ) % alignment == 0 );
}

// ----------------------------------------------------------------------------

void StackBlock::Destroy()
{
//	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	std::free( block_ );
	block_ = nullptr;
	freeSpot_ = nullptr;
}

// ----------------------------------------------------------------------------

void * StackBlock::Allocate( const std::size_t bytes, const std::size_t blockSize, const std::size_t alignment )
{
//	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	const std::size_t bytesNeeded = memwa::impl::CalculateAlignedSize( bytes, alignment ) + sizeof(ChunkInfo);
	const std::size_t bytesAvailable = ( block_ + blockSize ) - freeSpot_;
	const bool hasEnough = ( bytesNeeded <= bytesAvailable );
	if ( !hasEnough )
	{
		return nullptr;
	}

	unsigned char * p = reinterpret_cast< unsigned char * >( freeSpot_ );
	assert( p != nullptr );
	assert( p >= block_ );
	freeSpot_ += bytesNeeded;
	unsigned char * place = freeSpot_ - sizeof(ChunkInfo);
	ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( place );
	chunk->prevChunkSize_ = bytesNeeded;
	chunk->prevChunk_ = reinterpret_cast< unsigned char * >( p );
	assert( chunk->IsValid( block_, blockSize, alignment ) );
	assert( chunk->prevChunkSize_ + chunk->prevChunk_ == freeSpot_ );
	assert( reinterpret_cast< std::size_t >( p ) % alignment == 0 );
	assert( reinterpret_cast< unsigned char * >( chunk ) > p );

	return p;
}

// ----------------------------------------------------------------------------

bool StackBlock::Release( void * place, const std::size_t bytes, const std::size_t blockSize, const std::size_t alignment )
{
//	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	if ( IsEmpty( alignment ) )
	{
		return false;
	}
	unsigned char * p = freeSpot_ - sizeof(ChunkInfo);
	ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( p );
	assert( chunk->IsValid( block_, blockSize, alignment ) );
	assert( chunk->prevChunkSize_ + chunk->prevChunk_ == freeSpot_ );
	const std::size_t bytesNeeded = memwa::impl::CalculateAlignedSize( bytes, alignment ) + sizeof(ChunkInfo);
	if ( chunk->prevChunkSize_ != bytesNeeded )
	{
		throw std::invalid_argument( "Error found by StackAllocator. The memory requested to release does not match internal storage of that size." );
	}
	if ( place < chunk->prevChunk_ )
	{
		throw std::invalid_argument( "Error found by StackAllocator. The requested place to release is not the most recently allocated chunk in its block." );
	}
	if ( place != chunk->prevChunk_ )
	{
		throw std::invalid_argument( "Error found by StackAllocator. The requested place to release does not match internal storage of that place." );
	}
	freeSpot_ = chunk->prevChunk_;

	return true;
}

// ----------------------------------------------------------------------------

bool StackBlock::Resize( void * place, const std::size_t oldSize, const std::size_t newSize, const std::size_t blockSize, const std::size_t alignment )
{
//	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	if ( IsEmpty( alignment ) )
	{
//		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		return false;
	}

	unsigned char * p = freeSpot_ - sizeof(ChunkInfo);
	ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( p );
	assert( chunk->IsValid( block_, blockSize, alignment ) );
	assert( chunk->prevChunkSize_ + chunk->prevChunk_ == freeSpot_ );
	if ( place < chunk->prevChunk_ )
	{
		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		throw std::invalid_argument( "Error found by StackAllocator. The requested place to resize is not the most recently allocated chunk in its block." );
	}
	if ( place != chunk->prevChunk_ )
	{
		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		throw std::invalid_argument( "Error found by StackAllocator. The requested place to release does not match internal storage of that place." );
	}
	const std::size_t oldBytesNeeded = memwa::impl::CalculateAlignedSize( oldSize, alignment ) + sizeof(ChunkInfo);
	if ( chunk->prevChunkSize_ != oldBytesNeeded )
	{
/*		std::cout << __FUNCTION__ << " : " << __LINE__
			<< "   prev chunk size : " << chunk->prevChunkSize_
			<< "   old bytes needed : " << oldBytesNeeded
			<< "   old size : " << oldSize << std::endl; */
		throw std::invalid_argument( "Error found by StackAllocator. The memory requested to resize does not match internal storage of that size." );
	}

	const std::size_t newBytesNeeded = memwa::impl::CalculateAlignedSize( newSize, alignment ) + sizeof(ChunkInfo);
	if ( oldBytesNeeded < newBytesNeeded )
	{
		const std::size_t byteDifference = freeSpot_ - block_;
		const std::size_t bytesAvailable = blockSize - byteDifference;
		const std::size_t neededDifference = newBytesNeeded - oldBytesNeeded;
		const bool hasEnough = ( neededDifference <= bytesAvailable );
		if ( !hasEnough )
		{
/*			std::cout << __FUNCTION__ << " : " << __LINE__ << "  blockSize: " << blockSize
				<< "  oldSize: " << oldSize << "  newSize: " << newSize << "  alignment: " << alignment
				<< "  available: " << bytesAvailable << "  bytesNeeded: " << newBytesNeeded << std::endl;
			OutputContents( blockSize, alignment ); */
			return false;
		}
	}

	freeSpot_ = chunk->prevChunk_ + newBytesNeeded;
	unsigned char * prevChunk = chunk->prevChunk_;
	p = freeSpot_ - sizeof(ChunkInfo);
	chunk = reinterpret_cast< ChunkInfo * >( p );
	chunk->prevChunk_ = prevChunk;
	chunk->prevChunkSize_ = newBytesNeeded;
	assert( chunk->IsValid( block_, blockSize, alignment ) );

	return true;
}

// ----------------------------------------------------------------------------

bool StackBlock::IsBelowAddress( const void * chunk, const std::size_t blockSize ) const
{
	const bool below = ( block_ + blockSize <= chunk );
	return below;
}

// ----------------------------------------------------------------------------

bool StackBlock::HasAddress( const void * chunk, const std::size_t blockSize ) const
{
	if ( chunk < block_ )
	{
		return false;
	}
	if ( chunk >= block_ + blockSize )
	{
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------------

bool StackBlock::HasBytesAvailable( const std::size_t bytes, const std::size_t blockSize, const std::size_t alignment ) const
{
	const std::size_t bytesNeeded = memwa::impl::CalculateAlignedSize( bytes, alignment ) + sizeof(ChunkInfo);
	const std::size_t bytesAvailable = ( block_ + blockSize ) - freeSpot_;
	const bool hasEnough = ( bytesNeeded <= bytesAvailable );
	return hasEnough;
}

// ----------------------------------------------------------------------------

bool StackBlock::IsEmpty( const std::size_t alignment ) const
{
	const bool empty = ( block_ == freeSpot_ );
	return empty;
}

// ----------------------------------------------------------------------------

std::size_t StackBlock::GetChunkSize( const unsigned int index, const std::size_t blockSize, const std::size_t alignment ) const
{
	if ( block_ == freeSpot_ )
	{
		return 0;
	}
	unsigned int count = 0;
	unsigned char * place = freeSpot_ - sizeof(ChunkInfo);
	while ( place > block_ )
	{
		const ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( place );
		assert( chunk->IsValid( block_, blockSize, alignment ) );
		if ( index == count )
		{
			return chunk->prevChunkSize_ - sizeof(ChunkInfo);
		}
		place = chunk->prevChunk_ - sizeof(ChunkInfo);
		++count;
	}
	assert( place == block_ ); // pointer to previous chunk may not be before first chunk.
	return 0;
}

// ----------------------------------------------------------------------------

unsigned int StackBlock::GetObjectCount( const std::size_t blockSize, const std::size_t alignment ) const
{
	if ( block_ == freeSpot_ )
	{
		return 0;
	}
	unsigned int count = 0;
	unsigned char * place = freeSpot_;
	while ( place > block_ )
	{
		place -= sizeof(ChunkInfo);
		assert( place > block_ );
		const ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( place );
		assert( chunk->IsValid( block_, blockSize, alignment ) );
		place = chunk->prevChunk_;
		++count;
	}
	assert( place == block_ ); // pointer to previous chunk may not be before first chunk.
	return count;
}

// ----------------------------------------------------------------------------

std::size_t StackBlock::GetFreeBytes( const std::size_t blockSize ) const
{
	const std::size_t byteDifference = freeSpot_ - block_;
	const std::size_t bytesAvailable = blockSize - byteDifference;
	if ( bytesAvailable < sizeof(ChunkInfo) )
	{
		return 0;
	}
	return bytesAvailable - sizeof(ChunkInfo);
}

// ----------------------------------------------------------------------------

unsigned char * StackBlock::GetPreviousPlace( unsigned char * place, const std::size_t blockSize, const std::size_t alignment )
{
	unsigned char * p = place - sizeof(ChunkInfo);
	const ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( p );
	assert( chunk->IsValid( block_, blockSize, alignment ) );
	unsigned char * prev = chunk->prevChunk_;
	return prev;
}

// ----------------------------------------------------------------------------

bool StackBlock::IsCorrupt( const std::size_t blockSize, const std::size_t alignment ) const
{
//	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
//	OutputContents( blockSize, alignment );
	assert( this != nullptr );
	if ( block_ == nullptr )
	{
		assert( freeSpot_ == nullptr );
		return false;
	}
	assert( block_ <= freeSpot_ );
	assert( block_ + blockSize >= freeSpot_ );

/*
	const std::size_t remainder = reinterpret_cast< std::size_t >( block_ ) % alignment;
	const std::size_t padding = ( remainder == 0 ) ? 0 : alignment - remainder;
	const unsigned char * firstPlace = block_ + padding;
	assert( firstPlace <= freeSpot_ );
	const bool empty = ( firstPlace == freeSpot_ );
*/
	const bool empty = ( block_ == freeSpot_ );

	if ( !empty )
	{
		const std::size_t byteDifference = freeSpot_ - block_;
		assert( byteDifference > sizeof(ChunkInfo) );
		unsigned char * place = freeSpot_;
		while ( place > block_ )
		{
			place -= sizeof(ChunkInfo);
			assert( place > block_ );
			const ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( place );
			assert( chunk->IsValid( block_, blockSize, alignment ) );
			assert( chunk->prevChunk_ + chunk->prevChunkSize_ <= block_ + blockSize );
			place = chunk->prevChunk_;
		}
		assert( place == block_ ); // pointer to previous chunk may not be before first chunk.
	}
	return false;
}

// ----------------------------------------------------------------------------

// #ifdef MEMWA_DEBUGGING_ALLOCATORS

void StackBlock::OutputContents( const std::size_t blockSize, const std::size_t alignment ) const
{
//	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	const bool empty = ( freeSpot_ == block_ );
	const std::size_t * p = reinterpret_cast< std::size_t * >( block_ );
	const std::size_t * f = reinterpret_cast< std::size_t * >( freeSpot_ );
	const std::size_t freeBytes = ( block_ + blockSize ) - freeSpot_;
	const std::size_t usedBytes = freeSpot_ - block_;
	std::cout << '\t' << this
		<< '\t' << " Block: " << reinterpret_cast< std::size_t >( p )
		<< '\t' << " Free Spot: " << reinterpret_cast< std::size_t >( f )
		<< '\t' << " Free Bytes: " << freeBytes
		<< '\t' << " Used Bytes: " << usedBytes
		<< '\t' << " Empty? " << empty
		<< std::endl;

	if ( !empty )
	{
		unsigned char * place = freeSpot_;
		while ( place > block_ )
		{
			place -= sizeof(ChunkInfo);
			const ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( place );
			std::cout << "\t\t  place: " << reinterpret_cast< std::size_t >( chunk )
				<< " \t chunk: " << reinterpret_cast< std::size_t >( chunk->prevChunk_ )
				<< " \t chunk size: " << chunk->prevChunkSize_
				<< " \t object size: " << chunk->prevChunkSize_ - sizeof(ChunkInfo) << std::endl;
			assert( chunk->IsValid( block_, blockSize, alignment ) );
			place = chunk->prevChunk_;
		}
		assert( place == block_ );
	}
}

// #endif

// ----------------------------------------------------------------------------

const bool StackBlock::ChunkInfo::IsValid( const unsigned char * block, const std::size_t blockSize, const std::size_t alignment ) const
{
	assert( this != nullptr );
	assert( prevChunk_ != nullptr );
	assert( prevChunk_ >= block );
	assert( prevChunkSize_ >= sizeof(ChunkInfo) + alignment );
	assert( prevChunkSize_ < blockSize );
	const unsigned char * const pThis = reinterpret_cast< const unsigned char * const >( this );
	assert( prevChunkSize_ + prevChunk_ - sizeof(ChunkInfo) == pThis );
	return true;
}

// ----------------------------------------------------------------------------

}
