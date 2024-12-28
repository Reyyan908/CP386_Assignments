# Do not edit the contents of this file.
CC = gcc
CFLAGS = -Wall -g -std=gnu99
LDFLAGS = -lrt -lpthread
TARGET = fcfs sudoku
OBJFILES = fcfs.o sudoku.o
all: $(TARGET)
sudoku: sudoku.c
	$(CC) $(CFLAGS) -o sudoku sudoku.c $(LDFLAGS)
fcfs: fcfs.c
	$(CC) $(CFLAGS) -o fcfs fcfs.c 
runq1: sudoku
	./sudoku sample_in_sudoku.txt
# For all using mac system can use make runq1_mac because it contains Pthread library is included like -pthread
runq1_mac: sudoku_mac
	./sudoku sample_in_sudoku.txt
runq2: fcfs
	./fcfs sample_in_schedule.txt
clean:
	rm -rf $(OBJFILES) $(TARGET) *~ 
	rm -rf *.out