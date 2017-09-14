# Memwa

# Table of Contents
&nbsp;[Introduction](https://github.com/richsposato/Memwa#introduction) <br/> 
&nbsp;&nbsp;&nbsp;&nbsp;[Terminology](https://github.com/richsposato/Memwa#terminology) <br/> 
&nbsp;[Benefits of Memwa](https://github.com/richsposato/Memwa#benefits-of-memwa) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Time](https://github.com/richsposato/Memwa#time) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Memory Use](https://github.com/richsposato/Memwa#memory-use) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Alignment-Aware](https://github.com/richsposato/Memwa#alignment-aware) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[C++17 Compatible](https://github.com/richsposato/Memwa#c17-compatible) <br/>
&nbsp;[Using Memwa](https://github.com/richsposato/Memwa#using-memwa) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Build Instructions](https://github.com/richsposato/Memwa#build-instructions) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Alignment](https://github.com/richsposato/Memwa#alignment) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Exceptions](https://github.com/richsposato/Memwa#exceptions) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Recommendations](https://github.com/richsposato/Memwa#recommendations) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[With STL Containers](https://github.com/richsposato/Memwa#with-stl-containers) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Examples](https://github.com/richsposato/Memwa#examples) <br/>
&nbsp;[Allocators](https://github.com/richsposato/Memwa#allocators) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[LinearAllocator](https://github.com/richsposato/Memwa#linearallocator) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[StackAllocator](https://github.com/richsposato/Memwa#stackallocator) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[PoolAllocator](https://github.com/richsposato/Memwa#poolallocator) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[TinyObjectAllocator](https://github.com/richsposato/Memwa#tinyobjectallocator) <br/>
&nbsp;[Testing Memwa](https://github.com/richsposato/Memwa#testing-memwa) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Functionality](https://github.com/richsposato/Memwa#functionality) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Timing Tests](https://github.com/richsposato/Memwa#timing-tests) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Memory Tests](https://github.com/richsposato/Memwa#memory-tests) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Multi-threaded](https://github.com/richsposato/Memwa#multi-threaded) <br/>
&nbsp;[Design of Memwa](https://github.com/richsposato/Memwa#design-of-memwa) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Levels of Responsibility](https://github.com/richsposato/Memwa#levels-of-responsibility) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Block](https://github.com/richsposato/Memwa#block) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[BlockInfo](https://github.com/richsposato/Memwa#blockinfo) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Single-Threaded Allocator](https://github.com/richsposato/Memwa#single-threaded-allocator) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Multi-Threaded Allocator](https://github.com/richsposato/Memwa#multi-threaded-allocator) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[AllocatorManager](https://github.com/richsposato/Memwa#allocatormanager) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Supporting Classes](https://github.com/richsposato/Memwa#supporting-classes) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;[Design Considerations](https://github.com/richsposato/Memwa#design-considerations) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Portability](https://github.com/richsposato/Memwa#portability) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Time Performance](https://github.com/richsposato/Memwa#time-performance) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Space Performance](https://github.com/richsposato/Memwa#space-performance) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[STL Container Compatiblility](https://github.com/richsposato/Memwa#stl-container-compatiblility) <br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[Embedded Software](https://github.com/richsposato/Memwa#embedded-software) <br/>
&nbsp;[Writing Memwa Based Allocators](https://github.com/richsposato/Memwa#writing-memwa-based-allocators) <br/>

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

## **Memory Use**

## **Alignment-Aware**

Memwa is an alignment-aware allocator. When code requests a chunk of memory that is aligned on 1, 2, 4, 8, 16, or 32 byte boundaries, a Memwa allocator can fill that request. Although the C++ Standard says allocator "behavior is undefined if this is not a valid alignment value", Memwa will simply throw std::invalid_argument if the alignment is not correct.

## **C++17 compatible**

The 2017 version of C++ introduced several features related to memory allocation.
* The std::align_val_t parameter.
 Several recently added new and delete operators use the std::align_val_t parameter for alignment-aware allocators.
* The allocator trait for is_always_equal.
 Memwa's allocator adapter class provides the is_always_equal trait so compilers don't assume all Memwa allocators are equal.

# Using Memwa

## **Build Instructions**

## **Alignment**
For now, Memwa supports alignment on a power-of-two boundary up to 32. (e.g. - 1, 2, 4, 8, 16, or 32). This may change in the future. Some allocators support alignments as small as 1 or 2 bytes, while others support a minimum of 4 bytes.

## **Exceptions**
Memwa will throw exceptions under these conditions.

**std::bad_alloc** if it is unable to allocate memory. Before any Memwa allocator throws bad_alloc, it will attempt to free unused blocks of memory maintained by any other Memwa allocator.

**std::invalid_argument** if any of these conditions are met.
* The alignment parameter to a Release or Allocate function is greater than the allocator's initial alignment.
* The requested size is greater than what the allocator can provide.
* The requested size does not match the size specified for a PoolAllocator or TinyObjectAllocator.

## **With STL Containers**

Memwa provides an adapter template class so programmers can use Memwa allocators with STL containers.

## **Recommendations**
* Configure the allocators to pre-allocate blocks whose sizes are the same as the CPU caches.
* Use the hint parameter to increase likelihood of locality.
  The Release functions accept a hint parameter - which is defaulted to nullptr - so the next chunk of memory will be allocated within the same block as the previous chunk of memory.
* Don't underalign. Underalignment leads to wasted space.
  Underalignment is when a program wants to align an object on a boundary that is much bigger than the object size. For example, if an object is 4 bytes, it is a waste of space to align it on 8 byte boundaries. The first 4 bytes of a chunk would store the object, and the second 4 bytes would be wasted. It's more space efficient to align those objects on 4 byte boundaries.
* Don't overalign. Overalignment leads to wasted time.
  Overalignment is when an object is aligned on a smaller byte boundary than it should be. If an object is 8 bytes, it is better to align it on 8 byte boundaries than to align it on 2 or 4 byte boundaries. This can lead to suboptimal behavior as the compiler must create additional instructions to fetch data that is partly in one 4-byte word and then get the rest of the data that is in another 4 byte word.
* Create a new Memwa allocator for each purpose. Don't reuse an allocator for multiple purposes.

## **Examples**

# Allocators

## **LinearAllocator**

The purpose of LinearAllocator is to allocate chunks of memory very quickly that never need to be released. LinearAllocator will pre-allocate blocks of memory and then suballocate chunks from within those blocks as requested. It maintains no information about the locations or sizes of those chunks so it can't release them.

### Uses:
* For chunks of any size from 1 byte to the size of a block.
* For alignments of any size from 1 byte to 32 bytes.
* Chunks of memory allocated at program start-up and never released.
* Static const objects that are never destructed.

## **StackAllocator**

StackAllocator will pre-allocate 1 or more large blocks and then suballocate chunks from within that block. It uses the 4 bytes before each chunk to store the size of the previous chunk. By storing the size, an allocator can release chunks at the end of a stack. It can't release chunks before the end.

### Uses:
* For objects whose alignment should be on 4 byte boundaries or more. This allocator is not intended for alignment on 1 or 2 byte boundaries.
* For objects of any size from 4 bytes to the size of a block. This allocator is not intended for chunks smaller than 4 bytes.
* For memory that are released in the opposite order in which they were allocated. (first-in-last-out)
* Chunks of memory allocated at program start-up and never released.
* Static const objects that are never destructed.

## **PoolAllocator**

### Uses:
* For node-based STL containers that always allocate objects of the same size. (e.g. - std::list, std::set, or std::map)
* For code that must allocate from hundreds to millions of objects that are all the same size.
* For memory that could be allocated and released in any order.

## **TinyObjectAllocator**

This allocator provides a time and space efficient way to allocate many small chunks of memory. Unlike the other allocators which use pointers and size fields to keep track of allocated memory, TinyObjectAllocator uses stealth indexes within each unallocated block. This allocator was based on a rewrite of the SmallObjectAllocator in the Loki project.

### Uses:
* For hundreds or thousands of objects that are all the same size.
* For objects whose alignments can be on 1 or 2 byte boundaries. (If the object should be aligned on a boundary of 4 or more, consider PoolAllocator instead.)
* For objects whose sizes are smaller than a pointer. (Pointers are usually 4 or 8 bytes.)
* For memory that could be allocated and released in any order.
* For node-based STL containers that always allocate objects of the same size. (e.g. - std::list, std::set, or std::map)

# Testing Memwa

## **Functionality**

The purpose of functionality tests is to demonstrate that each allocator provides an expected output for particular input values.

### Negative Tests

These tests provide invalid input parameters (e.g. - wrong size or wrong alignment) to see if the allocator will throw an exception.

### Positive Tests

These tests show the allocators return correct values for valid input.

## **Timing Tests**

## **Memory Tests**

## **Multi-threaded**

# Design of Memwa

## **Levels of Responsibility**

### Block

The Block is the lowest level of Memwa. Each instance of a Block class (e.g. - PoolBlock, LinearBlock, etc...) maintains a single block of memory. This class maintains internal pointers and indexes used for the block. All of its actions are constant-time and often trivial so that the block does not impose any runtime costs on higher classes in Memwa. Each block object is designed for trivial copying so they can be stored in containers.

### BlockInfo

BlockInfo maintains a container of blocks and common information (e.g. - block-size, alignment, and object-size) about those blocks. It provides functions to search blocks for an address, allocate a chunk from a block, and to release a chunk from a block. Its internal duties include creating new blocks, removing empty blocks, and keeping blccks sorted. Each allocator object has a single BlockInfo object to maintain its blocks.

### Single-Threaded Allocator

The single-threaded allocators implement Memwa's allocator interface class. These classes will check if the input parameters are valid, and will then call the appropriate functions in their BlockInfo data members.

### Multi-Threaded Allocator

The multi-threaded allocators are mere subclasses of the single-threaded allocators. Each function in the multi-threaded allocators will lock a mutex and then call the appropriate function in the base class. The lock uses RAII so that any exception will automatically unlock the mutex.

### AllocatorManager

The AllocatorManager is at the top of the Memwa project. It provides a factory function to create any type of Memwa allocator on demand, to destroy a Memwa allocator, and to trim unused memory from Memwa allocators.

### Supporting Classes

## **Design Considerations**

### **Portability**

### **Time Performance**

The lowest levels of each Memwa allocator require only O(1) operations to allocate and release a chunk. A Block should only need to adjust one or two pointers and/or indexes for each allocation and release. Most functions at the next level up required O(1) operations for almost all allocations and releases, use O(log N) operations occasionally, and use O(N) operations sparingly.

### **Space Performance**

### **STL Container Compatiblility**

### **Embedded Software**

# Writing Memwa Based Allocators
