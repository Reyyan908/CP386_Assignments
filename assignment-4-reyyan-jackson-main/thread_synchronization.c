/*
 -------------------------------------
 File: thread_synchronization.c
 Project: Assignment 4
 -------------------------------------
 Author:  Reyyan Qureshi
 ID:      169033850
 Email:   qure3850@mylaurier.ca
 Version  2024-11-18
 -------------------------------------
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>

sem_t running;
sem_t even;
sem_t odd;

void logStart(char *tID); //function to log that a new thread is started
void logFinish(char *tID); //function to log that a thread has finished its time

void startClock(); //function to start program clock
long getCurrentTime(); //function to check current time since clock was started
time_t programClock; //the global timer/clock for the program

typedef struct thread //represents a single thread, you can add more members if required
{
	char tid[50]; //id of the thread as read from file
	unsigned int startTime;
	int state;
	pthread_t handle;
	int retVal;
	int isFirstThread; //identify first thread
} Thread;

//global variables
Thread *threads = NULL;
int threadCount;

int threadsLeft(Thread *threads, int threadCount);
int threadToStart(Thread *threads, int threadCount);
void* threadRun(void *t); //the thread function, the code executed by each thread
int readFile(char *fileName, Thread **threads); //function to read the file content and build array of threads
int oppositeThreadExists(int opposite, Thread *currentThread); //check if opposite y value threads exist

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Input file name missing...exiting with error code -1\n");
		return -1;
	}

	sem_init(&running, 0, 1); 
	sem_init(&even, 0, 0); //starts with 0 to block even threads
	sem_init(&odd, 0, 0); //starts with 0 to block odd threads

	threadCount = readFile(argv[1], &threads);

	//identify first running thread
	int firstThreadIndex = 0;
	unsigned int earliestStartTime = threads[0].startTime;
	for (int i = 1; i < threadCount; i++) {
		if (threads[i].startTime < earliestStartTime) {
			earliestStartTime = threads[i].startTime;
			firstThreadIndex = i;
		} 
	}

	for (int i = 0; i < threadCount; i++) { 
		threads[i].isFirstThread = 0; //set all isFirstThreads to 0
	}
	threads[firstThreadIndex].isFirstThread = 1; //mark the first threads

	startClock();
	while (threadsLeft(threads, threadCount) > 0) //put a suitable condition here to run your program
	{
		sleep(1); 
		for (int i = 0; i < threadCount; i++) {
			if (threads[i].state == 0 && threads[i].startTime <= getCurrentTime()) {
				threads[i].state = 1;
				threads[i].retVal = pthread_create(&(threads[i].handle), NULL, threadRun, &threads[i]);
			}
		}
	}
	//wait for all threads to finish
	for (int i = 0; i < threadCount; i++) {
		pthread_join(threads[i].handle, NULL);
	}
	
	sem_destroy(&running);
	sem_destroy(&even);
	sem_destroy(&odd);

	return 0;
}

int readFile(char *fileName, Thread **threads) //do not modify this method
{
	FILE *in = fopen(fileName, "r");
	if (!in) {
		printf(
				"Child A: Error in opening input file...exiting with error code -1\n");
		return -1;
	}

	struct stat st;
	fstat(fileno(in), &st);
	char *fileContent = (char*) malloc(((int) st.st_size + 1) * sizeof(char));
	fileContent[0] = '\0';
	while (!feof(in)) {
		char line[100];
		if (fgets(line, 100, in) != NULL) {
			strncat(fileContent, line, strlen(line));
		}
	}
	fclose(in);

	char *command = NULL;
	int threadCount = 0;
	char *fileCopy = (char*) malloc((strlen(fileContent) + 1) * sizeof(char));
	strcpy(fileCopy, fileContent);
	command = strtok(fileCopy, "\r\n");
	while (command != NULL) {
		threadCount++;
		command = strtok(NULL, "\r\n");
	}
	*threads = (Thread*) malloc(sizeof(Thread) * threadCount);

	char *lines[threadCount];
	command = NULL;
	int i = 0;
	command = strtok(fileContent, "\r\n");
	while (command != NULL) {
		lines[i] = malloc(sizeof(command) * sizeof(char));
		strcpy(lines[i], command);
		i++;
		command = strtok(NULL, "\r\n");
	}

	for (int k = 0; k < threadCount; k++) {
		char *token = NULL;
		int j = 0;
		token = strtok(lines[k], ";");
		while (token != NULL) {
			(*threads)[k].state = 0;
			(*threads)[k].isFirstThread = 0;
			if (j == 0)
				strcpy((*threads)[k].tid, token);
			if (j == 1)
				(*threads)[k].startTime = atoi(token);
			j++;
			token = strtok(NULL, ";");
		}
	}
	return threadCount;
}

void logStart(char *tID) {
	printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char *tID) {
	printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

int threadsLeft(Thread *threads, int threadCount) {
	int remainingThreads = 0;
	for (int k = 0; k < threadCount; k++) {
		if (threads[k].state > -1)
			remainingThreads++;
	}
	return remainingThreads;
}

int threadToStart(Thread *threads, int threadCount) {
	for (int k = 0; k < threadCount; k++) {
		if (threads[k].state == 0 && threads[k].startTime <= getCurrentTime())
			return k;
	}
	return -1;
}

int oppositeThreadExists(int opposite, Thread *currentThread) { //check if any threads with opposite y IDS still exist (aside from current thread) 
	for (int i = 0; i < threadCount; i++) {
		if (threads[i].state >= 0 && &threads[i] != currentThread) {
			int y = threads[i].tid[2] - '0'; //get y digit from threadID
			int o = y % 2; //check if even or odd
			if (o == opposite) {
				return 1; //thread with opposite y ID exists
			}
		}
	}
	return 0;
}

void* threadRun(void *t) //implement this function in a suitable way
{
	Thread* thread = (Thread*)t;
	int yDigit = thread -> tid[2] - '0'; //extracting y from txy 
	int yOpposite = yDigit % 2; //check whether even or odd
	int oppositeyVal = 1 - yOpposite;

	logStart(thread->tid);

	if (thread -> isFirstThread) { //determine first thread
		sem_wait(&running); //lock critical section

		//critical section starts here
			printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(),
		thread->tid);
		//critical section ends here

		logFinish(thread->tid);
		thread->state = -1; //thread marked as finished
		sem_post(&running); //release critical section

		if(yOpposite == 0) { //next semaphore being signaled 
			sem_post(&odd); //odd proceeds
		} else {
			sem_post(&even); //even proceeds
		}
		pthread_exit(0);
	} else { //other threads wait for semaphore to be signaled
		int needToWait = oppositeThreadExists(oppositeyVal, thread);
		if(needToWait) {
			if (yOpposite == 0) {
				sem_wait(&even); //wait until allowed to proceed
			} else {
				sem_wait(&odd); //wait until allowed to proceed
			}
		}
		
		sem_wait(&running); //entering critical section 
		//critical section starts here
			printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(),
		thread->tid);
		//critical section ends here

		logFinish(thread->tid);
		thread->state = -1; //thread marked as finished
		sem_post(&running); //release critical section

		//check if any opposite y values left
		if (oppositeThreadExists(oppositeyVal, thread)) {
			if (oppositeyVal == 0) { //signal the semaphore
				sem_post(&even);
			} else {
				sem_post(&odd);
			}
		} else { //no opposite threads left, signal own value to avoid starvation
			if (yOpposite == 0) {
				sem_post(&even);
			} else {
				sem_post(&odd);
			}
		}
		pthread_exit(0);
	}
}

void startClock() {
	programClock = time(NULL);
}

long getCurrentTime() //invoke this method whenever you want check how much time units passed
//since you invoked startClock()
{
	time_t now;
	now = time(NULL);
	return now - programClock;
}
