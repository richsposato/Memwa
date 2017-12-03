
#include "../../src/LinearBlock.hpp"

#include "UnitTest.hpp"

#include <algorithm>

#include <cstdlib>

using namespace std;
using namespace memwa;


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

	blockSize = 800;
	alignment = 32;
	TestLinearBlock( u, blockSize, alignment );

	/// @note Make note that LinearAllocator can't support alignment of 16 or more.
}

// ----------------------------------------------------------------------------
