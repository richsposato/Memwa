
#include "AllocatorManager.hpp"

#include "CommandLineArgs.hpp"

#include "UnitTest.hpp"

#include <iostream>

#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>

using namespace std;
using namespace memwa;

void TestPoolBlock();
void TestTinyBlock();
void TestLinearBlock();
void TestStackBlock();
void TestStackBlockResize();
void TestStackBlockComplex();

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

	void * place = nullptr;
	std::size_t address = 0;

	allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 4;
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
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 2048;
		allocatorInfo.objectSize = 128;
		allocatorInfo.alignment = 16;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 4;
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
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 2048;
		allocatorInfo.objectSize = 128;
		allocatorInfo.alignment = 16;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 1;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 2;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 4;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.blockSize = 4000;
		allocatorInfo.objectSize = 200;
		allocatorInfo.alignment = 8;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 2048;
		allocatorInfo.objectSize = 128;
		allocatorInfo.alignment = 16;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 1;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 2;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 1000;
		allocatorInfo.objectSize = 100;
		allocatorInfo.alignment = 4;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		Allocator * allocator = nullptr;
		allocatorInfo.blockSize = 4000;
		allocatorInfo.objectSize = 200;
		allocatorInfo.alignment = 8;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 2048;
		allocatorInfo.objectSize = 128;
		allocatorInfo.alignment = 16;
		Allocator * allocator = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
		UNIT_TEST_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) != nullptr, "Should allocate correct size." );
		address = reinterpret_cast< std::size_t >( place );
		UNIT_TEST_WITH_MSG( u, ( address % allocatorInfo.alignment == 0 ), "Address should be aligned as requested." );
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
	TestLinearBlock();
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
