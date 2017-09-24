
#include "TinyBlock.hpp"

#include "ManagerImpl.hpp"

#include <cassert>
#include <climits>
#include <cstdlib>

#include <bitset>
#include <iostream>

namespace memwa
{

// ----------------------------------------------------------------------------

TinyBlock::TinyBlock( std::size_t blockSize, std::size_t objectSize, std::size_t alignment, unsigned int objectsPerPool ) :
    block_( static_cast< unsigned char * >( ::malloc( blockSize ) ) ),
    freeSpot_( 0 ),
    freeSpotCount_( UCHAR_MAX )
{
/*    std::cout << __FUNCTION__ << " : " << __LINE__ << "   blockSize:" << blockSize
        << "   objectsPerPool: " << objectsPerPool
        << "   objectSize: " << objectSize << std::endl;
*/
    assert( blockSize == UCHAR_MAX * objectSize );
    if ( nullptr == block_ )
    {
        throw std::bad_alloc();
    }
    std::size_t blockPlace = reinterpret_cast< std::size_t >( block_ );
    assert( blockPlace % alignment == 0 );
    unsigned char i = 0;
    for ( unsigned char * place = block_; i < UCHAR_MAX; place += objectSize )
    {
        *place = ++i;
    }
//    std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
}

// ----------------------------------------------------------------------------

TinyBlock::TinyBlock( std::size_t objectSize ) :
    block_( static_cast< unsigned char * >( ::malloc( objectSize * UCHAR_MAX ) ) ),
    freeSpot_( 0 ),
    freeSpotCount_( UCHAR_MAX )
{
//    std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
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
    unsigned char * toRelease = static_cast< unsigned char * >( place );
    assert( toRelease >= block_ );
    const std::size_t offset = static_cast< std::size_t >( toRelease - block_ );
/*
    std::cout << __FUNCTION__ << " : " << __LINE__
        << "   block: " << reinterpret_cast< std::size_t >( block_ )
        << "  place: " << reinterpret_cast< std::size_t >( place )
        << "   size: " << objectSize << "   offset: " << offset << std::endl;
*/
    // Alignment check
    assert( offset % objectSize == 0 );
    unsigned char index = static_cast< unsigned char >( offset / objectSize );

#if defined(DEBUG) || defined(_DEBUG)
    // Check if block was already deleted.  Attempting to delete the same
    // block more than once causes TinyBlock's linked-list of stealth indexes to
    // become corrupt.  And causes count of freeSpotCount_ to be wrong.
    if ( 0 < freeSpotCount_ )
    {
        assert( freeSpot_ != index );
    }
#endif

    *toRelease = freeSpot_;
    freeSpot_ = index;
    // Truncation check
    assert( freeSpot_ == offset / objectSize );

    ++freeSpotCount_;
}

// ----------------------------------------------------------------------------

bool TinyBlock::IsBelowAddress( const void * place, std::size_t poolSize ) const
{
    const bool below = ( block_ + poolSize <= place );
    return below;
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

}
