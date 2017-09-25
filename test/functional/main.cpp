
#include "AllocatorManager.hpp"

#include "CommandLineArgs.hpp"

#include "TinyObjectAllocator.hpp"
#include "../../src/PoolBlock.hpp"
#include "../../src/TinyBlock.hpp"
#include "../../src/StackBlock.hpp"
#include "../../src/LinearBlock.hpp"

#include "UnitTest.hpp"

#include <algorithm>
#include <iostream>

#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>

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

	std::size_t objectSize = 1;
	std::size_t alignment = 1;
	TestTinyBlock( u, objectSize, alignment );

	objectSize = 2;
	alignment = 2;
	TestTinyBlock( u, objectSize, alignment );

	objectSize = 4;
	alignment = 4;
	TestTinyBlock( u, objectSize, alignment );

	objectSize = 4;
	alignment = 2;
	TestTinyBlock( u, objectSize, alignment );

	objectSize = 8;
	alignment = 4;
	TestTinyBlock( u, objectSize, alignment );

	objectSize = 16;
	alignment = 8;
	TestTinyBlock( u, objectSize, alignment );

	objectSize = 12;
	alignment = 4;
	TestTinyBlock( u, objectSize, alignment );

	objectSize = 32;
	alignment = 8;
	TestTinyBlock( u, objectSize, alignment );

/*
	objectSize = 32;
	alignment = 16;
	TestTinyBlock( u, objectSize, alignment );
*/
}

// ----------------------------------------------------------------------------

void TestLinearBlock( ut::UnitTest * u, const std::size_t blockSize, const std::size_t alignment )
{
	LinearBlock block( blockSize, alignment );

	// First do basic tests of empty block.
	std::size_t bytesBefore = block.GetFreeBytes( blockSize );
	const bool alignmentSupported = ( blockSize == bytesBefore );
	if ( alignmentSupported )
	{
		UNIT_TEST( u, bytesBefore == blockSize );
		UNIT_TEST( u, block.IsEmpty( alignment ) );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize, blockSize, alignment ) );
	}
	else
	{
		UNIT_TEST( u, bytesBefore + alignment >= blockSize );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - alignment, blockSize, alignment ) );
	}
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );

	// Do tests while allocating chunks til block is full.
	void * holder[ 100 ];
	void * chunk = nullptr;
	for ( unsigned int ii = 0; ii < 100; ++ii )
	{
		bytesBefore = block.GetFreeBytes( blockSize );
		if ( bytesBefore < alignment )
		{
			break;
		}
		UNIT_TEST( u, bytesBefore <= blockSize );
		const std::size_t randomSize = rand() % 64;
		const std::size_t objectSize = std::min( randomSize, bytesBefore ) + alignment;
/*		std::cout << __FUNCTION__ << " : " << __LINE__ << "\t  blockSize: " << blockSize
			<< "\t objectSize: " << objectSize
			<< "\t alignment: " << alignment << std::endl; */
		chunk = block.Allocate( objectSize, blockSize, alignment );
		if ( nullptr == chunk )
		{
			break;
		}
		std::size_t bytesAfter = block.GetFreeBytes( blockSize );
		UNIT_TEST( u, bytesAfter < bytesBefore );
		UNIT_TEST( u, !block.IsEmpty( blockSize ) );
		UNIT_TEST( u, chunk != nullptr );
		UNIT_TEST( u, reinterpret_cast< std::size_t >( chunk ) % alignment == 0 );
		UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );

		// This for loop checks that each chunk allocated from the same block has a unique address.
		for ( unsigned int jj = 0; jj < ii; ++jj )
		{
			void * c = holder[ jj ];
			UNIT_TEST( u, chunk != c );
		}

		unsigned char * tooHigh = reinterpret_cast< unsigned char * >( chunk ) + blockSize;
		unsigned char * tooLow = reinterpret_cast< unsigned char * >( chunk ) - blockSize;
		holder[ ii ] = chunk;
		UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooLow, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( nullptr, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooHigh, blockSize ) );
		UNIT_TEST( u, block.IsBelowAddress( tooHigh, blockSize ) );
	}
	if ( alignmentSupported )
	{
		UNIT_TEST( u, !block.IsEmpty( blockSize ) );
	}

	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestLinearBlock()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test LinearBlock" );

	std::size_t blockSize = 400;
	std::size_t alignment = 4;
	TestLinearBlock( u, blockSize, alignment );

	blockSize = 400;
	alignment = 8;
	TestLinearBlock( u, blockSize, alignment );

	blockSize = 400;
	alignment = 16;
	TestLinearBlock( u, blockSize, alignment );

	/// @note Make note that LinearAllocator can't support alignment of 16 or more.
}

// ----------------------------------------------------------------------------

void TestStackBlock( ut::UnitTest * u, const std::size_t blockSize, const std::size_t alignment )
{
	StackBlock block( blockSize, alignment );

	// First do basic tests of empty block.
	std::size_t bytesBefore = block.GetFreeBytes( blockSize );
	const bool alignmentSupported = ( blockSize == bytesBefore + sizeof(StackBlock::ChunkInfo) );
	if ( alignmentSupported )
	{
		UNIT_TEST( u, bytesBefore + sizeof(StackBlock::ChunkInfo) == blockSize );
		UNIT_TEST( u, block.IsEmpty( alignment ) );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - sizeof(StackBlock::ChunkInfo), blockSize, alignment ) );
	}
	else
	{
		UNIT_TEST( u, bytesBefore + alignment + sizeof(StackBlock::ChunkInfo) >= blockSize );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - sizeof(StackBlock::ChunkInfo), blockSize, alignment ) );
	}
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );

	// Do tests while allocating chunks til block is full.
	void * holder[ 100 ];
	std::size_t sizes[ 100 ]; 
	void * chunk = nullptr;
	int count = 0;
	for ( unsigned int ii = 0; ii < 100; ++ii )
	{
		bytesBefore = block.GetFreeBytes( blockSize );
		if ( bytesBefore < alignment )
		{
			break;
		}
		UNIT_TEST( u, bytesBefore <= blockSize );
		const std::size_t randomSize = rand() % 64;
		const std::size_t objectSize = std::min( randomSize, bytesBefore ) + alignment;
		chunk = block.Allocate( objectSize, blockSize, alignment );
		if ( nullptr == chunk )
		{
			break;
		}
		++count;
		const std::size_t bytesAfter = block.GetFreeBytes( blockSize );
		UNIT_TEST( u, bytesAfter < bytesBefore );
		UNIT_TEST( u, !block.IsEmpty( blockSize ) );
		UNIT_TEST( u, chunk != nullptr );
		UNIT_TEST( u, reinterpret_cast< std::size_t >( chunk ) % alignment == 0 );
		UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );

		// This for loop checks that each chunk allocated from the same block has a unique address.
		for ( unsigned int jj = 0; jj < ii; ++jj )
		{
			void * c = holder[ jj ];
			UNIT_TEST( u, chunk != c );
		}

		unsigned char * tooHigh = reinterpret_cast< unsigned char * >( chunk ) + blockSize;
		unsigned char * tooLow = reinterpret_cast< unsigned char * >( chunk ) - blockSize;
		holder[ ii ] = chunk;
		sizes[ ii ]= objectSize;
		UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooLow, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( nullptr, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooHigh, blockSize ) );
		UNIT_TEST( u, block.IsBelowAddress( tooHigh, blockSize ) );
	}

	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	UNIT_TEST( u, !block.IsEmpty( blockSize ) );

	// Do tests while releasing chunks til block is empty.
	for ( int ii = count - 1; ii >= 0; --ii )
	{
		chunk = holder[ ii ];
		const std::size_t objectSize = sizes[ ii ];
		unsigned char * tooHigh = reinterpret_cast< unsigned char * >( chunk ) + blockSize;
		UNIT_TEST( u, !block.IsEmpty( alignment ) );
		UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
		UNIT_TEST( u, !block.HasAddress( tooHigh, blockSize ) );
		UNIT_TEST( u, block.IsBelowAddress( tooHigh, blockSize ) );
		bytesBefore = block.GetFreeBytes( blockSize );
		UNIT_TEST( u, block.Release( chunk, objectSize, blockSize, alignment ) );
		const std::size_t bytesAfter = block.GetFreeBytes( blockSize );
		UNIT_TEST( u, bytesAfter > bytesBefore );
		holder[ ii ] = nullptr;
	}

	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	if ( alignmentSupported )
	{
		UNIT_TEST( u, block.IsEmpty( blockSize ) );
	}

	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestStackBlock()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test StackBlock" );

	std::size_t blockSize = 400;
	std::size_t alignment = 4;
	TestStackBlock( u, blockSize, alignment );

	blockSize = 400;
	alignment = 8;
	TestStackBlock( u, blockSize, alignment );

/*
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	blockSize = 400;
	alignment = 16;
	TestStackBlock( u, blockSize, alignment );
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
*/
}

// ----------------------------------------------------------------------------

void TestStackBlockResize( ut::UnitTest * u, const std::size_t blockSize, const std::size_t alignment )
{
	StackBlock block( blockSize, alignment );

	// First do basic tests of empty block.
	std::size_t bytesBefore = block.GetFreeBytes( blockSize );
	const bool alignmentSupported = ( blockSize == bytesBefore + sizeof(StackBlock::ChunkInfo) );
	if ( alignmentSupported )
	{
		UNIT_TEST( u, bytesBefore + sizeof(StackBlock::ChunkInfo) == blockSize );
		UNIT_TEST( u, block.IsEmpty( alignment ) );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - sizeof(StackBlock::ChunkInfo), blockSize, alignment ) );
	}
	else
	{
		UNIT_TEST( u, bytesBefore + alignment >= blockSize + sizeof(StackBlock::ChunkInfo) );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - alignment, blockSize, alignment ) );
	}
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );

	void * chunk = nullptr;
	std::size_t objectSize = 0;
	// first fill the block with several chunks.
	for ( unsigned int ii = 0; ii < 100; ++ii )
	{
		bytesBefore = block.GetFreeBytes( blockSize );
		if ( bytesBefore < alignment )
		{
			break;
		}
		UNIT_TEST( u, bytesBefore <= blockSize );
		const std::size_t randomSize = rand() % 64;
		objectSize = std::min( randomSize, bytesBefore ) + alignment;
		chunk = block.Allocate( objectSize, blockSize, alignment );
		if ( nullptr == chunk )
		{
			break;
		}
		const std::size_t bytesAfter = block.GetFreeBytes( blockSize );
		UNIT_TEST( u, bytesAfter < bytesBefore );
		UNIT_TEST( u, !block.IsEmpty( blockSize ) );
		UNIT_TEST( u, chunk != nullptr );
		UNIT_TEST( u, reinterpret_cast< std::size_t >( chunk ) % alignment == 0 );
		UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
		if ( bytesAfter < blockSize / 4 )
		{
			break;
		}
	}

	unsigned int shrinkCount = 0;   // case 0
	unsigned int sameSizeCount = 0; // case 1
	unsigned int expandCount = 0;   // case 2
	unsigned int enlargeCount = 0;  // case 3
	unsigned int takeAllCount = 0;  // case 4
	unsigned int tooBigCount = 0;   // case 5
	// Now do a resize test.
	const unsigned int objectCount = block.GetObjectCount( blockSize, alignment );
	for ( int ii = 0; ii < 400; ++ii )
	{
		bytesBefore = block.GetFreeBytes( blockSize );
		UNIT_TEST( u, bytesBefore < blockSize );
		UNIT_TEST( u, objectCount > 0 );
		const bool hasSpace = ( 0 < bytesBefore );
		objectSize = block.GetChunkSize( 0, blockSize, alignment );
		UNIT_TEST( u, objectSize < blockSize - bytesBefore );
		UNIT_TEST( u, objectSize >= alignment );
		const int action = rand() % 6;
		switch ( action )
		{
			default:
			{
				assert( false );
				break;
			}
			case 0:
			{
				++shrinkCount;
				// test resize when newSize is smaller than objectSize.
				std::size_t newSize = ( rand() % objectSize ) + 1;
				if ( objectSize == newSize )
				{
					newSize = objectSize - 1;
				}
				UNIT_TEST( u, block.Resize( chunk, objectSize, newSize, blockSize, alignment ) );
				const std::size_t bytesAfterResize = block.GetFreeBytes( blockSize );
				if ( objectSize - newSize < alignment )
				{
					UNIT_TEST( u, bytesBefore == bytesAfterResize );
				}
				else if ( 0 != bytesBefore )
				{
					UNIT_TEST( u, bytesBefore < bytesAfterResize );
				}
				break;
			}
			case 1:
			{
				++sameSizeCount;
				// test resize when newSize is same as objectSize.
				UNIT_TEST( u, block.Resize( chunk, objectSize, objectSize, blockSize, alignment ) );
				const std::size_t bytesAfterResize = block.GetFreeBytes( blockSize );
				UNIT_TEST( u, bytesBefore == bytesAfterResize );
				break;
			}
			case 2:
			{
				++expandCount;
				// test resize when newSize is larger than objectSize but smaller than available bytes.
				const std::size_t newSize = objectSize + ( bytesBefore / 2 );
				UNIT_TEST( u, block.Resize( chunk, objectSize, newSize, blockSize, alignment ) );
				const std::size_t bytesAfterResize = block.GetFreeBytes( blockSize );
				if ( newSize - objectSize >= alignment )
				{
					UNIT_TEST( u, bytesBefore > bytesAfterResize );
				}
				else if ( 0 != bytesBefore )
				{
					UNIT_TEST( u, bytesBefore >= bytesAfterResize );
				}
				break;
			}
			case 3:
			{
				++enlargeCount;
				// test resize when newSize is just smaller than available bytes.
				const std::size_t newSize = objectSize + bytesBefore + sizeof(StackBlock::ChunkInfo) - 1;
				const bool withinInfoSize = ( newSize - objectSize <= sizeof(StackBlock::ChunkInfo) );
				const bool resized = block.Resize( chunk, objectSize, newSize, blockSize, alignment );
				UNIT_TEST( u, ( resized == hasSpace ) || withinInfoSize );
				const std::size_t bytesAfterResize = block.GetFreeBytes( blockSize );
				if ( withinInfoSize )
				{
					UNIT_TEST( u, bytesBefore == bytesAfterResize );
				}
				else if ( 0 != bytesBefore )
				{
					UNIT_TEST( u, bytesBefore > bytesAfterResize );
				}
				break;
			}
			case 4:
			{
				++takeAllCount;
				// test resize when newSize consumes exactly the number of available bytes.
				const std::size_t newSize = objectSize + bytesBefore + sizeof(StackBlock::ChunkInfo);
				const bool withinInfoSize = ( newSize - objectSize <= sizeof(StackBlock::ChunkInfo) );
				const bool resized = block.Resize( chunk, objectSize, newSize, blockSize, alignment );
				UNIT_TEST( u, ( resized == hasSpace ) || withinInfoSize );
				const std::size_t bytesAfterResize = block.GetFreeBytes( blockSize );
				UNIT_TEST( u, 0 == bytesAfterResize );
				break;
			}
			case 5:
			{
				++tooBigCount;
				// test resize when newSize is larger than available bytes.
				const std::size_t newSize = objectSize + bytesBefore + sizeof(StackBlock::ChunkInfo) + 1;
				UNIT_TEST( u, !block.Resize( chunk, objectSize, newSize, blockSize, alignment ) );
				const std::size_t bytesAfterResize = block.GetFreeBytes( blockSize );
				UNIT_TEST( u, bytesBefore == bytesAfterResize );
				break;
			}
		}
		UNIT_TEST( u, objectCount == block.GetObjectCount( blockSize, alignment ) );
		UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	}

	std::cout << "Count of various resize tests" << std::endl
		<< "\t Shrink: " << shrinkCount << std::endl
		<< "\t Same Size: " << sameSizeCount << std::endl
		<< "\t Expand: " << expandCount << std::endl
		<< "\t Enlarge: " << enlargeCount << std::endl
		<< "\t Take All: " << takeAllCount << std::endl
		<< "\t Too Big: " << tooBigCount << std::endl;

	UNIT_TEST( u, objectCount == block.GetObjectCount( blockSize, alignment ) );
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestStackBlockResize()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test StackBlock Resize" );

	std::size_t blockSize = 400;
	std::size_t alignment = 4;
	TestStackBlockResize( u, blockSize, alignment );

	blockSize = 400;
	alignment = 8;
	TestStackBlockResize( u, blockSize, alignment );

/*
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	blockSize = 400;
	alignment = 16;
	TestStackBlockResize( u, blockSize, alignment );
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
*/
}

// ----------------------------------------------------------------------------

void TestStackBlockComplex( ut::UnitTest * u, const std::size_t blockSize, const std::size_t alignment )
{
	StackBlock block( blockSize, alignment );

	// First do basic tests of empty block.
	std::size_t bytesBefore = block.GetFreeBytes( blockSize );
	const bool alignmentSupported = ( blockSize == bytesBefore );
	if ( alignmentSupported )
	{
		UNIT_TEST( u, bytesBefore == blockSize );
		UNIT_TEST( u, block.IsEmpty( alignment ) );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize, blockSize, alignment ) );
	}
	else
	{
		UNIT_TEST( u, bytesBefore + alignment >= blockSize );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - alignment, blockSize, alignment ) );
	}
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );

	void * holder[ 100 ];
	std::size_t sizes[ 100 ]; 
	void * chunk = nullptr;

	// This test combines allocating, resizing, and releasing.
	memset( holder, 0, sizeof(holder) );
	memset( sizes, 0, sizeof(sizes) );
	unsigned int index = 0;
	std::size_t objectSize = 0;
	for ( unsigned int ii = 0; ii < 200; ++ii )
	{
		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
		chunk = holder[ index ];
		const unsigned int action =
			( index == 100 ) ? 0 :
			( index == 0 ) ? 2 : rand() % 3;
		if ( ( action == 1 ) && ( chunk != nullptr ) )
		{
			std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
			// resize
			objectSize = sizes[ index ];
			const std::size_t bytesBeforeResize = block.GetFreeBytes( blockSize );
			UNIT_TEST( u, !block.IsEmpty( blockSize ) );
			UNIT_TEST( u, chunk != nullptr );
			const std::size_t randomSize = rand() % bytesBeforeResize;
			const std::size_t newSize = std::min( randomSize, bytesBeforeResize ) + alignment;
			const bool newSizeBigger = ( objectSize < newSize );
			const bool sameSizes = ( objectSize == newSize );
			const bool tooCloseInSizes = sameSizes
				|| ( newSizeBigger && ( ( newSize - objectSize ) < alignment ) )
				|| ( ( objectSize - newSize ) < alignment );
			UNIT_TEST( u, block.Resize( chunk, objectSize, newSize, blockSize, alignment ) );
			const std::size_t bytesAfterResize = block.GetFreeBytes( blockSize );
			if ( !tooCloseInSizes )
			{
				if ( newSizeBigger )
				{
					UNIT_TEST( u, bytesAfterResize < bytesBeforeResize );
				}
				else if ( sameSizes )
				{
					UNIT_TEST( u, bytesAfterResize == bytesBeforeResize );
				}
				else
				{
					UNIT_TEST( u, bytesAfterResize > bytesBeforeResize );
				}
			}
		}
		else if ( ( action == 0 ) && ( chunk != nullptr ) )
		{
			std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
			// release
			objectSize = sizes[ index ];
			const std::size_t bytesBeforeRelease = block.GetFreeBytes( blockSize );
			UNIT_TEST( u, block.HasAddress( chunk, blockSize ) );
			UNIT_TEST( u, block.Release( chunk, objectSize, blockSize, alignment ) );
			const std::size_t bytesAfterRelease = block.GetFreeBytes( blockSize );
			UNIT_TEST( u, bytesBeforeRelease < bytesAfterRelease );
			holder[ index ] = nullptr;
			sizes[ index ] = 0;
			chunk = nullptr;
			--index;
		}
		else if ( chunk == nullptr )
		{
			// allocate
			bytesBefore = block.GetFreeBytes( blockSize );
			if ( bytesBefore < alignment )
			{
				continue;
			}
			UNIT_TEST( u, bytesBefore <= blockSize );
			const std::size_t randomSize = rand() % 64;
			const std::size_t objectSize = std::min( randomSize, bytesBefore ) + ( 2 * alignment );
			std::cout << __FUNCTION__ << " : " << __LINE__ << "  blockSize: " << blockSize << "  alignment: " << alignment
				<< "  objectSize: " << objectSize << std::endl;
			chunk = block.Allocate( objectSize, blockSize, alignment );
			if ( nullptr == chunk )
			{
				continue;
			}
			++index;
			const std::size_t bytesAfter = block.GetFreeBytes( blockSize );
			holder[ index ] = chunk;
			sizes[ index ] = objectSize;
			UNIT_TEST( u, bytesAfter < bytesBefore );
			UNIT_TEST( u, !block.IsEmpty( blockSize ) );
			UNIT_TEST( u, chunk != nullptr );
			UNIT_TEST( u, reinterpret_cast< std::size_t >( chunk ) % alignment == 0 );
		}
	}

	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestStackBlockComplex()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test StackBlock Complex" );

	std::size_t blockSize = 400;
	std::size_t alignment = 4;
	TestStackBlockComplex( u, blockSize, alignment );

	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
/*
	blockSize = 400;
	alignment = 8;
	TestStackBlockComplex( u, blockSize, alignment );

	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	blockSize = 400;
	alignment = 16;
	TestStackBlockComplex( u, blockSize, alignment );
	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
*/
}

// ----------------------------------------------------------------------------

void TestAllocatorManager()
{

	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Allocator Manager" );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
	allocatorInfo.initialBlocks = 8;
	allocatorInfo.blockSize = 4096;
	allocatorInfo.objectSize = 8;
	allocatorInfo.alignment = 4;

	// Check if AllocatorManager won't allow calls to any functions before creating the manager.
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), std::logic_error, "Not allowed to call TrimEmptyBlocks before calling CreateManager." );
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, !AllocatorManager::DestroyAllocator( nullptr, false ), std::logic_error, "Not allowed to call DestroyAllocator before calling CreateManager." );
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::logic_error, "Not allowed to call CreateAllocator before calling CreateManager." );

	UNIT_TEST_WITH_MSG( u,  AllocatorManager::CreateManager( false, 4096 ), "Creation should pass since AllocatorManager does exist yet." );
	UNIT_TEST_WITH_MSG( u, !AllocatorManager::CreateManager( false, 4096 ), "Creation should fail since AllocatorManager already exists." );

	UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should work after calling CreateManager." );
	UNIT_TEST_WITH_MSG( u, !AllocatorManager::DestroyAllocator( nullptr, false ), "DestroyAllocator should fail because parameter is nullptr." );

	allocatorInfo.blockSize = 255;
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument, "CreateAllocator should fail if blockSize is less than 256." );
	allocatorInfo.blockSize = 256;

	allocatorInfo.initialBlocks = 0;
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument, "CreateAllocator should fail if block count is zero." );
	allocatorInfo.initialBlocks = 1;

	allocatorInfo.alignment = 0;
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument, "CreateAllocator should fail if alignment is zero." );

	allocatorInfo.alignment = 64;
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument, "CreateAllocator should fail if alignment is over 32." );

	allocatorInfo.alignment = 3;
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument, "CreateAllocator should fail if alignment is not a power of 2." );

	allocatorInfo.alignment = 16;
	allocatorInfo.blockSize = 1000;
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument, "CreateAllocator should fail if alignment is not block size is not a multiple of alignment." );
	allocatorInfo.blockSize = 1024;

	allocatorInfo.type = static_cast< AllocatorManager::AllocatorType >( 57 );
	UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument, "CreateAllocator should fail if type is not valid." );

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 4096;
		allocatorInfo.objectSize = 8;
		allocatorInfo.alignment = 4;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
//		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		UNIT_TEST_WITH_MSG( u, AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should work after calling CreateAllocator." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail if no allocators exist." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 100;
		allocatorInfo.blockSize = 4000;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should work after calling CreateAllocator." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail if no allocators exist." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should work after calling CreateAllocator." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail if no allocators exist." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should work after calling CreateAllocator." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail if no allocators exist." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 4096;
		allocatorInfo.objectSize = 8;
		allocatorInfo.alignment = 4;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
		UNIT_TEST_WITH_MSG( u, allocator->TrimEmptyBlocks(), "Should be able to call TrimEmptyBlocks on new allocator." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail after making allocator TrimEmptyBlocks." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
		UNIT_TEST_WITH_MSG( u, allocator->TrimEmptyBlocks(), "Should be able to call TrimEmptyBlocks on new allocator." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail after making allocator TrimEmptyBlocks." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
		UNIT_TEST_WITH_MSG( u, allocator->TrimEmptyBlocks(), "Should be able to call TrimEmptyBlocks on new allocator." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail after making allocator TrimEmptyBlocks." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
		UNIT_TEST_WITH_MSG( u, allocator->TrimEmptyBlocks(), "Should be able to call TrimEmptyBlocks on new allocator." );
		UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should fail after making allocator TrimEmptyBlocks." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 100;
		allocatorInfo.blockSize = 4096;
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) == nullptr, std::invalid_argument, "CreateAllocator should fail since blockSize is not a multiple of objectSize." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 2;
		allocatorInfo.alignment = 2;
		allocatorInfo.blockSize = 4096;
		UNIT_TEST_FOR_EXCEPTION( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) == nullptr, std::invalid_argument );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 1;
		allocatorInfo.alignment = 1;
		allocatorInfo.blockSize = 4096;
		UNIT_TEST_FOR_EXCEPTION( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) == nullptr, std::invalid_argument );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 4;
		allocatorInfo.alignment = 2;
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) == nullptr, std::invalid_argument, "CreateAllocator should fail since alignment is too small." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 4;
		allocatorInfo.alignment = 1;
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) == nullptr, std::invalid_argument, "CreateAllocator should fail since alignment is too small." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
		allocatorInfo.objectSize = 1;
		allocatorInfo.alignment = 4;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "Parameters are valid for StackAllocator" );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
		allocatorInfo.objectSize = 2;
		allocatorInfo.alignment = 4;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "Parameters are valid for StackAllocator" );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

}

// ----------------------------------------------------------------------------

void TestAlignment()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Alignment" );

	AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
	allocatorInfo.initialBlocks = 1;
	allocatorInfo.blockSize = 1000;
	allocatorInfo.objectSize = 100;
	allocatorInfo.alignment = 4;

	void * place = nullptr;
	std::size_t address = 0;

	{
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.blockSize = 4000;
		allocatorInfo.objectSize = 200;
		allocatorInfo.alignment = 8;
//	    std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//	    std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

} 

// ----------------------------------------------------------------------------

void PrintDataTypeSizes()
{
    std::cout << "sizeof(char) = " << sizeof(char) << std::endl;
    std::cout << "sizeof(short) = " << sizeof(short) << std::endl;
    std::cout << "sizeof(int) = " << sizeof(int) << std::endl;
    std::cout << "sizeof(long) = " << sizeof(long) << std::endl;
    std::cout << "sizeof(long long) = " << sizeof(long long) << std::endl;
    std::cout << "sizeof(float) = " << sizeof(float) << std::endl;
    std::cout << "sizeof(double) = " << sizeof(double) << std::endl;
    std::cout << "sizeof(long double) = " << sizeof(long double) << std::endl;
    std::cout << "sizeof(std::intmax_t) = " << sizeof(std::intmax_t) << std::endl;
    std::cout << "sizeof(std::uintmax_t) = " << sizeof(std::uintmax_t) << std::endl;
    std::cout << "sizeof(std::uintptr_t) = " << sizeof(std::uintptr_t) << std::endl;
    std::cout << "UCHAR_MAX = " << UCHAR_MAX << std::endl;
    std::cout << "Max Supported Alignment = " << memwa::AllocatorManager::GetMaxSupportedAlignment() << std::endl;
}

// ----------------------------------------------------------------------------

int main( int argc, const char * const argv[] )
{
	const CommandLineArgs args( argc, argv );
	if ( !args.IsValid() )
	{
		cout << "Your command line parameters are invalid!" << endl;
		args.ShowHelp();
		return 1;
	}
	if ( args.DoShowHelp() )
	{
		args.ShowHelp();
		return 0;
	}
	if ( args.DoShowDataSizes() )
	{
		PrintDataTypeSizes();
		return 0;
	}

	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	const bool deleteAtExitTime = args.DeleteAtExitTime();
	const ut::UnitTestSet::OutputOptions options = args.GetOutputOptions();
	const ut::UnitTestSet::ErrorState status = ut::UnitTestSet::Create(
		"Memwa Functionality Tests", args.GetTextFileName(), args.GetHtmlFileName(),
		args.GetXmlFileName(), options, deleteAtExitTime );
	if ( ( status != ut::UnitTestSet::Success )
	  && ( status != ut::UnitTestSet::AlreadyExists ) )
	{
		cout << "An error occurred when creating the UnitTestSet singleton!"
			 << endl;
		return 2;
	}

	std::srand( std::time( 0 ) );
//	TestLinearBlock();
	TestStackBlock();
	TestStackBlockResize();
//	TestStackBlockComplex();
	TestPoolBlock();
	TestTinyBlock();
	TestAllocatorManager();
	TestAlignment();

	if ( !args.DoMakeTableAtExitTime() )
	{
		uts.OutputSummary();
	}
	return 0;
}

// ----------------------------------------------------------------------------
