
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

void * LinearAllocator::Allocate( std::size_t size, std::size_t alignment )
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	void * p = info_.Allocate( size );
	return p;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * LinearAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment )
#else
void * LinearAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment )
#endif
{

	void * p = Allocate( size, alignment );
	if ( nullptr != p )
	{
		return p;
	}
	if ( TrimEmptyBlocks() )
	{
		p = LinearAllocator::Allocate( size, alignment );
		if ( nullptr != p )
		{
			return p;
		}
	}
	if ( Allocator::TrimEmptyBlocks() )
	{
		p = LinearAllocator::Allocate( size, alignment );
	}
	if ( ( nullptr == p ) && doThrow )
	{
		throw std::bad_alloc();
	}

	return p;
}

// ----------------------------------------------------------------------------

void * LinearAllocator::Allocate( std::size_t size, bool doThrow )
{
	void * p = LinearAllocator::Allocate( size, doThrow, info_.alignment_ );
	return p;
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

void * ThreadSafeLinearAllocator::Allocate( std::size_t size, bool doThrow )
{
	LockGuard guard( mutex_ );
	return LinearAllocator::Allocate( size, doThrow );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafeLinearAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment )
#else
void * ThreadSafeLinearAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment )
#endif
{
	LockGuard guard( mutex_ );
	return LinearAllocator::Allocate( size, doThrow, alignment );
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
