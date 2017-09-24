
#include "LinearAllocator.hpp"

#include "LockGuard.hpp"

#include "BlockInfo.hpp"
#include "LinearBlock.hpp"
#include "ManagerImpl.hpp"

#include <cstdlib>
#include <cassert>

#include <new>
#include <algorithm>
#include <stdexcept>


namespace memwa
{

// ----------------------------------------------------------------------------

LinearAllocator::LinearAllocator( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment ) :
	Allocator(),
	info_( initialBlocks, blockSize, alignment )
{
}

// ----------------------------------------------------------------------------

LinearAllocator::~LinearAllocator()
{
}

// ----------------------------------------------------------------------------

void LinearAllocator::Destroy()
{
	info_.Destroy();
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * LinearAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * LinearAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	void * p = LinearAllocator::Allocate( size, hint );
	return p;
}

// ----------------------------------------------------------------------------

void * LinearAllocator::Allocate( std::size_t size, const void * hint )
{
	if ( info_.blockSize_ < size )
	{
		throw std::invalid_argument( "Requested allocation size must be smaller than the memory block size." );
	}
	void * p = info_.Allocate( size, hint );
	if ( nullptr != p )
	{
		return p;
	}
	if ( info_.TrimEmptyBlocks() )
	{
		p = info_.Allocate( size, hint );
		if ( nullptr != p )
		{
			return p;
		}
	}
	if ( memwa::impl::ManagerImpl::GetManager()->TrimEmptyBlocks( this ) )
	{
		p = info_.Allocate( size, hint );
	}
	if ( nullptr == p )
	{
		throw std::bad_alloc();
	}

	return p;
}

// ----------------------------------------------------------------------------

unsigned long long LinearAllocator::GetMaxSize( std::size_t objectSize ) const
{
	const unsigned long long bytesAvailable = memwa::impl::GetTotalAvailableMemory();
	const unsigned long long maxPossibleObjects = bytesAvailable / objectSize;
	return maxPossibleObjects;
}

// ----------------------------------------------------------------------------

bool LinearAllocator::HasAddress( void * place ) const
{
	const bool hasIt = info_.HasAddress( place );
	return hasIt;
}

// ----------------------------------------------------------------------------

bool LinearAllocator::TrimEmptyBlocks()
{
	const bool trimmedAny = info_.TrimEmptyBlocks();
	return trimmedAny;
}

// ----------------------------------------------------------------------------

bool LinearAllocator::IsCorrupt() const
{
	assert( nullptr != this );
	const bool corrupt = info_.IsCorrupt();
	return corrupt;
}

// ----------------------------------------------------------------------------

float LinearAllocator::GetFragmentationPercent() const
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

ThreadSafeLinearAllocator::ThreadSafeLinearAllocator( unsigned int initialBlocks, std::size_t blockSize,
	std::size_t alignment ) :
	LinearAllocator( initialBlocks, blockSize, alignment ),
	mutex_()
{
}

// ----------------------------------------------------------------------------

ThreadSafeLinearAllocator::~ThreadSafeLinearAllocator()
{
}

// ----------------------------------------------------------------------------

void * ThreadSafeLinearAllocator::Allocate( std::size_t size, const void * hint )
{
	LockGuard guard( mutex_ );
	return LinearAllocator::Allocate( size, hint );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafeLinearAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * ThreadSafeLinearAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
	LockGuard guard( mutex_ );
	return LinearAllocator::Allocate( size, alignment, hint );
}

// ----------------------------------------------------------------------------

bool ThreadSafeLinearAllocator::HasAddress( void * place) const
{
	LockGuard guard( mutex_ );
	return LinearAllocator::HasAddress( place );
}

// ----------------------------------------------------------------------------

bool ThreadSafeLinearAllocator::TrimEmptyBlocks()
{
	LockGuard guard( mutex_ );
	return LinearAllocator::TrimEmptyBlocks();
}

// ----------------------------------------------------------------------------

bool ThreadSafeLinearAllocator::IsCorrupt() const
{
	LockGuard guard( mutex_ );
	return LinearAllocator::IsCorrupt();
}

// ----------------------------------------------------------------------------

float ThreadSafeLinearAllocator::GetFragmentationPercent() const
{
	LockGuard guard( mutex_ );
	return LinearAllocator::GetFragmentationPercent();
}

// ----------------------------------------------------------------------------

} // end project namespace
