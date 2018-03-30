
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

static bool showProximityCounts_ = true;

// ----------------------------------------------------------------------------

void AddChunkThread( ut::UnitTest * u, Allocator * allocator, unsigned int objectSize, ChunkList & chunks, std::mutex & mutex )
{
	const unsigned int loopCount = 1000;
	try
	{
		for ( unsigned int ii = 0; ii < loopCount; ++ii )
		{
			void * place = allocator->Allocate( objectSize );
			MyLockGuard lock( mutex );
			UNIT_TEST( u, ( place != nullptr ) );
			lock.unlock();
			chunks.AddChunk( place );
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
}

// ----------------------------------------------------------------------------

void ReleaseChunkThread( ut::UnitTest * u, Allocator * allocator, unsigned int objectSize, ChunkList & chunks, std::mutex & mutex )
{
	const unsigned int loopCount = 1000;
	try
	{
		for ( unsigned int ii = 0; ii < loopCount; ++ii )
		{
			void * place = chunks.GetTopChunk();
			const bool released = allocator->Release( place, objectSize );
			MyLockGuard lock( mutex );
			UNIT_TEST( u, released );
			lock.unlock();
			chunks.RemoveTopChunk();
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
}

// ----------------------------------------------------------------------------

void RunDuplicateTest( ut::UnitTest * u, Allocator * allocator, const AllocatorManager::AllocatorParameters allocatorInfo )
{
	std::mutex unitTestMutex;
	const unsigned int reserveCount = 1000;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];

	ChunkList chunkArray[ threadCount ];
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		chunks.Reserve( reserveCount );
	}

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		threads[ ii ] = new std::thread( AddChunkThread, u, allocator, allocatorInfo.objectSize, std::ref( chunks ), std::ref( unitTestMutex ) );
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

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		UNIT_TEST( u, chunks.AreUnique() );
		for ( unsigned int jj = ii+1; jj < threadCount; ++jj )
		{
			ChunkList & other = chunkArray[ jj ];
			const bool duplicates = chunks.AnyDuplicates( other );
			UNIT_TEST( u, !duplicates );
		}
	}

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		threads[ ii ] = new std::thread( ReleaseChunkThread, u, allocator, allocatorInfo.objectSize, std::ref( chunks ), std::ref( unitTestMutex ) );
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

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		UNIT_TEST( u, chunks.GetCount() == 0 );
	}
}

// ----------------------------------------------------------------------------

void DoDuplicatePoolThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Pool Allocator Duplicate Address Thread Test" << std::endl; 
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
	RunDuplicateTest( u, allocator, allocatorInfo );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoDuplicateTinyThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Tiny Allocator Duplicate Address Test" << std::endl; 
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
	RunDuplicateTest( u, allocator, allocatorInfo );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoDuplicateStackThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Stack Allocator Duplicate Address Test" << std::endl; 
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
	std::mutex unitTestMutex;
	const unsigned int reserveCount = 1000;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];

	ChunkList chunkArray[ threadCount ];
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		chunks.Reserve( reserveCount );
	}

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		threads[ ii ] = new std::thread( AddChunkThread, u, allocator, allocatorInfo.objectSize, std::ref( chunks ), std::ref( unitTestMutex ) );
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

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		const bool allUnique = chunks.AreUnique();
		UNIT_TEST( u, allUnique );
		for ( unsigned int jj = ii+1; jj < threadCount; ++jj )
		{
			ChunkList & other = chunkArray[ jj ];
			const bool duplicates = chunks.AnyDuplicates( other );
			UNIT_TEST( u, !duplicates );
		}
	}

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void DoDuplicateLinearThreadTest( bool showProximityCounts )
{
	std::cout << "Simple Linear Allocator Duplicate Address Test" << std::endl; 
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
	std::mutex unitTestMutex;
	const unsigned int reserveCount = 1000;
	const unsigned int threadCount = 16;
	std::thread * threads[ threadCount ];

	ChunkList chunkArray[ threadCount ];
	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		chunks.Reserve( reserveCount );
	}

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		threads[ ii ] = new std::thread( AddChunkThread, u, allocator, allocatorInfo.objectSize, std::ref( chunks ), std::ref( unitTestMutex ) );
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

	for ( unsigned int ii = 0; ii < threadCount; ++ii )
	{
		ChunkList & chunks = chunkArray[ ii ];
		const bool allUnique = chunks.AreUnique();
		UNIT_TEST( u, allUnique );
		for ( unsigned int jj = ii+1; jj < threadCount; ++jj )
		{
			ChunkList & other = chunkArray[ jj ];
			const bool duplicates = chunks.AnyDuplicates( other );
			UNIT_TEST( u, !duplicates );
		}
	}

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------
