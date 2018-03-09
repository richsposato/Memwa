
#include "../../include/AllocatorManager.hpp"

#include "ChunkList.hpp"

#include "UnitTest.hpp"

#include <iostream>

#include <cstring>
#include <cassert>

using namespace std;
using namespace memwa;


// ----------------------------------------------------------------------------

void AllocateTinyChunks( ut::UnitTest * u, Allocator * allocator, unsigned int chunkCount,
	ChunkList & chunks, AllocatorManager::AllocatorParameters & allocatorInfo )
{
	assert( nullptr != u );
	assert( nullptr != allocator );
	assert( allocatorInfo.objectSize > 4 );
	assert( chunks.GetCount() == 0 );
	for ( unsigned int ii = 0; ii < chunkCount; ++ii )
	{
		void * place = nullptr;
		UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) );
		UNIT_TEST( u, ( place != nullptr ) );
		chunks.AddChunk( place );
	}
}

// ----------------------------------------------------------------------------

void TestTinyAllocator( bool multithreaded )
{
	// These tests check if address returned by allocator is on an alignment boundary.
	// It means the address returned by an allocator is a multiple of alignment.

	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Tiny Allocator" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does exist." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 0;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	const unsigned int chunkCount = 10000;
	void * place = nullptr;

	{
		ChunkList chunks( chunkCount );
		AllocateTinyChunks( u, allocator, chunkCount, chunks, allocatorInfo );
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			place = chunks.GetChunk( 0 );
			UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
			chunks.RemoveChunk( 0 );
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

	{
		ChunkList chunks( chunkCount );
		AllocateTinyChunks( u, allocator, chunkCount, chunks, allocatorInfo );
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		for ( int ii = chunkCount - 1; ii >= 0; --ii )
		{
			place = chunks.GetChunk( ii );
			UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
			chunks.RemoveChunk( ii );
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

	UNIT_TEST( u, allocator != nullptr );
	{
		ChunkList chunks( chunkCount );
		AllocateTinyChunks( u, allocator, chunkCount, chunks, allocatorInfo );
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			const ChunkList::ChunkSpot spot = chunks.GetRandomChunk();
			void * place = spot.first;
			UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
			unsigned int index = spot.second;
			chunks.RemoveChunk( index );
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

/*
	{
		ChunkList chunks( chunkCount );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * hint = nullptr;
			if ( ii > 10 )
			{
				hint = chunks.GetChunk( ii - 10 );
			}
			void * place = nullptr;
			UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize, hint ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place );
		}
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			const ChunkList::ChunkSpot spot = chunks.GetRandomChunk();
			void * place = spot.first;
			UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
			unsigned int index = spot.second;
			chunks.RemoveChunk( index );
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}
/*
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------
