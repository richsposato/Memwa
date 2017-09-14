# Memwa

# Table of Contents
&nbsp;[Introduction](https://github.com/richsposato/Memwa#introduction) <br/> 
&nbsp;&nbsp;[Terminology](https://github.com/richsposato/Memwa#build-instructions) <br/> 
&nbsp;[Benefits of Memwa](https://github.com/richsposato/Memwa#Benefits-of-Memwa) <br/>
&nbsp;&nbsp;[Time](https://github.com/richsposato/Memwa#Time) <br/>
&nbsp;&nbsp;[Memory Use](https://github.com/richsposato/Memwa#Memory-Use) <br/>
&nbsp;&nbsp;[Alignment-Aware](https://github.com/richsposato/Memwa#Alignment-Aware) <br/>
&nbsp;&nbsp;[C++17 Compatible](https://github.com/richsposato/Memwa#C++17-Compatible) <br/>
&nbsp;[Using Memwa](https://github.com/richsposato/Memwa#Using-Memwa) <br/>
&nbsp;&nbsp;[Build Instructions](https://github.com/richsposato/Memwa#Build-Instructions) <br/>
&nbsp;&nbsp;[Alignment](https://github.com/richsposato/Memwa#Alignment) <br/>
&nbsp;&nbsp;[Exceptions](https://github.com/richsposato/Memwa#Exceptions) <br/>
&nbsp;&nbsp;[Recommendations](https://github.com/richsposato/Memwa#Recommendations) <br/>
&nbsp;&nbsp;[With STL Containers](https://github.com/richsposato/Memwa# ) <br/>
&nbsp;&nbsp;[Examples](https://github.com/richsposato/Memwa#Examples) <br/>
&nbsp;[Allocators](https://github.com/richsposato/Memwa#Allocators) <br/>
&nbsp;&nbsp;[LinearAllocator](https://github.com/richsposato/Memwa#LinearAllocator) <br/>
&nbsp;&nbsp;[StackAllocator](https://github.com/richsposato/Memwa#StackAllocator) <br/>
&nbsp;&nbsp;[PoolAllocator](https://github.com/richsposato/Memwa#PoolAllocator) <br/>
&nbsp;&nbsp;[TinyObjectAllocator](https://github.com/richsposato/Memwa#TinyObjectAllocator) <br/>
&nbsp;[Testing Memwa](https://github.com/richsposato/Memwa#Testing-Memwa) <br/>
&nbsp;&nbsp;[Functionality](https://github.com/richsposato/Memwa#Functionality) <br/>
&nbsp;&nbsp;[Timing Tests](https://github.com/richsposato/Memwa#Timing-Tests) <br/>
&nbsp;&nbsp;[Memory Tests](https://github.com/richsposato/Memwa#Memory-Tests) <br/>
&nbsp;&nbsp;[Multi-threaded](https://github.com/richsposato/Memwa#Multi-threaded) <br/>
&nbsp;[Design of Memwa](https://github.com/richsposato/Memwa#Design-of-Memwa) <br/>
&nbsp;&nbsp;[Levels of Responsibility](https://github.com/richsposato/Memwa#Levels-of-Responsibility) <br/>
&nbsp;&nbsp;[Design Considerations](https://github.com/richsposato/Memwa#Design-Considerations) <br/>
&nbsp;&nbsp;&nbsp;[Portability](https://github.com/richsposato/Memwa#Portability) <br/>
&nbsp;&nbsp;&nbsp;[Time Performance](https://github.com/richsposato/Memwa#Time-Performance) <br/>
&nbsp;&nbsp;&nbsp;[Space Performance](https://github.com/richsposato/Memwa#Space-Performance) <br/>
&nbsp;&nbsp;&nbsp;[STL Container Compatiblility](https://github.com/richsposato/Memwa#STL-Container-Compatiblility) <br/>
&nbsp;&nbsp;&nbsp;[Embedded Software](https://github.com/richsposato/Memwa#Embedded-Software) <br/>
&nbsp;[Writing Memwa Based Allocators](https://github.com/richsposato/Memwa#Writing-Memwa-Based-Allocators) <br/>

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

Memwa is an alignment-aware allocator. When code requests a chunk of memory that is aligned on 1, 2, 4, 8, 16, or 32 byte boundaries, a Memwa allocator can fill that request.

## **C++17 compatible**

The 2017 version of C++ introduced several new and delete operators which use a std::align_val_t parameter.

# Using Memwa

## **Build Instructions**

## **Alignment**
For now, Memwa supports alignment on a power-of-two boundary up to 32. (e.g. - 1, 2, 4, 8, 16, or 32). This may change in the future. Some allocators support alignments as small as 1 or 2 bytes, while others support a minimum of 4 bytes.

## **Exceptions**
Memwa will throw exceptions under these conditions.

**std::bad_alloc** if it is unable to allocate memory.

**std::invalid_argument** if any of these conditions are met.
* The alignment parameter to a Release or Allocate function is greater than the allocator's initial alignment.
* The requested size is greater than what the allocator can provide.
* The requested size does not match the size specified for a PoolAllocator or TinyObjectAllocator.

## **With STL Containers**

## **Recommendations**
* Configure the allocators to pre-allocate blocks whose sizes are the same as the CPU caches.
* Use the hint parameter to increase likelihood of locality.
  The Release functions accept a hint parameter - which is defaulted to nullptr - so the next chunk of memory will be allocated within the same block as the previous chunk of memory.
* Don't underalign.
  Underalignment is when a program wants to align an object on a boundary that is much bigger than the object size. For example, if an object is 4 bytes, it is a waste of space to align it on 8 byte boundaries. The first 4 bytes of a chunk would store the object, and the second 4 bytes would be wasted. It's more space efficient to align those objects on 4 byte boundaries.
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
* For memory that must be allocated and released in any order.

## **TinyObjectAllocator**

This allocator provides a time and space efficient way to allocate many small chunks of memory. Unlike the other allocators which use pointers and size fields to keep track of allocated memory, TinyObjectAllocator uses stealth indexes within each unallocated block.

### Uses:
* For hundreds or thousands of objects that are all the same size.
* For objects whose alignments can be on 1 or 2 byte boundaries. (If the object should be aligned on a boundary of 4 or more, consider PoolAllocator instead.)
* For objects whose sizes are smaller than a pointer. (Pointers are usually 4 or 8 bytes.)
* For memory that must be allocated and released in any order.

# Testing Memwa

## **Functionality**

## **Timing Tests**

## **Memory Tests**

## **Multi-threaded**

# Design of Memwa

## **Levels of Responsibility**

## **Design Considerations**

### **Portability**

### **Time Performance**

The lowest levels of each Memwa allocator require only O(1) operations to allocate and release a chunk.

### **Space Performance**

### **STL Container Compatiblility**

### **Embedded Software**

# Writing Memwa Based Allocators
