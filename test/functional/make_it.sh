#!/bin/bash

rm functionality_test.exe
rm *.o

echo "Compile main.cpp";            g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c main.cpp -o main.o
echo "Compile ChunkList.cpp";       g++ -std=c++14 -Wall -o ChunkList.o -c ChunkList.cpp
echo "Compile CommandLineArgs.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c CommandLineArgs.cpp -o CommandLineArgs.o
echo "Compile TestTinyBlock.cpp";   g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestTinyBlock.cpp -o TestTinyBlock.o
echo "Compile TestPoolBlock.cpp";   g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestPoolBlock.cpp -o TestPoolBlock.o
echo "Compile TestStackBlock.cpp";  g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestStackBlock.cpp -o TestStackBlock.o
echo "Compile TestLinearBlock.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestLinearBlock.cpp -o TestLinearBlock.o
echo "Compile TestTinyAllocator.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestTinyAllocator.cpp -o TestTinyAllocator.o
echo "Compile TestPoolAllocator.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestPoolAllocator.cpp -o TestPoolAllocator.o
echo "Compile TestStackAllocator.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestStackAllocator.cpp -o TestStackAllocator.o
echo "Compile TestLinearAllocator.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestLinearAllocator.cpp -o TestLinearAllocator.o
echo "Compile TestMultithreaded.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestMultithreaded.cpp -o TestMultithreaded.o
echo "Compile ComplexMultithreadedTest.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c ComplexMultithreadedTest.cpp -o ComplexMultithreadedTest.o
echo "Compile TestMultithreadedDuplicates.cpp"; g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c TestMultithreadedDuplicates.cpp -o TestMultithreadedDuplicates.o

echo "Linking"
g++ -std=c++14 -Wall -o functionality_test.exe \
	main.o \
	ChunkList.o \
	TestTinyBlock.o \
	TestPoolBlock.o \
	TestStackBlock.o \
	TestLinearBlock.o \
	CommandLineArgs.o \
	TestMultithreaded.o \
	TestPoolAllocator.o \
	TestTinyAllocator.o \
	TestStackAllocator.o \
	TestLinearAllocator.o \
	ComplexMultithreadedTest.o \
	TestMultithreadedDuplicates.o \
	../../src/obj/AllocatorManager.o \
	../../src/obj/TinyBlock.o \
	../../src/obj/PoolBlock.o \
	../../src/obj/StackBlock.o \
	../../src/obj/LinearBlock.o \
	../../src/obj/PoolAllocator.o \
	../../src/obj/StackAllocator.o \
	../../src/obj/LinearAllocator.o \
	../../src/obj/TinyObjectAllocator.o \
	../../../Hestia/CppUnitTest/src/obj/UnitTest.o
echo "Done!"
