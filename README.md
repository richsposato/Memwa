# Memwa

# Introduction

Memwa is the Haitian Creole word for Memory.

This is a collection of memory allocators for C++.

## Terminology

These terms are used within the documentation and code for Memwa.

* **Chunk** An individual piece of memory provided by an Allocate function to consumers of Memwa.
* **Place** The address of a chunk.
* **Block** A large area of pre-allocated memory which is subdivided into many chunks.

# Benefits of Memwa

## **Time**

## **Memory Consumption**

## **Alignment-Aware**

Memwa is an alignment-aware allocator. When code requests a chunk of memory that is aligned on 1, 2, 4, 8, or 16 byte boundaries, a Memwa allocator can fill that request.

## **C++17 compatible**

The 2017 version of C++ introduced several new and delete operators which use a std::align_val_t parameter.

# Allocators

## **LinearAllocator**

The purpose of LinearAllocator is to allocate chunks of memory very quickly that never need to be released. LinearAllocator will pre-allocate blocks of memory and then suballocate chunks from within those blocks as requested. It maintains no information about the locations or sizes of those chunks so it can't release them.

### Uses:
* Chunks of memory allocated at program start-up.
* Static const objects that are never destructed.

## **StackAllocator**

### Uses:
* For objects of any size from 1 byte to the size of a block.
* For memory that are released in the opposite order in which they were allocated. (first-in-last-out)

## **PoolAllocator**

### Uses:
* For node-based STL containers that always allocate objects of the same size. (e.g. - std::list, std::set, or std::map)
* For code that must allocate hundreds or thousands of objects that are all the same size.
* For memory that must be allocated and released in any order.

## **TinyObjectAllocator**

This allocator provides a time and space efficient way to allocate many small chunks of memory. Unlike the other allocators which use pointers and size fields to keep track of allocated memory, TinyObjectAllocator uses stealth indexes within each unallocated block.

### Uses:
* For objects who sizes are smaller than a pointer.
* For memory that must be allocated and released in any order.

# Testing Memwa

## **Functionality**

## **Time**

## **Memory**

## **Multi-threaded**

# Using Memwa

## **Compiling**

## **Alignment**

## **Exceptions**
Memwa will throw exceptions under these conditions.

**std::bad_alloc** if it is unable to allocate memory.

**std::invalid_argument if any of these conditions are met.
* The alignment parameter to a Release or Allocate function is greater than the allocator's initial alignment.
* The requested size is greater than what the allocator can provide.
* The requested size does not match the size specified for a PoolAllocator or TinyObjectAllocator.

## **Recommendations**
* Configure the allocators to pre-allocate blocks whose sizes are the same as the CPU caches.
* Use the hint parameter to increase likelihood of locality.
* Don't underalign.
* Don't overalign.
* Create a new Memwa allocator for each purpose. Don't reuse an allocator for multiple purposes.

## **Examples**

# Design of Memwa

## **Levels of Responsibility**

## **Portability**

## **Design Considerations**

### **Time Performance**

The lowest levels of each Memwa allocator require only O(1) operations to allocate and release a chunk.

### **Space Performance**

### **STL Container Compatiblility**

### **Low Memory for Embedded Software**

# Using Allocators with STL Containers

## **Adapter Class**

## **Recommendations**

## **Examples**

# Writing Your Own Memwa Based Allocators

