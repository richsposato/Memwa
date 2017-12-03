
#include "../../src/PoolBlock.hpp"

#include "UnitTest.hpp"

#include <cstdlib>

using namespace std;
using namespace memwa;

// ----------------------------------------------------------------------------

void TestPoolBlock( ut::UnitTest * u, const std::size_t blockSize, const std::size_t alignedSize, const std::size_t alignment )
{
	const unsigned int objectsPerPool = blockSize / alignedSize;
	PoolBlock block( blockSize, alignedSize, alignment, objectsPerPool );

	// First do basic tests of empty block.
	UNIT_TEST_WITH_MSG( u, block.IsEmpty(), "new block should be empty." );
	UNIT_TEST_WITH_MSG( u, !block.IsFull(), "new block should not be full." );
	UNIT_TEST_WITH_MSG( u, block.GetInUseCount() == 0, "new block should have no chunks in use." );
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment, alignedSize ) );

	// Do tests while allocating chunks til block is full.
	void * holder[ objectsPerPool ];
	void * chunk = nullptr;
	for ( unsigned int ii = 0; ii < objectsPerPool; ++ii )
	{
		chunk = block.Allocate();
		// This for loop checks that each chunk allocated from the same block has a unique address.
		for ( unsigned int jj = 0; jj < ii; ++jj )
		{
			void * c = holder[ jj ];
			UNIT_TEST( u, chunk != c );
		}
		unsigned char * tooHigh = reinterpret_cast< unsigned char * >( chunk ) + blockSize;
		unsigned char * tooLow = reinterpret_cast< unsigned char * >( chunk ) - blockSize;
		holder[ ii ] = chunk;
		UNIT_TEST( u, chunk != nullptr );
		UNIT_TEST( u, reinterpret_cast< std::size_t >( chunk ) % alignment == 0 );
		UNIT_TEST( u, !block.IsEmpty() );
		UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooLow, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( nullptr, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooHigh, blockSize ) );
		UNIT_TEST( u, block.IsBelowAddress( tooHigh, blockSize ) );
		UNIT_TEST( u, block.GetInUseCount() == 1 + ii );
	}
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment, alignedSize ) );
	UNIT_TEST( u, !block.IsEmpty() );
	UNIT_TEST( u, block.IsFull() );
	UNIT_TEST( u, block.GetInUseCount() == objectsPerPool );
	chunk = block.Allocate();
	UNIT_TEST( u, chunk == nullptr );

	// Do tests while releasing chunks til block is empty.
	for ( unsigned int ii = 0; ii < objectsPerPool; ++ii )
	{
		chunk = holder[ ii ];
		unsigned char * tooHigh = reinterpret_cast< unsigned char * >( chunk ) + blockSize;
		UNIT_TEST( u, !block.IsEmpty() );
		UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooHigh, blockSize ) );
		UNIT_TEST( u, block.IsBelowAddress( tooHigh, blockSize ) );
		UNIT_TEST( u, block.Release( chunk ) );
		UNIT_TEST( u, block.GetInUseCount() == objectsPerPool - 1 - ii );
		holder[ ii ] = nullptr;
	}
	UNIT_TEST( u, block.IsEmpty() );
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment, alignedSize ) );

	for ( unsigned int ii = 0; ii < 1000; ++ii )
	{
		const unsigned int index = rand() % objectsPerPool;
		void * chunk = holder[ index ];
		if ( chunk == nullptr )
		{
			const unsigned int countBefore = block.GetInUseCount();
			chunk = block.Allocate();
			const unsigned int countAfter = block.GetInUseCount();
			UNIT_TEST( u, countAfter - 1 == countBefore );
			UNIT_TEST( u, chunk != nullptr );
			UNIT_TEST( u, reinterpret_cast< std::size_t >( chunk ) % alignment == 0 );
			UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
			holder[ index ] = chunk;
		}
		else
		{
			UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
			const unsigned int countBefore = block.GetInUseCount();
			UNIT_TEST( u, block.Release( chunk ) );
			const unsigned int countAfter = block.GetInUseCount();
			UNIT_TEST( u, countAfter + 1 == countBefore );
			holder[ index ] = nullptr;
			chunk = nullptr;
		}
		UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment, alignedSize ) );
	}

	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestPoolBlock()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test PoolBlock" );

	std::size_t blockSize = 80;
	std::size_t alignedSize = 8;
	std::size_t alignment = 4;
	TestPoolBlock( u, blockSize, alignedSize, alignment );

	blockSize = 96;
	alignedSize = 16;
	alignment = 8;
	TestPoolBlock( u, blockSize, alignedSize, alignment );
/*
	blockSize = 96;
	alignedSize = 16;
	alignment = 16;
	TestPoolBlock( u, blockSize, alignedSize, alignment );
*/
	/// @todo Make note that PoolAllocator won't support alignment of 32.
}

// ----------------------------------------------------------------------------
