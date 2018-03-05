
#include "ChunkList.hpp"

#include <iostream>
#include <stdexcept>

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

bool ChunkList::AddChunk( void * place, std::size_t bytes )
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

bool ChunkList::RemoveChunk()
{
	if ( chunks_.size() == 0 )
	{
		return false;
	}
	chunks_.pop_back();
	return true;
}

// ----------------------------------------------------------------------------

const ChunkInfo * ChunkList::GetTopChunk() const
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

bool ChunkList::IsSorted() const
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
		const ChunkInfo & chunk = chunks_[ ii ];
		void * place = chunk.GetPlace();
		std::size_t address = reinterpret_cast< std::size_t >( place );
		std::cout << "  place: " << address << "   bytes: " << chunk.GetSize() << std::endl;
	}
}

// ----------------------------------------------------------------------------
