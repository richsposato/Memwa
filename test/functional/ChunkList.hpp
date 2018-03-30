
#pragma once

#include <vector>

// ----------------------------------------------------------------------------

class ChunkList
{
public:

	typedef std::pair< void *, unsigned int > ChunkSpot;

	explicit ChunkList( unsigned int count = 0 );

	~ChunkList();

	void Reserve( unsigned int count );

	bool AddChunk( void * place );

	bool RemoveTopChunk();

	bool RemoveChunk( unsigned int index );

	const void * GetChunk( unsigned int index ) const;

	void * GetChunk( unsigned int index )
	{
		return const_cast< void * >(
			const_cast< const ChunkList * >( this )->GetChunk( index ) );
	}

	const void * GetTopChunk() const;

	void * GetTopChunk()
	{
		return const_cast< void * >(
			const_cast< const ChunkList * >( this )->GetTopChunk() );
	}

	ChunkSpot GetRandomChunk() const;

	void Output() const;

	bool IsSorted() const;

	bool AreUnique() const;

	bool AnyDuplicates( const ChunkList & that ) const;

	unsigned int GetCount() const;

private:


	typedef std::vector< void * > Chunks;
	typedef Chunks::iterator ChunksIter;
	typedef Chunks::const_iterator ChunksCIter;

	Chunks chunks_;

};

// ----------------------------------------------------------------------------

class ChunkInfo
{
public:

	ChunkInfo( void * place, std::size_t bytes ) : size_( bytes ), place_( place ) {}

	std::size_t GetSize() const { return size_; }
	void * GetPlace() const { return place_; }

	void SetSize( std::size_t bytes ) { size_ = bytes; }

	bool operator < ( const ChunkInfo & that ) const
	{
		return ( place_ < that.place_ );
	}

private:

	std::size_t size_;
	void * place_;
};

// ----------------------------------------------------------------------------

class SizedChunkList
{
public:

	typedef std::pair< ChunkInfo, unsigned int > ChunkSpot;

	explicit SizedChunkList( unsigned int count = 0 );

	~SizedChunkList();

	void Reserve( unsigned int count );

	bool AddChunk( void * place, std::size_t bytes );

	bool RemoveTopChunk();

	bool RemoveChunk( unsigned int index );

	const ChunkSpot GetChunk( unsigned int index ) const;

	ChunkSpot GetChunk( unsigned int index )
	{
		return ( const_cast< const SizedChunkList * >( this ) )->GetChunk( index );
	}

	const ChunkInfo * GetTopChunk() const;

	ChunkInfo * GetTopChunk()
	{
		return const_cast< ChunkInfo * >(
			const_cast< const SizedChunkList * >( this )->GetTopChunk() );
	}

	ChunkSpot GetRandomChunk() const;

	void Output() const;

	bool IsSorted() const;

	bool AreUnique() const;

	bool AnyDuplicates( const SizedChunkList & that ) const;

	unsigned int GetCount() const;

private:

	typedef std::vector< ChunkInfo > Chunks;
	typedef Chunks::iterator ChunksIter;
	typedef Chunks::const_iterator ChunksCIter;

	Chunks chunks_;

};

// ----------------------------------------------------------------------------

enum Actions
{
	AllocateOne = 0,
	AllocateMany,
	AllocateOneHint,
	AllocateManyHint,
	ReleaseOneTop,
	ReleaseManyTop,
	ReleaseOneBottom,
	ReleaseManyBottom,
	ReleaseOneRandom,
	ReleaseManyRandom,
};

Actions ChooseAction( const ChunkList & chunks );

Actions ChooseAction( const SizedChunkList & chunks );

Actions ChooseAllocateAction( const ChunkList & chunks );

Actions ChooseAllocateAction( const SizedChunkList & chunks );

// ----------------------------------------------------------------------------
