
#pragma once

#include <cstddef> // For std::size_t.

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
	StackBlock( const std::size_t blockSize, const std::size_t alignment );

	/// There is no destructor, so the Destroy function releases the block.
	void Destroy();

	/** Allocates a chunk of particular size.
	 @return Pointer to chunk, or nullptr.
	 */
	void * Allocate( const std::size_t size, const std::size_t blockSize, const std::size_t alignment );

	/** Releases the memory chunk at chunk. This will only release a chunk if the address is within
	 the block and the chunk is at the top of the block's local stack. It will not release chunks
	 below the top of the stack. This means chunks should be released in reverse order of how they
	 were allocated.
	 @return True if released, false if not.
	 */
	bool Release( void * chunk, const std::size_t size, const std::size_t blockSize, const std::size_t alignment );

	bool Resize( void * chunk, const std::size_t oldSize, const std::size_t newSize, const std::size_t blockSize, const std::size_t alignment );

	/// Returns true if the chunk is within this memory block.
	bool HasAddress( const void * chunk, const std::size_t blockSize ) const;

	/// Returns true if the top of this memory block is below the chunk.
	bool IsBelowAddress( const void * chunk, const std::size_t blockSize ) const;

	/// Returns true if this block has enough bytes left to allocate a chunk.
	bool HasBytesAvailable( const std::size_t bytes, const std::size_t blockSize, const std::size_t alignment ) const;

	bool operator < ( const StackBlock & that ) const
	{
		return ( block_ < that.block_ );
	}

	/// Returns true if this memory block has no allocations.
	bool IsEmpty( const std::size_t alignment ) const;

	std::size_t GetChunkSize( const unsigned int index, const std::size_t blockSize, const std::size_t alignment ) const;

	unsigned int GetObjectCount( const std::size_t blockSize, const std::size_t alignment ) const;

	/// Returns true if this is corrupt, else false if not corrupt.
	bool IsCorrupt( std::size_t blockSize, std::size_t alignment ) const;

	/// Returns the number of available bytes within this block.
	std::size_t GetFreeBytes( std::size_t blockSize ) const;

	unsigned char * GetAddress() const
	{
		return block_;
	}

// #ifdef MEMWA_DEBUGGING_ALLOCATORS

	/// Used only for debugging. Dumps info on this block to stdout.
	void OutputContents( std::size_t blockSize, std::size_t alignment ) const;

// #endif

	/// A few bytes of memory in each chunk stores the size of the previous chunk.
	struct ChunkInfo
	{
		const bool IsValid( const unsigned char * block, std::size_t blockSize, const std::size_t alignment ) const;
		unsigned char * prevChunk_;
		std::size_t prevChunkSize_;
	};

private:

	/// Returns address of previous chunk of memory.
	unsigned char * GetPreviousPlace( unsigned char * chunk, std::size_t blockSize, std::size_t alignment );

	/// Pointer to base of entire memory page allocated.
	unsigned char * block_;
	/// Pointer to next free spot within this block.
	unsigned char * freeSpot_;
};

// ----------------------------------------------------------------------------

}
