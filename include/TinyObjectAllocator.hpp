

#pragma once

#include "AllocatorManager.hpp"

#include "BlockInfo.hpp"

#include <cstddef> // For std::size_t.

#include <mutex>

namespace memwa
{

class AllocatorManager;

/** @class TinyObjectAllocator

 # Usage Patterns
 You can use TinyObjectAllocator for:
 - Objects that are all the same size.
 - Objects that are allocated and released in any order.
 */
class TinyObjectAllocator : public memwa::Allocator
{
public:

    /** Allocates a chunk of memory from a block.
     @param size Number of bytes to allocate.
     @return Pointer to chunk of memory of at least size bytes, or nullptr if it could not allocate.
     */
    virtual void * Allocate( std::size_t size, const void * hint = nullptr ) override;

#if __cplusplus > 201402L
    // This code is for C++ 2017.

    virtual void * Allocate( std::size_t size, std::align_val_t alignment, const void * hint = nullptr ) override;

#else

    virtual void * Allocate( std::size_t size, std::size_t alignment, const void * hint = nullptr ) override;

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

    virtual unsigned long long GetMaxSize( std::size_t objectSize ) const override;

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
    TinyObjectAllocator( unsigned int initialBlocks, std::size_t objectSize, std::size_t alignment );

    /// The destructor will delete all blocks if the destroy flag is set.
    virtual ~TinyObjectAllocator();

    /// Goes through container of blocks to delete each one.
    virtual void Destroy() override;

private:

    friend class memwa::AllocatorManager;

    TinyObjectAllocator() = delete;
    TinyObjectAllocator( const TinyObjectAllocator & ) = delete;
    TinyObjectAllocator( TinyObjectAllocator && ) = delete;
    TinyObjectAllocator & operator = ( const TinyObjectAllocator & ) = delete;
    TinyObjectAllocator & operator = ( TinyObjectAllocator && ) = delete;

    void * Allocate( std::size_t size, std::size_t alignment );

    TinyBlockInfo info_;

};

// ----------------------------------------------------------------------------

class ThreadSafeTinyObjectAllocator : public TinyObjectAllocator
{
public:

    /** Allocates a chunk of memory from a block.
     @param size Number of bytes to allocate.
     @return Pointer to chunk of memory of at least size bytes, or nullptr if it could not allocate.
     */
    virtual void * Allocate( std::size_t size, const void * hint = nullptr ) override;

#if __cplusplus > 201402L
    // This code is for C++ 2017.

    virtual void * Allocate( std::size_t size, std::align_val_t alignment, const void * hint = nullptr ) override;

#else

    virtual void * Allocate( std::size_t size, std::size_t alignment, const void * hint = nullptr ) override;

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

    ThreadSafeTinyObjectAllocator( unsigned int initialBlocks, std::size_t objectSize, std::size_t alignment );

    virtual ~ThreadSafeTinyObjectAllocator();

    virtual void Destroy() override;

    ThreadSafeTinyObjectAllocator() = delete;
    ThreadSafeTinyObjectAllocator( const ThreadSafeTinyObjectAllocator & ) = delete;
    ThreadSafeTinyObjectAllocator( ThreadSafeTinyObjectAllocator && ) = delete;
    ThreadSafeTinyObjectAllocator & operator = ( const ThreadSafeTinyObjectAllocator & ) = delete;

    mutable std::mutex mutex_;

};

// ----------------------------------------------------------------------------

} // end project namespace
