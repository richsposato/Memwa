
g++ -std=c++14 -Wall -I../include -c AllocatorManager.cpp    -o AllocatorManager.o
g++ -std=c++14 -Wall -I../include -c LinearBlock.cpp         -o LinearBlock.o
g++ -std=c++14 -Wall -I../include -c LinearAllocator.cpp     -o LinearAllocator.o
g++ -std=c++14 -Wall -I../include -c StackAllocator.cpp      -o StackAllocator.o
g++ -std=c++14 -Wall -I../include -c PoolAllocator.cpp       -o PoolAllocator.o
g++ -std=c++14 -Wall -I../include -c TinyObjectAllocator.cpp -o TinyObjectAllocator.o
