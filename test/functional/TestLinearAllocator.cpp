
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

void TestLinearAllocator( bool multithreaded, bool showProximityCounts )
{
	const char * threadType = ( multithreaded ) ? "Multi-Threaded" : "Single-Threaded";
	std::cout << "Basic Functionality " << threadType << " Tiny Allocator Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Linear Allocator" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
	allocatorInfo.objectSize = 32;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2048;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;

	const unsigned int chunkCount = 10000;
	std::size_t bytes = 0;
	std::size_t hintSpot = 0;
	std::size_t placeSpot = 0;
	std::size_t hintDifference = 0; // Difference between hint location and allocated chunk.
	unsigned int hintTestCount = 0; // Number of allocations with a hint.
	unsigned int hintProximityCount = 0; // Number of times an allocated chunk is within blockSize of the hint.

	{
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//		std::cout << "Allocator Type: " << typeid( *allocator ).name() << std::endl;
		SizedChunkList chunks( chunkCount );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * place = nullptr;
			bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
			UNIT_TEST( u, ( place = allocator->Allocate( bytes ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place, bytes );
		}
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//		std::cout << "Allocator Type: " << typeid( *allocator ).name() << std::endl;
		SizedChunkList chunks( chunkCount );
		for ( unsigned int ii = 0; ii < chunkCount; ++ii )
		{
			void * place = nullptr;
			bytes = rand() % ( allocatorInfo.blockSize / 4 ) + 1;
			std::size_t alignment = ( ii % 2 == 1 ) ? allocatorInfo.alignment / 2 : allocatorInfo.alignment / 4;
			UNIT_TEST( u, ( place = allocator->Allocate( bytes, alignment ) ) );
			UNIT_TEST( u, ( place != nullptr ) );
			chunks.AddChunk( place, bytes );
		}
		UNIT_TEST( u, chunks.GetCount() == chunkCount );
		UNIT_TEST( u, chunks.AreUnique() );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
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
		std::cout << __FUNCTION__ << " : " << __LINE__ << "  hint proximity: " << hintProximityCount << " out of tests: " << hintTestCount << std::endl; 
		UNIT_TEST( u, hintProximityCount * 4 > hintTestCount );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
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
			std::size_t alignment = ( ii % 2 == 1 ) ? allocatorInfo.alignment / 2 : allocatorInfo.alignment / 4;
			UNIT_TEST( u, ( place = allocator->Allocate( bytes, alignment, hint ) ) );
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
		UNIT_TEST( u, hintProximityCount * 4 > hintTestCount );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	if ( showProximityCounts )
	{
		std::cout << __FUNCTION__ << " : " << __LINE__ << "  hint proximity: " << hintProximityCount << " out of tests: " << hintTestCount << std::endl;
	}
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoLinearThreadSafetyTest()
{
	std::cout << "Linear Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Linear Thread-Safety Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
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
