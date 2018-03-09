
#include "../../src/StackBlock.hpp"

#include "ChunkList.hpp"

#include "UnitTest.hpp"

#include <iostream>

#include <cstring>
#include <cassert>

using namespace std;
using namespace memwa;


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

	std::cout << std::endl << "Simple Test of Various BlockSizes and Alignments with StackBlock." << std::endl
		<< "Block Size \t Alignment" << std::endl
		<< "========================" << std::endl;

	std::size_t blockSize = 400;
	std::size_t alignment = 4;
	TestStackBlock( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

	blockSize = 400;
	alignment = 8;
	TestStackBlock( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

	blockSize = 400;
	alignment = 16;
	TestStackBlock( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;
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

/*	std::cout << "Count of various resize tests" << std::endl
		<< "\t Shrink: " << shrinkCount << std::endl
		<< "\t Same Size: " << sameSizeCount << std::endl
		<< "\t Expand: " << expandCount << std::endl
		<< "\t Enlarge: " << enlargeCount << std::endl
		<< "\t Take All: " << takeAllCount << std::endl
		<< "\t Too Big: " << tooBigCount << std::endl;
*/
	UNIT_TEST( u, objectCount == block.GetObjectCount( blockSize, alignment ) );
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestStackBlockResize()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test StackBlock Resize" );

	std::cout << std::endl << "Testing Resize of Various BlockSizes and Alignments with StackBlock." << std::endl
		<< "Block Size \t Alignment" << std::endl
		<< "========================" << std::endl;

	std::size_t blockSize = 400;
	std::size_t alignment = 4;
	TestStackBlockResize( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

	blockSize = 400;
	alignment = 8;
	TestStackBlockResize( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

	blockSize = 400;
	alignment = 16;
	TestStackBlockResize( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

/*
	blockSize = 1600;
	alignment = 32;
	TestStackBlockResize( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;
*/
}

// ----------------------------------------------------------------------------

void TestStackBlockComplex( ut::UnitTest * u, const std::size_t blockSize, const std::size_t alignment )
{
	StackBlock block( blockSize, alignment );

	// First do basic tests of empty block.
	const std::size_t chunkSize = sizeof( StackBlock::ChunkInfo );
	std::size_t bytesBefore = block.GetFreeBytes( blockSize );
	const bool alignmentSupported = ( blockSize == bytesBefore );
//	std::cout << "blockSize: " << blockSize << "\t alignment: " << alignment << "\t bytesBefore: " << bytesBefore << std::endl;
	if ( alignmentSupported )
	{
		UNIT_TEST( u, bytesBefore + chunkSize == blockSize );
		UNIT_TEST( u, block.IsEmpty( alignment ) );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - chunkSize, blockSize, alignment ) );
	}
	else
	{
		UNIT_TEST( u, bytesBefore + alignment + chunkSize >= blockSize );
		UNIT_TEST( u, block.HasBytesAvailable( blockSize - ( alignment + chunkSize ), blockSize, alignment ) );
	}
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	unsigned int objectCount = block.GetObjectCount( blockSize, alignment );
	UNIT_TEST( u, objectCount == 0 );

	SizedChunkList chunks( 100 );
	ChunkInfo * chunk = nullptr;
	std::size_t objectSize = 0;

	// This test combines allocating, resizing, and releasing.
	for ( unsigned int ii = 0; ii < 200; ++ii )
	{
		assert( chunks.IsSorted() );
//		std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
		UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
		UNIT_TEST( u, objectCount == block.GetObjectCount( blockSize, alignment ) );
		chunk = chunks.GetTopChunk();
		const unsigned int action =
			( chunks.GetCount() == 0 ) ? 2 : rand() % 3;
		if ( ( action == 1 ) && ( chunk != nullptr ) )
		{
			// resize
			objectSize = chunk->GetSize();
			const std::size_t freeByteCountBeforeResize = block.GetFreeBytes( blockSize );
			UNIT_TEST( u, !block.IsEmpty( blockSize ) );
			UNIT_TEST( u, chunk->GetPlace() != nullptr );
			if ( freeByteCountBeforeResize != 0 )
			{
				const std::size_t randomSize = rand() % freeByteCountBeforeResize;
				const std::size_t newSize = std::min( randomSize, freeByteCountBeforeResize ) + alignment;
				const bool newSizeBigger = ( objectSize < newSize );
				const bool sameSizes = ( objectSize == newSize );
				const bool tooCloseInSizes = sameSizes
					|| ( newSizeBigger && ( ( newSize - objectSize ) < alignment ) )
					|| ( ( objectSize - newSize ) < alignment );
				UNIT_TEST( u, block.Resize( chunk->GetPlace(), objectSize, newSize, blockSize, alignment ) );
				chunk->SetSize( newSize );
				UNIT_TEST( u, objectCount == block.GetObjectCount( blockSize, alignment ) );
				const std::size_t freeByteCountAfterResize = block.GetFreeBytes( blockSize );
/*				std::cout << __FUNCTION__ << " : " << __LINE__
					<< " resize  newSize : " << newSize
					<< "  objectSize : " << objectSize
					<< "  free bytes before: " << freeByteCountBeforeResize
					<< "  free bytes after : " << freeByteCountAfterResize << std::endl;
*/
				if ( !tooCloseInSizes )
				{
					if ( newSizeBigger )
					{
						UNIT_TEST( u, freeByteCountAfterResize < freeByteCountBeforeResize );
					}
					else if ( sameSizes )
					{
						UNIT_TEST( u, freeByteCountAfterResize == freeByteCountBeforeResize );
					}
					else
					{
						UNIT_TEST( u, freeByteCountAfterResize > freeByteCountBeforeResize );
					}
				}
//				std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
			}
		}
		else if ( ( action == 0 ) && ( chunk != nullptr ) )
		{
			// release
//			std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
			objectSize = chunk->GetSize();
			const std::size_t freeByteCountBeforeResize = block.GetFreeBytes( blockSize );
			UNIT_TEST( u, blockSize != 0 );
			UNIT_TEST( u, objectSize != 0 );
			UNIT_TEST( u, block.HasAddress( chunk->GetPlace(), blockSize ) );
			UNIT_TEST( u, block.Release( chunk->GetPlace(), objectSize, blockSize, alignment ) );
			UNIT_TEST( u, objectCount - 1 == block.GetObjectCount( blockSize, alignment ) );
			const std::size_t freeByteCountAfterResize = block.GetFreeBytes( blockSize );
/*			std::cout << __FUNCTION__ << " : " << __LINE__
				<< " release   blockSize : " << blockSize
				<< "  objectSize : " << objectSize
				<< "  free bytes before: " << freeByteCountBeforeResize
				<< "  free bytes after : " << freeByteCountAfterResize << std::endl;
*/
			UNIT_TEST( u, freeByteCountBeforeResize < freeByteCountAfterResize );
			chunk = nullptr;
			chunks.RemoveTopChunk();
			objectCount = block.GetObjectCount( blockSize, alignment );
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
/*
			std::cout << __FUNCTION__ << " : " << __LINE__ << " allocate   blockSize: " << blockSize << "  alignment: " << alignment
				<< "  objectSize: " << objectSize << std::endl;
*/
			void * place = block.Allocate( objectSize, blockSize, alignment );
			if ( nullptr == place )
			{
				continue;
			}
			UNIT_TEST( u, objectCount + 1 == block.GetObjectCount( blockSize, alignment ) );
			const std::size_t bytesAfter = block.GetFreeBytes( blockSize );
			chunks.AddChunk( place, objectSize );
			UNIT_TEST( u, bytesAfter < bytesBefore );
			UNIT_TEST( u, !block.IsEmpty( blockSize ) );
			UNIT_TEST( u, place != nullptr );
			UNIT_TEST( u, reinterpret_cast< std::size_t >( chunk ) % alignment == 0 );
			objectCount = block.GetObjectCount( blockSize, alignment );
		}
	}

//	std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
	UNIT_TEST( u, !block.IsCorrupt( blockSize, alignment ) );
	block.Destroy();
}

// ----------------------------------------------------------------------------

void TestStackBlockComplex()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test StackBlock Complex" );

	std::cout << std::endl << "Testing Various BlockSizes and Alignments with StackBlock." << std::endl
		<< "Block Size \t Alignment" << std::endl
		<< "========================" << std::endl;

	std::size_t blockSize = 400;
	std::size_t alignment = 4;
	TestStackBlockComplex( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

	blockSize = 400;
	alignment = 8;
	TestStackBlockComplex( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

	blockSize = 400;
	alignment = 16;
	TestStackBlockComplex( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;

/*	blockSize = 1600;
	alignment = 32;
	TestStackBlockComplex( u, blockSize, alignment );
	std::cout << blockSize << "\t\t" << alignment << std::endl;
*/
}

// ----------------------------------------------------------------------------

void TestStackExceptions()
{
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "StackBlock Exceptions" );

	const std::size_t blockSize = 400;
	const std::size_t alignment = 16;
	StackBlock block( blockSize, alignment );

	std::size_t objectSize = 32;
	void * place = block.Allocate( objectSize, blockSize, alignment );
	UNIT_TEST( u, nullptr != place );

	std::size_t newSize = 60;
	UNIT_TEST_FOR_EXCEPTION( u, block.Resize( place, 16, newSize, blockSize, alignment ), std::invalid_argument );
	UNIT_TEST_FOR_EXCEPTION( u, block.Resize( place, 33, newSize, blockSize, alignment ), std::invalid_argument );
	void * badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) - alignment );
	UNIT_TEST_FOR_EXCEPTION( u, block.Resize( badPlace, objectSize, newSize, blockSize, alignment ), std::invalid_argument );
	badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) + alignment );
	UNIT_TEST_FOR_EXCEPTION( u, block.Resize( badPlace, objectSize, newSize, blockSize, alignment ), std::invalid_argument );
	badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) - blockSize );
	UNIT_TEST_FOR_EXCEPTION( u, block.Resize( badPlace, objectSize, newSize, blockSize, alignment ), std::invalid_argument );
	badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) + blockSize );
	UNIT_TEST_FOR_EXCEPTION( u, block.Resize( badPlace, objectSize, newSize, blockSize, alignment ), std::invalid_argument );

	UNIT_TEST( u, block.Resize( place, objectSize, 31, blockSize, alignment ) );
	newSize = 33;
	UNIT_TEST( u, block.Resize( place, objectSize, newSize, blockSize, alignment ) );
	objectSize = newSize;
	UNIT_TEST( u, block.Resize( place, objectSize, newSize, blockSize, alignment ) );

	UNIT_TEST_FOR_EXCEPTION( u, block.Release( place, objectSize - alignment, blockSize, alignment ), std::invalid_argument );
	UNIT_TEST_FOR_EXCEPTION( u, block.Release( place, objectSize + alignment, blockSize, alignment ), std::invalid_argument );
	badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) - alignment );
	UNIT_TEST_FOR_EXCEPTION( u, block.Release( badPlace, objectSize, blockSize, alignment ), std::invalid_argument );
	badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) + alignment );
	UNIT_TEST_FOR_EXCEPTION( u, block.Release( badPlace, objectSize, blockSize, alignment ), std::invalid_argument );
	badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) - blockSize );
	UNIT_TEST_FOR_EXCEPTION( u, block.Release( badPlace, objectSize, blockSize, alignment ), std::invalid_argument );
	badPlace = reinterpret_cast< void * >( reinterpret_cast< std::size_t >( place ) + blockSize );
	UNIT_TEST_FOR_EXCEPTION( u, block.Release( badPlace, objectSize, blockSize, alignment ), std::invalid_argument );

	UNIT_TEST( u, block.Release( place, objectSize, blockSize, alignment ) );
}

// ----------------------------------------------------------------------------
