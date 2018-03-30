
#include "ChunkList.hpp"

#include <iostream>
#include <set>
#include <stdexcept>

#include <cstdlib>

typedef std::set< void * > ChunkSet;
typedef ChunkSet::iterator ChunkSetIter;
typedef ChunkSet::const_iterator ChunkSetCIter;
typedef std::pair< ChunkSetCIter, bool > ChunkSetInsertResult;

typedef std::set< ChunkInfo > SizedChunkSet;
typedef SizedChunkSet::iterator SizedChunkSetIter;
typedef SizedChunkSet::const_iterator SizedChunkSetCIter;
typedef std::pair< SizedChunkSetCIter, bool > SizedChunkSetInsertResult;

// ----------------------------------------------------------------------------

ChunkList::ChunkList( unsigned int count ) :
	chunks_()
{
	chunks_.reserve( count );
}

// ----------------------------------------------------------------------------

ChunkList::~ChunkList()
{
}

// ----------------------------------------------------------------------------

void ChunkList::Reserve( unsigned int count )
{
	chunks_.reserve( count );
}

// ----------------------------------------------------------------------------

bool ChunkList::AddChunk( void * place )
{
	if ( place == nullptr )
	{
		return false;
	}
	chunks_.push_back( place );
	return true;
}

// ----------------------------------------------------------------------------

bool ChunkList::RemoveTopChunk()
{
	if ( chunks_.size() == 0 )
	{
		return false;
	}
	chunks_.pop_back();
	return true;
}

// ----------------------------------------------------------------------------

bool ChunkList::RemoveChunk( unsigned int index )
{
	if ( chunks_.size() <= index )
	{
		return false;
	}
	ChunksIter it( chunks_.begin() );
	std::advance( it, index );
	chunks_.erase( it );
	return true;
}

// ----------------------------------------------------------------------------

const void * ChunkList::GetChunk( unsigned int index ) const
{
	if ( chunks_.size() <= index )
	{
		return nullptr;
	}
	ChunksCIter cit( chunks_.begin() );
	cit += index;
	const void * place = *cit;
	return place;
}

// ----------------------------------------------------------------------------

const void * ChunkList::GetTopChunk() const
{
	const unsigned int count = chunks_.size();
	if ( count == 0 )
	{
		return nullptr;
	}
	const void * chunk = chunks_.at( count - 1 );
	return chunk;
}

// ----------------------------------------------------------------------------

ChunkList::ChunkSpot ChunkList::GetRandomChunk() const
{
	ChunkSpot spot( nullptr, 0 );
	const unsigned int count = chunks_.size();
	if ( count == 0 )
	{
		return spot;
	}
	const unsigned int index = rand() % count;
	void * place = chunks_[ index ];
	spot.first = place;
	spot.second = index;
	return spot;
}

// ----------------------------------------------------------------------------

bool ChunkList::AreUnique() const
{
	const unsigned int count = chunks_.size();
	if ( count < 2 )
	{
		return true;
	}

	ChunkSet chunkSet;
	const ChunksCIter end( chunks_.end() );
	for ( ChunksCIter cit( chunks_.begin() ); cit != end; ++cit )
	{
		void * chunk = *cit;
		const ChunkSetInsertResult result = chunkSet.insert( chunk );
		const bool success = result.second;
		if ( !success )
		{
			return false;
		}
	}
	return ( count == chunkSet.size() );
}

// ----------------------------------------------------------------------------

bool ChunkList::AnyDuplicates( const ChunkList & that ) const
{
	const unsigned int count = chunks_.size();
	if ( count == 0 )
	{
		return false;
	}
	const unsigned int thatCount = that.chunks_.size();
	if ( thatCount == 0 )
	{
		return false;
	}

	ChunkSet mine;
	for ( unsigned int ii = 0; ii < count; ++ii )
	{
		void * chunk = chunks_[ ii ];
		mine.insert( chunk );
	}
	for ( unsigned int ii = 0; ii < thatCount; ++ii )
	{
		void * chunk = that.chunks_[ ii ];
		const ChunkSetInsertResult result = mine.insert( chunk );
		if ( !result.second )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------------

bool ChunkList::IsSorted() const
{
	const unsigned int count = chunks_.size();
	if ( count < 2 )
	{
		return true;
	}
	const void * place = chunks_[ 0 ];
	for ( unsigned int ii = 1; ii < count; ++ii )
	{
		const void * chunk = chunks_[ ii ];
		if ( place >= chunk )
		{
			return false;
		}
		place = chunk;
	}
	return true;
}

// ----------------------------------------------------------------------------

unsigned int ChunkList::GetCount() const
{
	const unsigned int count = chunks_.size();
	return count;
}

// ----------------------------------------------------------------------------

void ChunkList::Output() const
{
	const unsigned int count = chunks_.size();
	for ( unsigned int ii = 0; ii < count; ++ii )
	{
		const void * chunk = chunks_[ ii ];
		std::size_t address = reinterpret_cast< std::size_t >( chunk );
		std::cout << "  place: " << address << std::endl;
	}
}

// ----------------------------------------------------------------------------

SizedChunkList::SizedChunkList( unsigned int count ) :
	chunks_()
{
	chunks_.reserve( count );
}

// ----------------------------------------------------------------------------

SizedChunkList::~SizedChunkList()
{
}

// ----------------------------------------------------------------------------

void SizedChunkList::Reserve( unsigned int count )
{
	chunks_.reserve( count );
}

// ----------------------------------------------------------------------------

bool SizedChunkList::AddChunk( void * place, std::size_t bytes )
{
	if ( ( place == nullptr ) || ( 0 == bytes ) )
	{
		return false;
	}

	ChunkInfo chunk( place, bytes );
	chunks_.push_back( chunk );
	return true;
}

// ----------------------------------------------------------------------------

bool SizedChunkList::RemoveTopChunk()
{
	if ( chunks_.size() == 0 )
	{
		return false;
	}
	chunks_.pop_back();
	return true;
}

// ----------------------------------------------------------------------------

bool SizedChunkList::RemoveChunk( unsigned int index )
{
	if ( chunks_.size() <= index )
	{
		return false;
	}
	ChunksIter it( chunks_.begin() );
//	it += index;
	std::advance( it, index );
	chunks_.erase( it );
	return true;
}

// ----------------------------------------------------------------------------

const SizedChunkList::ChunkSpot SizedChunkList::GetChunk( unsigned int index ) const
{
	if ( chunks_.size() <= index )
	{
		throw std::logic_error( "Index out of range." );
	}
	ChunksCIter cit( chunks_.begin() );
	cit += index;
	const ChunkInfo & info = *cit; 
	ChunkSpot spot( info, index );
	return spot;
}

// ----------------------------------------------------------------------------

const ChunkInfo * SizedChunkList::GetTopChunk() const
{
	const unsigned int count = chunks_.size();
	if ( count == 0 )
	{
		return nullptr;
	}
	const ChunkInfo & chunk = chunks_.at( count - 1 );
	return &chunk;
}

// ----------------------------------------------------------------------------

SizedChunkList::ChunkSpot SizedChunkList::GetRandomChunk() const
{
	const unsigned int count = chunks_.size();
	if ( count == 0 )
	{
		ChunkInfo info( nullptr, 0 );
		ChunkSpot spot( info, 0 );
		return spot;
	}
	const unsigned int index = rand() % count;
	const ChunkInfo & info = chunks_[ index ]; 
	ChunkSpot spot( info, index );
	return spot;
}

// ----------------------------------------------------------------------------

bool SizedChunkList::AreUnique() const
{
	const unsigned int count = chunks_.size();
	if ( count < 2 )
	{
		return true;
	}

	SizedChunkSet chunkSet;
	const ChunksCIter end( chunks_.end() );
	for ( ChunksCIter cit( chunks_.begin() ); cit != end; ++cit )
	{
		const ChunkInfo & chunk = *cit;
		const SizedChunkSetInsertResult result = chunkSet.insert( chunk );
		const bool success = result.second;
		if ( !success )
		{
			return false;
		}
	}
	return ( count == chunkSet.size() );
}

// ----------------------------------------------------------------------------

bool SizedChunkList::AnyDuplicates( const SizedChunkList & that ) const
{
	const unsigned int count = chunks_.size();
	if ( count == 0 )
	{
		return false;
	}
	const unsigned int thatCount = that.chunks_.size();
	if ( thatCount == 0 )
	{
		return false;
	}

	SizedChunkSet mine;
	ChunksCIter end( chunks_.end() );
	for ( ChunksCIter cit( chunks_.begin() ); cit != end; ++cit )
	{
		const ChunkInfo & chunk = *cit;
		mine.insert( chunk );
	}
	end = chunks_.end();
	for ( ChunksCIter cit( chunks_.begin() ); cit != end; ++cit )
	{
		const ChunkInfo & chunk = *cit;
		const SizedChunkSetInsertResult result = mine.insert( chunk );
		if ( !result.second )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------------

bool SizedChunkList::IsSorted() const
{
	const unsigned int count = chunks_.size();
	if ( count < 2 )
	{
		return true;
	}
	const ChunkInfo & first = chunks_[ 0 ];
	void * place = first.GetPlace();
	for ( unsigned int ii = 1; ii < count; ++ii )
	{
		const ChunkInfo & chunk = chunks_[ ii ];
		if ( place >= chunk.GetPlace() )
		{
			return false;
		}
		place = chunk.GetPlace();
	}
	return true;
}

// ----------------------------------------------------------------------------

unsigned int SizedChunkList::GetCount() const
{
	const unsigned int count = chunks_.size();
	return count;
}

// ----------------------------------------------------------------------------

void SizedChunkList::Output() const
{
	const unsigned int count = chunks_.size();
	for ( unsigned int ii = 0; ii < count; ++ii )
	{
		const ChunkInfo & chunk = chunks_[ ii ];
		void * place = chunk.GetPlace();
		std::size_t address = reinterpret_cast< std::size_t >( place );
		std::cout << "  place: " << address << "   bytes: " << chunk.GetSize() << std::endl;
	}
}

// ----------------------------------------------------------------------------

Actions ChooseAction( const ChunkList & chunks )
{
	if ( 0 == chunks.GetCount() )
	{
		unsigned int action = rand() % Actions::AllocateOneHint;
		return static_cast< Actions >( action );
	}
	unsigned int action = rand() % ( Actions::ReleaseManyRandom + 1 );
	return static_cast< Actions >( action );
}

// ----------------------------------------------------------------------------

Actions ChooseAction( const SizedChunkList & chunks )
{
	if ( 0 == chunks.GetCount() )
	{
		unsigned int action = rand() % Actions::AllocateOneHint;
		return static_cast< Actions >( action );
	}
	unsigned int action = rand() % ( Actions::ReleaseManyRandom + 1 );
	return static_cast< Actions >( action );
}

// ----------------------------------------------------------------------------

Actions ChooseAllocateAction( const ChunkList & chunks )
{
	if ( 0 == chunks.GetCount() )
	{
		unsigned int action = rand() % Actions::AllocateOneHint;
		return static_cast< Actions >( action );
	}
	unsigned int action = rand() % ( Actions::AllocateManyHint + 1 );
	return static_cast< Actions >( action );
}

// ----------------------------------------------------------------------------

Actions ChooseAllocateAction( const SizedChunkList & chunks )
{
	if ( 0 == chunks.GetCount() )
	{
		unsigned int action = rand() % Actions::AllocateOneHint;
		return static_cast< Actions >( action );
	}
	unsigned int action = rand() % ( Actions::AllocateManyHint + 1 );
	return static_cast< Actions >( action );
}

// ----------------------------------------------------------------------------
