
#include <TinyObjectAllocator.hpp>

#include "TinyBlock.hpp"
#include "ManagerImpl.hpp"

#include <cassert>

namespace memwa
{

// ----------------------------------------------------------------------------

TinyObjectAllocator::TinyObjectAllocator( unsigned int initialBlocks, std::size_t objectSize, std::size_t alignment ) :
    info_( initialBlocks, objectSize * UCHAR_MAX, objectSize, alignment )
{
    assert( objectSize <= TinyBlock::MaxObjectSize );
//    std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
}

// ----------------------------------------------------------------------------

TinyObjectAllocator::~TinyObjectAllocator( void )
{
}

// ----------------------------------------------------------------------------

void * TinyObjectAllocator::Allocate( std::size_t size, const void * hint )
{
    const std::size_t alignedSize = memwa::impl::CalculateAlignedSize( size, info_.alignment_ );
    assert( alignedSize <= TinyBlock::MaxObjectSize );
    if ( info_.objectSize_ < alignedSize )
    {
        throw std::invalid_argument( "Error! Requested size is too large for TinyObjectAllocator." );
    }

    void * place = info_.Allocate( hint );
    if ( nullptr != place )
    {
        return place;
    }
    if ( TrimEmptyBlocks() )
    {
        place = info_.Allocate( hint );
        if ( nullptr != place )
        {
            return place;
        }
    }
    memwa::impl::ManagerImpl::GetManager()->TrimEmptyBlocks( this );
    place = info_.Allocate( hint );
    if ( nullptr == place )
    {
        throw std::bad_alloc();
    }

    return place;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * TinyObjectAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * TinyObjectAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
    if ( alignment > info_.alignment_ )
    {
        throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
    }
    void * place = Allocate( size, hint );
    return place;
}

// ----------------------------------------------------------------------------

bool TinyObjectAllocator::Release( void * place, std::size_t objectSize )
{
    if ( nullptr == place )
    {
        return false;
    }
    assert( objectSize <= TinyBlock::MaxObjectSize );
    if ( memwa::impl::CalculateAlignedSize( objectSize, info_.alignment_ ) != info_.objectSize_ )
    {
        throw std::invalid_argument( "Requested object size does not match pool object size." );
    }
    const bool success = info_.Release( place );
    return success;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool TinyObjectAllocator::Release( void * place, std::size_t size, std::align_val_t alignment )
#else
bool TinyObjectAllocator::Release( void * place, std::size_t size, std::size_t alignment )
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
    assert( size <= TinyBlock::MaxObjectSize );
    const bool success = info_.Release( place );
    return success;
}

// ----------------------------------------------------------------------------

unsigned long long TinyObjectAllocator::GetMaxSize( std::size_t objectSize ) const
{
    const unsigned long long bytesAvailable = memwa::impl::GetTotalAvailableMemory();
    const unsigned long long maxPossibleObjects = bytesAvailable / objectSize;
    return maxPossibleObjects;
}

// ----------------------------------------------------------------------------

bool TinyObjectAllocator::HasAddress( void * place ) const
{
    const bool hasIt = info_.HasAddress( place );
    return hasIt;
}

// ----------------------------------------------------------------------------

bool TinyObjectAllocator::TrimEmptyBlocks( void )
{
    const bool trimmed = info_.TrimEmptyBlocks();
    return trimmed;
}

// ----------------------------------------------------------------------------

void TinyObjectAllocator::Destroy()
{
    info_.Destroy();
}

// ----------------------------------------------------------------------------

bool TinyObjectAllocator::IsCorrupt( void ) const
{
    assert( nullptr != this );
    const bool corrupt = info_.IsCorrupt();
    return corrupt;
}

// ----------------------------------------------------------------------------

float TinyObjectAllocator::GetFragmentationPercent() const
{
    const unsigned int poolCount = info_.blocks_.size();
    if ( 0 == poolCount )
    {
        return 0.0;
    }
    unsigned int objectCount = 0;
    TinyBlockInfo::BlocksCIter end( info_.blocks_.end() );
    for ( TinyBlockInfo::BlocksCIter it( info_.blocks_.begin() ); it != end; ++it )
    {
        const TinyBlock & block = *it;
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

ThreadSafeTinyObjectAllocator::ThreadSafeTinyObjectAllocator( unsigned int initialBlocks, std::size_t objectSize, std::size_t alignment ) :
    TinyObjectAllocator( initialBlocks, objectSize, alignment ),
    mutex_()
{
}

// ----------------------------------------------------------------------------

ThreadSafeTinyObjectAllocator::~ThreadSafeTinyObjectAllocator()
{
}

// ----------------------------------------------------------------------------

void * ThreadSafeTinyObjectAllocator::Allocate( std::size_t size, const void * hint )
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::Allocate( size, hint );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafeTinyObjectAllocator::Allocate( std::size_t size, std::align_val_t alignment, const void * hint )
#else
void * ThreadSafeTinyObjectAllocator::Allocate( std::size_t size, std::size_t alignment, const void * hint )
#endif
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::Allocate( size, alignment, hint );
}

// ----------------------------------------------------------------------------

bool ThreadSafeTinyObjectAllocator::Release( void * place, std::size_t size )
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::Release( place, size );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool ThreadSafeTinyObjectAllocator::Release( void * place, std::size_t size, std::align_val_t alignment )
#else
bool ThreadSafeTinyObjectAllocator::Release( void * place, std::size_t size, std::size_t alignment )
#endif
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::Release( place, size, alignment );
}

// ----------------------------------------------------------------------------

bool ThreadSafeTinyObjectAllocator::HasAddress( void * place ) const
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::HasAddress( place );
}

// ----------------------------------------------------------------------------

bool ThreadSafeTinyObjectAllocator::TrimEmptyBlocks()
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::TrimEmptyBlocks();
}

// ----------------------------------------------------------------------------

bool ThreadSafeTinyObjectAllocator::IsCorrupt() const
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::IsCorrupt();
}

// ----------------------------------------------------------------------------

float ThreadSafeTinyObjectAllocator::GetFragmentationPercent() const
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::GetFragmentationPercent();
}

// ----------------------------------------------------------------------------

void ThreadSafeTinyObjectAllocator::Destroy()
{
    LockGuard guard( mutex_ );
    TinyObjectAllocator::Destroy();
}

// ----------------------------------------------------------------------------

} // end project namespace
