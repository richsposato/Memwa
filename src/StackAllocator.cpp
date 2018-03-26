
#include "StackAllocator.hpp"

#include "LockGuard.hpp"

#include "StackBlock.hpp"
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

void * StackAllocator::Allocate( std::size_t size, const void * hint )
{
	if ( info_.blockSize_ < size )
	{
		throw std::invalid_argument( "Requested allocation size must be smaller than the memory block size." );
	}
	void * p = info_.Allocate( size, hint );
	return p;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * StackAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * StackAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}

	void * p = StackAllocator::Allocate( size, hint );
	if ( nullptr != p )
	{
		return p;
	}
	if ( info_.TrimEmptyBlocks() )
	{
		p = StackAllocator::Allocate( size, hint );
		if ( nullptr != p )
		{
			return p;
		}
	}
	memwa::impl::ManagerImpl::GetManager()->TrimEmptyBlocks( this );
	p = StackAllocator::Allocate( size, hint );
	if ( nullptr == p )
	{
		throw std::bad_alloc();
	}

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
bool StackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment )
#else
bool StackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment )
#endif
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	const bool success = StackAllocator::Resize( place, oldSize, newSize );
	return success;
}

// ----------------------------------------------------------------------------

bool StackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize )
{
	if ( nullptr == place )
	{
		throw std::invalid_argument( "Unable to resize if pointer is nullptr." );
	}
	if ( info_.blockSize_ < newSize )
	{
		throw std::invalid_argument( "Requested allocation size must be smaller than the memory block size." );
	}
	if ( oldSize == newSize )
	{
		return true;
	}
	if ( 0 == newSize )
	{
		return info_.Release( place, oldSize );
	}
	const bool success = info_.Resize( place, oldSize, newSize );
	return success;
}

// ----------------------------------------------------------------------------

unsigned long long StackAllocator::GetMaxSize( std::size_t objectSize ) const
{
	const unsigned long long bytesAvailable = memwa::impl::GetTotalAvailableMemory();
	const unsigned long long maxPossibleObjects = bytesAvailable / objectSize;
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

void * ThreadSafeStackAllocator::Allocate( std::size_t size, const void * hint )
{
	LockGuard guard( mutex_ );
	return StackAllocator::Allocate( size );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafeStackAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * ThreadSafeStackAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
	LockGuard guard( mutex_ );
	return StackAllocator::Allocate( size, alignment, hint );
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
bool ThreadSafeStackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment )
#else
bool ThreadSafeStackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment )
#endif
{
	LockGuard guard( mutex_ );
	return StackAllocator::Resize( place, oldSize, newSize, alignment );
}

// ----------------------------------------------------------------------------

bool ThreadSafeStackAllocator::Resize( void * place, std::size_t oldSize, std::size_t newSize )
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
