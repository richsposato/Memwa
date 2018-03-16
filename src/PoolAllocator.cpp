
#include "PoolAllocator.hpp"

#include "PoolBlock.hpp"
#include "ManagerImpl.hpp"
#include "LockGuard.hpp"

#include <cassert>

#include <new>
#include <stdexcept>

#ifdef DEBUGGING_ALLOCATORS
	#include <iostream>
	#include <iterator>
#endif

namespace memwa
{

// ----------------------------------------------------------------------------

PoolAllocator::PoolAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t objectSize, std::size_t alignment ) :
	Allocator(),
	info_( initialBlocks, blockSize, objectSize, alignment )
{
}

// ----------------------------------------------------------------------------

PoolAllocator::~PoolAllocator()
{
}

// ----------------------------------------------------------------------------

void PoolAllocator::Destroy()
{
	info_.Destroy();
}

// ----------------------------------------------------------------------------

void * PoolAllocator::Allocate( std::size_t size, const void * hint )
{
	const std::size_t alignedSize = memwa::impl::CalculateAlignedSize( size, info_.alignment_ );
	if ( info_.objectSize_ < alignedSize )
	{
		throw std::invalid_argument( "Error! Requested size is too large for PoolAllocator." );
	}

	void * p = info_.Allocate( hint );
	if ( nullptr != p )
	{
		return p;
	}
	memwa::impl::ManagerImpl::GetManager()->TrimEmptyBlocks( this );
	p = info_.Allocate( hint );
	if ( nullptr == p )
	{
		throw std::bad_alloc();
	}

	return p;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * PoolAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * PoolAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	void * p = PoolAllocator::Allocate( size, hint );
	return p;
}

// ----------------------------------------------------------------------------

bool PoolAllocator::Release( void * place, std::size_t size )
{
	if ( nullptr == place )
	{
		return false;
	}
	if ( memwa::impl::CalculateAlignedSize( size, info_.alignment_ ) != info_.objectSize_ )
	{
		throw std::invalid_argument( "Requested object size does not match pool object size." );
	}
	const bool success = info_.Release( place );
	return success;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool PoolAllocator::Release( void * place, std::size_t size, std::align_val_t alignment )
#else
bool PoolAllocator::Release( void * place, std::size_t size, std::size_t alignment )
#endif
{
	if ( nullptr == place )
	{
		return false;
	}
	if ( alignment != info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment must match initial alignment." );
	}
	if ( memwa::impl::CalculateAlignedSize( size, info_.alignment_ ) != info_.objectSize_ )
	{
		throw std::invalid_argument( "Requested object size does not match pool object size." );
	}
	const bool success = info_.Release( place );
	return success;
}

// ----------------------------------------------------------------------------

unsigned long long PoolAllocator::GetMaxSize( std::size_t objectSize ) const
{
	const unsigned long long bytesAvailable = memwa::impl::GetTotalAvailableMemory();
	const unsigned long long maxPossibleObjects = bytesAvailable / objectSize;
	return maxPossibleObjects;
}

// ----------------------------------------------------------------------------

bool PoolAllocator::HasAddress( void * place ) const
{
	const bool hasIt = info_.HasAddress( place );
	return hasIt;
}

// ----------------------------------------------------------------------------

bool PoolAllocator::TrimEmptyBlocks()
{
	const bool trimmed = info_.TrimEmptyBlocks();
	return trimmed;
}

// ----------------------------------------------------------------------------

bool PoolAllocator::IsCorrupt() const
{
	assert( nullptr != this );
	const bool corrupt = info_.IsCorrupt();
	return corrupt;
}

// ----------------------------------------------------------------------------

float PoolAllocator::GetFragmentationPercent() const
{
	const unsigned int poolCount = info_.blocks_.size();
	if ( 0 == poolCount )
	{
		return 0.0;
	}
	unsigned int objectCount = 0;
	PoolBlockInfo::BlocksCIter end( info_.blocks_.end() );
	for ( PoolBlockInfo::BlocksCIter it( info_.blocks_.begin() ); it != end; ++it )
	{
		const PoolBlock & block = *it;
		objectCount += block.GetInUseCount();
	}

	const unsigned int objectsPerPool = ( info_.blockSize_ / info_.objectSize_ );
	std::size_t poolsNeeded = objectCount / objectsPerPool;
	if ( objectCount % objectsPerPool != 0 )
	{
		++poolsNeeded;
	}
	const unsigned int excessPools = poolCount - poolsNeeded;
	const float percent = (float)excessPools / (float)poolCount;
	return percent;
}

// ----------------------------------------------------------------------------

#ifdef DEBUGGING_ALLOCATORS

void PoolAllocator::OutputContents() const
{
	PoolCIter here( recent_ );
	const unsigned int recentDistance = std::distance( availablePool_.begin(), here );
	const signed int recentIndex = ( recent_ == availablePool_.end() ) ? -1 : recentDistance;
	std::cout << "This PoolAllocator: " << this
		<< '\t' << " Pool Size: " << poolSize_
		<< '\t' << " Block Size: " << sizeof(PoolBlock)
		<< '\t' << " Aligned Size: " << alignedSize_
		<< '\t' << " Alignment: " << alignment_
		<< '\t' << " Recent Pool: " << recentIndex
		<< '\t' << " Fragmentation: " << GetFragmentationPercent()
		<< std::endl;

	std::cout << "Total object count in PoolAllocator: " << objectCount << std::endl;
}

#endif

// ----------------------------------------------------------------------------

ThreadSafePoolAllocator::ThreadSafePoolAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t objectSize, std::size_t alignment ) :
	PoolAllocator( initialBlocks, blockSize, objectSize, alignment ),
	mutex_()
{
}

// ----------------------------------------------------------------------------

ThreadSafePoolAllocator::~ThreadSafePoolAllocator()
{
}

// ----------------------------------------------------------------------------

void * ThreadSafePoolAllocator::Allocate( std::size_t size, const void * hint )
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::Allocate( size, hint );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafePoolAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * ThreadSafePoolAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::Allocate( size, alignment, hint );
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::Release( void * place, std::size_t size )
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::Release( place, size );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool ThreadSafePoolAllocator::Release( void * place, std::size_t size, std::align_val_t alignment )
#else
bool ThreadSafePoolAllocator::Release( void * place, std::size_t size, std::size_t alignment )
#endif
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::Release( place, size, alignment );
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::HasAddress( void * place ) const
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::HasAddress( place );
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::TrimEmptyBlocks()
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::TrimEmptyBlocks();
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::IsCorrupt() const
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::IsCorrupt();
}

// ----------------------------------------------------------------------------

float ThreadSafePoolAllocator::GetFragmentationPercent() const
{
	LockGuard guard( mutex_ );
    assert( guard.IsLocked() );
	return PoolAllocator::GetFragmentationPercent();
}

// ----------------------------------------------------------------------------

} // end project namespace
