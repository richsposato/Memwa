
rm ./obj/*.o

if [ ! -d "obj" ]; then
	mkdir obj
fi

echo "Compile AllocatorManager.cpp";    g++ -std=c++14 -Wall -I../include -c AllocatorManager.cpp    -o ./obj/AllocatorManager.o
echo "Compile TinyBlock.cpp";           g++ -std=c++14 -Wall -I../include -c TinyBlock.cpp           -o ./obj/TinyBlock.o
echo "Compile PoolBlock.cpp";           g++ -std=c++14 -Wall -I../include -c PoolBlock.cpp           -o ./obj/PoolBlock.o
echo "Compile StackBlock.cpp";          g++ -std=c++14 -Wall -I../include -c StackBlock.cpp          -o ./obj/StackBlock.o
echo "Compile LinearBlock.cpp";         g++ -std=c++14 -Wall -I../include -c LinearBlock.cpp         -o ./obj/LinearBlock.o
echo "Compile PoolAllocator.cpp";       g++ -std=c++14 -Wall -I../include -c PoolAllocator.cpp       -o ./obj/PoolAllocator.o
echo "Compile StackAllocator.cpp";      g++ -std=c++14 -Wall -I../include -c StackAllocator.cpp      -o ./obj/StackAllocator.o
echo "Compile LinearAllocator.cpp";     g++ -std=c++14 -Wall -I../include -c LinearAllocator.cpp     -o ./obj/LinearAllocator.o
echo "Compile TinyObjectAllocator.cpp"; g++ -std=c++14 -Wall -I../include -c TinyObjectAllocator.cpp -o ./obj/TinyObjectAllocator.o
echo "Done!"
