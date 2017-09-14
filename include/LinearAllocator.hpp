
#pragma once

#include "AllocatorManager.hpp"

#include "BlockInfo.hpp"

#include <cstddef> // For std::size_t.

#include <mutex>

namespace memwa
{

class AllocatorManager;
class LinearBlock;

/** @class LinearAllocator
 This memory handler allocates chunks very quickly, but does not release any chunks within a block.
 Since it is not a general purpose allocator, it focuses on very quick allocations.

 # Usage Patterns
 - Objects that are allocated once and never released.
 - Allocating a stack for a local thread. The stack is allocated all at once, and released all at once.
 - The many objects that are allocated at the start of a program and never released.
 */
class LinearAllocator : public Allocator
{
public:

	/** Allocates a chunk of memory from a block.
	 @param size Number of bytes to allocate.
	 @param doThrow True if this should throw a bad_alloc exception instead of returning nullptr.
	 @return Pointer to chunk of memory of at least size bytes, or nullptr if it could not allocate.
	 */
	virtual void * Allocate( std::size_t size, bool doThrow, const void * hint = nullptr ) override;

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual void * Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint = nullptr ) override;

#else

	virtual void * Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint = nullptr ) override;

#endif

	virtual unsigned long long GetMaxSize( std::size_t objectSize ) const override;

	bool HasAddress( void * place, std::size_t alignment ) const;

	/// Returns true if a block of memory managed by this object owns the chunk at the place
	virtual bool HasAddress( void * place ) const override;

	/// Deletes any blocks that have zero allocations.
	virtual bool TrimEmptyBlocks() override;

	/// Returns true if any memory block was corrupted.
	virtual bool IsCorrupt() const override;

	virtual float GetFragmentationPercent() const override;

#ifdef DEBUGGING_ALLOCATORS
	/// Used only for debugging. Dumps info about each block to stdout.
	void OutputContents() const;
#endif

protected:

	/** Creats allocator by creating blocks for a specific alignment size. This will throw a bad_alloc
	 exception if it can't pre-allocate the number of blocks requested.
	 @param initialBlocks Number of blocks to pre-allocate.
	 @param blockSize Number of bytes in each block. Should be multiple of alignment.
	 @param alignment Byte boundaries to align allocations. Must be power of two. (e.g. - 1, 2, 4, 8, or 16.)

	 @note If you make the block size the same as either the L1 or L2 cache sizes on the CPU, which are 32K
	  and 256K respectively for many Intel chips, then the CPU can load an entire block of objects into the
	  cache at once. If your program uses many objects at the same time and those are on the same memory
	  page, the program is less likely to cause a cache miss.

	 @return True for success. False if alignment is invalid, blockSize is not a multiple of alignment, or
	  initialBlocks is zero.
	 */
	LinearAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment );

	virtual ~LinearAllocator();

private:

	friend class memwa::AllocatorManager;

	LinearAllocator() = delete;
	LinearAllocator( const LinearAllocator & ) = delete;
	LinearAllocator( LinearAllocator && ) = delete;
	LinearAllocator & operator = ( const LinearAllocator & ) = delete;
	LinearAllocator & operator = ( LinearAllocator && ) = delete;

	/// Goes through container of blocks to delete each one.
	virtual void Destroy();

	void * Allocate( std::size_t size, std::size_t alignment, const void * hint );

	LinearBlockInfo info_;

};

class ThreadSafeLinearAllocator : public LinearAllocator
{
public:

	/** Allocates a chunk of memory from a block.
	 @param size Number of bytes to allocate.
	 @param doThrow True if this should throw a bad_alloc exception instead of returning nullptr.
	 @return Pointer to chunk of memory of at least size bytes, or nullptr if it could not allocate.
	 */
	virtual void * Allocate( std::size_t size, bool doThrow, const void * hint = nullptr ) override;

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual void * Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint = nullptr ) override;

#else

	virtual void * Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint = nullptr ) override;

#endif

	/// Returns true if a block of memory managed by this object owns the chunk at the place
	virtual bool HasAddress( void * place ) const override;

	/// Deletes any blocks that have zero allocations.
	virtual bool TrimEmptyBlocks() override;

	/// Returns true if any memory block was corrupted.
	virtual bool IsCorrupt() const override;

	virtual float GetFragmentationPercent() const override;

private:

	friend class memwa::AllocatorManager;

	ThreadSafeLinearAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment );

	virtual ~ThreadSafeLinearAllocator();

	ThreadSafeLinearAllocator() = delete;
	ThreadSafeLinearAllocator( const ThreadSafeLinearAllocator & ) = delete;
	ThreadSafeLinearAllocator( ThreadSafeLinearAllocator && ) = delete;
	ThreadSafeLinearAllocator & operator = ( const ThreadSafeLinearAllocator & ) = delete;
	ThreadSafeLinearAllocator & operator = ( ThreadSafeLinearAllocator && ) = delete;

	mutable std::mutex mutex_;

};

// ----------------------------------------------------------------------------

} // end project namespace
