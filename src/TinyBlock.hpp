
#pragma once

#include <cstddef> // For std::size_t.
#include <climits> // For UCHAR_MAX.

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

    TinyBlock( std::size_t blockSize, std::size_t alignedSize, std::size_t alignment, unsigned int objectsPerPool );

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

}
