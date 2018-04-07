#!/bin/bash

rm performance_test.exe
rm *.o

echo "Compile main.cpp";            g++ -std=c++14 -Wall -I../../include -c main.cpp -o main.o
echo "Compile Stopwatch.cpp";       g++ -std=c++14 -Wall -o Stopwatch.o -c Stopwatch.cpp
echo "Compile CommandLineArgs.cpp"; g++ -std=c++14 -Wall -c CommandLineArgs.cpp -o CommandLineArgs.o
echo "Compile MemwaAllocator.cpp";  g++ -std=c++14 -Wall -I../../include -c MemwaAllocator.cpp -o MemwaAllocator.o
echo "Compile TestMemwa.cpp";       g++ -std=c++14 -Wall -I../../include -c TestMemwa.cpp -o TestMemwa.o
echo "Compile TestDefault.cpp";     g++ -std=c++14 -Wall -I../../include -c TestDefault.cpp -o TestDefault.o

echo "Linking"
g++ -std=c++14 -Wall -o performance_test.exe \
	main.o \
	Stopwatch.o \
	MemwaAllocator.o \
	TestMemwa.o \
	TestDefault.o \
	../functional/ChunkList.o \
	../../src/obj/AllocatorManager.o \
	../../src/obj/TinyBlock.o \
	../../src/obj/PoolBlock.o \
	../../src/obj/StackBlock.o \
	../../src/obj/LinearBlock.o \
	../../src/obj/PoolAllocator.o \
	../../src/obj/StackAllocator.o \
	../../src/obj/LinearAllocator.o \
	../../src/obj/TinyObjectAllocator.o \
	CommandLineArgs.o
echo "Done!"
