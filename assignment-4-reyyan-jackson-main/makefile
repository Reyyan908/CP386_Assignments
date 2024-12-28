#Do not edit the contents of this file.
CC = gcc
CFLAGS = -Wall -g -std=gnu99
LDFLAGS = -lrt -lpthread
TARGET = thread_synchronization resource_management 
OBJFILES = resource_management.o thread_synchronization.o 
all: $(TARGET)
resource_management: resource_management.c
	$(CC) $(CFLAGS) -o resource_management resource_management.c $(LDFLAGS)
thread_synchronization: thread_synchronization.c
	$(CC) $(CFLAGS) -o thread_synchronization thread_synchronization.c $(LDFLAGS)
runq1: resource_management
	./resource_management
runq2: thread_synchronization
	./thread_synchronization sample3_in.txt
clean:
	rm -f $(OBJFILES) $(TARGET) *~  *.exe *.out