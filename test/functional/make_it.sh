
rm functionality_test.exe

g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c main.cpp -o main.o
g++ -std=c++14 -Wall -I../../include -I../../../Hestia/CppUnitTest/include -c CommandLineArgs.cpp -o CommandLineArgs.o

g++ -std=c++14 -Wall -o functionality_test.exe \
	main.o \
	CommandLineArgs.o \
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