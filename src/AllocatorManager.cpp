
#include "AllocatorManager.hpp"

#include "ManagerImpl.hpp"
#include "LinearAllocator.hpp"
#include "StackAllocator.hpp"
#include "PoolAllocator.hpp"
#include "TinyObjectAllocator.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace memwa
{
namespace impl
{

ManagerImpl * ManagerImpl::impl_ = nullptr;

// ----------------------------------------------------------------------------

#if defined(unix) || defined(__unix__) || defined(__unix)

#include <unistd.h>

unsigned long long GetTotalAvailableMemory()
{
	const long pageCount = sysconf( _SC_PHYS_PAGES );
	const long pageSize = sysconf( _SC_PAGE_SIZE );
	const unsigned long long totalBytes = pageCount * pageSize;
	return totalBytes;
}

#endif

// ----------------------------------------------------------------------------

#if defined(_WIN64) || defined(_WIN64)

#include <windows.h>

unsigned long long GetTotalAvailableMemory()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof( status );
	GlobalMemoryStatusEx( &status );
	return status.ullAvailVirtual;
}

#endif

// ----------------------------------------------------------------------------

std::size_t CalculateAlignmentPadding( std::size_t requestedAlignment )
{
	const std::size_t maxAlignment = GetMaxSupportedAlignment();
	if ( requestedAlignment < maxAlignment )
	{
		return 0;
	}
	const std::size_t padding = requestedAlignment - maxAlignment;
	return padding;
}

// ----------------------------------------------------------------------------

std::size_t CalculateAlignedSize( std::size_t bytes, std::size_t alignment )
{
	std::size_t multiplier = std::max( bytes / alignment, std::size_t(1) );
	if ( multiplier * alignment < bytes )
	{
		multiplier++;
	}
	const std::size_t bytesNeeded = ( multiplier * alignment );
	return bytesNeeded;
}

// ----------------------------------------------------------------------------

bool GetConsistentAlignment( std::size_t alignment )
{
    const unsigned int chunkCount = 16;
    void * places[ chunkCount ];
    memset( places, 0, sizeof(places) );
    bool consistent = true;

    for ( unsigned int ii = 0;  ii < chunkCount; ++ii )
    {
        const std::size_t size = 1024 + rand() % 4096;
        void * chunk = malloc( size );
        places[ ii ] = chunk;
    }

    for ( unsigned int ii = 0;  ii < chunkCount; ++ii )
    {
        void * chunk = places[ ii ];
        const std::size_t place = reinterpret_cast< const std::size_t >( chunk );
        if ( place % alignment != 0 )
        {
            consistent = false;
        }
        free( chunk );
    }

    return consistent;
}

// ----------------------------------------------------------------------------

std::size_t GetMaxSupportedAlignment()
{
    static std::size_t maxAlignment = 0;

    if ( maxAlignment == 0 )
    {
        std::srand( std::time( 0 ) );
        std::size_t alignment = 64;
        while ( alignment > 1 )
        {
            const bool isConsistentAlignment = GetConsistentAlignment( alignment );
            if ( isConsistentAlignment )
            {
                break;
            }
            alignment /= 2;
        }
        maxAlignment = alignment;
    }

    return maxAlignment;
}

// ----------------------------------------------------------------------------

bool ManagerImpl::CreateManager( bool multithreaded, std::size_t internalBlockSize )
{
	if ( impl_ != nullptr )
	{
		return false;
	}
	impl_ = new ManagerImpl( multithreaded, internalBlockSize );
	const bool success = ( impl_ != nullptr );
	return success;
}

// ----------------------------------------------------------------------------

ManagerImpl::ManagerImpl( bool multithreaded, std::size_t internalBlockSize ) :
	multithreaded_( multithreaded ),
	mutex_(),
	allocators_(),
	oldHandler_( std::set_new_handler( &NewHandler ) ),
	blockSize_( internalBlockSize ),
	alignment_( defaultAlignment ),
	common_( blockSize_, alignment_ )
{
	allocators_.reserve( 4 );
}

// ----------------------------------------------------------------------------

ManagerImpl::~ManagerImpl()
{
	std::set_new_handler( oldHandler_ );
}

// ----------------------------------------------------------------------------

void * ManagerImpl::Allocate( std::size_t bytes )
{
	LockGuard guard( mutex_, multithreaded_ );
	void * p = common_.Allocate( bytes, blockSize_, alignment_ );
	assert( p != nullptr );
	return p;
}

// ----------------------------------------------------------------------------

bool ManagerImpl::AddAllocator( Allocator * allocator )
{
	LockGuard guard( mutex_, multithreaded_ );

	const AllocatorsIter end( allocators_.end() );
	AllocatorsIter here( end );
	for ( AllocatorsIter it( allocators_.begin() ); it != end; ++it )
	{
		Allocator * a = *it;
		if ( a == allocator )
		{
			return false;
		}
		if ( a == nullptr )
		{
			*it = allocator;
			return true;
		}
	}

	allocators_.push_back( allocator );
	return true;
}

// ----------------------------------------------------------------------------

bool ManagerImpl::RemoveAllocator( Allocator * allocator )
{
	LockGuard guard( mutex_, multithreaded_ );

	const AllocatorsIter end( allocators_.end() );
	for ( AllocatorsIter it( allocators_.begin() ); it != end; ++it )
	{
		Allocator * a = *it;
		if ( a == allocator )
		{
			*it = nullptr;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------------

bool ManagerImpl::TrimEmptyBlocks( Allocator * allocator )
{
	LockGuard guard( mutex_, multithreaded_ );
	static bool entered = false;
	if ( entered )
	{
		// Don't let the current thread enter this function if it is already on the stack.
		return false;
	}
	ReentryGuard entryGuard( entered );

	const AllocatorsIter newEnd( std::remove( allocators_.begin(), allocators_.end(), nullptr ) );
	allocators_.erase( newEnd, allocators_.end() );
	allocators_.shrink_to_fit();

	bool anyTrimmed = false;
	const AllocatorsIter end( allocators_.end() );
	for ( AllocatorsIter it( allocators_.begin() ); it != end; ++it )
	{
		Allocator * a = *it;
		if ( ( a == allocator ) || ( a == nullptr ) )
		{
			continue;
		}
		assert( a != nullptr );
		if ( a->TrimEmptyBlocks() )
		{
			anyTrimmed = true;
		}
	}
	if ( anyTrimmed )
	{
		return true;
	}

	if ( oldHandler_ != nullptr )
	{
		// The default new_handler may terminate the program by calling the abort or exit function.
		// Because it might end the program, calling it is a last-ditch effort that should only be
		// done if no other allocators could free up any available memory.
		try
		{
			( oldHandler_ )();
			// If this line is executed, it means the previous new_handler did not exit the program
			// and did not throw an exception, so assume it released some memory.
			anyTrimmed = true;
		}
		catch ( ... )
		{
			// The default new_handler might throws std::bad_alloc. Just catch the
			// exception and continue. Assume it did not release any memory.
		}
	}

	return anyTrimmed;
}

// ----------------------------------------------------------------------------

void ManagerImpl::NewHandler()
{
	assert( nullptr != impl_ );
	impl_->TrimEmptyBlocks();
}

// ----------------------------------------------------------------------------

void CheckInitializationParameters( const AllocatorManager::AllocatorParameters & info )
{
	if ( ( info.alignment == 0 ) || ( AllocatorManager::MaxAlignment < info.alignment ) )
	{
		throw std::invalid_argument( "Memory alignment must be greater than zero and not greater than 32." );
	}
	if ( ( info.alignment & ( info.alignment - 1 ) ) != 0 )
	{
		throw std::invalid_argument( "Memory alignment must be a power of 2." );
	}
	if ( info.initialBlocks == 0 )
	{
		throw std::invalid_argument( "Number of memory blocks may not be zero!" );
	}
	if ( info.type != AllocatorManager::AllocatorType::Tiny )
	{
		if ( info.blockSize <= info.alignment )
		{
			throw std::invalid_argument( "Memory block size should be bigger than alignment." );
		}
		if ( info.blockSize < 255 )
		{
			throw std::invalid_argument( "Memory block size should be bigger than 255 bytes." );
		}
		if ( ( info.alignment != 1 ) && ( info.blockSize % info.alignment != 0 ) )
		{
			throw std::invalid_argument( "Memory block size must be a multiple of alignment." );
		}
	}
}

// ----------------------------------------------------------------------------

} // end anonymous namespace

// ----------------------------------------------------------------------------

Allocator::Allocator()
{
	memwa::impl::ManagerImpl::GetManager()->AddAllocator( this );
}

// ----------------------------------------------------------------------------

Allocator::~Allocator()
{
	memwa::impl::ManagerImpl::GetManager()->RemoveAllocator( this );
}

// ----------------------------------------------------------------------------

bool Allocator::TrimEmptyBlocks()
{
	const bool success = memwa::impl::ManagerImpl::GetManager()->TrimEmptyBlocks( this );
	return success;
}

// ----------------------------------------------------------------------------

bool Allocator::Release( void * place, std::size_t size )
{
	return false;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool Allocator::Release( void * place, std::size_t size, std::align_val_t alignment )
#else
bool Allocator::Release( void * place, std::size_t size, std::size_t alignment )
#endif
{
	return false;
}

// ----------------------------------------------------------------------------

#if __cplusplus > 201402L
bool Allocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment )
#else
bool Allocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment )
#endif
{
	throw std::logic_error( "This allocator does not support resize operations." );
}

// ----------------------------------------------------------------------------

bool Allocator::Resize( void * place, std::size_t oldSize, std::size_t newSize )
{
	throw std::logic_error( "This allocator does not support resize operations." );
}

// ----------------------------------------------------------------------------

void Allocator::Destroy()
{
}

// ----------------------------------------------------------------------------

bool AllocatorManager::CreateManager( bool multithreaded, std::size_t initalBlockSize )
{
	const bool success = memwa::impl::ManagerImpl::CreateManager( multithreaded, initalBlockSize );
	return success;
}

// ----------------------------------------------------------------------------

Allocator * AllocatorManager::CreateAllocator( const AllocatorParameters & info )
{
	memwa::impl::ManagerImpl * impl = memwa::impl::ManagerImpl::GetManager();
	if ( nullptr == impl )
	{
		throw std::logic_error( "Error! The AllocatorManager::CreateManager must be called before AllocatorManager::CreateAllocator." );
	}

	memwa::impl::CheckInitializationParameters( info );
	const std::size_t alignedSize = memwa::impl::CalculateAlignedSize( info.objectSize, info.alignment );

	Allocator * allocator = nullptr;
	if ( impl->IsMultithreaded() )
	{
		switch ( info.type )
		{
			case AllocatorType::Stack :
			{
				void * place = impl->Allocate( sizeof(ThreadSafeStackAllocator) + sizeof(void *) );
				allocator = new ( place ) ThreadSafeStackAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Pool :
			{
				if ( info.blockSize % alignedSize != 0 )
				{
					throw std::invalid_argument( "Blocksize should be an exact multiple of objectSize for ThreadSafePoolAllocator." );
				}
				if ( alignedSize < sizeof(void *) )
				{
					throw std::invalid_argument(
						"ThreadSafePoolAllocator should not be used to allocate object sizes smaller than a pointer. Use ThreadSafeTinyObjectAllocator instead." );
				}
				if ( info.alignment < 4 )
				{
					throw std::invalid_argument(
						"ThreadSafePoolAllocator should not be used alignment smaller than 4 bytes. Use ThreadSafeTinyObjectAllocator instead." );
				}
				void * place = impl->Allocate( sizeof(ThreadSafePoolAllocator) + sizeof(void *) );
				allocator = new ( place ) ThreadSafePoolAllocator( info.initialBlocks, info.blockSize, alignedSize, info.alignment );
				break;
			}
			case AllocatorType::Linear :
			{
				void * place = impl->Allocate( sizeof(ThreadSafeLinearAllocator) + sizeof(void *) );
				allocator = new ( place ) ThreadSafeLinearAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Tiny :
			{
				void * place = impl->Allocate( sizeof(ThreadSafeTinyObjectAllocator) + sizeof(void *) );
				allocator = new ( place ) ThreadSafeTinyObjectAllocator( info.initialBlocks, alignedSize, info.alignment );
				break;
			}
			default:
			{
				throw std::invalid_argument( "Unrecognized allocator type." );
			}
		}
	}
	else
	{
		switch ( info.type )
		{
			case AllocatorType::Stack :
			{
				void * place = impl->Allocate( sizeof(StackAllocator) + sizeof(void *) );
				allocator = new ( place ) StackAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Pool :
			{
				if ( info.blockSize % alignedSize != 0 )
				{
					throw std::invalid_argument( "Blocksize should be an exact multiple of objectSize for ThreadSafePoolAllocator." );
				}
				if ( alignedSize < sizeof(void *) )
				{
					throw std::invalid_argument(
						"PoolAllocator should not be used to allocate object sizes smaller than a pointer. Use TinyObjectAllocator instead." );
				}
				if ( info.alignment < 4 )
				{
					throw std::invalid_argument(
						"PoolAllocator should not be used alignment smaller than 4 bytes. Use TinyObjectAllocator instead." );
				}
				void * place = impl->Allocate( sizeof(PoolAllocator) + sizeof(void *) );
				allocator = new ( place ) PoolAllocator( info.initialBlocks, info.blockSize, alignedSize, info.alignment );
				break;
			}
			case AllocatorType::Linear :
			{
				void * place = impl->Allocate( sizeof(LinearAllocator) + sizeof(void *) );
				allocator = new ( place ) LinearAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Tiny :
			{
//				std::cout << __FUNCTION__ << " : " << __LINE__ << "  alignedSize: " << alignedSize << "  alignment:" << info.alignment << std::endl;
				void * place = impl->Allocate( sizeof(TinyObjectAllocator) + sizeof(void *) );
//				std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
				allocator = new ( place ) TinyObjectAllocator( info.initialBlocks, alignedSize, info.alignment );
//				std::cout << __FUNCTION__ << " : " << __LINE__ << std::endl;
				break;
			}
			default:
			{
				throw std::invalid_argument( "Unrecognized allocator type." );
			}
		}
	}

	assert( allocator != nullptr );
	return allocator;
}

// ----------------------------------------------------------------------------

bool AllocatorManager::DestroyAllocator( Allocator * allocator, bool releaseMemory )
{
	memwa::impl::ManagerImpl * impl = memwa::impl::ManagerImpl::GetManager();
	if ( nullptr == impl )
	{
		throw std::logic_error( "Error! The AllocatorManager::CreateManager must be called before AllocatorManager::DestroyAllocator." );
	}
	if ( allocator == nullptr )
	{
		return false;
	}
	if ( releaseMemory )
	{
		allocator->Destroy();
	}
	allocator->~Allocator();
	/// @todo Eventually this function will call impl to release the memory used by allocator.

	return true;
}

// ----------------------------------------------------------------------------

bool AllocatorManager::TrimEmptyBlocks()
{
	memwa::impl::ManagerImpl * impl = memwa::impl::ManagerImpl::GetManager();
	if ( nullptr == impl )
	{
		throw std::logic_error( "Error! The AllocatorManager::CreateManager must be called before AllocatorManager::TrimEmptyBlocks." );
	}
	const bool success = impl->TrimEmptyBlocks();
	return success;
}

// ----------------------------------------------------------------------------

std::size_t AllocatorManager::GetMaxSupportedAlignment()
{
	return memwa::impl::GetMaxSupportedAlignment();
}

// ----------------------------------------------------------------------------

} // end project namespace
