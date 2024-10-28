#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>

#define READY 0
#define RUNNING 1
#define MAX_PROCESSES 200
#define INPUT_SIZE 512
#define HISTORY_SIZE 100
#define MAX_PROGRAM_NAME 50

// global variables
int num_processes = 0;
int *total_processes;
int CPU_CYCLES = 0;

int TSLICE;
int NCPU;
int child_pid;
timer_t timer;

long long start_ms;
long long end_ms;

// Struct for process
typedef struct Process
{
    pid_t pid;
    int state;
    char name[50];
    struct timespec start_time;
    struct timespec end_time;
    double execution_time;
    long long wait_time;
    int prev_cycle;
    int current_cycle;
    int priority;
} process;

// Required arrays and variables
process *process_table;

process *ready_queue1;
int *front1;
int *rear1;
process *ready_queue2;
int *front2;
int *rear2;
process *ready_queue3;
int *front3;
int *rear3;
process *ready_queue4;
int *front4;
int *rear4;

process *running_queue;
int *front_r;
int *rear_r;

// queue.c
process create_process(char *name, int priority);
void add_process_r(process p);
void add_process(process p);
void add_process_table(process p);
bool is_empty();
process remove_process(process p);
process remove_process_r(process p);
void empty_process_table();

// timer.c
static void sigalrm_handler(int sig);
static int set_sigalrm(int sig, int flags, void (*handler)(int));
static int timer_handler(long long frequency_nsec);

// SimpleScheduler.c
void scheduler();
void displayProcesses();
void move_ready_to_running(int *front, int *rear, process *ready_queue);

#endif