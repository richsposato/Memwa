
#include "StackAllocator.hpp"

#include "LockGuard.hpp"

#include "BlockInfo.hpp"
#include "ManagerImpl.hpp"

#include <new>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>

#include <cstdlib>
#include <cassert>

namespace memwa
{

// ----------------------------------------------------------------------------

/** @class StackBlock Info about a single block of memory.

 This class was designed to act as a POD - Plain Old Data - object so that it can be easily copied
 and moved with zero extra allocations or steps. This means there is no destructor to delete the
 memory block since such a destructor would be called whenever a container of these reallocated its
 own memory.

 This class does not store the block size or alignment size. Those values are provided by the
 StackAllocator object that owns this StackBlock. Since those values would be the same for all
 StackBlocks owned by the same StackAllocator, there is no need to store duplicate values in
 each StackBlock. By not storing them here, each act of copying a StackBlock is quicker.

 Down at this level, each operation has to be simple and efficient so the memory handler uses as
 few CPU cycles as possible. Each action should be done in constant time.
 */
class StackBlock
{
public:

	/** Allocates a block of memory of a given size to match a given alignment.
	 @param blockSize Number of bytes in each block.
	 @param alignment Byte boundaries to align allocations. Must be power of two. (e.g. - 1, 2, 4, 8, or 16.)
	 This will throw an exception if it can't allocate a block.
	 */
	StackBlock( std::size_t blockSize, std::size_t alignment );

	/// There is no destructor, so the Destroy function releases the block.
	void Destroy();

	/** Allocates a chunk of particular size.
	 @return Pointer to chunk, or nullptr.
	 */
	void * Allocate( std::size_t size, std::size_t blockSize, std::size_t alignment );

	/** Releases the memory chunk at chunk. This will only release a chunk if the address is within
	 the block and the chunk is at the top of the block's local stack. It will not release chunks
	 below the top of the stack. This means chunks should be released in reverse order of how they
	 were allocated.
	 @return True if released, false if not.
	 */
	bool Release( void * chunk, std::size_t size, std::size_t alignment );

	std::size_t Resize( void * chunk, std::size_t oldSize, std::size_t newSize, std::size_t blockSize, std::size_t alignment );

	/// Returns true if the chunk is within this memory block.
	bool HasAddress( const void * chunk, std::size_t blockSize ) const;

	/// Returns true if the top of this memory block is below the chunk.
	bool IsBelowAddress( const void * chunk, std::size_t blockSize ) const;

	/// Returns true if this block has enough bytes left to allocate a chunk.
	bool HasBytesAvailable( std::size_t bytes, std::size_t blockSize, std::size_t alignment ) const;

	bool operator < ( const StackBlock & that ) const
	{
		return ( block_ < that.block_ );
	}

	/// Returns true if this memory block has no allocations.
	bool IsEmpty( std::size_t alignment ) const;

	/// Returns true if this is corrupt, else false if not corrupt.
	bool IsCorrupt( std::size_t blockSize, std::size_t alignment ) const;

	/// Returns the number of available bytes within this block.
	std::size_t GetFreeBytes( std::size_t blockSize ) const;

	unsigned char * GetAddress() const
	{
		return block_;
	}

#ifdef MEMWA_DEBUGGING_ALLOCATORS

	/// Used only for debugging. Dumps info on this block to stdout.
	void OutputContents( std::size_t blockSize ) const;

#endif

private:

	/// A few bytes of memory in each chunk stores the size of the previous chunk.
	struct ChunkInfo
	{
		std::size_t prevChunkSize_;
	};

	/// Returns address of previous chunk of memory.
	static unsigned char * GetPreviousPlace( unsigned char * chunk );

	/// Calculates the number of bytes needed for an allocation of certain byte size and alignment.
	std::size_t CalculateBytesNeeded( std::size_t bytes, std::size_t alignment ) const;

	/// Pointer to base of entire memory page allocated.
	unsigned char * block_;
	/// Pointer to next free spot within this block.
	unsigned char * freeSpot_;
};

// ----------------------------------------------------------------------------

StackBlock::StackBlock( std::size_t size, std::size_t alignment ) :
	block_( reinterpret_cast< unsigned char * >( std::malloc( size ) ) ),
	freeSpot_( block_ )
{
	if ( nullptr == block_ )
	{
		throw std::bad_alloc();
	}
	// This if statement is in case the caller wants this allocator's alignment to be bigger than operating system alignment.
	const std::size_t remainder = reinterpret_cast< std::size_t >( block_ ) % alignment;
	if ( remainder != 0 )
	{
		freeSpot_ += remainder;
		assert( reinterpret_cast< std::size_t >( freeSpot_ ) % alignment == 0 );
	}
}

// ----------------------------------------------------------------------------

void StackBlock::Destroy()
{
	std::free( block_ );
	block_ = nullptr;
}

// ----------------------------------------------------------------------------

std::size_t StackBlock::CalculateBytesNeeded( std::size_t bytes, std::size_t alignment ) const
{
	const std::size_t bytesPlusInfo = bytes + sizeof(ChunkInfo);
	std::size_t multiplier = std::max( bytesPlusInfo / alignment, std::size_t(1) );
	if ( multiplier * alignment < bytesPlusInfo )
	{
		multiplier++;
	}
	const std::size_t bytesNeeded = ( multiplier * alignment );
	return bytesNeeded;
}

// ----------------------------------------------------------------------------
  
void * StackBlock::Allocate( std::size_t bytes, std::size_t blockSize, std::size_t alignment )
{
	const std::size_t bytesNeeded = CalculateBytesNeeded( bytes, alignment );
	const std::size_t bytesAvailable = ( block_ + blockSize ) - freeSpot_;
	const bool hasEnough = ( bytesNeeded <= bytesAvailable );
	if ( !hasEnough )
	{
		return nullptr;
	}

	void * p = freeSpot_;
	freeSpot_ += bytesNeeded;
	unsigned char * place = freeSpot_ - sizeof(ChunkInfo);
	ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( place );
	chunk->prevChunkSize_ = bytesNeeded;

	return p;
}

// ----------------------------------------------------------------------------

bool StackBlock::Release( void * place, std::size_t bytes, std::size_t alignment )
{
	if ( IsEmpty( alignment ) )
	{
		return false;
	}
	unsigned char * p = freeSpot_ - sizeof(ChunkInfo);
	ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( p );
	const std::size_t bytesNeeded = CalculateBytesNeeded( bytes, alignment );
	if ( chunk->prevChunkSize_ != bytesNeeded )
	{
		return false;
	}
	p = freeSpot_ - chunk->prevChunkSize_;
	if ( place != p )
	{
		return false;
	}
	freeSpot_ = p;
	return true;
}

// ----------------------------------------------------------------------------

std::size_t StackBlock::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t blockSize, std::size_t alignment )
{
	if ( oldSize == newSize )
	{
		return newSize;
	}
	if ( IsEmpty( alignment ) )
	{
		return 0;
	}
	unsigned char * p = freeSpot_ - sizeof(ChunkInfo);
	const std::size_t oldBytesNeeded = CalculateBytesNeeded( oldSize, alignment );
	ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( p );
	if ( chunk->prevChunkSize_ != oldBytesNeeded )
	{
		return 0;
	}
	p = freeSpot_ - chunk->prevChunkSize_;
	if ( place != p )
	{
		return 0;
	}
	const std::size_t bytesNeeded = CalculateBytesNeeded( newSize, alignment );
	if ( oldSize < newSize )
	{
		const std::size_t bytesAvailable = ( block_ + blockSize ) - p;
		const bool hasEnough = ( bytesNeeded <= bytesAvailable );
		if ( !hasEnough )
		{
			return 0;
		}
	}

	freeSpot_ = p + bytesNeeded;
	unsigned char * newChunk = freeSpot_ - sizeof(ChunkInfo);
	chunk = reinterpret_cast< ChunkInfo * >( newChunk );
	chunk->prevChunkSize_ = bytesNeeded;

	return true;
}

// ----------------------------------------------------------------------------

bool StackBlock::IsBelowAddress( const void * chunk, std::size_t blockSize ) const
{
	const bool below = ( block_ + blockSize <= chunk );
	return below;
}

// ----------------------------------------------------------------------------

bool StackBlock::HasAddress( const void * chunk, std::size_t blockSize ) const
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

bool StackBlock::HasBytesAvailable( std::size_t bytes, std::size_t blockSize, std::size_t alignment ) const
{
	const std::size_t bytesNeeded = CalculateBytesNeeded( bytes, alignment );
	const std::size_t bytesAvailable = ( block_ + blockSize ) - freeSpot_;
	const bool hasEnough = ( bytesNeeded <= bytesAvailable );
	return hasEnough;
}

// ----------------------------------------------------------------------------

bool StackBlock::IsEmpty( std::size_t alignment ) const
{
	const bool empty = ( block_ == freeSpot_ );
	return empty;
}

// ----------------------------------------------------------------------------

std::size_t StackBlock::GetFreeBytes( std::size_t blockSize ) const
{
	const std::size_t bytesAvailable = ( block_ + blockSize ) - freeSpot_;
	return bytesAvailable;	
}

// ----------------------------------------------------------------------------

unsigned char * StackBlock::GetPreviousPlace( unsigned char * place )
{
	unsigned char * p = place - sizeof(ChunkInfo);
	const ChunkInfo * chunk = reinterpret_cast< ChunkInfo * >( p );
	unsigned char * prev = place - chunk->prevChunkSize_;
	return prev;
}

// ----------------------------------------------------------------------------

bool StackBlock::IsCorrupt( std::size_t blockSize, std::size_t alignment ) const
{
	assert( this != nullptr );
	assert( block_ != nullptr );
	assert( block_ <= freeSpot_ );
	assert( block_ + blockSize >= freeSpot_ );

	const std::size_t remainder = reinterpret_cast< std::size_t >( block_ ) % alignment;
	const unsigned char * firstPlace = block_ + remainder;
	const bool empty = ( firstPlace == freeSpot_ );

	if ( !empty )
	{
		unsigned char * prev = GetPreviousPlace( freeSpot_ );
		while ( prev > firstPlace )
		{
			prev = GetPreviousPlace( prev );
		}
		assert( prev == firstPlace ); // pointer to previous chunk may not be before first chunk.
	}
	return false;
}

// ----------------------------------------------------------------------------

#ifdef MEMWA_DEBUGGING_ALLOCATORS

void StackBlock::OutputContents( std::size_t blockSize ) const
{
	const void * p = reinterpret_cast< const void * >( block_ );
	const void * f = reinterpret_cast< const void * >( freeSpot_ );
	const std::size_t freeBytes = ( block_ + blockSize ) - freeSpot_;
	const std::size_t usedBytes = freeSpot_ - block_;
	std::cout << '\t' << this
		<< '\t' << " Block: " << p
		<< '\t' << " Free Spot: " << f
		<< '\t' << " Free Bytes: " << freeBytes
		<< '\t' << " Used Bytes: " << usedBytes
		<< std::endl;
}

#endif

// ----------------------------------------------------------------------------

StackAllocator::StackAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment ) :
	Allocator(),
	info_( initialBlocks, blockSize, alignment )
{
}

// ----------------------------------------------------------------------------

StackAllocator::~StackAllocator()
{
}

// ----------------------------------------------------------------------------

void StackAllocator::Destroy()
{
	info_.Destroy();
}

// ----------------------------------------------------------------------------

void * StackAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	if ( info_.blockSize_ < size )
	{
		// return some error condition through output parameter?
		throw std::invalid_argument( "Requested allocation size must be smaller than the memory block size." );
	}

	void * p = info_.Allocate( size, hint );
	return p;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * StackAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint )
#else
void * StackAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint )
#endif
{

	void * p = Allocate( size, alignment, hint );
	if ( nullptr != p )
	{
		return p;
	}
	if ( TrimEmptyBlocks() )
	{
		p = Allocate( size, alignment, hint );
		if ( nullptr != p )
		{
			return p;
		}
	}
	memwa::impl::ManagerImpl::GetManager()->TrimEmptyBlocks( this );
	p = Allocate( size, alignment, hint );
	if ( ( nullptr == p ) && doThrow )
	{
		throw std::bad_alloc();
	}

	return p;
}

// ----------------------------------------------------------------------------

void * StackAllocator::Allocate( std::size_t size, bool doThrow, const void * hint )
{
	void * p = StackAllocator::Allocate( size, doThrow, hint );
	return p;
}

// ----------------------------------------------------------------------------

bool StackAllocator::Release( void * place, std::size_t size )
{
	if ( nullptr == place )
	{
		return false;
	}
	const bool success = info_.Release( place, size );
	return success;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool StackAllocator::Release( void * place, std::size_t size, std::align_val_t alignment )
#else
bool StackAllocator::Release( void * place, std::size_t size, std::size_t alignment )
#endif
{
	if ( nullptr == place )
	{
		return false;
	}
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	const bool success = info_.Release( place, size );
	return success;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
std::size_t StackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment )
#else
std::size_t StackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment )
#endif
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	const std::size_t actualSize = info_.Resize( place, oldSize, newSize );
	return actualSize;
}

// ----------------------------------------------------------------------------

std::size_t StackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize )
{
	const std::size_t actualSize = Resize( place, oldSize, newSize );
	return actualSize;
}

// ----------------------------------------------------------------------------

unsigned long long StackAllocator::GetMaxSize( std::size_t objectSize ) const
{
	const unsigned long long bytesAvailable = memwa::impl::GetTotalAvailableMemory();
	const unsigned long long maxPossibleBlocks = bytesAvailable / info_.blockSize_;
	const unsigned long long maxPossibleObjects = maxPossibleBlocks / objectSize;
	return maxPossibleObjects;
}

// ----------------------------------------------------------------------------

bool StackAllocator::HasAddress( void * place ) const
{
	const bool hasIt = info_.HasAddress( place );
	return hasIt;
}

// ----------------------------------------------------------------------------

bool StackAllocator::TrimEmptyBlocks()
{
	const bool trimmed = info_.TrimEmptyBlocks();
	return trimmed;
}

// ----------------------------------------------------------------------------

bool StackAllocator::IsCorrupt() const
{
	assert( nullptr != this );
	const bool corrupt = info_.IsCorrupt();
	return corrupt;
}

// ----------------------------------------------------------------------------

float StackAllocator::GetFragmentationPercent() const
{
	std::size_t blockCount = 0;
	std::size_t excessBlocks = 0;
	info_.GetBlockCounts( blockCount, excessBlocks );
	if ( 0 == blockCount )
	{
		return 0.0F;
	}
	const float percent = (float)excessBlocks / (float)blockCount;
	return percent;
}

// ----------------------------------------------------------------------------

#ifdef MEMWA_DEBUGGING_ALLOCATORS

void StackAllocator::OutputContents() const
{
	std::cout << "This StackAllocator: " << this
		<< '\t' << " Fragmentation: " << GetFragmentationPercent()
		<< std::endl;
	for ( unsigned int ii = 0; ii < alignmentCount; ++ii )
	{
		StackBlockInfo * info = info_[ ii ];
		if ( info != nullptr )
		{
			info_.OutputContents();
		}
	}
}

#endif

// ----------------------------------------------------------------------------

ThreadSafeStackAllocator::ThreadSafeStackAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment ) :
	StackAllocator( initialBlocks, blockSize, alignment ),
	mutex_()
{
}

// ----------------------------------------------------------------------------

ThreadSafeStackAllocator::~ThreadSafeStackAllocator()
{
}

// ----------------------------------------------------------------------------

void * ThreadSafeStackAllocator::Allocate( std::size_t size, bool doThrow, const void * hint )
{
	LockGuard guard( mutex_ );
	return StackAllocator::Allocate( size, doThrow );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafeStackAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint )
#else
void * ThreadSafeStackAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint )
#endif
{
	LockGuard guard( mutex_ );
	return StackAllocator::Allocate( size, doThrow, alignment );
}

// ----------------------------------------------------------------------------

bool ThreadSafeStackAllocator::Release( void * place, std::size_t size )
{
	LockGuard guard( mutex_ );
	return StackAllocator::Release( place, size );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool ThreadSafeStackAllocator::Release( void * place, std::size_t size, std::align_val_t alignment )
#else
bool ThreadSafeStackAllocator::Release( void * place, std::size_t size, std::size_t alignment )
#endif
{
	LockGuard guard( mutex_ );
	return StackAllocator::Release( place, size, alignment );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
std::size_t ThreadSafeStackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment )
#else
std::size_t ThreadSafeStackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment )
#endif
{
	LockGuard guard( mutex_ );
	return StackAllocator::Resize( place, oldSize, newSize, alignment );
}

// ----------------------------------------------------------------------------

std::size_t ThreadSafeStackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize )
{
	LockGuard guard( mutex_ );
	return StackAllocator::Resize( place, oldSize, newSize );
}

// ----------------------------------------------------------------------------

bool ThreadSafeStackAllocator::HasAddress( void * place ) const
{
	LockGuard guard( mutex_ );
	return StackAllocator::HasAddress( place );
}

// ----------------------------------------------------------------------------

bool ThreadSafeStackAllocator::TrimEmptyBlocks()
{
	LockGuard guard( mutex_ );
	return StackAllocator::TrimEmptyBlocks();
}

// ----------------------------------------------------------------------------

bool ThreadSafeStackAllocator::IsCorrupt() const
{
	LockGuard guard( mutex_ );
	return StackAllocator::IsCorrupt();
}

// ----------------------------------------------------------------------------

float ThreadSafeStackAllocator::GetFragmentationPercent() const
{
	LockGuard guard( mutex_ );
	return StackAllocator::GetFragmentationPercent();
}

// ----------------------------------------------------------------------------

} // end project namespace
