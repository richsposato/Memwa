
#include "../../include/AllocatorManager.hpp"

#include "ChunkList.hpp"

#include "UnitTest.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#include <cassert>

using namespace memwa;

typedef std::unique_lock< std::mutex > MyLockGuard;
typedef std::chrono::duration< unsigned int, std::milli > MillisecondDuration;
typedef std::chrono::milliseconds millis;

static bool showProximityCounts_ = true;

// ----------------------------------------------------------------------------

void SleepForRandomTime()
{
	const unsigned int sleepTime = rand() % 6;
	const MillisecondDuration d( sleepTime );
	const millis milliseconds( d );
	std::this_thread::sleep_for( milliseconds );
}

// ----------------------------------------------------------------------------

void SimpleMultithreadedTest( ut::UnitTest * u, Allocator * allocator, unsigned int objectSize, std::mutex & mutex )
{
	const unsigned int reserveCount = 1000;
	const unsigned int loopCount = 1000;
	void * place = nullptr;
	bool released = false;
	ChunkList chunks( reserveCount );

	try
	{
		for ( unsigned int ii = 0; ii < loopCount; ++ii )
		{
			place = allocator->Allocate( objectSize );
			MyLockGuard lock( mutex );
			UNIT_TEST( u, ( place != nullptr ) );
			lock.unlock();
			chunks.AddChunk( place );
			SleepForRandomTime();
		}
		{
			const bool allUnique = chunks.AreUnique();
			MyLockGuard lock( mutex );
			UNIT_TEST( u, allUnique );
			lock.unlock();
		}
		for ( unsigned int ii = 0; ii < loopCount; ++ii )
		{
			place = chunks.GetTopChunk();
			released = allocator->Release( place, objectSize );
			MyLockGuard lock( mutex );
			UNIT_TEST( u, released );
			lock.unlock();
			chunks.RemoveTopChunk();
			SleepForRandomTime();
		}
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}
	catch ( const std::exception & ex )
	{
		MyLockGuard lock( mutex );
		UNIT_TEST_WITH_MSG( u, false, ex.what() );
	}
	catch ( ... )
	{
		MyLockGuard lock( mutex );
		UNIT_TEST_WITH_MSG( u, false, "Caught unknown exception." );
	}
}

// ----------------------------------------------------------------------------

void SimpleMultithreadedStackTest( ut::UnitTest * u, Allocator * allocator,
	const AllocatorManager::AllocatorParameters & allocatorInfo, std::mutex & mutex )
{
	const unsigned int loopCount = 1000;
	void * place = nullptr;
	ChunkList chunks( loopCount );

	try
	{
		const std::size_t bytes = rand() % allocatorInfo.objectSize + allocatorInfo.alignment;
		for ( unsigned int ii = 0; ii < loopCount; ++ii )
		{
			place = allocator->Allocate( bytes );
			MyLockGuard lock( mutex );
			UNIT_TEST( u, ( place != nullptr ) );
			lock.unlock();
			chunks.AddChunk( place );
			SleepForRandomTime();
		}
	}
	catch ( const std::exception & ex )
	{
		MyLockGuard lock( mutex );
		UNIT_TEST_WITH_MSG( u, false, ex.what() );
	}
	catch ( ... )
	{
		MyLockGuard lock( mutex );
		UNIT_TEST_WITH_MSG( u, false, "Caught unknown exception." );
	}
	const bool allUnique = chunks.AreUnique();
	MyLockGuard lock( mutex );
	UNIT_TEST( u, allUnique );
	UNIT_TEST( u, chunks.GetCount() == loopCount );
}

// ----------------------------------------------------------------------------

void RunSimpleThreadTest( ut::UnitTest * u, Allocator * allocator, const AllocatorManager::AllocatorParameters allocatorInfo )
{
	std::mutex unitTestMutex_;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		threads[ ii ] = new std::thread( SimpleMultithreadedTest, u, allocator, allocatorInfo.objectSize, std::ref( unitTestMutex_ ) );
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

void DoSimplePoolThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Pool Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Pool Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2048;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	showProximityCounts_ = showProximityCounts;
	RunSimpleThreadTest( u, allocator, allocatorInfo );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoSimpleTinyThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Tiny Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Tiny Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 16 * 256;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	showProximityCounts_ = showProximityCounts;
	RunSimpleThreadTest( u, allocator, allocatorInfo );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoSimpleStackThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Stack Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Stack Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 4096;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	showProximityCounts_ = showProximityCounts;
	std::mutex unitTestMutex_;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		threads[ ii ] = new std::thread( SimpleMultithreadedStackTest, u, allocator, std::ref( allocatorInfo ), std::ref( unitTestMutex_ ) );
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

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoSimpleLinearThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Linear Allocator Thread-Safety Functionality Test" << std::endl; 
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Linear Thread Test" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( true, 4096 ), "Creation should pass since AllocatorManager does not exist yet." );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
	allocatorInfo.objectSize = 16;
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 4096;
	allocatorInfo.initialBlocks = 1;
	Allocator * allocator = nullptr;
	UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );

	showProximityCounts_ = showProximityCounts;
	std::mutex unitTestMutex_;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		threads[ ii ] = new std::thread( SimpleMultithreadedStackTest, u, allocator, std::ref( allocatorInfo ), std::ref( unitTestMutex_ ) );
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

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------
