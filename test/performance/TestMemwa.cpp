
#define TEST_MEMWA

#include "Thingy.hpp"

#include "TestFunctions.hpp"

typedef Thingy<   8 >   Thingy8;
typedef Thingy<  64 >  Thingy64;
typedef Thingy< 256 > Thingy256;

// ----------------------------------------------------------------------------

template< class Object >
bool TestMemwaPoolForward( StopwatchPair & timers )
{

	const bool created = memwa::AllocatorManager::CreateManager( false, 4096 );
	if ( !created )
	{
		return false;
	}

	memwa::AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = memwa::AllocatorManager::AllocatorType::Pool;
	allocatorInfo.objectSize = sizeof( Object );
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2000 * sizeof( Object );
	allocatorInfo.initialBlocks = ( GetLoopCount() / allocatorInfo.blockSize ) + 1;
	memwa::Allocator * allocator = memwa::AllocatorManager::CreateAllocator( allocatorInfo );
	MemwaAllocator::SetAllocator( allocator );

	TestSameOrder< Object >( timers );

	MemwaAllocator::RemoveAllocator();
	memwa::AllocatorManager::DestroyAllocator( allocator, true );
	memwa::AllocatorManager::DestroyManager( true );

	return true;
}

// ----------------------------------------------------------------------------

template< class Object >
bool TestMemwaTinyForward( StopwatchPair & timers )
{

	const bool created = memwa::AllocatorManager::CreateManager( false, 4096 );
	if ( !created )
	{
		return false;
	}

	memwa::AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = memwa::AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = sizeof( Object );
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 255 * sizeof( Object );
	allocatorInfo.initialBlocks = ( GetLoopCount() / allocatorInfo.blockSize ) + 1;
	memwa::Allocator * allocator = memwa::AllocatorManager::CreateAllocator( allocatorInfo );
	MemwaAllocator::SetAllocator( allocator );

	TestSameOrder< Object >( timers );

	MemwaAllocator::RemoveAllocator();
	memwa::AllocatorManager::DestroyAllocator( allocator, true );
	memwa::AllocatorManager::DestroyManager( true );

	return true;
}

// ----------------------------------------------------------------------------

template< class Object >
bool TestMemwaPoolReverse( StopwatchPair & timers )
{

	const bool created = memwa::AllocatorManager::CreateManager( false, 4096 );
	if ( !created )
	{
		return false;
	}

	memwa::AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = memwa::AllocatorManager::AllocatorType::Pool;
	allocatorInfo.objectSize = sizeof( Object );
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2000 * sizeof( Object );
	allocatorInfo.initialBlocks = ( GetLoopCount() / allocatorInfo.blockSize ) + 1;
	memwa::Allocator * allocator = memwa::AllocatorManager::CreateAllocator( allocatorInfo );
	MemwaAllocator::SetAllocator( allocator );

	TestReverseOrder< Object >( timers );

	MemwaAllocator::RemoveAllocator();
	memwa::AllocatorManager::DestroyAllocator( allocator, true );
	memwa::AllocatorManager::DestroyManager( true );

	return true;
}

// ----------------------------------------------------------------------------

template< class Object >
bool TestMemwaPoolRandom( StopwatchPair & timers )
{

	const bool created = memwa::AllocatorManager::CreateManager( false, 4096 );
	if ( !created )
	{
		return false;
	}

	memwa::AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = memwa::AllocatorManager::AllocatorType::Pool;
	allocatorInfo.objectSize = sizeof( Object );
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2000 * sizeof( Object );
	allocatorInfo.initialBlocks = ( GetLoopCount() / allocatorInfo.blockSize ) + 1;
	memwa::Allocator * allocator = memwa::AllocatorManager::CreateAllocator( allocatorInfo );
	MemwaAllocator::SetAllocator( allocator );

	TestRandomOrder< Object >( timers );

	MemwaAllocator::RemoveAllocator();
	memwa::AllocatorManager::DestroyAllocator( allocator, true );
	memwa::AllocatorManager::DestroyManager( true );

	return true;
}

// ----------------------------------------------------------------------------

template< class Object >
bool TestMemwaTinyReverse( StopwatchPair & timers )
{

	const bool created = memwa::AllocatorManager::CreateManager( false, 4096 );
	if ( !created )
	{
		return false;
	}

	memwa::AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = memwa::AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = sizeof( Object );
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 255 * sizeof( Object );
	allocatorInfo.initialBlocks = ( GetLoopCount() / allocatorInfo.blockSize ) + 1;
	memwa::Allocator * allocator = memwa::AllocatorManager::CreateAllocator( allocatorInfo );
	MemwaAllocator::SetAllocator( allocator );

	TestReverseOrder< Object >( timers );

	MemwaAllocator::RemoveAllocator();
	memwa::AllocatorManager::DestroyAllocator( allocator, true );
	memwa::AllocatorManager::DestroyManager( true );

	return true;
}

// ----------------------------------------------------------------------------

template< class Object >
bool TestMemwaTinyRandom( StopwatchPair & timers )
{

	const bool created = memwa::AllocatorManager::CreateManager( false, 4096 );
	if ( !created )
	{
		return false;
	}

	memwa::AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = memwa::AllocatorManager::AllocatorType::Tiny;
	allocatorInfo.objectSize = sizeof( Object );
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2000 * sizeof( Object );
	allocatorInfo.initialBlocks = ( GetLoopCount() / allocatorInfo.blockSize ) + 1;
	memwa::Allocator * allocator = memwa::AllocatorManager::CreateAllocator( allocatorInfo );
	MemwaAllocator::SetAllocator( allocator );

	TestRandomOrder< Object >( timers );

	MemwaAllocator::RemoveAllocator();
	memwa::AllocatorManager::DestroyAllocator( allocator, true );
	memwa::AllocatorManager::DestroyManager( true );

	return true;
}

// ---------------------------------------------------------------------------

template< class Object >
bool TestMemwaStackReverse( StopwatchPair & timers )
{

	const bool created = memwa::AllocatorManager::CreateManager( false, 4096 );
	if ( !created )
	{
		return false;
	}

	memwa::AllocatorManager::AllocatorParameters allocatorInfo;
	allocatorInfo.type = memwa::AllocatorManager::AllocatorType::Stack;
	allocatorInfo.objectSize = sizeof( Object );
	allocatorInfo.alignment = 8;
	allocatorInfo.blockSize = 2000 * sizeof( Object );
	allocatorInfo.initialBlocks = ( GetLoopCount() / allocatorInfo.blockSize ) + 1;
	memwa::Allocator * allocator = memwa::AllocatorManager::CreateAllocator( allocatorInfo );
	MemwaAllocator::SetAllocator( allocator );

	TestReverseOrder< Object >( timers );

	MemwaAllocator::RemoveAllocator();
	memwa::AllocatorManager::DestroyAllocator( allocator, true );
	memwa::AllocatorManager::DestroyManager( true );

	return true;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolForward8( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolForward< Thingy8 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolForward64( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolForward< Thingy64 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolForward256( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolForward< Thingy256 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolReverse8( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolReverse< Thingy8 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolReverse64( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolReverse< Thingy64 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolReverse256( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolReverse< Thingy256 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolRandom8( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolRandom< Thingy8 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolRandom64( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolRandom< Thingy64 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaPoolRandom256( StopwatchPair & timers )
{
	const bool okay = TestMemwaPoolRandom< Thingy256 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaTinyForward8( StopwatchPair & timers )
{
	const bool okay = TestMemwaTinyForward< Thingy8 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaTinyForward64( StopwatchPair & timers )
{
	const bool okay = TestMemwaTinyForward< Thingy64 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaTinyReverse8( StopwatchPair & timers )
{
	const bool okay = TestMemwaTinyReverse< Thingy8 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaTinyReverse64( StopwatchPair & timers )
{
	const bool okay = TestMemwaTinyReverse< Thingy64 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaTinyRandom8( StopwatchPair & timers )
{
	const bool okay = TestMemwaTinyRandom< Thingy8 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaTinyRandom64( StopwatchPair & timers )
{
	const bool okay = TestMemwaTinyRandom< Thingy64 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaStackReverse8( StopwatchPair & timers )
{
	const bool okay = TestMemwaStackReverse< Thingy8 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaStackReverse64( StopwatchPair & timers )
{
	const bool okay = TestMemwaStackReverse< Thingy64 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------

bool TestMemwaStackReverse256( StopwatchPair & timers )
{
	const bool okay = TestMemwaStackReverse< Thingy256 >( timers );
	return okay;
}

// ----------------------------------------------------------------------------
