
#include "AllocatorManager.hpp"

#include "ManagerImpl.hpp"
#include "LinearAllocator.hpp"
#include "StackAllocator.hpp"
#include "PoolAllocator.hpp"
#include "TinyObjectAllocator.hpp"

#include <cassert>

#include <algorithm>
#include <stdexcept>

namespace memwa
{
namespace impl
{

// ----------------------------------------------------------------------------

std::size_t GetIndex( std::size_t alignment )
{
	switch ( alignment )
	{
		case 1:  return 0;
		case 2:  return 1;
		case 4:  return 2;
		case 8:  return 3;
		case 16: return 4;
		default: assert( false );
	}
	return 5;
}

// ----------------------------------------------------------------------------

bool ManagerImpl::CreateManager( bool multithreaded, std::size_t internalBlockSize )
{
	if ( impl_ != nullptr )
	{
		return true;
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
	common_( internalBlockSize, defaultAlignment )
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
	void * p = common_.Allocate( bytes, AllocatorManager::CommonBlockSize, defaultAlignment );
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
			here = it;
		}
	}

	if ( here == end )
	{
		allocators_.push_back( allocator );
	}
	else
	{
		*here = allocator;
	}

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
		if ( a->TrimEmptyBlocks() )
		{
			anyTrimmed = true;
		}
	}

	if ( !anyTrimmed && ( oldHandler_ != nullptr ) )
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

void CheckInitializationParameters( unsigned int initialBlocks, std::size_t blockSize,
	std::size_t alignment )
{
	if ( blockSize == 0 )
	{
		throw std::invalid_argument( "Memory block size may not be zero!" );
	}
	if ( ( alignment == 0 ) || ( AllocatorManager::MaxAlignment < alignment ) )
	{
		throw std::invalid_argument( "Memory alignment must be greater than zero and not greater than 32." );
	}
	if ( ( alignment & ( alignment - 1 ) ) != 0 )
	{
		throw std::invalid_argument( "Memory alignment must be a power of 2." );
	}
	if ( ( alignment != 1 ) && ( blockSize % alignment != 0 ) )
	{
		throw std::invalid_argument( "Memory block size must be a multiple of alignment." );
	}
	if ( initialBlocks == 0 )
	{
		throw std::invalid_argument( "Number of memory blocks may not be zero!" );
	}
}

// ----------------------------------------------------------------------------

} // end anonymous namespace

// ----------------------------------------------------------------------------

std::size_t CalculateBytesNeeded( std::size_t bytes, std::size_t alignment )
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
std::size_t Allocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment )
#else
std::size_t Allocator::Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment )
#endif
{
	return 0;
}

// ----------------------------------------------------------------------------

std::size_t Allocator::Resize( void * place, std::size_t oldSize, std::size_t newSize )
{
	return 0;
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

	memwa::impl::CheckInitializationParameters( info.initialBlocks, info.blockSize, info.alignment );

	Allocator * allocator = nullptr;
	if ( impl->IsMultithreaded() )
	{
		switch ( info.type )
		{
			case AllocatorType::Stack :
			{
				void * place = impl->Allocate( sizeof(ThreadSafeStackAllocator) );
				allocator = new ( place ) ThreadSafeStackAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Pool :
			{
				void * place = impl->Allocate( sizeof(PoolAllocator) );
				allocator = new ( place ) ThreadSafePoolAllocator( info.initialBlocks, info.blockSize, info.objectSize, info.alignment );
				break;
			}
			case AllocatorType::Linear :
			{
				void * place = impl->Allocate( sizeof(ThreadSafeLinearAllocator) );
				allocator = new ( place ) ThreadSafeLinearAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Tiny :
			{
				void * place = impl->Allocate( sizeof(ThreadSafeTinyObjectAllocator) );
				allocator = new ( place ) ThreadSafeTinyObjectAllocator( info.initialBlocks, info.objectSize, info.alignment );
			}
			default:
				assert( false );
				break;
		}
	}
	else
	{
		switch ( info.type )
		{
			case AllocatorType::Stack :
			{
				void * place = impl->Allocate( sizeof(StackAllocator) );
				allocator = new ( place ) StackAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Pool :
			{
				void * place = impl->Allocate( sizeof(PoolAllocator) );
				allocator = new ( place ) PoolAllocator( info.initialBlocks, info.blockSize, info.objectSize, info.alignment );
				break;
			}
			case AllocatorType::Linear :
			{
				void * place = impl->Allocate( sizeof(LinearAllocator) );
				allocator = new ( place ) LinearAllocator( info.initialBlocks, info.blockSize, info.alignment );
				break;
			}
			case AllocatorType::Tiny :
			{
				void * place = impl->Allocate( sizeof(TinyObjectAllocator) );
				allocator = new ( place ) TinyObjectAllocator( info.initialBlocks, info.objectSize, info.alignment );
			}
			default:
				assert( false );
				break;
		}
	}
	return allocator;
}

// ----------------------------------------------------------------------------

bool AllocatorManager::DestroyAllocator( Allocator * allocator, bool releaseMemory )
{
	if ( allocator == nullptr )
	{
		return false;
	}
	memwa::impl::ManagerImpl * impl = memwa::impl::ManagerImpl::GetManager();
	if ( nullptr == impl )
	{
		throw std::logic_error( "Error! The AllocatorManager::CreateManager must be called before AllocatorManager::DestroyAllocator." );
	}
	if ( releaseMemory )
	{
		allocator->Destroy();
	}
	delete allocator;
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

} // end project namespace
