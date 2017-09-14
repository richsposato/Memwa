
#pragma once

#include "LockGuard.hpp"

#include "LinearBlock.hpp"

#include <new>
#include <vector>

// ----------------------------------------------------------------------------

namespace memwa
{

	class Allocator;

namespace impl
{

std::size_t GetIndex( std::size_t alignment );

void CheckInitializationParameters( unsigned int initialBlocks, std::size_t blockSize, std::size_t alignment );

/// Returns the number of bytes the operating system will allow for allocation.
unsigned long long GetTotalAvailableMemory();

// ----------------------------------------------------------------------------

class ManagerImpl
{
public:

	static const std::size_t defaultAlignment = 8;

	static ManagerImpl * GetManager()
	{
		return impl_;
	}

	static bool CreateManager( bool multithreaded, std::size_t internalBlockSize );

	void * Allocate( std::size_t bytes );

	bool AddAllocator( Allocator * allocator );

	bool RemoveAllocator( Allocator * allocator );

	/// Deletes any blocks that have zero allocations.
	bool TrimEmptyBlocks( Allocator * allocator = nullptr );

	bool IsMultithreaded() const
	{
		return multithreaded_;
	}

private:

	typedef std::vector< Allocator * > Allocators;
	typedef Allocators::iterator AllocatorsIter;

	ManagerImpl( bool multithreaded, std::size_t internalBlockSize );

	~ManagerImpl();

	static void NewHandler();

	static ManagerImpl * impl_;

	bool multithreaded_;

	std::mutex mutex_;

	Allocators allocators_;

	std::new_handler oldHandler_;

	LinearBlock common_;

};

// ----------------------------------------------------------------------------

} // end internal namespace

} // end project namespace
