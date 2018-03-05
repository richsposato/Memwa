
#pragma once

#include <vector>

class ChunkInfo
{
public:

	ChunkInfo( void * place, std::size_t bytes ) : size_( bytes ), place_( place ) {}

	std::size_t GetSize() const { return size_; }
	void * GetPlace() const { return place_; }

	void SetSize( std::size_t bytes ) { size_ = bytes; }

private:

	std::size_t size_;
	void * place_;
};

// ----------------------------------------------------------------------------

class ChunkList
{
public:

	explicit ChunkList( unsigned int count );

	~ChunkList();

	bool AddChunk( void * place, std::size_t bytes );

	bool RemoveChunk();

	const ChunkInfo * GetTopChunk() const;

	ChunkInfo * GetTopChunk()
	{
		return const_cast< ChunkInfo * >(
			const_cast< const ChunkList * >( this )->GetTopChunk() );
	}

	void Output() const;

	bool IsSorted() const;

	unsigned int GetCount() const;

private:


	typedef std::vector< ChunkInfo > Chunks;

	Chunks chunks_;

};

// ----------------------------------------------------------------------------
