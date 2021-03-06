
#pragma once

#include <cassert>
#include <cstddef> // For std::size_t.

	#include <iostream>
#ifdef MEMWA_DEBUGGING_ALLOCATORS
#endif

#include <algorithm>
#include <vector>

namespace memwa
{
//namespace impl
//{
	class LinearBlock;
	class StackBlock;
	class PoolBlock;
	class ListBlock;
	class TinyBlock;
//};

// ----------------------------------------------------------------------------

template < class BlockType >
struct BlockInfo
{

	typedef std::vector< BlockType > Blocks;
	typedef typename Blocks::iterator BlocksIter;
	typedef typename Blocks::const_iterator BlocksCIter;

//	typedef typename BlockInfo< BlockType > MyType;

	BlockInfo( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment ) :
		blockSize_( blockSize ),
		alignment_( alignment ),
		blocks_(),
		recent_()
	{
		try
		{
			blocks_.reserve( initialBlocks );
			for ( unsigned int ii = 0; ii < initialBlocks; ++ii )
			{
				BlockType block( blockSize_, alignment_ );
				blocks_.push_back( block );
			}
			if ( initialBlocks != 1 )
			{
				std::sort( blocks_.begin(), blocks_.end() );
			}
			recent_ = blocks_.begin();
		}
		catch ( ... )
		{
			Destroy();
			throw std::bad_alloc();
		}
	}

	/// This constructor is used by PoolAllocator and TinyBlockAllocator.
	BlockInfo( unsigned int initialBlocks, std::size_t blockSize, std::size_t objectSize, std::size_t alignment ) :
		blockSize_( blockSize ),
		alignment_( alignment ),
		blocks_(),
		recent_()
	{
//	    std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		const unsigned int objectsPerPool = blockSize / objectSize;
		try
		{
			blocks_.reserve( initialBlocks );
			for ( unsigned int ii = 0; ii < initialBlocks; ++ii )
			{
				BlockType block( blockSize_, objectSize, alignment, objectsPerPool );
				blocks_.push_back( block );
			}
			if ( initialBlocks != 1 )
			{
				std::sort( blocks_.begin(), blocks_.end() );
			}
			recent_ = blocks_.begin();
//	    	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		}
		catch ( ... )
		{
			Destroy();
			throw std::bad_alloc();
		}
//	    std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	}

	~BlockInfo()
	{
	}

	void Destroy()
	{
		const BlocksIter end( blocks_.end() );
		for ( BlocksIter it( blocks_.begin() ); it != end; ++it )
		{
			BlockType & block = *it;
			block.Destroy();
		}
		blocks_.clear();
	}

	void * Allocate( std::size_t size, const void * hint )
	{
		if ( blockSize_ < size )
		{
			throw std::bad_alloc();
		}

		BlocksIter end( blocks_.end() );
		// Check hint first.
		if ( nullptr != hint )
		{
			BlocksIter it = GetBlock( hint );
			if ( it != end )
			{
				BlockType & block = *it;
				void * p = block.Allocate( size, blockSize_, alignment_ );
				if ( nullptr != p )
				{
					recent_ = it;
					return p;
				}
			}
		}

		// Check most recently used block next.
		if ( recent_ != end )
		{
			BlockType & block = *recent_;
			void * p = block.Allocate( size, blockSize_, alignment_ );
			if ( nullptr != p )
			{
				return p;
			}
		}

		// Now search through all existing blocks to see if any can allocate.
		BlocksIter begin( blocks_.begin() );
		BlocksIter it( begin );
		while ( it != end )
		{
			BlockType & block = *it;
			void * p = block.Allocate( size, blockSize_, alignment_ );
			if ( nullptr != p )
			{
				recent_ = it;
				return p;
			}
			++it;
		}

		// Now try to create a new block and insert it into container.
		BlockType block( blockSize_, alignment_ );
		void * p = block.Allocate( size, blockSize_, alignment_ );
		assert( nullptr != p );
		it = std::lower_bound( begin, end, block );
		recent_ = blocks_.insert( it, block );
		return p;
	}

	bool Release( void * place, std::size_t size )
	{
		// Programs often release chunks they recently allocated, so check recently used block first.
		if ( recent_ != blocks_.end() )
		{
			BlockType & block = *recent_;
			if ( block.HasAddress( place, blockSize_ ) )
			{
				const bool success = block.Release( place, size, blockSize_, alignment_ );
				if ( success && block.IsEmpty( alignment_ ) )
				{
					block.Destroy();
					blocks_.erase( recent_ );
					recent_ = blocks_.end();
				}
				return success;
			}
		}

		BlocksIter it( GetBlock( place ) );
		if ( it == blocks_.end() )
		{
			return false;
		}	
		BlockType & block = *it;
		const bool success = block.Release( place, size, blockSize_, alignment_ );
		if ( success && block.IsEmpty( alignment_ ) )
		{
			const bool resetRecent = ( it == recent_ );
			block.Destroy();
			blocks_.erase( it );
			if ( resetRecent || ( blocks_.size() == 0 ) || ( blocks_.end() < recent_ ) )
			{
				recent_ = blocks_.end();
			}
		}
		return success;
	}

	bool Resize( void * place, std::size_t oldSize, std::size_t newSize )
	{
		if ( blockSize_ < newSize )
		{
			return false;
		}
		if ( recent_ != blocks_.end() )
		{
			BlockType & block = *recent_;
			if ( block.HasAddress( place, blockSize_ ) )
			{
				const bool success = block.Resize( place, oldSize, newSize, blockSize_, alignment_ );
				return success;
			}
		}
		BlocksIter it( GetBlock( place ) );
		if ( it == blocks_.end() )
		{
			return false;
		}	
		BlockType & block = *it;
		const bool success = block.Resize( place, oldSize, newSize, blockSize_, alignment_ );
		return success;
	}

	bool HasAddress( const void * place ) const
	{
		const BlocksCIter it( GetBlock( place ) );
		const BlocksCIter end( blocks_.end() );
		const bool hasIt = ( it != end );
		return hasIt;
	}

	BlocksIter GetBlock( const void * place )
	{
		const BlocksIter end( blocks_.end() );
		BlocksIter here( blocks_.begin() );
		BlocksIter it;
	    typename std::iterator_traits< BlocksIter >::difference_type step;
	    typename std::iterator_traits< BlocksIter >::difference_type count = std::distance( here, end );
	 
		// Do binary search to find BlockType with place.
	    while ( count > 0 )
	    {
	        it = here; 
	        step = count / 2; 
	        std::advance( it, step );
	        const BlockType & block = *it;
	        if ( block.HasAddress( place, blockSize_ ) )
	        {
	        	return it;
	        }
	        if ( block.IsBelowAddress( place, blockSize_ ) )
	        {
	            here = ++it; 
	            count -= step + 1; 
	        }
	        else
	        {
	            count = step;
	        }
	    }

	    const BlockType & block = *here;
	    if ( block.HasAddress( place, blockSize_ ) )
	    {
			return here;
		}
		return end;
	}

	BlocksCIter GetBlock( const void * place ) const
	{
		BlockInfo * pThis = const_cast< BlockInfo * >( this );
		BlocksIter it( pThis->GetBlock( place ) );
		BlocksCIter cit( it );
		return cit;
	}

	bool TrimEmptyBlocks()
	{
		bool foundAny = false;

		Blocks temp;
		temp.reserve( blocks_.size() );
		BlocksIter end( blocks_.end() );
		for ( BlocksIter it( blocks_.begin() ); it != end; ++it )
		{
			BlockType & block = *it;
			if ( block.IsEmpty( alignment_ ) )
			{
				block.Destroy();
				foundAny = true;
			}
			else
			{
				temp.push_back( block );
			}
		}
		temp.swap( blocks_ );
		temp.shrink_to_fit();
		recent_ = blocks_.end();

		return foundAny;
	}

	bool IsCorrupt() const
	{
		assert( nullptr != this );

		const BlocksCIter end( blocks_.end() );
		for ( BlocksCIter it( blocks_.begin() ); it != end; ++it )
		{
			const BlockType & block = *it;
			assert( !block.IsCorrupt( blockSize_, alignment_ ) );
		}
		return false;
	}

	void GetBlockCounts( std::size_t & blockCount, std::size_t & excessBlocks ) const
	{
		std::size_t usedByteCount = 0;
		const BlocksCIter end( blocks_.end() );
		for ( BlocksCIter it( blocks_.begin() ); it != end; ++it )
		{
			const BlockType & block = *it;
			const std::size_t freeBytes = block.GetFreeBytes( blockSize_ );
			const std::size_t usedBytes = blockSize_ - freeBytes;
			usedByteCount += usedBytes;
		}
		std::size_t blocksNeeded = usedByteCount / blockSize_;
		if ( usedByteCount % blockSize_ != 0 )
		{
			++blocksNeeded;
		}
		blockCount += blocks_.size();
		excessBlocks += blocks_.size() - blocksNeeded;
	}

#ifdef MEMWA_DEBUGGING_ALLOCATORS

	void OutputContents() const
	{
		BlocksCIter here( recent_ );
		const unsigned int recentDistance = std::distance( blocks_.begin(), here );
		const signed int recentIndex = ( recent_ == blocks_.end() ) ? -1 : recentDistance;
		std::cout << " Block Info"
			<< '\t' << "Block Size: " << blockSize_
			<< '\t' << "Alignment: " << alignment_
			<< '\t' << "Recent Block: " << recentIndex
			<< '\t' << "Block Count: " << blocks_.size()
			<< '\t' << "Capacity: " << blocks_.capacity()
			<< std::endl;

		if ( blocks_.size() != 0 )
		{
			BlocksCIter end( blocks_.end() );
			for ( here = blocks_.begin(); end != here; ++here )
			{
				const BlockType & block = *here;
				block.OutputContents( blockSize_ );
			}
		}
	}

#endif

	/// Size of entire memory page.
	std::size_t blockSize_;
	/// Byte alignment of allocations. (e.g. - 1, 2, 4, 8, 16, or 32.)
	std::size_t alignment_;
	/// Container of memory blocks.
	Blocks blocks_;
	/// Iterator to mostly recently used block to allocate memory.
	BlocksIter recent_;
};

// ----------------------------------------------------------------------------

template < class BlockType >
struct AnyPoolBlockInfo : BlockInfo< BlockType >
{

	typedef BlockInfo< BlockType > BaseClass;
	typedef typename BaseClass::Blocks Blocks;
	typedef typename BaseClass::BlocksIter BlocksIter;
	typedef typename BaseClass::BlocksCIter BlocksCIter;

	AnyPoolBlockInfo( unsigned int initialBlocks, std::size_t blockSize, std::size_t objectSize, std::size_t alignment ) :
		BaseClass( initialBlocks, blockSize, objectSize, alignment ),
		objectSize_( objectSize )
	{
//    	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	}

	~AnyPoolBlockInfo() {}

	void * Allocate( const void * hint )
	{

		BlocksIter end( BaseClass::blocks_.end() );
		// Check hint first.
		if ( nullptr != hint )
		{
			BlocksIter it = BaseClass::GetBlock( hint );
			if ( it != end )
			{
				BlockType & block = *it;
				assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
				void * p = block.Allocate();
				if ( nullptr != p )
				{
					assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
					assert( p != hint );
					BaseClass::recent_ = it;
					return p;
				}
			}
		}

		// Check most recently used block next.
		if ( BaseClass::recent_ != end )
		{
			BlockType & block = *( BaseClass::recent_ );
			assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
			void * p = block.Allocate();
			assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
			if ( nullptr != p )
			{
				return p;
			}
		}

		// Now search through all existing blocks to see if any can allocate.
		BlocksIter begin( BaseClass::blocks_.begin() );
		BlocksIter it( begin );
		while ( it != end )
		{
			BlockType & block = *it;
			assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
			void * p = block.Allocate();
			assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
			if ( nullptr != p )
			{
				BaseClass::recent_ = it;
				return p;
			}
			++it;
		}

		// Now try to create a new block and insert it into container.
		const unsigned int objectsPerPool = BaseClass::blockSize_ / objectSize_;
		BlockType block( BaseClass::blockSize_, objectSize_, BaseClass::alignment_, objectsPerPool );
		assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
		void * p = block.Allocate();
		assert( nullptr != p );
		assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
		it = std::lower_bound( begin, end, block );
		BaseClass::recent_ = BaseClass::blocks_.insert( it, block );
		return p;
	}

	bool Release( void * place )
	{
		// Programs often release chunks they recently allocated, so check recently used block first.
		if ( BaseClass::recent_ != BaseClass::blocks_.end() )
		{
			BlockType & block = *( BaseClass::recent_ );
			if ( block.HasAddress( place, BaseClass::blockSize_ ) )
			{
				assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
				const bool success = block.Release( place );
				assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
				if ( success && block.IsEmpty() )
				{
					block.Destroy();
					BaseClass::blocks_.erase( BaseClass::recent_ );
					BaseClass::recent_ = BaseClass::blocks_.end();
				}
				return success;
			}
		}

		BlocksIter it( BaseClass::GetBlock( place ) );
		if ( it == BaseClass::blocks_.end() )
		{
			return false;
		}	
		BlockType & block = *it;
		assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
		const bool success = block.Release( place );
		assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
		if ( success && block.IsEmpty() )
		{
			const bool resetRecent = ( it == BaseClass::recent_ );
			block.Destroy();
			BaseClass::blocks_.erase( it );
			if ( resetRecent || ( BaseClass::blocks_.size() == 0 ) || ( BaseClass::blocks_.end() < BaseClass::recent_ ) )
			{
				BaseClass::recent_ = BaseClass::blocks_.end();
			}
		}
		return success;
	}

	bool TrimEmptyBlocks()
	{
		bool foundAny = false;

		Blocks temp;
		temp.reserve( BaseClass::blocks_.size() );
		BlocksIter end( BaseClass::blocks_.end() );
		for ( BlocksIter it( BaseClass::blocks_.begin() ); it != end; ++it )
		{
			BlockType & block = *it;
			if ( block.IsEmpty() )
			{
				block.Destroy();
				foundAny = true;
			}
			else
			{
				temp.push_back( block );
			}
		}
		temp.swap( BaseClass::blocks_ );
		temp.shrink_to_fit();
		BaseClass::recent_ = BaseClass::blocks_.end();

		return foundAny;
	}

	bool IsCorrupt() const
	{
		assert( nullptr != this );

		const BlocksCIter end( BaseClass::blocks_.end() );
		for ( BlocksCIter it( BaseClass::blocks_.begin() ); it != end; ++it )
		{
			const BlockType & block = *it;
			assert( !block.IsCorrupt( BaseClass::blockSize_, BaseClass::alignment_, objectSize_ ) );
		}
		return false;
	}

	/// Size of each object maintained by the allocator.
	std::size_t objectSize_;

};

// ----------------------------------------------------------------------------

template < class BlockType >
struct TinyBlockPoolInfo : AnyPoolBlockInfo< BlockType >
{

	typedef AnyPoolBlockInfo< BlockType > BaseClass;
	typedef typename BaseClass::Blocks Blocks;
	typedef typename BaseClass::BlocksIter BlocksIter;
	typedef typename BaseClass::BlocksCIter BlocksCIter;

	TinyBlockPoolInfo( unsigned int initialBlocks, std::size_t blockSize, std::size_t objectSize, std::size_t alignment ) :
		BaseClass( initialBlocks, blockSize, objectSize, alignment )
	{
//		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	}

	~TinyBlockPoolInfo() {}

	void * Allocate( const void * hint )
	{
		assert( !IsCorrupt() );

		BlocksIter end( BaseClass::blocks_.end() );
		// Check hint first.
		if ( nullptr != hint )
		{
			BlocksIter it = BaseClass::GetBlock( hint );
			if ( it != end )
			{
				assert( BaseClass::blocks_.size() != 0 );
				BlockType & block = *it;
				void * p = block.Allocate( BaseClass::objectSize_ );
				if ( nullptr != p )
				{
					BaseClass::recent_ = it;
					assert( !IsCorrupt() );
					return p;
				}
			}
		}

		// Check most recently used block next.
		if ( BaseClass::recent_ != end )
		{
			assert( BaseClass::blocks_.size() != 0 );
			BlockType & block = *( BaseClass::recent_ );
			void * p = block.Allocate( BaseClass::objectSize_ );
			if ( nullptr != p )
			{
				assert( !IsCorrupt() );
				return p;
			}
		}

		// Now search through all existing blocks to see if any can allocate.
		BlocksIter begin( BaseClass::blocks_.begin() );
		BlocksIter it( begin );
		while ( it != end )
		{
			assert( BaseClass::blocks_.size() != 0 );
			BlockType & block = *it;
			void * p = block.Allocate( BaseClass::objectSize_ );
			if ( nullptr != p )
			{
				BaseClass::recent_ = it;
				assert( !IsCorrupt() );
				return p;
			}
			++it;
		}

		// Now try to create a new block and insert it into container.
		const unsigned int objectsPerPool = BaseClass::blockSize_ / BaseClass::objectSize_;
		BlockType block( BaseClass::blockSize_, BaseClass::objectSize_, BaseClass::alignment_, objectsPerPool );
		void * p = block.Allocate( BaseClass::objectSize_ );
		assert( nullptr != p );
		it = std::lower_bound( begin, end, block );
		BaseClass::recent_ = BaseClass::blocks_.insert( it, block );
		assert( !IsCorrupt() );
		return p;
	}

	bool Release( void * place )
	{
		assert( !IsCorrupt() );
		// Programs often release chunks they recently allocated, so check recently used block first.
		if ( BaseClass::recent_ != BaseClass::blocks_.end() )
		{
			BlockType & block = *( BaseClass::recent_ );
			if ( block.HasAddress( place, BaseClass::blockSize_ ) )
			{
				block.Release( place, BaseClass::objectSize_ );
				if ( block.IsEmpty() )
				{
					block.Destroy();
					BaseClass::blocks_.erase( BaseClass::recent_ );
					BaseClass::recent_ = BaseClass::blocks_.end();
				}
				assert( !IsCorrupt() );
				return true;
			}
		}

		BlocksIter it( BaseClass::GetBlock( place ) );
		if ( it == BaseClass::blocks_.end() )
		{
			assert( !IsCorrupt() );
			return false;
		}
		BlockType & block = *it;
		block.Release( place, BaseClass::objectSize_ );
		if ( block.IsEmpty() )
		{
			const bool resetRecent = ( it == BaseClass::recent_ );
			block.Destroy();
			BaseClass::blocks_.erase( it );
			if ( resetRecent || ( BaseClass::blocks_.size() == 0 ) || ( BaseClass::recent_ > BaseClass::blocks_.end() ) )
			{
				BaseClass::recent_ = BaseClass::blocks_.end();
			}
		}
		assert( !IsCorrupt() );
		return true;
	}

	bool IsCorrupt() const
	{
		assert( nullptr != this );
		const BlocksCIter begin( BaseClass::blocks_.begin() );
		const BlocksCIter end( BaseClass::blocks_.end() );
		if ( BaseClass::recent_ != end )
		{
			assert( begin <= BaseClass::recent_ );
			assert( BaseClass::recent_ < end );
		}
		for ( BlocksCIter it( begin ); it != end; ++it )
		{
			const BlockType & block = *it;
			assert( !block.IsDestroyed() );
			assert( !block.IsCorrupt( BaseClass::objectSize_ ) );
		}
		return false;
	}

};

// ----------------------------------------------------------------------------

typedef BlockInfo< LinearBlock > LinearBlockInfo;
typedef BlockInfo< StackBlock > StackBlockInfo;
typedef BlockInfo< ListBlock > ListBlockInfo;

typedef AnyPoolBlockInfo< PoolBlock > PoolBlockInfo;
typedef TinyBlockPoolInfo< TinyBlock > TinyBlockInfo;

// ----------------------------------------------------------------------------

} // end project namespace
