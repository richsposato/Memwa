
#pragma once

#include "AllocatorManager.hpp"

#include "BlockInfo.hpp"

#include <cstddef> // For std::size_t.

#include <mutex>
#include <vector>

namespace memwa
{

class AllocatorManager;

/** @class StackAllocator
 This memory handler allocates chunks using stack-like behavior; which means it allocates in a linear
 order and expects to release them in reverse order. Since it is not a general purpose allocator, it
 can allocate and release very quickly.

 # Usage Patterns
 You can use StackAllocator for:
 - Objects that are allocated once and never released.
 - Objects that are always released in reverse order of how they were allocated.
 - Objects that are released only when they are constructed as temporary values, such as when returned
   from a function, because then the most recently constructed object is also the one being released.
 */
class StackAllocator : public Allocator
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

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual std::size_t Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment ) override;

#else

	virtual std::size_t Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment ) override;

#endif

	/**
	 */
	virtual std::size_t Resize( void * place, std::size_t oldSize, std::size_t newSize ) override;

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
	StackAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment );

	/// The destructor will delete all blocks if the destroy flag is set.
	virtual ~StackAllocator();

private:

	friend class memwa::AllocatorManager;

	StackAllocator() = delete;
	StackAllocator( const StackAllocator & ) = delete;
	StackAllocator( StackAllocator && ) = delete;
	StackAllocator & operator = ( const StackAllocator & ) = delete;
	StackAllocator & operator = ( StackAllocator && ) = delete;

	/// Goes through container of blocks to delete each one.
	void Destroy();

	void * Allocate( std::size_t size, std::size_t alignment );

	StackBlockInfo info_;

};

// ----------------------------------------------------------------------------

class ThreadSafeStackAllocator : public StackAllocator
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

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual std::size_t Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment ) override;

#else

	virtual std::size_t Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment ) override;

#endif

	/**
	 */
	virtual std::size_t Resize( void * place, std::size_t oldSize, std::size_t newSize ) override;

	/// Returns true if a block of memory managed by this object owns the chunk at the place
	virtual bool HasAddress( void * place ) const override;

	/// Deletes any blocks that have zero allocations.
	virtual bool TrimEmptyBlocks() override;

	/// Returns true if any memory block was corrupted.
	virtual bool IsCorrupt() const override;

	virtual float GetFragmentationPercent() const override;

private:

	friend class memwa::AllocatorManager;

	ThreadSafeStackAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment );

	virtual ~ThreadSafeStackAllocator();

	ThreadSafeStackAllocator() = delete;
	ThreadSafeStackAllocator( const ThreadSafeStackAllocator & ) = delete;
	ThreadSafeStackAllocator( ThreadSafeStackAllocator && ) = delete;
	ThreadSafeStackAllocator & operator = ( const ThreadSafeStackAllocator & ) = delete;

	mutable std::mutex mutex_;

};

// ----------------------------------------------------------------------------

} // end project namespace
