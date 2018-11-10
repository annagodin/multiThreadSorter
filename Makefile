all: multiThreadSorter_thread.c mergesort.o

multiThreadSorter_thread: multiThreadSorter_thread.o mergesort.o
	gcc -pthread -Wall -o multiThreadSorter_thread multiThreadSorter_thread.o mergesort.o
mergesort.o: mergesort.c
	gcc -c mergesort.c
clean:
	rm -f multiThreadSorter_thread mergesort.o
