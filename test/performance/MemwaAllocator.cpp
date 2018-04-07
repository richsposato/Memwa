
#include "Thingy.hpp"

#include <cassert>

memwa::Allocator * MemwaAllocator::allocator_ = nullptr;

// ----------------------------------------------------------------------------

bool MemwaAllocator::SetAllocator( memwa::Allocator * allocator )
{
    if ( allocator == nullptr )
    {
        return false;
    }
    if ( allocator_ != nullptr )
    {
        return false;
    }
    allocator_ = allocator;
    return true;
}

// ----------------------------------------------------------------------------

void * MemwaAllocator::operator new ( std::size_t size )
#ifndef _MSC_VER
    /// @note MSVC complains about non-empty exception specification lists.
    throw ( std::bad_alloc )
#endif
{
    void * place = allocator_->Allocate( size );
    return place;
}

// ----------------------------------------------------------------------------

void * MemwaAllocator::operator new ( std::size_t size, const std::nothrow_t & ) throw ()
{
    void * place = allocator_->Allocate( size );
    return place;
}

// ----------------------------------------------------------------------------

void MemwaAllocator::operator delete ( void * p, std::size_t size ) throw ()
{
    bool released = allocator_->Release( p, size );
    assert( released );
}

// ----------------------------------------------------------------------------

void MemwaAllocator::operator delete ( void * p, std::size_t size, const std::nothrow_t & ) throw()
{
    bool released = allocator_->Release( p, size );
    assert( released );
}

// ----------------------------------------------------------------------------

void * MemwaAllocator::operator new [] ( std::size_t size )
#ifndef _MSC_VER
    /// @note MSVC complains about non-empty exception specification lists.
    throw ( std::bad_alloc )
#endif
{
    void * place = allocator_->Allocate( size );
    return place;
}

// ----------------------------------------------------------------------------

void * MemwaAllocator::operator new [] ( std::size_t size, const std::nothrow_t & ) throw ()
{
    void * place = allocator_->Allocate( size );
    return place;
}

// ----------------------------------------------------------------------------

void MemwaAllocator::operator delete [] ( void * p, std::size_t size ) throw ()
{
    bool released = allocator_->Release( p, size );
    assert( released );
}

// ----------------------------------------------------------------------------

void MemwaAllocator::operator delete [] ( void * p, std::size_t size, const std::nothrow_t & ) throw()
{
    bool released = allocator_->Release( p, size );
    assert( released );
}

// ----------------------------------------------------------------------------
