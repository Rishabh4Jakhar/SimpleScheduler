#include "scheduler.h"

void scheduler()
{
    if (ready_queue1[*front1].pid != 0)
    {
        move_ready_to_running(front1, rear1, ready_queue1);
    }
    else if (ready_queue2[*front2].pid != 0)
    {
        move_ready_to_running(front2, rear2, ready_queue2);
    }
    else if (ready_queue3[*front3].pid != 0)
    {
        move_ready_to_running(front3, rear3, ready_queue3);
    }
    else if (ready_queue4[*front4].pid != 0)
    {
        move_ready_to_running(front4, rear4, ready_queue4);
    }
    else
    {
        printf("All ready queues are empty\n");
    }
}

void displayProcesses()
{
    printf("--------------------------------\n");
    double total_execution = 0;
    long long total_wait = 0;
    for (int i = 0; i < *total_processes; i++)
    {
        printf("%s (%d)\nPID: %d\nExecution time: %f ms\nWait time: %lld ms\n", process_table[i].name, process_table[i].priority, process_table[i].pid, process_table[i].execution_time, process_table[i].wait_time);
        printf("--------------------------------\n");
        total_execution += process_table[i].execution_time;
        total_wait += process_table[i].wait_time;
    }
    printf("\n");
    printf("Average Execution Time: %f\n", total_execution / *total_processes);
    printf("Average Wait Time: %lld\n", total_wait / *total_processes);
}

void move_ready_to_running(int *front, int *rear, process *ready_queue)
{
    int end = *front + NCPU;
    int start = *front;
    for (int i = start; i < end; i++)
    {
        if (ready_queue[i].pid != 0)
        {
            printf("READY QUEUE --->\n");
            for (int j = *front; j < *rear + 1; j++)
            {
                printf("%s (%d)\n", ready_queue[j].name, ready_queue[j].priority);
            }
            printf("Runned process: %s (%d)\n", ready_queue[i].name, ready_queue[i].priority);
            if (clock_gettime(CLOCK_MONOTONIC, &ready_queue[i].start_time) == -1)
            {
                perror("SimpleScheduler.c: clock_gettime");
                exit(EXIT_FAILURE);
            }
            if (kill(ready_queue[i].pid, SIGCONT) == -1)
            {
                perror("SimpleScheduler.c: SIGCONT failed\n");
                exit(1);
            }
            process p = remove_process(ready_queue[i]);
            p.current_cycle = CPU_CYCLES + 1;
            p.wait_time += (p.current_cycle - p.prev_cycle - 1) * TSLICE;
            p.prev_cycle = p.current_cycle;
            p.state = RUNNING;
            add_process_r(p);
        }
        else
        {
            printf("Ready queue is empty\n");
            break;
        }
    }
}