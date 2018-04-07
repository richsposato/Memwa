
#pragma once

#include <AllocatorManager.hpp>

#include <stdexcept>

#include <cstddef> // For std::size_t.

// ----------------------------------------------------------------------------

class BoostPool
{
public:

    /// Throwing single-object new throws bad_alloc when allocation fails.
    static void * operator new ( std::size_t size )
#ifndef _MSC_VER
    /// @note MSVC complains about non-empty exception specification lists.
    	throw ( std::bad_alloc )
#endif
    	;

    /// Non-throwing single-object new returns NULL if allocation fails.
    static void * operator new ( std::size_t size, const std::nothrow_t & ) throw ();

    /// Placement single-object new merely calls global placement new.
    inline static void * operator new ( std::size_t size, void * place )
    {
        return ::operator new( size, place );
    }

    /// Single-object delete.
    static void operator delete ( void * p, std::size_t size ) throw ();

    /** Non-throwing single-object delete is only called when nothrow
     new operator is used, and the constructor throws an exception.
     */
    static void operator delete ( void * p, const std::nothrow_t & ) throw();

    /// Placement single-object delete merely calls global placement delete.
    inline static void operator delete ( void * p, void * place )
    {
        return ::operator delete( p, place );
    }

    /// Throwing array-object new throws bad_alloc when allocation fails.
    /// @note MSVC complains about non-empty exception specification lists.
    static void * operator new [] ( std::size_t size )
#ifndef _MSC_VER
        /// @note MSVC complains about non-empty exception specification lists.
        throw ( std::bad_alloc )
#endif
        ;

    /// Non-throwing array-object new returns NULL if allocation fails.
    static void * operator new [] ( std::size_t size, const std::nothrow_t & ) throw ();

    /// Placement array-object new merely calls global placement new.
    inline static void * operator new [] ( std::size_t size, void * place )
    {
        return ::operator new ( size, place );
    }

    /// Array-object delete.
    static void operator delete [] ( void * p, std::size_t size ) throw ();

    /** Non-throwing array-object delete is only called when nothrow
     new operator is used, and the constructor throws an exception.
     */
    static void operator delete [] ( void * p, const std::nothrow_t & ) throw();

    /// Placement array-object delete merely calls global placement delete.
    inline static void operator delete [] ( void * p, void * place )
    {
        ::operator delete ( p, place );
    }

};

// ----------------------------------------------------------------------------

class MemwaAllocator
{
public:

	static bool SetAllocator( memwa::Allocator * allocator );

	static void RemoveAllocator()
	{
	    allocator_ = nullptr;
	}

    /// Throwing single-object new throws bad_alloc when allocation fails.
    static void * operator new ( std::size_t size )
#ifndef _MSC_VER
    /// @note MSVC complains about non-empty exception specification lists.
    	throw ( std::bad_alloc )
#endif
    	;

    /// Non-throwing single-object new returns NULL if allocation fails.
    static void * operator new ( std::size_t size, const std::nothrow_t & ) throw ();

    /// Placement single-object new merely calls global placement new.
    inline static void * operator new ( std::size_t size, void * place )
    {
        return ::operator new( size, place );
    }

    /// Single-object delete.
    static void operator delete ( void * p, std::size_t size ) throw ();

    /** Non-throwing single-object delete is only called when nothrow
     new operator is used, and the constructor throws an exception.
     */
    static void operator delete ( void * p, std::size_t size, const std::nothrow_t & ) throw();

    /// Placement single-object delete merely calls global placement delete.
    inline static void operator delete ( void * p, void * place )
    {
        ::operator delete( p, place );
    }

    /// Throwing array-object new throws bad_alloc when allocation fails.
    /// @note MSVC complains about non-empty exception specification lists.
    static void * operator new [] ( std::size_t size )
#ifndef _MSC_VER
        /// @note MSVC complains about non-empty exception specification lists.
        throw ( std::bad_alloc )
#endif
        ;

    /// Non-throwing array-object new returns NULL if allocation fails.
    static void * operator new [] ( std::size_t size, const std::nothrow_t & ) throw ();

    /// Placement array-object new merely calls global placement new.
    inline static void * operator new [] ( std::size_t size, void * place )
    {
        return ::operator new ( size, place );
    }

    /// Array-object delete.
    static void operator delete [] ( void * p, std::size_t size ) throw ();

    /** Non-throwing array-object delete is only called when nothrow
     new operator is used, and the constructor throws an exception.
     */
    static void operator delete [] ( void * p, std::size_t size, const std::nothrow_t & ) throw();

    /// Placement array-object delete merely calls global placement delete.
    inline static void operator delete [] ( void * p, void * place )
    {
        ::operator delete ( p, place );
    }

private:

	static memwa::Allocator * allocator_;

};

// ----------------------------------------------------------------------------

template < std::size_t ArraySize >
class Thingy
#if defined( TEST_LOKI_SMALL_OBJECT )
	: public Loki::SmallValueObject
#elif defined( TEST_BOOST_POOL )
	: public BoostPool
#elif defined( TEST_MEMWA )
	: public MemwaAllocator
#endif
{
public:

	Thingy() {}

	Thingy( const Thingy & that ) {}

	Thingy( Thingy && that ) {}

	~Thingy() {}

	Thingy & operator = ( const Thingy & that ) { return *this; }

	Thingy & operator = ( Thingy && that ) { return *this; }

	Thingy & swap( Thingy & that ) { return *this; }

private:

	char array[ ArraySize ];

};

// ----------------------------------------------------------------------------
