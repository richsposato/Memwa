
#include "AllocatorManager.hpp"

#include "CommandLineArgs.hpp"

#include "UnitTest.hpp"

#include <iostream>
#include <typeinfo>

#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <ctime>

using namespace std;
using namespace memwa;

extern void TestPoolBlock();
extern void TestTinyBlock();
extern void TestLinearBlock();
extern void TestStackBlock();
extern void TestStackBlockResize();
extern void TestStackBlockComplex();
extern void TestStackExceptions();

extern void TestLinearAllocator( bool multithreaded, bool showProximityCounts );
extern void TestStackAllocator( bool multithreaded, bool showProximityCounts );
extern void TestTinyAllocator( bool multithreaded, bool showProximityCounts );
extern void TestPoolAllocator( bool multithreaded, bool showProximityCounts );

extern void ComplexTestStackAllocator( bool multithreaded, bool showProximityCounts );
extern void ComplexTestTinyAllocator( bool multithreaded, bool showProximityCounts );
extern void ComplexTestPoolAllocator( bool multithreaded, bool showProximityCounts );

extern void DoSimplePoolThreadTest( bool showProximityCounts );
extern void DoSimpleTinyThreadTest( bool showProximityCounts );
extern void DoSimpleStackThreadTest( bool showProximityCounts );
extern void DoSimpleLinearThreadTest( bool showProximityCounts );

extern void DoDuplicateLinearThreadTest( bool showProximityCounts );
extern void DoDuplicateStackThreadTest( bool showProximityCounts );
extern void DoDuplicateTinyThreadTest( bool showProximityCounts );
extern void DoDuplicatePoolThreadTest( bool showProximityCounts );

extern void DoComplexLinearThreadTest( bool showProximityCounts );
extern void DoComplexStackThreadTest( bool showProximityCounts );
extern void DoComplexTinyThreadTest( bool showProximityCounts );
extern void DoComplexPoolThreadTest( bool showProximityCounts );

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

	UNIT_TEST_WITH_MSG( u, !AllocatorManager::DestroyManager( true ), "Destruction should not pass since AllocatorManager does not exist yet." );

	UNIT_TEST_WITH_MSG( u,  AllocatorManager::CreateManager( false, 4096 ), "Creation should pass since AllocatorManager does exist." );
	UNIT_TEST_WITH_MSG( u, !AllocatorManager::CreateManager( false, 4096 ), "Creation should fail since AllocatorManager already exists." );

	UNIT_TEST_WITH_MSG( u, !AllocatorManager::TrimEmptyBlocks(), "TrimEmptyBlocks should work after calling CreateManager." );
	UNIT_TEST_WITH_MSG( u, !AllocatorManager::DestroyAllocator( nullptr, false ), "DestroyAllocator should fail because parameter is nullptr." );

	{
		Allocator * allocator = nullptr;
		allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.blockSize = 4096;
		allocatorInfo.objectSize = 8;
		allocatorInfo.alignment = 4;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "CreateAllocator should pass." );
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

	UNIT_TEST_WITH_MSG( u,  AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
	UNIT_TEST_WITH_MSG( u, !AllocatorManager::DestroyManager( true ), "Destruction should not pass since AllocatorManager does not exist." );
}

// ----------------------------------------------------------------------------

void TestManagerExceptions( bool multithreaded )
{

	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Manager Exceptions" );

	// Check if AllocatorManager won't allow calls to any functions before creating the manager.
	// Not allowed to call TrimEmptyBlocks before calling CreateManager.
	UNIT_TEST_FOR_EXCEPTION( u, !AllocatorManager::TrimEmptyBlocks(), std::logic_error );
	// Not allowed to call DestroyAllocator before calling CreateManager.
	UNIT_TEST_FOR_EXCEPTION( u, !AllocatorManager::DestroyAllocator( nullptr, false ), std::logic_error );
	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.initialBlocks = 8;
		allocatorInfo.blockSize = 4000;
		allocatorInfo.objectSize = 8;
		allocatorInfo.alignment = 4;
		// Not allowed to call CreateAllocator before calling CreateManager.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::logic_error );
	}

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does exist." );

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.initialBlocks = 8;
		allocatorInfo.blockSize = 255;
		allocatorInfo.objectSize = 8;
		allocatorInfo.alignment = 4;
		// CreateAllocator should fail if blockSize is less than 256.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.blockSize = 256;
		allocatorInfo.objectSize = 8;
		allocatorInfo.alignment = 4;
		allocatorInfo.initialBlocks = 0;
		// CreateAllocator should fail if block count is zero.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.blockSize = 400;
		allocatorInfo.objectSize = 8;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.alignment = 0;
		// CreateAllocator should fail if alignment is zero.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.blockSize = 256;
		allocatorInfo.objectSize = 8;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.alignment = 64;
		// CreateAllocator should fail if alignment is over 32.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.blockSize = 256;
		allocatorInfo.objectSize = 8;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.alignment = 3;
		// CreateAllocator should fail if alignment is not a power of 2.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.alignment = 16;
		allocatorInfo.blockSize = 300;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.objectSize = 8;
		// CreateAllocator should fail if alignment is not block size is not a multiple of alignment.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.blockSize = 1024;
		allocatorInfo.objectSize = 8;
		allocatorInfo.initialBlocks = 1;
		allocatorInfo.alignment = 8;
		allocatorInfo.type = static_cast< AllocatorManager::AllocatorType >( 57 );
		// CreateAllocator should fail if type is not valid.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.initialBlocks = 8;
		allocatorInfo.blockSize = 255;
		allocatorInfo.objectSize = sizeof(void *) - 1;
		allocatorInfo.alignment = 4;
		// CreateAllocator should fail if object size < sizeof(void *).
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
		allocatorInfo.blockSize = 30;
		allocatorInfo.alignment = 32;
		allocatorInfo.objectSize = 8;
		allocatorInfo.initialBlocks = 1;
		// CreateAllocator should fail if blockSize is smaller than alignment.
		UNIT_TEST_FOR_EXCEPTION( u, nullptr == AllocatorManager::CreateAllocator( allocatorInfo ), std::invalid_argument );
	}

	// Test for exceptions for Pool allocator.
	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 100;
		allocatorInfo.blockSize = 4096;
		allocatorInfo.alignment = 16;
		allocatorInfo.initialBlocks = 1;
		// CreateAllocator should fail since blockSize is not a multiple of objectSize.
		UNIT_TEST_FOR_EXCEPTION( u, ( AllocatorManager::CreateAllocator( allocatorInfo ) == nullptr ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 2;
		allocatorInfo.alignment = 2;
		allocatorInfo.blockSize = 4096;
		allocatorInfo.initialBlocks = 1;
		UNIT_TEST_FOR_EXCEPTION( u, ( AllocatorManager::CreateAllocator( allocatorInfo ) == nullptr ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 1;
		allocatorInfo.alignment = 1;
		allocatorInfo.blockSize = 4096;
		allocatorInfo.initialBlocks = 1;
		UNIT_TEST_FOR_EXCEPTION( u, ( AllocatorManager::CreateAllocator( allocatorInfo ) == nullptr ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 4;
		allocatorInfo.alignment = 2;
		allocatorInfo.blockSize = 1024;
		allocatorInfo.initialBlocks = 1;
		// CreateAllocator should fail since alignment is too small.
		UNIT_TEST_FOR_EXCEPTION( u, ( AllocatorManager::CreateAllocator( allocatorInfo ) == nullptr ), std::invalid_argument );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 4;
		allocatorInfo.alignment = 1;
		allocatorInfo.blockSize = 1024;
		allocatorInfo.initialBlocks = 1;
		// CreateAllocator should fail since alignment is too small.
		UNIT_TEST_FOR_EXCEPTION( u, ( AllocatorManager::CreateAllocator( allocatorInfo ) == nullptr ), std::invalid_argument );
	}

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void TestAllocatorExceptions( bool multithreaded )
{

	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Allocator Exceptions" );

	UNIT_TEST_WITH_MSG( u, AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does exist." );

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Linear;
		allocatorInfo.objectSize = 4;
		allocatorInfo.alignment = 8;
		allocatorInfo.blockSize = 1024;
		allocatorInfo.initialBlocks = 1;
		Allocator * allocator = nullptr;
		std::size_t badAlignment = allocatorInfo.alignment * 2;
		void * place = nullptr;
		void * hint = nullptr;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//		std::cout << __FUNCTION__ << " : " << __LINE__ << "  Allocator Type: " << typeid( *allocator ).name() << std::endl;
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.blockSize + 1 ) ), std::invalid_argument, "Should throw since requested size is larger than block size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.blockSize + 1, hint ) ), std::invalid_argument, "Should throw since requested size is larger than block size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.blockSize ), std::logic_error, "Should throw since tiny allocator does not allow resize." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.blockSize, allocatorInfo.alignment ), std::logic_error, "Should throw since tiny allocator does not allow resize." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.blockSize, badAlignment ) ), std::invalid_argument, "Should throw since requested alignment is larger than block alignment." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Pool;
		allocatorInfo.objectSize = 64;
		allocatorInfo.alignment = 8;
		allocatorInfo.blockSize = 1024;
		allocatorInfo.initialBlocks = 1;
		Allocator * allocator = nullptr;
		void * place = nullptr;
		void * hint = nullptr;
		const std::size_t badAlignment = allocatorInfo.alignment * 2;
		const std::size_t requestedSize = allocatorInfo.objectSize + allocatorInfo.alignment;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//		std::cout << __FUNCTION__ << " : " << __LINE__ << "  Allocator Type: " << typeid( *allocator ).name() << std::endl;
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( requestedSize ) ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.blockSize, badAlignment ) ), std::invalid_argument, "Should throw since requested alignment is larger than block alignment." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( requestedSize, hint ) ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.blockSize, badAlignment, hint ) ), std::invalid_argument, "Should throw since requested alignment is larger than block alignment." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) );
		UNIT_TEST( u, ( place != nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize + allocatorInfo.alignment ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize - allocatorInfo.alignment ), std::invalid_argument, "Should throw since requested size is smaller than block's object size." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize, badAlignment ), std::invalid_argument, "Should throw since requested alignment is larger than block's alignment." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize + allocatorInfo.alignment, allocatorInfo.alignment ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize - allocatorInfo.alignment, allocatorInfo.alignment ), std::invalid_argument, "Should throw since requested size is smaller than block's object size." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.blockSize ), std::logic_error, "Should throw since tiny allocator does not allow resize." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.blockSize, allocatorInfo.alignment ), std::logic_error, "Should throw since tiny allocator does not allow resize." );
		UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Stack;
		allocatorInfo.objectSize = 64;
		allocatorInfo.alignment = 8;
		allocatorInfo.blockSize = 1024;
		allocatorInfo.initialBlocks = 1;
		Allocator * allocator = nullptr;
		void * place = nullptr;
		void * hint = nullptr;
		const std::size_t requestedSize = allocatorInfo.blockSize + allocatorInfo.alignment;
		const std::size_t badAlignment = allocatorInfo.alignment * 2;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//		std::cout << __FUNCTION__ << " : " << __LINE__ << "  Allocator Type: " << typeid( *allocator ).name() << std::endl;
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( requestedSize ) ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize, badAlignment ) ), std::invalid_argument, "Should throw since requested alignment is larger than block's alignment." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( requestedSize, hint ) ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.blockSize, badAlignment, hint ) ), std::invalid_argument, "Should throw since requested alignment is larger than block alignment." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) );
		UNIT_TEST( u, ( place != nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize, badAlignment ), std::invalid_argument, "Should throw since requested alignment is larger than block's alignment." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.objectSize + allocatorInfo.alignment, badAlignment ), std::invalid_argument, "Should throw since requested alignment is larger than block's alignment." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( nullptr, allocatorInfo.objectSize, allocatorInfo.blockSize + allocatorInfo.alignment ), std::invalid_argument, "Should throw since pointer is null." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.objectSize + allocatorInfo.alignment, badAlignment ), std::invalid_argument, "Should throw since requested alignment is larger than block's alignment." );
		UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	{
		AllocatorManager::AllocatorParameters allocatorInfo;
		allocatorInfo.type = AllocatorManager::AllocatorType::Tiny;
		allocatorInfo.objectSize = 16;
		allocatorInfo.alignment = 8;
		allocatorInfo.blockSize = 1024;
		allocatorInfo.initialBlocks = 1;
		Allocator * allocator = nullptr;
		void * place = nullptr;
		void * hint = nullptr;
		const std::size_t badAlignment = allocatorInfo.alignment * 2;
		const std::size_t requestedSize = allocatorInfo.blockSize + allocatorInfo.alignment;
		UNIT_TEST_WITH_MSG( u, ( allocator = AllocatorManager::CreateAllocator( allocatorInfo ) ) != nullptr, "allocator should not be nullptr." );
//		std::cout << __FUNCTION__ << " : " << __LINE__ << "  Allocator Type: " << typeid( *allocator ).name() << std::endl;
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize + 1 ) ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.objectSize, badAlignment ) ), std::invalid_argument, "Should throw since requested alignment is larger than block's alignment." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( requestedSize, hint ) ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, ( place = allocator->Allocate( allocatorInfo.blockSize, badAlignment, hint ) ), std::invalid_argument, "Should throw since requested alignment is larger than block alignment." );
		UNIT_TEST( u, ( place == nullptr ) );
		UNIT_TEST( u, ( place = allocator->Allocate( allocatorInfo.objectSize ) ) );
		UNIT_TEST( u, ( place != nullptr ) );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize + 1 ), std::invalid_argument, "Should throw since provided size is larger than block's object size." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize, badAlignment ), std::invalid_argument, "Should throw since requested alignment is larger than block's alignment." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize + allocatorInfo.alignment, allocatorInfo.alignment ), std::invalid_argument, "Should throw since requested size is larger than block's object size." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize - allocatorInfo.alignment, allocatorInfo.alignment ), std::invalid_argument, "Should throw since requested size is smaller than block's object size." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.blockSize ), std::logic_error, "Should throw since tiny allocator does not allow resize." );
		UNIT_TEST_FOR_EXCEPTION_WITH_MSG( u, allocator->Resize( place, allocatorInfo.objectSize, allocatorInfo.blockSize, allocatorInfo.alignment ), std::logic_error, "Should throw since tiny allocator does not allow resize." );
		UNIT_TEST( u, allocator->Release( place, allocatorInfo.objectSize ) );
		UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyAllocator( allocator, true ), "DestroyAllocator should pass since parameter is valid." );
	}

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
}

// ----------------------------------------------------------------------------

void TestAlignment( bool multithreaded )
{
	// These tests check if address returned by allocator is on an alignment boundary.
	// It means the address returned by an allocator is a multiple of alignment.

	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	ut::UnitTest * u = uts.AddUnitTest( "Test Alignment" );

	AllocatorManager::AllocatorParameters allocatorInfo;
	UNIT_TEST_WITH_MSG( u,  AllocatorManager::CreateManager( multithreaded, 4096 ), "Creation should pass since AllocatorManager does exist." );

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
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
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
		UNIT_TEST_WITH_MSG( u, allocator->Release( place, allocatorInfo.objectSize ), "Allocator should release memory." );
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

	UNIT_TEST_WITH_MSG( u, AllocatorManager::DestroyManager( true ), "Destruction should pass since AllocatorManager exists." );
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

	const bool showProximityCounts = true;
	const bool deleteAtExitTime = args.DeleteAtExitTime();
	const ut::UnitTestSet::OutputOptions options = args.GetOutputOptions();
	const ut::UnitTestSet::ErrorState status = ut::UnitTestSet::Create(
		"Memwa Functionality Tests", args.GetTextFileName(), args.GetHtmlFileName(),
		args.GetXmlFileName(), options, deleteAtExitTime );
	ut::UnitTestSet & uts = ut::UnitTestSet::GetIt();
	if ( ( status != ut::UnitTestSet::Success )
	  && ( status != ut::UnitTestSet::AlreadyExists ) )
	{
		cout << "An error occurred when creating the UnitTestSet singleton!"
			 << endl;
		return 2;
	}

	std::srand( std::time( 0 ) );

	if ( args.RunBlockTests() )
	{
		TestLinearBlock();
		TestStackBlock();
		TestStackBlockResize();
		TestStackBlockComplex();
		TestStackExceptions();
		TestPoolBlock();
		TestTinyBlock();
	}

	if ( args.RunManagerTests() )
	{
		TestAllocatorManager();
	}
	if ( args.RunExceptionTests() )
	{
		TestManagerExceptions( false );
		TestAllocatorExceptions( false );
		TestManagerExceptions( true );
		TestAllocatorExceptions( true );
	}
	if ( args.RunAlignmentTests() )
	{
		TestAlignment( false );
		TestAlignment( true );
	}

	if ( args.RunSimpleTests() )
	{
		TestLinearAllocator( false, showProximityCounts );
		TestStackAllocator( false, showProximityCounts );
		TestTinyAllocator( false, showProximityCounts );
		TestPoolAllocator( false, showProximityCounts );

		TestLinearAllocator( true, showProximityCounts );
		TestStackAllocator( true, showProximityCounts );
		TestTinyAllocator( true, showProximityCounts );
		TestPoolAllocator( true, showProximityCounts );
	}

	if ( args.RunComplexTests() )
	{
		ComplexTestStackAllocator( false, showProximityCounts );
		ComplexTestTinyAllocator( false, showProximityCounts );
		ComplexTestPoolAllocator( false, showProximityCounts );

		ComplexTestStackAllocator( true, showProximityCounts );
		ComplexTestTinyAllocator( true, showProximityCounts );
		ComplexTestPoolAllocator( true, showProximityCounts );
	}

	if ( args.runThreadTests() )
	{
		DoSimplePoolThreadTest( showProximityCounts );
		DoSimpleTinyThreadTest( showProximityCounts );
		DoSimpleStackThreadTest( showProximityCounts );
		DoSimpleLinearThreadTest( showProximityCounts );

		DoDuplicateLinearThreadTest( showProximityCounts );
		DoDuplicateStackThreadTest( showProximityCounts );
		DoDuplicateTinyThreadTest( showProximityCounts );
		DoDuplicatePoolThreadTest( showProximityCounts );

		DoComplexTinyThreadTest( showProximityCounts );
		DoComplexPoolThreadTest( showProximityCounts );
		DoComplexLinearThreadTest( showProximityCounts );
		DoComplexStackThreadTest( showProximityCounts );
	}

	if ( args.DoMakeTableAtExitTime() )
	{
		uts.OutputSummary();
	}
	return 0;
}

// ----------------------------------------------------------------------------
