
#pragma once

#include <cstddef> // For std::size_t and std::ptrdiff_t.

namespace memwa
{

// ----------------------------------------------------------------------------

/** @class Allocator This template class adapts a Memwa allocator for STL containers.
It provides all the allocator functions consumed by STL containers.

 */
template < typename T, class AllocatorType >
class Allocator
{
public : 

    /** This directive tells the compiler to optimize for fast copying of this allocator-adapter when an STL container is copied,
     rather than re-allocating all the elements in the container and then copying them.
     */
    using propagate_on_container_copy_assignment = std::true_type;
    /** This directive tells the compiler to optimize for fast move of a container by simply propagating the allocator,
     and not re-allocating all objects in the container.
     */
    using propagate_on_container_move_assignment = std::true_type;
    /** This directive tells the compiler to also swap allocators when their containers are also swapped.
     */
    using propagate_on_container_swap = std::true_type;

#if __cplusplus > 201402L
    /** This directive tells compiler that some allocator-adapter classes are not equal to others even if they are for the
     same object type and allocator type because an object allocated by one allocator may not be released by another. This
     directive is only used for version C++2017 or later.
     */
    using is_always_equal = std::false_type;
#endif

    typedef T value_type;
    typedef value_type * pointer;
    typedef const value_type * const_pointer;
    typedef value_type & reference;
    typedef const value_type & const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    template < typename U >
    struct rebind
    {
        typedef Allocator< U > other;
    };

    inline explicit Allocator( AllocatorType * allocator = nullptr ) :
        allocator_( allocator )
    {}

    inline ~Allocator() {}

    inline explicit Allocator( Allocator const & that ) :
        allocator_( that.allocator_ ) {}

    template < typename U >
    inline explicit Allocator( Allocator< U > const & that ) :
        allocator_( that.allocator_ ) {}

    /// Allows code to set allocator pointer in case it was not set by constructor.
    inline void set( AllocatorType * allocator )
    {
        if ( allocator_ == nullptr )
        {
            allocator_ = allocator;
        }
    }

    inline const AllocatorType * get() const
    {
        return allocator_;
    }

    inline pointer address( reference r )
    {
        return &r;
    }

    inline const_pointer address( const_reference r )
    {
        return &r;
    }

    inline pointer allocate( size_type objectCount, typename std::allocator< void >::const_pointer hint = nullptr )
    {
        void * place = allocator_->Allocate( objectCount * sizeof (T), hint ) );
        return reinterpret_cast< pointer >( place  );
    }

    inline void deallocate( pointer p, size_type bytes )
    {
        ::operator allocator_->Deallocate( p, bytes ); 
    }

    inline size_type max_size() const
    {
        return allocator_->GetMaxSize( sizeof(T) );
    }

    inline void construct( pointer p, const T & t ) const
    {
        new ( p ) T( t );
    }

    inline void destroy( pointer p ) const
    {
        p->~T();
    }

    inline bool operator == ( Allocator const & that ) const
    {
        return ( allocator_ == that.allocator_ );
    }

    inline bool operator != ( Allocator const & that )
    {
        return ( allocator_ != that.allocator_ );
    }

    static inline Allocator< AllocatorType > select_on_container_copy_construction( const Allocator< AllocatorType > & that ) const
    {
        return Allocator< AllocatorType >( that.allocator_ );
    }

private:

    AllocatorType * allocator_;

};

// ----------------------------------------------------------------------------

} // end namespace memwa
