
#pragma once

#include "AllocatorManager.hpp"
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

void CheckInitializationParameters( const AllocatorManager::AllocatorParameters & info );

std::size_t CalculateAlignedSize( std::size_t bytes, std::size_t alignment );

/// Calculates amount of padding needed if requestedAlignment > maximum alignment.
std::size_t CalculateAlignmentPadding( std::size_t requestedAlignment );

/// Provides maximum alignment supported by operating system.
std::size_t GetMaxSupportedAlignment();

/// Returns the number of bytes the operating system will allow for allocation.
unsigned long long GetTotalAvailableMemory();

// ----------------------------------------------------------------------------

class ManagerImpl
{
public:

	static const std::size_t defaultAlignment = 4;

	static ManagerImpl * GetManager()
	{
		return impl_;
	}

	static bool CreateManager( bool multithreaded, std::size_t internalBlockSize );

	static bool DestroyManager( bool releaseAll );

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

	static void NewHandler();

	ManagerImpl( bool multithreaded, std::size_t internalBlockSize );

	~ManagerImpl();

	void ReleaseAllocators();

	static ManagerImpl * impl_;

	bool multithreaded_;

	std::mutex mutex_;

	Allocators allocators_;

	std::new_handler oldHandler_;

	/// Size of entire memory page.
	std::size_t blockSize_;
	/// Byte alignment of allocations. (e.g. - 1, 2, 4, 8, 16, or 32.)
	std::size_t alignment_;

	LinearBlock common_;

};

// ----------------------------------------------------------------------------

} // end internal namespace

} // end project namespace
