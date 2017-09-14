
#include <TinyObjectAllocator.hpp>

#include "ManagerImpl.hpp"

#include <cassert>
#include <climits>

#include <bitset>

namespace memwa
{

// ----------------------------------------------------------------------------

/** @struct TinyBlock
    @ingroup SmallObjectGroupInternal
 Contains info about each allocated TinyBlock - which is a collection of
 contiguous blocks.  Each block is the same size, as specified by the
 FixedAllocator.  The number of blocks in a TinyBlock depends upon page size.
 This is a POD-style struct with value-semantics.  All functions and data
 are private so that they can not be changed by anything other than the
 FixedAllocator which owns the TinyBlock.

 @par Minimal Interface
 For the sake of runtime efficiency, no constructor, destructor, or
 copy-assignment operator is defined. The inline functions made by the
 compiler should be sufficient, and perhaps faster than hand-crafted
 functions.  The lack of these functions allows vector to create and copy
 Chunks as needed without overhead.  The Init and Release functions do
 what the default constructor and destructor would do.  A TinyBlock is not in
 a usable state after it is constructed and before calling Init.  Nor is
 a TinyBlock usable after Release is called, but before the destructor.

 @par Efficiency
 Down near the lowest level of the allocator, runtime efficiencies trump
 almost all other considerations.  Each function does the minimum required
 of it.  All functions should execute in constant time to prevent higher-
 level code from unwittingly using a version of Shlemiel the Painter's
 Algorithm.

 @par Stealth Indexes
 The first char of each empty block contains the index of the next empty
 block.  These stealth indexes form a singly-linked list within the blocks.
 A TinyBlock is corrupt if this singly-linked list has a loop or is shorter
 than freeSpotCount_.  Much of the allocator's time and space efficiency
 comes from how these stealth indexes are implemented.
 */
class TinyBlock
{
public:

    /** Initializes a TinyBlock.
     @param objectSize Number of bytes per object.
     */
    explicit TinyBlock( std::size_t objectSize );

    TinyBlock( std::size_t blockSize, std::size_t alignment, std::size_t objectSize );

    /** Allocate a block within the TinyBlock.  Complexity is always O(1), and
     this will never throw.  Does not actually "allocate" by calling
     malloc, new, or any other function, but merely adjusts some internal
     indexes to indicate an already allocated block is no longer available.
     @return Pointer to block within TinyBlock.
     */
    void * Allocate( std::size_t objectSize );

    /** Deallocate a block within the TinyBlock. Complexity is always O(1), and
     this will never throw.  For efficiency, this assumes the address is
     within the block and aligned along the correct byte boundary.  An
     assertion checks the alignment, and a call to HasBlock is done from
     within VicinityFind.  Does not actually "deallocate" by calling free,
     delete, or other function, but merely adjusts some internal indexes to
     indicate a block is now available.
     */
    void Release( void * place, std::size_t objectSize );

    /// Releases the allocated block of memory.
    void Destroy();

    /** Determines if the TinyBlock has been corrupted.
     @param objectSize # of bytes in each object.
     @return True if TinyBlock is corrupt.
     */
    bool IsCorrupt( std::size_t objectSize ) const;

    /// Returns true if block at address P is inside this TinyBlock.
    bool HasAddress( const void * place, std::size_t blockSize ) const
    {
        const unsigned char * const here = static_cast< const unsigned char * >( place );
        return ( block_ <= here ) && ( here < block_ + blockSize );
    }

    bool IsBelowAddress( const void * place, std::size_t poolSize ) const;

    bool operator < ( const TinyBlock & that ) const
    {
        return ( block_ < that.block_ );
    }

    bool IsEmpty() const
    {
        return ( UCHAR_MAX == freeSpotCount_ );
    }

    bool IsFull() const
    {
        return ( 0 == freeSpotCount_ );
    }

    unsigned int GetInUseCount() const
    {
        return ( UCHAR_MAX - freeSpotCount_ );
    }

    /// Pointer to array of allocated blocks.
    unsigned char * block_;
    /// Index of first empty block.
    unsigned char freeSpot_;
    /// Count of empty blocks.
    unsigned char freeSpotCount_;
};

// ----------------------------------------------------------------------------

TinyBlock::TinyBlock( std::size_t blockSize, std::size_t alignment, std::size_t objectSize ) :
    block_( static_cast< unsigned char * >( ::malloc( objectSize * UCHAR_MAX ) ) ),
    freeSpot_( 0 ),
    freeSpotCount_( UCHAR_MAX )
{
    if ( nullptr == block_ )
    {
        throw std::bad_alloc();
    }
    unsigned char i = 0;
    for ( unsigned char * place = block_; i != UCHAR_MAX; place += objectSize )
    {
        *place = ++i;
    }
}

// ----------------------------------------------------------------------------

TinyBlock::TinyBlock( std::size_t objectSize ) :
    block_( static_cast< unsigned char * >( ::malloc( objectSize * UCHAR_MAX ) ) ),
    freeSpot_( 0 ),
    freeSpotCount_( UCHAR_MAX )
{
    if ( nullptr == block_ )
    {
        throw std::bad_alloc();
    }
    unsigned char i = 0;
    for ( unsigned char * place = block_; i != UCHAR_MAX; place += objectSize )
    {
        *place = ++i;
    }
}

// ----------------------------------------------------------------------------

void TinyBlock::Destroy()
{
    ::free( static_cast< void * >( block_ ) );
    block_ = nullptr;
    freeSpot_ = 0;
    freeSpotCount_ = UCHAR_MAX;
}

// ----------------------------------------------------------------------------

void * TinyBlock::Allocate( std::size_t objectSize )
{
    if ( IsFull() )
    {
        return nullptr;
    }

    assert( ( freeSpot_ * objectSize ) / objectSize == freeSpot_ );
    unsigned char * pResult = block_ + ( freeSpot_ * objectSize );
    freeSpot_ = *pResult;
    --freeSpotCount_;

    return pResult;
}

// ----------------------------------------------------------------------------

void TinyBlock::Release( void * place, std::size_t objectSize )
{
    assert( place >= block_ );

    unsigned char * toRelease = static_cast< unsigned char * >( place );
    // Alignment check
    assert( ( toRelease - block_ ) % objectSize == 0 );
    unsigned char index = static_cast< unsigned char >( ( toRelease - block_ ) / objectSize );

#if defined(DEBUG) || defined(_DEBUG)
    // Check if block was already deleted.  Attempting to delete the same
    // block more than once causes TinyBlock's linked-list of stealth indexes to
    // become corrupt.  And causes count of freeSpotCount_ to be wrong.
    if ( 0 < freeSpotCount_ )
        assert( freeSpot_ != index );
#endif

    *toRelease = freeSpot_;
    freeSpot_ = index;
    // Truncation check
    assert( freeSpot_ == ( toRelease - block_ ) / objectSize );

    ++freeSpotCount_;
}

// ----------------------------------------------------------------------------

bool TinyBlock::IsCorrupt( std::size_t objectSize ) const
{
    if ( IsFull() )
    {
        // Useless to do further corruption checks if all blocks allocated.
        return false;
    }
    unsigned char index = freeSpot_;

    /* If the bit at index was set in foundBlocks, then the stealth index was
     found on the linked-list.
     */
    std::bitset< UCHAR_MAX > foundBlocks;
    unsigned char * nextBlock = nullptr;

    /* The loop goes along singly linked-list of stealth indexes and makes sure
     that each index is within bounds (0 <= index < UCHAR_MAX) and that the
     index was not already found while traversing the linked-list.  The linked-
     list should have exactly freeSpotCount_ nodes, so the for loop will not
     check more than freeSpotCount_.  This loop can't check inside allocated
     blocks for corruption since such blocks are not within the linked-list.
     Contents of allocated blocks are not changed by TinyBlock.

      freeSpotCount_ == 5
      freeSpot_ -> 17 -> 29 -> 53 -> *17* -> 29 -> 53 ...
      No index should be repeated within the linked-list since that would
      indicate the presence of a loop in the linked-list.
     */
    for ( unsigned char cc = 0; ; )
    {
        nextBlock = block_ + ( index * objectSize );
        foundBlocks.set( index, true );
        ++cc;
        if ( cc >= freeSpotCount_ )
            // Successfully counted off number of nodes in linked-list.
            break;
        index = *nextBlock;
        if ( foundBlocks.test( index ) )
        {
            /* This implies that a block was corrupted due to a stray pointer
             or an operation on a nearby block overran the size of the block.
             Or perhaps the program tried to delete a block more than once.
             */
            assert( false );
            return true;
        }
    }
    if ( foundBlocks.count() != freeSpotCount_ )
    {
        /* This implies that the singly-linked-list of stealth indexes was
         corrupted.  Ideally, this should have been detected within the loop.
         */
        assert( false );
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------------

TinyObjectAllocator::TinyObjectAllocator( unsigned int initialBlocks, std::size_t objectSize, std::size_t alignment ) :
    info_( initialBlocks, objectSize * UCHAR_MAX, objectSize, alignment )
{
}

// ----------------------------------------------------------------------------

TinyObjectAllocator::~TinyObjectAllocator( void )
{
}

// ----------------------------------------------------------------------------

void * TinyObjectAllocator::Allocate( std::size_t objectSize, bool doThrow, const void * hint )
{
    const std::size_t alignedSize = memwa::CalculateBytesNeeded( objectSize, info_.alignment_ );
    if ( info_.objectSize_ != alignedSize )
    {
        throw std::invalid_argument( "Error! Tried to allocate wrong size in PoolAllocator." );
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
    if ( ( nullptr == place ) && doThrow )
    {
        throw std::bad_alloc();
    }

    return place;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * TinyObjectAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint )
#else
void * TinyObjectAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint )
#endif
{
    if ( alignment > info_.alignment_ )
    {
        throw std::invalid_argument( "Requested alignment size must be less than or equal to initial alignment size." );
    }
    void * place = Allocate( size, doThrow, hint );
    return place;
}

// ----------------------------------------------------------------------------

bool TinyObjectAllocator::Release( void * place, std::size_t objectSize )
{
    if ( nullptr == place )
    {
        return false;
    }
    if ( memwa::CalculateBytesNeeded( objectSize, info_.alignment_ ) != info_.objectSize_ )
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
    if ( memwa::CalculateBytesNeeded( size, info_.alignment_ ) != info_.objectSize_ )
    {
        throw std::invalid_argument( "Requested object size does not match pool object size." );
    }
    const bool success = info_.Release( place );
    return success;
}

// ----------------------------------------------------------------------------

unsigned long long TinyObjectAllocator::GetMaxSize( std::size_t objectSize ) const
{
    const unsigned long long bytesAvailable = memwa::impl::GetTotalAvailableMemory();
    const unsigned long long maxPossibleBlocks = bytesAvailable / info_.blockSize_;
    const unsigned long long maxPossibleObjects = maxPossibleBlocks / objectSize;
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

void * ThreadSafeTinyObjectAllocator::Allocate( std::size_t size, bool doThrow, const void * hint )
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::Allocate( size, doThrow );
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
void * ThreadSafeTinyObjectAllocator::Allocate( std::size_t size, bool doThrow, std::align_val_t alignment, const void * hint )
#else
void * ThreadSafeTinyObjectAllocator::Allocate( std::size_t size, bool doThrow, std::size_t alignment, const void * hint )
#endif
{
    LockGuard guard( mutex_ );
    return TinyObjectAllocator::Allocate( size, doThrow, alignment );
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
