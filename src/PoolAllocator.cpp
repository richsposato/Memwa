
#include "PoolAllocator.hpp"

#include "ManagerImpl.hpp"

#include <new>
#include <stdexcept>
#include <cassert>

#ifdef DEBUGGING_ALLOCATORS
	#include <iostream>
	#include <iterator>
#endif

namespace memwa
{

// ----------------------------------------------------------------------------

class PoolBlock
{
public:

	PoolBlock( std::size_t poolSize, unsigned int objectsPerPool, std::size_t alignedSize );

	void Destroy();

	void * Allocate();

	bool Release( void * place );

	bool HasAddress( const void * place, std::size_t poolSize ) const;

	bool IsBelowAddress( const void * place, std::size_t poolSize ) const;

	bool operator < ( const PoolBlock & that ) const
	{
		return ( block_ < that.block_ );
	}

	bool IsEmpty() const
	{
		return ( 0 == objectCount_ );
	}

	bool IsFull() const
	{
		return ( free_ == nullptr );
	}

	unsigned int GetInUseCount() const
	{
		return objectCount_;
	}

#ifdef DEBUGGING_ALLOCATORS

	void OutputContents() const;

#endif

	std::size_t * GetAddress() const
	{
		return block_;
	}

	bool IsCorrupt( std::size_t blockSize, std::size_t alignment, std::size_t objectSize ) const;

private:

	/// Pointer to block allocated to hold from 1 to N objects.
	std::size_t * block_;
	/// Pointer to next free chunk in pool.
	std::size_t * free_;
	/// Number of objects allocated so far.
	unsigned int objectCount_;
};

// ----------------------------------------------------------------------------

PoolBlock::PoolBlock( std::size_t poolSize, unsigned int objectsPerPool, std::size_t alignedSize ) :
	block_( reinterpret_cast< std::size_t * >( std::malloc( poolSize ) ) ),
	free_( block_ ),
	objectCount_( 0 )
{
	if ( nullptr == block_ )
	{
		throw std::bad_alloc();
	}
	std::size_t * end = block_ + poolSize;
	std::size_t * here = free_;
	unsigned int count = 0;
	std::size_t * next;
	do
	{
		next = here + alignedSize;
		const std::size_t place = reinterpret_cast< std::size_t >( next );
		*next = place;
		here = next;
		++count;
	} while ( next < end );
	assert( count == objectsPerPool );
}

// ----------------------------------------------------------------------------

void PoolBlock::Destroy()
{
	free( block_ );
	block_ = nullptr;
	free_ = nullptr;
}

// ----------------------------------------------------------------------------

void * PoolBlock::Allocate()
{
	if ( IsFull() )
	{
		return nullptr;
	}
	void * p = free_;
	std::size_t * place = reinterpret_cast< std::size_t * >( *free_ );
	free_ = place;
	++objectCount_;
	return p;
}

// ----------------------------------------------------------------------------

bool PoolBlock::Release( void * place )
{
	std::size_t * p = reinterpret_cast< std::size_t * >( place );
	*p = reinterpret_cast< std::size_t >( free_ );
	free_ = p;
	--objectCount_;
	return true;
}

// ----------------------------------------------------------------------------

bool PoolBlock::HasAddress( const void * place, std::size_t poolSize ) const
{
	if ( place < block_ )
	{
		return false;
	}
	if ( block_ + poolSize <= place )
	{
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------------

bool PoolBlock::IsBelowAddress( const void * place, std::size_t poolSize ) const
{
	const bool isBelow = ( block_ + poolSize <= place );
	return isBelow;
}

// ----------------------------------------------------------------------------

bool PoolBlock::IsCorrupt( std::size_t blockSize, std::size_t alignment, std::size_t objectSize ) const
{
	assert( nullptr != this );
	assert( block_ != nullptr );
	const unsigned int objectsPerPool = blockSize / objectSize;
	if ( free_ == nullptr )
	{
		assert( objectCount_ != 0 );
		assert( objectsPerPool == objectCount_ );
	}
	else
	{
		assert( objectsPerPool > objectCount_ );
		assert( free_ >= block_ );
		assert( free_ < block_ + blockSize );
		std::size_t * next = free_;
		bool checkedFirst = false;
		unsigned int freeCount = 0;
		unsigned int maxFreeCount = ( objectsPerPool - objectCount_ );
		while ( next != nullptr )
		{
			assert( next >= block_ );
			assert( next < block_ + blockSize );
			if ( checkedFirst )
			{
				assert( next != free_ );
			}
			next = reinterpret_cast< std::size_t * >( *next );
			checkedFirst = true;
			++freeCount;
			assert( freeCount <= maxFreeCount );
		}
		assert( freeCount == maxFreeCount );
	}
	return false;
}

// ----------------------------------------------------------------------------

#ifdef DEBUGGING_ALLOCATORS

void PoolBlock::OutputContents() const
{
	std::cout << '\t' << this
		<< '\t' << " Block: " << block_
		<< '\t' << " Free: " << free_
		<< '\t' << " In Use: " << objectCount_
		<< std::endl;
}

#endif

// ----------------------------------------------------------------------------

PoolAllocator::PoolAllocator( unsigned int initialBlocks, std::size_t blockSize,
	std::size_t alignment, std::size_t objectSize ) :
	Allocator(),
	info_( initialBlocks, blockSize, alignment, objectSize )
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

void * PoolAllocator::Allocate( std::size_t size, bool throwException, const void * hint )
{
	const std::size_t alignedSize = memwa::CalculateBytesNeeded( size, info_.alignment_ );
	if ( info_.objectSize_ != alignedSize )
	{
		throw std::invalid_argument( "Error! Tried to allocate wrong size in PoolAllocator." );
	}

	void * p = info_.Allocate( hint );
	if ( nullptr != p )
	{
		return p;
	}
	memwa::impl::ManagerImpl::GetManager()->TrimEmptyBlocks( this );
	p = info_.Allocate( hint );
	if ( ( nullptr == p ) && throwException )
	{
		throw std::bad_alloc();
	}

	return p;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * PoolAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint )
#else
void * PoolAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint )
#endif
{
	if ( alignment > info_.alignment_ )
	{
		throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
	}
	void * p = Allocate( size, doThrow, hint );
	return p;
}

// ----------------------------------------------------------------------------

bool PoolAllocator::Release( void * place, std::size_t size )
{
	if ( nullptr == place )
	{
		return false;
	}
	if ( memwa::CalculateBytesNeeded( size, info_.alignment_ ) != info_.objectSize_ )
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
	if ( memwa::CalculateBytesNeeded( size, info_.alignment_ ) != info_.objectSize_ )
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
	const unsigned long long maxPossibleBlocks = bytesAvailable / info_.blockSize_;
	const unsigned long long maxPossibleObjects = maxPossibleBlocks / objectSize;
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

ThreadSafePoolAllocator::ThreadSafePoolAllocator( unsigned int initialBlocks, std::size_t blockSize,
	std::size_t alignment, std::size_t objectSize ) :
	PoolAllocator( initialBlocks, blockSize, alignment, objectSize ),
	mutex_()
{
}

// ----------------------------------------------------------------------------

ThreadSafePoolAllocator::~ThreadSafePoolAllocator()
{
}

// ----------------------------------------------------------------------------

void * ThreadSafePoolAllocator::Allocate( std::size_t size, bool doThrow, const void * hint )
{
	LockGuard guard( mutex_ );
	return PoolAllocator::Allocate( size, doThrow, hint );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafePoolAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint )
#else
void * ThreadSafePoolAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint )
#endif
{
	LockGuard guard( mutex_ );
	return PoolAllocator::Allocate( size, doThrow, alignment, hint );
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::Release( void * place, std::size_t size )
{
	LockGuard guard( mutex_ );
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
	return PoolAllocator::Release( place, size, alignment );
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::HasAddress( void * place ) const
{
	LockGuard guard( mutex_ );
	return PoolAllocator::HasAddress( place );
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::TrimEmptyBlocks()
{
	LockGuard guard( mutex_ );
	return PoolAllocator::TrimEmptyBlocks();
}

// ----------------------------------------------------------------------------

bool ThreadSafePoolAllocator::IsCorrupt() const
{
	LockGuard guard( mutex_ );
	return PoolAllocator::IsCorrupt();
}

// ----------------------------------------------------------------------------

float ThreadSafePoolAllocator::GetFragmentationPercent() const
{
	LockGuard guard( mutex_ );
	return PoolAllocator::GetFragmentationPercent();
}

// ----------------------------------------------------------------------------

} // end project namespace
