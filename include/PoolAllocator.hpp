
#pragma once

#include "AllocatorManager.hpp"

#include "BlockInfo.hpp"

#include <cstddef> // For std::size_t.

#include <mutex>

namespace memwa
{

class AllocatorManager;

/** @class PoolAllocator

 # Usage Patterns
 You can use PoolAllocator for:
 - Objects that are all the same size.
 - Objects that are allocated and released in any order.
 */
class PoolAllocator : public Allocator
{
public:

	/** Allocates a chunk of memory from a block.
	 @param size Number of bytes to allocate.
	 @param doThrow True if this should throw a bad_alloc exception instead of returning nullptr.
	 @return Pointer to chunk of memory of at least size bytes, or nullptr if it could not allocate.
	 */
	virtual void * Allocate( std::size_t size, bool doThrow ) override;

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual void * Allocate( std::size_t size, bool doThrow, std::align_val_t alignment ) override;

#else

	virtual void * Allocate( std::size_t size, bool doThrow, std::size_t alignment ) override;

#endif

	/** Releases a chunk of memory.
	 @param place Address of chunk owned by this memory handler.
	 @param size Number of bytes in chunk.
	 */
	virtual bool Release( void * place, std::size_t size ) override;

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual bool Release( void * place, std::size_t size, std::align_val_t alignment ) override;

#else

	virtual bool Release( void * place, std::size_t size, std::size_t alignment ) override;

#endif

	/// Returns true if a block of memory managed by this object owns the chunk at the place
	virtual bool HasAddress( void * place ) const override;

	/// Deletes any blocks that have zero allocations.
	virtual bool TrimEmptyBlocks() override;

	/// Returns true if any memory block was corrupted.
	virtual bool IsCorrupt() const override;

	virtual float GetFragmentationPercent() const override;

#ifdef MEMWA_DEBUGGING_ALLOCATORS

	/// Used only for debugging. Dumps info about each block to stdout.
	void OutputContents() const;

#endif

protected:

	/// Creates allocator.
	PoolAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t objectSize, std::size_t alignment );

	/// The destructor will delete all blocks if the destroy flag is set.
	virtual ~PoolAllocator();

private:

	friend class memwa::AllocatorManager;

	PoolAllocator() = delete;
	PoolAllocator( const PoolAllocator & ) = delete;
	PoolAllocator( PoolAllocator && ) = delete;
	PoolAllocator & operator = ( const PoolAllocator & ) = delete;
	PoolAllocator & operator = ( PoolAllocator && ) = delete;

	/// Goes through container of blocks to delete each one.
	void Destroy();

	void * Allocate( std::size_t size, std::size_t alignment );

	PoolBlockInfo info_;

};

// ----------------------------------------------------------------------------

class ThreadSafePoolAllocator : public PoolAllocator
{
public:

	/** Allocates a chunk of memory from a block.
	 @param size Number of bytes to allocate.
	 @param doThrow True if this should throw a bad_alloc exception instead of returning nullptr.
	 @return Pointer to chunk of memory of at least size bytes, or nullptr if it could not allocate.
	 */
	virtual void * Allocate( std::size_t size, bool doThrow ) override;

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual void * Allocate( std::size_t size, bool doThrow, std::align_val_t alignment ) override;

#else

	virtual void * Allocate( std::size_t size, bool doThrow, std::size_t alignment ) override;

#endif

	/** Releases a chunk of memory.
	 @param place Address of chunk owned by this memory handler.
	 @param size Number of bytes in chunk.
	 */
	virtual bool Release( void * place, std::size_t size ) override;

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual bool Release( void * place, std::size_t size, std::align_val_t alignment ) override;

#else

	virtual bool Release( void * place, std::size_t size, std::size_t alignment ) override;

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

	ThreadSafePoolAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t objectSize, std::size_t alignment );

	virtual ~ThreadSafePoolAllocator();

	ThreadSafePoolAllocator() = delete;
	ThreadSafePoolAllocator( const ThreadSafePoolAllocator & ) = delete;
	ThreadSafePoolAllocator( ThreadSafePoolAllocator && ) = delete;
	ThreadSafePoolAllocator & operator = ( const ThreadSafePoolAllocator & ) = delete;

	mutable std::mutex mutex_;

};

// ----------------------------------------------------------------------------

} // end project namespace
