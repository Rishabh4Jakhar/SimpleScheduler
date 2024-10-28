#include "scheduler.h"

// Creating the process
process create_process(char *name, int priority)
{
    process p;
    if (&p == NULL)
    {
        perror("queue.c: Unable to create process\n");
        return p;
    }
    p.pid = 0;
    p.state = READY;
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';
    p.execution_time = 0;
    p.wait_time = 0;
    p.prev_cycle = 0;
    p.priority = priority;
    return p;
}

// Deleting all values from the process table after the scheduler ends
void empty_process_table()
{
    for (int i = 0; i < *total_processes; i++)
    {
        memset(&process_table[i], 0, sizeof(struct Process));
    }
    *total_processes = 0;
}

// Adding process at the end of the running queue
void add_process_r(process p)
{
    (*rear_r)++;
    running_queue[*rear_r] = p;
}

// Adding process at the end of the ready queues according to their priority
void add_process(process p)
{
    int pr = p.priority;
    // Ready queue priority level 1 (HIGHEST)
    if (pr == 1)
    {
        (*rear1)++;
        ready_queue1[*rear1] = p;
    }
    // Ready queue priority level 2
    else if (pr == 2)
    {
        (*rear2)++;
        ready_queue2[*rear2] = p;
    }
    // Ready queue priority level 3
    else if (pr == 3)
    {
        (*rear3)++;
        ready_queue3[*rear3] = p;
    }
    // Ready queue priority level 4 (LOWEST)
    else if (pr == 4)
    {
        (*rear4)++;
        ready_queue4[*rear4] = p;
    }
}

// Adding the process at the end of the process table
void add_process_table(process p)
{
    process_table[*total_processes] = p;
    (*total_processes)++;
}

// Checking if all ready queues and running queues are empty
// This indicates that all processes have terminated and the scheduler should end
bool is_empty()
{
    return (*front1 > *rear1 && *front2 > *rear2 && *front3 > *rear3 && *front4 > *rear4 && *front_r > *rear_r);
}

// Remove process from the ready queues according to their priority level
process remove_process(process p)
{
    int pr = p.priority;
    if (pr == 1)
    {
        if (*front1 > *rear1)
        {
            process dummy = create_process("None", 1);
            dummy.pid = -1;
            return dummy;
        }
        p = ready_queue1[*front1];
        (*front1)++;
    }
    else if (pr == 2)
    {
        if (*front2 > *rear2)
        {
            process dummy = create_process("None", 2);
            dummy.pid = -1;
            return dummy;
        }
        p = ready_queue2[*front2];
        (*front2)++;
    }
    else if (pr == 3)
    {
        if (*front3 > *rear3)
        {
            process dummy = create_process("None", 3);
            dummy.pid = -1;
            return dummy;
        }
        p = ready_queue3[*front3];
        (*front3)++;
    }
    else if (pr == 4)
    {
        if (*front4 > *rear4)
        {
            process dummy = create_process("None", 4);
            dummy.pid = -1;
            return dummy;
        }
        p = ready_queue4[*front4];
        (*front4)++;
    }
    return p;
}

// Removing process from the running queue
process remove_process_r(process p)
{
    if (*front_r > *rear_r)
    {
        process dummy = create_process("None", p.priority);
        dummy.pid = -1;
        return dummy;
    }
    p = running_queue[*front_r];
    (*front_r)++;
    return p;
}