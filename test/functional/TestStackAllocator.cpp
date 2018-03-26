
#include "../../include/AllocatorManager.hpp"

#include "ChunkList.hpp"

#include "UnitTest.hpp"

#include <iostream>
#include <typeinfo>

#include <cassert>
#include <cstring>

using namespace std;
using namespace memwa;


// ----------------------------------------------------------------------------

void AllocateStackChunks( ut::UnitTest * u, Allocator * allocator, unsigned int chunkCount,
	SizedChunkList & chunks, AllocatorManager::AllocatorParameters & allocatorInfo, bool setAlignment )
{
	assert( nullptr != u );
	assert( nullptr != allocator );
	assert( allocatorInfo.objectSize > 4 );
	assert( chunks.GetCount() == 0 );
	std::size_t bytes = 0;
	if ( setAlignment )
	{
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * place = nullptr;
			bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
			std::size_t alignment = ( ii % 2 == 1 ) ? allocatorInfo.alignment / 2 : allocatorInfo.alignment;
			UNIT_TEST( u, ( place = allocator->Allocate( bytes, alignment ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place, bytes );
		}
	}
	else
	{
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * place = nullptr;
			bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
			UNIT_TEST( u, ( place = allocator->Allocate( bytes ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place, bytes );
		}
	}
}

// ----------------------------------------------------------------------------

void TestStackAllocator( bool multithreaded, bool showProximityCounts )
{
	const char * threadType = ( multithreaded ) ? "Multi-Threaded" : "Single-Threaded";
	std::cout << "Basic Functionality " << threadType << " Stack Allocator Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Stack Allocator" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
	allocatorInfo.objectSize = 32;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2048;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//	std::cout << "Allocator Type: " << typeid( *allocator ).name() << std::endl;

	const unsigned int chunkCount = 10000;
	void * place = nullptr;
	std::size_t bytes = 0;
	std::size_t hintSpot = 0;
	std::size_t placeSpot = 0;
	std::size_t hintDifference = 0; // Difference between hint location and allocated chunk.
	unsigned int hintTestCount = 0; // Number of allocations with a hint.
	unsigned int hintProximityCount = 0; // Number of times an allocated chunk is within blockSize of the hint.

	{
		SizedChunkList chunks( chunkCount );
		AllocateStackChunks( u, allocator, chunkCount, chunks, allocatorInfo, true );
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		while ( chunks.GetCount() != 0 )
		{
			const ChunkInfo * info = chunks.GetTopChunk();
			assert( nullptr != info );
			place = info->GetPlace();
			bytes = info->GetSize();
			UNIT_TEST( u, allocator->Release( place, bytes ) );
			chunks.RemoveTopChunk();
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

	{
		SizedChunkList chunks( chunkCount );
		AllocateStackChunks( u, allocator, chunkCount, chunks, allocatorInfo, false );
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		while ( chunks.GetCount() != 0 )
		{
			const ChunkInfo * info = chunks.GetTopChunk();
			assert( nullptr != info );
			place = info->GetPlace();
			bytes = info->GetSize();
			UNIT_TEST( u, allocator->Release( place, bytes ) );
			chunks.RemoveTopChunk();
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

	{
		SizedChunkList chunks( chunkCount );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * hint = nullptr;
			if ( ii > 10 )
			{
				SizedChunkList::ChunkSpot spot = chunks.GetChunk( ii - 10 );
				hint = spot.first.GetPlace();
			}
			void * place = nullptr;
			bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
			UNIT_TEST( u, ( place = allocator->Allocate( bytes, hint ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place, bytes );
			UNIT_TEST( u, ( place != hint ) );
			hintSpot = reinterpret_cast< std::size_t >( hint );
			placeSpot = reinterpret_cast< std::size_t >( place );
			hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
			++hintTestCount;
			if ( hintDifference < allocatorInfo.blockSize )
			{
				++hintProximityCount;
			} 
		}
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		while ( chunks.GetCount() != 0 )
		{
			const ChunkInfo * info = chunks.GetTopChunk();
			assert( nullptr != info );
			place = info->GetPlace();
			bytes = info->GetSize();
			UNIT_TEST( u, allocator->Release( place, bytes, allocatorInfo.alignment ) );
			chunks.RemoveTopChunk();
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

	{
		SizedChunkList chunks( chunkCount );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * hint = nullptr;
			if ( ii > 10 )
			{
				const ChunkInfo * info = chunks.GetTopChunk();
				assert( nullptr != info );
				hint = info->GetPlace();
			}
			void * place = nullptr;
			bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
			UNIT_TEST( u, ( place = allocator->Allocate( bytes, allocatorInfo.alignment, hint ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place, bytes );
			UNIT_TEST( u, ( place != hint ) );
			hintSpot = reinterpret_cast< std::size_t >( hint );
			placeSpot = reinterpret_cast< std::size_t >( place );
			hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
			++hintTestCount;
			if ( hintDifference < allocatorInfo.blockSize )
			{
				++hintProximityCount;
			} 
		}
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		while ( chunks.GetCount() != 0 )
		{
			const ChunkInfo * info = chunks.GetTopChunk();
			assert( nullptr != info );
			place = info->GetPlace();
			bytes = info->GetSize();
			UNIT_TEST( u, allocator->Release( place, bytes ) );
			chunks.RemoveTopChunk();
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}

	if ( showProximityCounts )
	{
		const unsigned int percent = ( hintProximityCount * 100 ) / hintTestCount;
		std::cout << "hint proximity test: " << percent << "% out of " << hintTestCount << " tests." << std::endl
			<< "  Passing test is when allocator can allocate block near the hint 25% of the time." << std::endl;
	}
	UNIT_TEST( u, hintProximityCount * 4 > hintTestCount );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void ComplexTestStackAllocator( bool multithreaded, bool showProximityCounts )
{
	const char * threadType = ( multithreaded ) ? "Multi-Threaded" : "Single-Threaded";
	std::cout << "Complex Functionality " << threadType << " Stack Allocator Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Complex Test Stack Allocator" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
	allocatorInfo.objectSize = 32;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2048;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	std::size_t hintSpot = 0;
	std::size_t placeSpot = 0;
	std::size_t hintDifference = 0; // Difference between hint location and allocated chunk.
	unsigned int hintTestCount = 0; // Number of allocations with a hint.
	unsigned int hintProximityCount = 0; // Number of times an allocated chunk is within blockSize of the hint.
	const unsigned int reserveCount = 1000;
	const unsigned int loopCount = 10000;
	void * place = nullptr;
	std::size_t bytes = 0;
	SizedChunkList chunks( reserveCount );

	for ( unsigned int ii = 0; ii < loopCount; ++ii )
	{
		const Actions action = ChooseAction( chunks );
		switch ( action )
		{
			case Actions::AllocateOne :
			{
				bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
				UNIT_TEST( u, ( place = allocator->Allocate( bytes ) ) );
				UNIT_TEST( u, ( place != nullptr ) );
				chunks.AddChunk( place, bytes );
				break;
			}
			case Actions::AllocateMany :
			{
				const unsigned int count = rand() % 64;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
					UNIT_TEST( u, ( place = allocator->Allocate( bytes ) ) );
					UNIT_TEST( u, ( place != nullptr ) );
					chunks.AddChunk( place, bytes );
				}
				break;
			}
			case Actions::AllocateOneHint :
			{
				const void * hint = chunks.GetTopChunk()->GetPlace();
				bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
				UNIT_TEST( u, ( place = allocator->Allocate( bytes, allocatorInfo.alignment, hint ) ) );
				UNIT_TEST( u, ( place != nullptr ) );
				chunks.AddChunk( place, bytes );
				UNIT_TEST( u, ( place != hint ) );
				hintSpot = reinterpret_cast< std::size_t >( hint );
				placeSpot = reinterpret_cast< std::size_t >( place );
				hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
				++hintTestCount;
				if ( hintDifference < allocatorInfo.blockSize )
				{
					++hintProximityCount;
				} 
				break;
			}
			case Actions::AllocateManyHint :
			{
				const void * hint = chunks.GetTopChunk()->GetPlace();
				const unsigned int count = rand() % 32;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
					UNIT_TEST( u, ( place = allocator->Allocate( bytes, hint ) ) );
					UNIT_TEST( u, ( place != nullptr ) );
					chunks.AddChunk( place, bytes );
					UNIT_TEST( u, ( place != hint ) );
					hintSpot = reinterpret_cast< std::size_t >( hint );
					placeSpot = reinterpret_cast< std::size_t >( place );
					hintDifference = ( placeSpot < hintSpot ) ? hintSpot - placeSpot : placeSpot - hintSpot;
					++hintTestCount;
					if ( hintDifference < allocatorInfo.blockSize )
					{
						++hintProximityCount;
					} 
				}
				break;
			}
			case Actions::ReleaseOneTop :
			{
				const ChunkInfo * info = chunks.GetTopChunk();
				assert( nullptr != info );
				place = info->GetPlace();
				bytes = info->GetSize();
				UNIT_TEST( u, allocator->Release( place, bytes ) );
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
					const ChunkInfo * info = chunks.GetTopChunk();
					assert( nullptr != info );
					place = info->GetPlace();
					bytes = info->GetSize();
					UNIT_TEST( u, allocator->Release( place, bytes ) );
					chunks.RemoveTopChunk();
				}
				break;
			}
			default:
			{
				break;
			}
		}
		UNIT_TEST( u, chunks.AreUnique() );
	}

	while ( chunks.GetCount() != 0 )
	{
		const ChunkInfo * info = chunks.GetTopChunk();
		assert( nullptr != info );
		place = info->GetPlace();
		bytes = info->GetSize();
		UNIT_TEST( u, allocator->Release( place, bytes ) );
		chunks.RemoveTopChunk();
	}

	if ( showProximityCounts )
	{
		const unsigned int percent = ( hintProximityCount * 100 ) / hintTestCount;
		std::cout << "hint proximity test: " << percent << "% out of " << hintTestCount << " tests." << std::endl
			<< "  Passing test is when allocator can allocate block near the hint 25% of the time." << std::endl;
	}
	UNIT_TEST( u, hintProximityCount * 8 > hintTestCount );
	UNIT_TEST( u, chunks.GetCount() == 0 );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoStackThreadSafetyTest()
{
	std::cout << "Stack Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Stack Thread-Safety Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 0;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------
