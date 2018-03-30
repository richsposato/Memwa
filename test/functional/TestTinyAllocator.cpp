
#include "../../include/AllocatorManager.hpp"

#include "ChunkList.hpp"

#include "UnitTest.hpp"

#include <iostream>
#include <typeinfo>

#include <cassert>
#include <climits> // For UCHAR_MAX.
#include <cstring>

using namespace std;
using namespace memwa;


// ----------------------------------------------------------------------------

void AllocateTinyChunks( ut::UnitTest * u, Allocator * allocator, unsigned int chunkCount,
	ChunkList & chunks, AllocatorManager::AllocatorParameters & allocatorInfo, bool setAlignment )
{
	assert( nullptr != u );
	assert( nullptr != allocator );
	assert( allocatorInfo.objectSize > 4 );
	assert( chunks.GetCount() == 0 );
	if ( setAlignment )
	{
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * place = nullptr;
			std::size_t alignment = ( ii % 2 == 1 ) ? allocatorInfo.alignment / 2 : allocatorInfo.alignment;
			UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize, alignment ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place );
		}
	}
	else
	{
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * place = nullptr;
			UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place );
		}
	}
}

// ----------------------------------------------------------------------------

void TestTinyAllocator( bool multithreaded, bool showProximityCounts )
{
	const char * threadType = ( multithreaded ) ? "Multi-Threaded" : "Single-Threaded";
	std::cout << "Basic Functionality " << threadType << " Tiny Allocator Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Tiny Allocator" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 0;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//	std::cout << "Allocator Type: " << typeid( *allocator ).name() << std::endl;

	const unsigned int chunkCount = 10000;
	void * place = nullptr;
	std::size_t hintSpot = 0;
	std::size_t placeSpot = 0;
	std::size_t hintDifference = 0; // Difference between hint location and allocated chunk.
	unsigned int hintTestCount = 0; // Number of allocations with a hint.
	unsigned int hintProximityCount = 0; // Number of times an allocated chunk is within blockSize of the hint.

	{
		ChunkList chunks( chunkCount );
		AllocateTinyChunks( u, allocator, chunkCount, chunks, allocatorInfo, false );
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
		AllocateTinyChunks( u, allocator, chunkCount, chunks, allocatorInfo, true );
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		for ( int ii = chunkCount - 1; ii >= 0; --ii )
		{
			place = chunks.GetChunk( ii );
			UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize, allocatorInfo.alignment ) );
			chunks.RemoveChunk( ii );
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

	UNIT_TEST( u, allocator != nullptr );
	{
		ChunkList chunks( chunkCount );
		AllocateTinyChunks( u, allocator, chunkCount, chunks, allocatorInfo, false );
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
			UNIT_TEST( u, ( place != hint ) );
			hintSpot = reinterpret_cast< std::size_t >( hint );
			placeSpot = reinterpret_cast< std::size_t >( place );
			hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
			++hintTestCount;
			if ( hintDifference < allocatorInfo.objectSize * UCHAR_MAX )
			{
				++hintProximityCount;
			} 
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
			UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize, allocatorInfo.alignment, hint ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place );
			UNIT_TEST( u, ( place != hint ) );
			hintSpot = reinterpret_cast< std::size_t >( hint );
			placeSpot = reinterpret_cast< std::size_t >( place );
			hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
			++hintTestCount;
			if ( hintDifference < allocatorInfo.objectSize * UCHAR_MAX )
			{
				++hintProximityCount;
			} 
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

	if ( showProximityCounts )
	{
		const unsigned int percent = ( hintProximityCount * 100 ) / hintTestCount;
		std::cout << "hint proximity test: " << percent << "% out of " << hintTestCount << " tests." << std::endl
			<< "  Passing test is when allocator can allocate chunk near the hint more than 25% of the time." << std::endl;
	}
	UNIT_TEST( u, hintProximityCount * 4 > hintTestCount );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void ComplexTestTinyAllocator( bool multithreaded, bool showProximityCounts )
{
	const char * threadType = ( multithreaded ) ? "Multi-Threaded" : "Single-Threaded";
	std::cout << "Complex Functionality " << threadType << " Tiny Allocator Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Complex Test Tiny Allocator" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 0;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	std::size_t hintSpot = 0;
	std::size_t placeSpot = 0;
	std::size_t hintDifference = 0; // Difference between hint location and allocated chunk.
	unsigned int hintTestCount = 0; // Number of allocations with a hint.
	unsigned int hintProximityCount = 0; // Number of times an allocated chunk is within blockSize of the hint.
	unsigned int recencyTestCount = 0; // Number of times an allocated chunk should be within blockSize of just previously allocated chunk.
	unsigned int recencyActualCount = 0; // Number of times an allocated chunk is within blockSize of just previously allocated chunk.
	const unsigned int reserveCount = 1000;
	const unsigned int loopCount = 10000;
	void * place = nullptr;
	void * previous = nullptr;
	ChunkList chunks( reserveCount );

	for ( unsigned int ii = 0; ii < loopCount; ++ii )
	{
		const Actions action = ChooseAction( chunks );
		switch ( action )
		{
			case Actions::AllocateOne :
			{
				UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) );
				UNIT_TEST( u, ( place != nullptr ) );
				chunks.AddChunk( place );
				break;
			}
			case Actions::AllocateMany :
			{
				const unsigned int count = rand() % 256;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) );
					UNIT_TEST( u, ( place != nullptr ) );
					if ( nullptr != previous )
					{
						const std::size_t prevSpot = reinterpret_cast< std::size_t >( place );
						placeSpot = reinterpret_cast< std::size_t >( place );
						const std::size_t prevDifference = ( placeSpot < prevSpot ) ? prevSpot - placeSpot : placeSpot - prevSpot;
						++recencyTestCount;
						if ( prevDifference < allocatorInfo.objectSize * UCHAR_MAX )
						{
							++recencyActualCount;
						}
					}
					chunks.AddChunk( place );
					previous = place;
				}
				previous = nullptr;
				break;
			}
			case Actions::AllocateOneHint :
			{
				const void * hint = chunks.GetTopChunk();
				UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize, allocatorInfo.alignment, hint ) ) );
				UNIT_TEST( u, ( place != nullptr ) );
				chunks.AddChunk( place );
				UNIT_TEST( u, ( place != hint ) );
				hintSpot = reinterpret_cast< std::size_t >( hint );
				placeSpot = reinterpret_cast< std::size_t >( place );
				hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
				++hintTestCount;
				if ( hintDifference < allocatorInfo.objectSize * UCHAR_MAX )
				{
					++hintProximityCount;
				} 
				break;
			}
			case Actions::AllocateManyHint :
			{
				const void * hint = chunks.GetTopChunk();
				const unsigned int count = rand() % 256;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize, hint ) ) );
					UNIT_TEST( u, ( place != nullptr ) );
					chunks.AddChunk( place );
					UNIT_TEST( u, ( place != hint ) );
					hintSpot = reinterpret_cast< std::size_t >( hint );
					placeSpot = reinterpret_cast< std::size_t >( place );
					hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
					++hintTestCount;
					if ( hintDifference < allocatorInfo.objectSize * UCHAR_MAX )
					{
						++hintProximityCount;
					} 
				}
				break;
			}
			case Actions::ReleaseOneTop :
			{
				void * place = chunks.GetTopChunk();
				UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
				chunks.RemoveTopChunk();
				break;
			}
			case Actions::ReleaseManyTop :
			{
				const unsigned int countBefore = chunks.GetCount();
				unsigned int count = rand() % 128;
				if ( count > countBefore )
				{
					count = rand() % countBefore;
				}
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					void * place = chunks.GetTopChunk();
					UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
					chunks.RemoveTopChunk();
				}
				break;
			}
			case Actions::ReleaseOneBottom :
			{
				void * place = chunks.GetChunk( 0 );
				UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
				chunks.RemoveChunk( 0 );
				break;
			}
			case Actions::ReleaseManyBottom :
			{
				const unsigned int countBefore = chunks.GetCount();
				unsigned int count = rand() % 128;
				if ( count > countBefore )
				{
					count = rand() % countBefore;
				}
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					void * place = chunks.GetChunk( 0 );
					UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
					chunks.RemoveChunk( 0 );
				}
				break;
			}
			case Actions::ReleaseOneRandom :
			{
				const ChunkList::ChunkSpot spot = chunks.GetRandomChunk();
				void * place = spot.first;
				UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
				unsigned int index = spot.second;
				chunks.RemoveChunk( index );
				break;
			}
			case Actions::ReleaseManyRandom :
			{
				const unsigned int countBefore = chunks.GetCount();
				unsigned int count = rand() % 128;
				if ( count > countBefore )
				{
					count = rand() % countBefore;
				}
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					const ChunkList::ChunkSpot spot = chunks.GetRandomChunk();
					void * place = spot.first;
					UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
					unsigned int index = spot.second;
					chunks.RemoveChunk( index );
				}
				break;
			}
			default:
			{
				assert( false );
			}
		}
		UNIT_TEST( u, chunks.AreUnique() );
	}

	while ( chunks.GetCount() != 0 )
	{
		place = chunks.GetTopChunk();
		UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
		chunks.RemoveTopChunk();
	}

	if ( showProximityCounts )
	{
		unsigned int percent = ( hintProximityCount * 100 ) / hintTestCount;
		std::cout << "Hint proximity test: " << percent << "% out of " << hintTestCount << " tests." << std::endl
			<< "  Passing test is when allocator can allocate chunk near the hint more than 25% of the time." << std::endl;
		if ( 0 < recencyTestCount )
		{
			percent = ( recencyActualCount * 100 ) / recencyTestCount;
			std::cout << "Recency proximity test: " << percent << "% out of " << recencyTestCount << " tests." << std::endl
				<< "  Passing test is when allocator can allocate chunk near the most recently allocated chunk more than 50% of the time." << std::endl;
		}
	}

	UNIT_TEST( u, hintProximityCount * 4 > hintTestCount );
	if ( 0 < recencyTestCount )
	{
		UNIT_TEST( u, recencyActualCount * 2 > recencyTestCount );
	}
	UNIT_TEST( u, chunks.GetCount() == 0 );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------
