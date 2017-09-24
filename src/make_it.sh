
rm ./obj/*.o

if [ ! -d "obj" ]; then
	mkdir obj
fi

g++ -std=c++14 -Wall -I../include -c AllocatorManager.cpp    -o ./obj/AllocatorManager.o
g++ -std=c++14 -Wall -I../include -c TinyBlock.cpp           -o ./obj/TinyBlock.o
g++ -std=c++14 -Wall -I../include -c PoolBlock.cpp           -o ./obj/PoolBlock.o
g++ -std=c++14 -Wall -I../include -c StackBlock.cpp          -o ./obj/StackBlock.o
g++ -std=c++14 -Wall -I../include -c LinearBlock.cpp         -o ./obj/LinearBlock.o
g++ -std=c++14 -Wall -I../include -c PoolAllocator.cpp       -o ./obj/PoolAllocator.o
g++ -std=c++14 -Wall -I../include -c StackAllocator.cpp      -o ./obj/StackAllocator.o
g++ -std=c++14 -Wall -I../include -c LinearAllocator.cpp     -o ./obj/LinearAllocator.o
g++ -std=c++14 -Wall -I../include -c TinyObjectAllocator.cpp -o ./obj/TinyObjectAllocator.o

