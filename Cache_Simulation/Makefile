all: cache-sim

cache-sim: CacheSimulator.o
	g++ -g -std=c++11 CacheSimulator.o -o cache-sim

CacheSimulator.o: CacheSimulator.cpp
	g++ -c -g -std=c++11 CacheSimulator.cpp -o CacheSimulator.o

clean:
	rm -f *~ *.o cache-sim