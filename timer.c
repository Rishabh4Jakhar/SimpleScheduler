// Reference: https://gist.github.com/ivan-guerra/8366162f90970490d866617021867ae1

#include "scheduler.h"

// SIGALRM Handler: Tells the process what to do after SIGALRM signal is raised
static void sigalrm_handler(int sig)
{
    (void)sig;
    int start = *front_r;
    int end = *rear_r + 1;
    for (int i = start; i < end; i++)
    {
        printf("RUNNING QUEUE --->\n");
        for (int j = *front_r; j < *rear_r + 1; j++)
        {
            printf("%s\n", running_queue[j].name);
        }
        printf("Stopped process: %s\n", running_queue[i].name);
        running_queue[i].execution_time += TSLICE;
        // Stopping the process by passing SIGSTOP signal
        if (kill(running_queue[i].pid, SIGSTOP) == -1)
        {
            perror("timer.c: SIGSTOP signal failed\n");
            exit(1);
        }
        // Removing the process from running queue, changing the state to READY and adding it to ready queue
        process p = remove_process_r(running_queue[i]);
        p.state = READY;
        add_process(p);
    }
    if (is_empty())
    {
        // Scheduler ended
        printf("CPU CYCLE: %d\n", ++CPU_CYCLES);
        CPU_CYCLES = 0;
        displayProcesses();
        num_processes = 0;
        // deleting the timer
        timer_delete(timer);
        empty_process_table();
        kill(child_pid,SIGSTOP);
        printf("Scheduler ended\n");
        printf("Back to shell. Enter command to continue\n");
    }
    else
    {
        // One CPU CYCLE ended and the next one is started
        printf("CPU CYCLE: %d\n", ++CPU_CYCLES);
        scheduler();
    }
}

// Setting the SIGALRM and handling its parameters
static int set_sigalrm(int sig, int flags, void (*handler)(int))
{
    struct sigaction action;
    action.sa_flags = flags;
    action.sa_handler = handler;
    sigset_t current_mask, set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_SETMASK, NULL, &current_mask);
    // Checking if SIGALRM is blocked and unblocking it
    if (sigismember(&current_mask, SIGALRM))
    {
        sigprocmask(SIG_UNBLOCK, &set, NULL);
    }
    return sigaction(sig, &action, NULL);
}

// Creating the timer interrupt
static int timer_handler(long long frequency_nsec)
{
    struct sigevent notif;
    struct itimerspec alarm;
    notif.sigev_notify = SIGEV_SIGNAL;
    notif.sigev_signo = SIGALRM;
    notif.sigev_value.sival_ptr = &timer;
    if (timer_create(CLOCK_REALTIME, &notif, &timer) == -1)
    {
        return -1;
    }
    // Conversion factor
    static const long long SEC_TO_NSEC = 1000000000LL;
    // Setting the intial expiration time
    alarm.it_value.tv_sec = frequency_nsec / SEC_TO_NSEC;
    alarm.it_value.tv_nsec = frequency_nsec % SEC_TO_NSEC;
    // Setting the time intervals to create a repetitive timer
    alarm.it_interval.tv_sec = frequency_nsec / SEC_TO_NSEC;
    alarm.it_interval.tv_nsec = frequency_nsec % SEC_TO_NSEC;
    if (timer_settime(timer, 0, &alarm, NULL) == -1)
    {
        return -1;
    }
    return 0;
}