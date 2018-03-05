
#pragma once

#include <cstddef> // For std::size_t.

namespace memwa
{

	class AllocatorManager;

	namespace impl
	{
		class ManagerImpl;
	}

/** @todo
 Things to do:
 - Add try-catch blocks around each piece of code that can throw.
 - Write test code.
 - Write an exception class that inherits from std::bad_alloc.
 - Write logger allocator.
 - Write class that conforms to std::allocator requirements.
 - Add sourceAllocator pointer to struct AllocatorParameters.
 - Allow ManagerImpl to maintain multiple common allocator blocks.
 - Create containers that use PoolBlock or TinyBlock for internal use.
 */


// ----------------------------------------------------------------------------

class Allocator
{
public:

	/** Allocates a chunk of memory from a block.
	 @param size Number of bytes to allocate.
	 @return Pointer to chunk of memory of at least size bytes, or nullptr if it could not allocate.
	 */
	virtual void * Allocate( std::size_t size, const void * hint = nullptr ) = 0;

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual void * Allocate( std::size_t size, std::align_val_t alignment, const void * hint = nullptr ) = 0;

#else

	virtual void * Allocate( std::size_t size, std::size_t alignment, const void * hint = nullptr ) = 0;

#endif

	/** Releases a chunk of memory.
	 @param place Address of chunk owned by this memory handler.
	 @param size Number of bytes in chunk.
	 */
	virtual bool Release( void * place, std::size_t size );

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual bool Release( void * place, std::size_t size, std::align_val_t alignment );

#else

	virtual bool Release( void * place, std::size_t size, std::size_t alignment );

#endif

#if __cplusplus > 201402L
	// This code is for C++ 2017.

	virtual bool Resize( void * place, std::size_t oldSize, std::size_t newSize, std::align_val_t alignment );

#else

	virtual bool Resize( void * place, std::size_t oldSize, std::size_t newSize, std::size_t alignment );

#endif

	/**
	 */
	virtual bool Resize( void * place, std::size_t oldSize, std::size_t newSize );

	/// Provides count of maximum number of objects this can allocate at once.
	virtual unsigned long long GetMaxSize( std::size_t objectSize ) const = 0;

	/// Returns true if a block of memory managed by this object owns the chunk at the place
	virtual bool HasAddress( void * place ) const = 0;

	/// Deletes any blocks that have zero allocations.
	virtual bool TrimEmptyBlocks() = 0;

	/// Returns true if any memory block was corrupted.
	virtual bool IsCorrupt() const = 0;

	virtual float GetFragmentationPercent() const = 0;

protected:

	Allocator();

	virtual ~Allocator();

	virtual void Destroy();

private:

	friend class AllocatorManager;
	friend class impl::ManagerImpl;

	Allocator( const Allocator & ) = delete;
	Allocator( Allocator && ) = delete;
	Allocator & operator = ( const Allocator & ) = delete;
	Allocator & operator = ( Allocator && ) = delete;

};

// ----------------------------------------------------------------------------

/** @class AllocatorManager Creates and manages all allocators derived from memwa::Allocator.
 It provides a std::new_handler that can be called to release any unused memory when a memwa::Allocator
 or other allocators run out of memory.

 @note This class is a monotype design pattern. All the functions are static, and it can't be instantiated.
 */
class AllocatorManager
{
public:

	const static std::size_t MaxAlignment = 32;
	static const std::size_t CommonBlockSize = 1024;

	enum AllocatorType
	{
		Linear,
		Stack,
		Pool,
		Tiny, ///< For allocating objects from 1 through 128 bytes.
	};

	struct AllocatorParameters
	{
		AllocatorType type;
		unsigned int initialBlocks;
		std::size_t blockSize;
		std::size_t objectSize;
#if __cplusplus > 201402L	// This code is for C++ 2017.
		std::align_val_t alignment;
#else
		std::size_t alignment;
#endif
	};

	static bool CreateManager( bool multithreaded, std::size_t internalBlockSize = CommonBlockSize );

	static bool DestroyManager( bool releaseAll );

	static Allocator * CreateAllocator( const AllocatorParameters & allocatorInfo );

	/** Destroys the allocator made by CreateAllocator function.
	 @param allocator Pointer to object derived from allocator interface class, must have been made by
	  CreateAllocator function.
	 @param releaseMemory True if this function will release all memory blocks managed by allocator,
	  or false if it will leave them unreleased. Behavior is undefined if you try to release thosee
	  memory blocks after the allocator is destroyed.
	 @return True for success, false if allocator pointer is null or the allocator was not made by
	  this manager.
	 */
	static bool DestroyAllocator( Allocator * allocator, bool releaseMemory );

	/** Deletes any blocks that have zero allocations. All allocators managed by this class will this
	 function to release any available memory, so there is no need to call this if a Memwa allocator
	 fails. You may call this when any other allocators are unable to obtain memory.

	@note Possibility of Never Return
	 As an option of last resort if it can't free any memory managed by Memwa, this will call the old
	 std::new_handler (if there is one) to free up any memory that can be released. Since a new_handler
	 may call std::terminate to end the program, it is possible that a call to this function will never
	 return. If the new_handler throws an exception, this will merely return false.
	@return True if any blocks were released, false if none were.
	 */
	static bool TrimEmptyBlocks();

	/// Provides maximum alignment supported by the operating system.
	static std::size_t GetMaxSupportedAlignment();

private:

	AllocatorManager() = delete;
	~AllocatorManager() = delete;
	AllocatorManager( const AllocatorManager & ) = delete;
	AllocatorManager( AllocatorManager && ) = delete;
	AllocatorManager & operator = ( const AllocatorManager & ) = delete;
	AllocatorManager & operator = ( AllocatorManager && ) = delete;

};

// ----------------------------------------------------------------------------

} // end project namespace
