
#include "../../include/AllocatorManager.hpp"

#include "ChunkList.hpp"

#include "UnitTest.hpp"

#include <iostream>
#include <mutex>
#include <thread>

#include <cassert>

using namespace memwa;

typedef std::unique_lock< std::mutex > MyLockGuard;

static bool showProximityCounts_ = true;

// ----------------------------------------------------------------------------

void ComplexMultithreadedTest( ut::UnitTest * u, Allocator * allocator, const AllocatorManager::AllocatorParameters & allocatorInfo, std::mutex & mutex )
{

	bool released = false;
	std::size_t hintSpot = 0;
	std::size_t placeSpot = 0;
	std::size_t hintDifference = 0; // Difference between hint location and allocated chunk.
	unsigned int hintTestCount = 0; // Number of allocations with a hint.
	unsigned int hintProximityCount = 0; // Number of times an allocated chunk is within blockSize of the hint.
	const unsigned int reserveCount = 1000;
	const unsigned int loopCount = 1000;
	void * place = nullptr;
	ChunkList chunks( reserveCount );

	for ( unsigned int ii = 0; ii < loopCount; ++ii )
	{
		const Actions action = ChooseAction( chunks );
		switch ( action )
		{
			case Actions::AllocateOne :
			{
				place = allocator->Allocate( allocatorInfo.objectSize );
				MyLockGuard lock( mutex );
				UNIT_TEST( u, ( place != nullptr ) );
				lock.unlock();
				chunks.AddChunk( place );
				break;
			}
			case Actions::AllocateMany :
			{
				const unsigned int count = rand() % 256;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					place = allocator->Allocate( allocatorInfo.objectSize );
					MyLockGuard lock( mutex );
					UNIT_TEST( u, ( place != nullptr ) );
					lock.unlock();
					chunks.AddChunk( place );
				}
				break;
			}
			case Actions::AllocateOneHint :
			{
				const void * hint = chunks.GetTopChunk();
				place = allocator->Allocate( allocatorInfo.objectSize, allocatorInfo.alignment, hint );
				MyLockGuard lock( mutex );
				UNIT_TEST( u, ( place != nullptr ) );
				UNIT_TEST( u, ( place != hint ) );
				lock.unlock();
				chunks.AddChunk( place );
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
				const void * hint = chunks.GetTopChunk();
				const unsigned int count = rand() % 32;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					place = allocator->Allocate( allocatorInfo.objectSize, hint );
					MyLockGuard lock( mutex );
					UNIT_TEST( u, ( place != nullptr ) );
					UNIT_TEST( u, ( place != hint ) );
					lock.unlock();
					chunks.AddChunk( place );
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
				void * place = chunks.GetTopChunk();
				released = allocator->Release( place, allocatorInfo.objectSize );
				MyLockGuard lock( mutex );
				UNIT_TEST( u, released );
				lock.unlock();
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
					released = allocator->Release( place, allocatorInfo.objectSize );
					MyLockGuard lock( mutex );
					UNIT_TEST( u, released );
					lock.unlock();
					chunks.RemoveTopChunk();
				}
				break;
			}
			case Actions::ReleaseOneBottom :
			{
				void * place = chunks.GetChunk( 0 );
				released = allocator->Release( place, allocatorInfo.objectSize );
				MyLockGuard lock( mutex );
				UNIT_TEST( u, released );
				lock.unlock();
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
					released = allocator->Release( place, allocatorInfo.objectSize );
					MyLockGuard lock( mutex );
					UNIT_TEST( u, released );
					lock.unlock();
					chunks.RemoveChunk( 0 );
				}
				break;
			}
			case Actions::ReleaseOneRandom :
			{
				const ChunkList::ChunkSpot spot = chunks.GetRandomChunk();
				void * place = spot.first;
				released = allocator->Release( place, allocatorInfo.objectSize );
				MyLockGuard lock( mutex );
				UNIT_TEST( u, released );
				lock.unlock();
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
					released = allocator->Release( place, allocatorInfo.objectSize );
					MyLockGuard lock( mutex );
					UNIT_TEST( u, released );
					lock.unlock();
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
		{
			const bool allUnique = chunks.AreUnique();
			MyLockGuard lock( mutex );
			UNIT_TEST( u, allUnique );
			lock.unlock();
		}
	}

	while ( chunks.GetCount() != 0 )
	{
		place = chunks.GetTopChunk();
		released = allocator->Release( place, allocatorInfo.objectSize );
		MyLockGuard lock( mutex );
		UNIT_TEST( u, released );
		lock.unlock();
		chunks.RemoveTopChunk();
	}

	if ( showProximityCounts_ )
	{
		const unsigned int percent = ( hintProximityCount * 100 ) / hintTestCount;
		std::cout << "hint proximity test: " << percent << "% out of " << hintTestCount << " tests." << std::endl
			<< "  Passing test is when allocator can allocate chunk near the hint more than 25% of the time." << std::endl;
	}
	MyLockGuard lock( mutex );
	UNIT_TEST( u, hintProximityCount * 4 > hintTestCount );
	UNIT_TEST( u, chunks.GetCount() == 0 );
}

// ----------------------------------------------------------------------------

void ComplexMultithreadedStackTest( ut::UnitTest * u, Allocator * allocator,
	const AllocatorManager::AllocatorParameters & allocatorInfo, std::mutex & mutex )
{

	const unsigned int reserveCount = 1000;
	const unsigned int loopCount = 1000;
	void * place = nullptr;
	ChunkList chunks( reserveCount );

	for ( unsigned int ii = 0; ii < loopCount; ++ii )
	{
		const std::size_t bytes = rand() % allocatorInfo.objectSize + allocatorInfo.alignment;
		const Actions action = ChooseAllocateAction( chunks );
		switch ( action )
		{
			case Actions::AllocateOne :
			{
				place = allocator->Allocate( bytes );
				MyLockGuard lock( mutex );
				UNIT_TEST( u, ( place != nullptr ) );
				lock.unlock();
				chunks.AddChunk( place );
				break;
			}
			case Actions::AllocateMany :
			{
				const unsigned int count = 1 + rand() % 32;
				for ( unsigned int jj = 0; jj < count; ++jj )
				{
					place = allocator->Allocate( bytes );
					MyLockGuard lock( mutex );
					UNIT_TEST( u, ( place != nullptr ) );
					lock.unlock();
					chunks.AddChunk( place );
				}
				break;
			}
			case Actions::AllocateOneHint :
			case Actions::AllocateManyHint :
			case Actions::ReleaseOneTop :
			case Actions::ReleaseManyTop :
			case Actions::ReleaseOneBottom :
			case Actions::ReleaseManyBottom :
			case Actions::ReleaseOneRandom :
			case Actions::ReleaseManyRandom :
			default:
			{
				break;
			}
		}
		{
			const bool allUnique = chunks.AreUnique();
			MyLockGuard lock( mutex );
			UNIT_TEST( u, allUnique );
			lock.unlock();
		}
	}
}

// ----------------------------------------------------------------------------

void RunComplexThreadTest( ut::UnitTest * u, Allocator * allocator, const AllocatorManager::AllocatorParameters allocatorInfo )
{
	std::mutex unitTestMutex_;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		threads[ ii ] = new std::thread( ComplexMultithreadedTest, u, allocator, std::ref( allocatorInfo ), std::ref( unitTestMutex_ ) );
	}
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		std::thread * t = threads[ ii ];
		t->join();
	}
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		std::thread * t = threads[ ii ];
		delete t;
	}
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
}

// ----------------------------------------------------------------------------

void DoComplexPoolThreadTest( bool showProximityCounts )
{
	std::cout << "Complex Pool Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Complex Pool Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2048;
	allocatorInfo.initialBlocks = 16;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	showProximityCounts_ = showProximityCounts;
	RunComplexThreadTest( u, allocator, allocatorInfo );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoComplexTinyThreadTest( bool showProximityCounts )
{
	std::cout << "Complex Tiny Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Complex Tiny Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 16 * 256;
	allocatorInfo.initialBlocks = 16;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;

	showProximityCounts_ = showProximityCounts;
	RunComplexThreadTest( u, allocator, allocatorInfo );
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void RunComplexStackThreadTest( ut::UnitTest * u, Allocator * allocator, const AllocatorManager::AllocatorParameters allocatorInfo )
{
	std::mutex unitTestMutex_;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		threads[ ii ] = new std::thread( ComplexMultithreadedStackTest, u, allocator, std::ref( allocatorInfo ), std::ref( unitTestMutex_ ) );
	}
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		std::thread * t = threads[ ii ];
		t->join();
	}
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		std::thread * t = threads[ ii ];
		delete t;
	}
}

// ----------------------------------------------------------------------------

void DoComplexStackThreadTest( bool showProximityCounts )
{
	std::cout << "Complex Stack Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Complex Stack Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 4096;
	allocatorInfo.initialBlocks = 16;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	showProximityCounts_ = showProximityCounts;
	RunComplexStackThreadTest( u, allocator, allocatorInfo );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoComplexLinearThreadTest( bool showProximityCounts )
{
	std::cout << "Complex Linear Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Complex Linear Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 4096;
	allocatorInfo.initialBlocks = 16;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	showProximityCounts_ = showProximityCounts;
	RunComplexStackThreadTest( u, allocator, allocatorInfo );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------
