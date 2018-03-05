
#include "../../src/TinyBlock.hpp"

#include "UnitTest.hpp"

#include <iostream>

#include <cstdlib>

using namespace std;
using namespace memwa;


// ----------------------------------------------------------------------------

void TestTinyBlock( ut::UnitTest * u, const std::size_t objectSize, const std::size_t alignment )
{
	const std::size_t objectsPerPool = UCHAR_MAX;
	const std::size_t blockSize = objectsPerPool * objectSize;
	TinyBlock block( blockSize, objectSize, alignment, objectsPerPool );

	// First do basic tests of empty block.
	UNIT_TEST_WITH_MSG( u, block.IsEmpty(), "new block should be empty." );
	UNIT_TEST_WITH_MSG( u, !block.IsFull(), "new block should not be full." );
	UNIT_TEST_WITH_MSG( u, block.GetInUseCount() == 0, "new block should have no chunks in use." );
	UNIT_TEST( u, !block.IsCorrupt( objectSize ) );

	// Do tests while allocating chunks til block is full.
	void * holder[ objectsPerPool ];
	void * chunk = nullptr;
	for ( unsigned int ii = 0; ii < objectsPerPool; ++ii )
	{
		chunk = block.Allocate( objectSize );
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
	UNIT_TEST( u, !block.IsCorrupt( objectSize ) );
	UNIT_TEST( u, !block.IsEmpty() );
	UNIT_TEST( u, block.IsFull() );
	UNIT_TEST( u, block.GetInUseCount() == objectsPerPool );
	chunk = block.Allocate( objectSize );
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
		block.Release( chunk, objectSize );
		UNIT_TEST( u, block.GetInUseCount() == objectsPerPool - 1 - ii );
		holder[ ii ] = nullptr;
	}
	UNIT_TEST( u, block.IsEmpty() );
	UNIT_TEST( u, !block.IsCorrupt( objectSize ) );

	for ( unsigned int ii = 0; ii < objectsPerPool * 10; ++ii )
	{
		const unsigned int index = rand() % objectsPerPool;
		chunk = holder[ index ];
		if ( chunk == nullptr )
		{
			const unsigned int countBefore = block.GetInUseCount();
			chunk = block.Allocate( objectSize );
			const unsigned int countAfter = block.GetInUseCount();
			UNIT_TEST( u, countAfter - 1 == countBefore );
			UNIT_TEST( u, chunk != nullptr );
			const std::size_t chunkAddress = reinterpret_cast< std::size_t >( chunk );
			UNIT_TEST( u, chunkAddress % alignment == 0 );
/*			if ( chunkAddress % alignment != 0 )
			{
				std::cout << "\t chunk: " << chunkAddress
					<< "  objectSize: " << objectSize
					<< "  alignment: " << alignment
					<< "  offset: " << chunkAddress % alignment << std::endl;
			} */
			UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
			holder[ index ] = chunk;
		}
		else
		{
			UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
			const unsigned int countBefore = block.GetInUseCount();
			block.Release( chunk, objectSize );
			const unsigned int countAfter = block.GetInUseCount();
			UNIT_TEST( u, countAfter + 1 == countBefore );
			holder[ index ] = nullptr;
			chunk = nullptr;
		}
		UNIT_TEST( u, !block.IsCorrupt( objectSize ) );
	}

	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestTinyBlock()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test TinyBlock" );

	std::cout << std::endl << "Testing Various BlockSizes and Alignments with TinyBlock." << std::endl
		<< "Object Size \t Alignment" << std::endl
		<< "==========================" << std::endl;

	std::size_t objectSize = 1;
	std::size_t alignment = 1;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 2;
	alignment = 2;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 4;
	alignment = 4;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 4;
	alignment = 2;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 8;
	alignment = 4;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 16;
	alignment = 8;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 12;
	alignment = 4;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 32;
	alignment = 8;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;

	objectSize = 32;
	alignment = 16;
	TestTinyBlock( u, objectSize, alignment );
	std::cout << objectSize << "\t\t" << alignment << std::endl;
}

// ----------------------------------------------------------------------------
