# OS Assignment 3

## Simple Scheduler
 - A simple scheduler implemented in c .
 - It runs a NCPU number of processes and after TSLICE millisecond, it stops the execution of running  processes and runs the NCPU processes. It follows a round-robin scheduling policy.
- It is repeated till the process terminates .
- NCPU and TSLICE (in milliseconds) are taken as command line parameters along with shell.
- Timer interrupts are generated to raise SIGALRM signal after every TSLICE millisecond which signals the scheduler to switch the process.
- Use of signals such as SIGSTOP and SIGCONT is done for passing and resuming the child process.
- The simple scheduler is daemon ,that is it is a background process that runs simultaneously along with  the shell.It starts scheduling only when SIGCONT is passed to it  .
- After pressing ctrl+c, job completion stats and history of commands is shown.
- Priority is 1 being the highest priority and 4 being the lowest priority.
- `submit ./a.out <priority=1>`
- 4 separate queues are created to handle the priority
- **NON-PREEMPTIVE PRIORITY** scheduling is followed
- If a process of higher priority arrives before the process of lower priority completes its TSLICE, it will wait and will start executing only in the next TSLICE


## Contributions :
- `Rishabh Jakhar (2023435)` - Made simple-scheduler, timer and queue
- `Uday Pandita (2023563)` - Updated assignment 2 shell code to suit this assignment, made readme and example executables (helloworld.c, fib.c) and dummy_main.h

## Github repository link:

[Github Repo](https://github.com/Rishabh4Jakhar/SimpleScheduler)
