#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 100 

//structure to hold thread info
struct threadInfo { 
    int p_id; 
    int arr_time; 
    int burst_time;
    int waiting_time;
    int turn_around_time;
    int completion_time;
};

int main() {
    struct threadInfo threads[MAX_THREADS]; //array to hold thread info
    int threadnum = 0; //number of threads

    //open file + error handle
    FILE *fp = fopen("sample_in_schedule.txt", "r");
    if (fp == NULL) {
        printf("Error: Cannot open file sample_in_schedule.txt\n");
        return 1;
    }

    //read file + error handle
    while (fscanf(fp, "%d, %d, %d", &threads[threadnum].p_id, &threads[threadnum].arr_time, &threads[threadnum].burst_time) == 3) {
        threadnum++; //thread count increment
        if (threadnum >= MAX_THREADS) {
            printf("Error: Exceeded maximum number of threads (%d)\n", MAX_THREADS);
            break;
        }
    }

    fclose(fp);

    //sort threads by arrival time, by ID if equal
    for (int i = 0; i < threadnum - 1; i++) {
        for (int j = i + 1; j < threadnum; j++) {
            if (threads[i].arr_time > threads[j].arr_time || 
                (threads[i].arr_time == threads[j].arr_time && threads[i].p_id > threads[j].p_id)) {
                //swap threads[i] and threads[j]
                struct threadInfo temp = threads[i];
                threads[i] = threads[j];
                threads[j] = temp;
            }
        }
    }

    int current_time = 0;
    int total_waiting_time = 0;
    int total_turnaround_time = 0;

    //fcfs scheduling
    for (int i = 0; i < threadnum; i++) {
        if (current_time < threads[i].arr_time) { //update current time when new threads arrival is compared
            current_time = threads[i].arr_time;
        }

        threads[i].completion_time = current_time + threads[i].burst_time;
        threads[i].turn_around_time = threads[i].completion_time - threads[i].arr_time;
        threads[i].waiting_time = threads[i].turn_around_time - threads[i].burst_time;

        current_time = threads[i].completion_time; //move current time to completion time

        total_waiting_time += threads[i].waiting_time;
        total_turnaround_time += threads[i].turn_around_time;
    }

    float average_waiting_time = (float)total_waiting_time / threadnum;
    float average_turnaround_time = (float) total_turnaround_time / threadnum;

    //results
    printf("ThreadID     Arrival Time     Burst Time         Completion Time     Turn-Around Time          Waiting Time\n");
    for (int i = 0; i < threadnum; i++) {
        printf("%-18d%-18d%-20d%-22d%-18d%-19d\n", 
                threads[i].p_id,
                threads[i].arr_time,
                threads[i].burst_time,
                threads[i].completion_time,
                threads[i].turn_around_time,
                threads[i].waiting_time);
    }

    printf("\nThe average waiting time: %.2f\n", average_waiting_time);
    printf("The average turn-around time: %.2f\n", average_turnaround_time);

    return 0;

}