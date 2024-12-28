#Do not edit the contents of this file.
CC = gcc
CFLAGS = -Werror -Wall -g -std=gnu99
TARGET = filecopy  file_directory 
OBJFILES = filecopy.o file_directory.o 

all: $(TARGET)
filecopy: filecopy.c
	$(CC) $(CFLAGS) -o filecopy filecopy.c
file_file_directory: file_directory.c
	$(CC) $(CFLAGS) -o file_directory file_directory.c
runq1: file_directory.c
	./file_directory
runq1-file_directory: file_directory.c
	sh test_F24.sh
runq2-err: filecopy.c
	./filecopy
runq2: filecopy.c
	./filecopy input.txt output.txt
runq2-strace: filecopy.c
	strace -c ./filecopy input.txt output.txt
clean:
	rm -rf $(OBJFILES) $(TARGET)  $(TARGET) *.exe  *~ *.out *.dSYM.pkgf *.dSYM
