/*
 * CS 170 - Theory of Operating Systems
 * By: Matthew S Montoya
 * Purpose: Measure the time it takes for the OS to perform context swapping
 * Last Modified: 14 February 2020
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

#define MAX 300
__clock_t swap_start_time, swap_end_time;
int cycle;
double average_swap_time, variance, sum_swap_time;

/* Runner executes the program */
int main() {

    cycle = 0;
    sum_swap_time = 0.0;

    /* Loop Experiment 100 times */
    while(cycle < 100){
        /* Ensure Context-Switching Processes are located on CPU Core 0 */
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(0, &set);
        sched_setaffinity(0,sizeof(cpu_set_t), &set);

        char *test_input = "I pray that I graduate this semester.";

        /* Create Pipe */
        int pid[2];
        pipe(pid);

        /* An issue occured while piping */
        if(pid < 0){
            printf("Pipe Failed. Quitting Program.\n");
            exit(1);
        }

        int subprocess = fork();

        /* An issue occured while forking */
        if (subprocess < 0) {
            printf("Fork Failed. Quitting Program.\n");
            exit(1);
        }

        /* Run the child process */
        else if (subprocess == 0) {

            /* Log The End of the 1st Context Swap */
            swap_end_time = clock();

            /* Calculate the time of context swapping */
            double total_swap_time = (swap_end_time - swap_start_time) / (double)CLOCKS_PER_SEC;
            sum_swap_time += total_swap_time;
            printf("Swap time is: %.9lf\n", total_swap_time);

            close(pid[1]);                                  // Close the write end of the pipe
            read(pid[0], test_input, sizeof(test_input));   // Read the user input
            close(pid[0]);                                  // Close the read end of the pipe
            exit(0);
        }

        /* Run the parent process */
        else {
            close(pid[0]);                                  // Close the read end of the pipe
            write(pid[1], test_input, sizeof(test_input));  // Write the user enput
            close(pid[1]);                                  // Close the write end of the pipe

            /* Log The Start of the 1st Context Swap */
            swap_start_time = clock();
            waitpid(subprocess, NULL, 0);                   // Swap processes
        }
        cycle++;
    }
    average_swap_time = sum_swap_time/(double)cycle;
    printf("The average time to perform context swapping is:\n%.9lf seconds\n", average_swap_time);

    return 0;
}