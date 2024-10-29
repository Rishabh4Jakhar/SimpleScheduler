#include "scheduler.h"
#include "SimpleScheduler.c"
#include "timer.c"
#include "queue.c"
#include <errno.h>

// struct for each cmd stored in history
struct ComParam {
    char cmd[INPUT_SIZE];
    time_t start_time;
    time_t end_time;
    double duration;
    pid_t proc_pid;
};

// struct to create array for history
struct ComHist {
    struct ComParam record[HISTORY_SIZE];
    int histCount;
};

struct ComHist history;

// Function to display details of each command when the program is terminated using Ctrl + C
void disEnd() {
    printf("\nJob Completion Summary:\n");
    printf("--------------------------------\n");
    for (int i = 0; i < *total_processes; i++) {
        printf("Name: %s\nPID: %d\nCompletion Time: %.2f ms\n\n",
               process_table[i].name, process_table[i].pid,
               process_table[i].execution_time);
        printf("--------------------------------\n");
    }    
    printf("History of Commands: ")
    printf("--------------------------------\n");
    for (int i = 0; i < history.histCount; i++)
    {
        struct ComParam record = history.record[i];
        // conversion of start and end time to string structures
        struct tm *start_time_inf = localtime(&record.start_time);
        char start_time_buff[80];
        strftime(start_time_buff, sizeof(start_time_buff), "%Y-%m-%d %H:%M:%S", start_time_inf);
        struct tm *end_time_info = localtime(&record.end_time);
        char end_time_buffer[80];
        strftime(end_time_buffer, sizeof(end_time_buffer), "%Y-%m-%d %H:%M:%S", end_time_info);
        printf("%s\nProcess PID: %d\n", record.cmd, record.proc_pid);
        printf("Start time: %s\nEnd Time: %s\nProcess Duration: %f\n", start_time_buff, end_time_buffer, record.duration);
        printf("--------------------------------\n");
    }
}

// SIGINT (Ctrl + C) handler
static void my_handler(int signum) {
    printf("\nCtrl + C pressed\n");
    disEnd();
    exit(0);
}

// displaying cmd history
void disHist() {
    history.record[history.histCount].proc_pid = getpid();
    for (int i = 0; i < history.histCount + 1; i++)
    {
        printf("%d  %s\n", i + 1, history.record[i].cmd);
    }
}

// running shell cmds
int create_process_and_run(char **args) {
    int status = fork();
    if (status < 0) {
        perror("shell.c: Fork Failed");
        exit(1);
    }
    else if (status == 0)
    {
        int check = execvp(args[0], args);
        if (check == -1)
        {
            printf("%s: command not found\n", args[0]);
            exit(1);
        }
    }
    else
    {
        int child_status;
        // Wait for the child to complete
        wait(&child_status);
        if (WIFEXITED(child_status))
        {
            int exit_code = WEXITSTATUS(child_status);
        }
        else
        {
            printf("Child process did not exit normally.\n");
        }
    }
    return status;
}

// launch function
int launch(char **args)
{
    int status;
    status = create_process_and_run(args);
    if (status > 0)
    {
        history.record[history.histCount].proc_pid = status;
    }
    else
    {
        history.record[history.histCount].proc_pid = 0;
    }
    return status;
}

// taking input from the terminal
char *read_user_input()
{
    char *input = (char *)malloc(INPUT_SIZE);
    if (input == NULL)
    {
        perror("Can't allocate memory\n");
        free(input);
        exit(EXIT_FAILURE);
    }
    size_t size = 0;
    int read = getline(&input, &size, stdin);
    if (read != -1)
    {
        return input;
    }
    else
    {
        perror("Error while reading line\n");
        free(input);
        exit(1);
    }
}

// function to strip the leading and trailing spaces
char *strip(char *string)
{
    char stripped[strlen(string) + 1];
    int len = 0;
    int flag;
    if (string[0] != ' ')
    {
        flag = 1;
    }
    else
    {
        flag = 0;
    }
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != ' ' && flag == 0)
        {
            stripped[len++] = string[i];
            flag = 1;
        }
        else if (flag == 1)
        {
            stripped[len++] = string[i];
        }
        else if (string[i] != ' ')
        {
            flag = 1;
        }
    }
    stripped[len] = '\0';
    char *final_strip = (char *)malloc(INPUT_SIZE);
    if (final_strip == NULL)
    {
        perror("Memory allocation failed\n");
        exit(1);
    }
    memcpy(final_strip, stripped, INPUT_SIZE);
    return final_strip;
}

// splitting the command according to specified delimiter
char **tokenize(char *cmd, const char delim[2])
{
    char **args = (char **)malloc(INPUT_SIZE * sizeof(char *));
    if (args == NULL)
    {
        perror("Memory allocation failed\n");
        exit(1);
    }
    int count = 0;
    char *token = strtok(cmd, delim);
    while (token != NULL)
    {
        args[count++] = strip(token);
        token = strtok(NULL, delim);
    }
    return args;
}

// Checking for quotation marks and backslash in the input
bool validate_cmd(char *cmd)
{
    if (strchr(cmd, '\\') || strchr(cmd, '\"') || strchr(cmd, '\''))
    {
        return true;
    }
    return false;
}

// SIGCHLD: Invoked when the child process terminates
// Final execution time is calculated and the process is deleted from the running queue
void sigchld_handler(int signum)
{
    int status;
    pid_t pid;
    int saved_errno = errno;
    // returns the pid of the terminated child
    // WNOHANG: returns 0 if the status information of any process is not available i.e. the process has not terminated
    // In other words, the loop will only run after the background processes running has terminated or changed state
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = *front_r; i < *rear_r + 1; i++)
        {
            if (running_queue[i].pid == pid)
            {
                if (clock_gettime(CLOCK_MONOTONIC, &running_queue[i].end_time) == -1)
                {
                    perror("shell.c: clock_gettime");
                    exit(EXIT_FAILURE);
                }
                printf("Process terminated: %s\n", running_queue[i].name);
                running_queue[i].execution_time += ((running_queue[i].end_time.tv_sec - running_queue[i].start_time.tv_sec) * 1000) + (running_queue[i].end_time.tv_nsec + running_queue[i].start_time.tv_nsec) / 1e6;
                for (int k = 0; k < *total_processes; k++)
                {
                    if (process_table[k].pid == running_queue[i].pid)
                    {
                        process_table[k] = running_queue[i];
                        break;
                    }
                }
                for (int j = i; j < *rear_r; j++)
                {
                    running_queue[j] = running_queue[j + 1];
                }
                (*rear_r)--;
                break;
            }
        }
        // Updating the history
        for (int i = 0; i < history.histCount; i++)
        {
            if (history.record[i].proc_pid == pid)
            {
                history.record[i].end_time = time(NULL);
                history.record[i].duration = difftime(
                    history.record[i].end_time,
                    history.record[i].start_time);
            }
        }
    }
    errno = saved_errno;
}

// main shell loop
void shell_loop()
{
    // Setting the function for SIGINT (Ctrl + C)
    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        perror("SIGINT handling failed");
        exit(1);
    }
    // Setting the function for SIGCHLD
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR)
    {
        perror("SIGCHLD handling failed");
        exit(1);
    }
    // Setting the function for timer interrupt
    if (set_sigalrm(SIGALRM, SA_RESTART, sigalrm_handler) == -1)
    {
        perror("SIGALRM handling failed");
        exit(1);
    }
    // Creating the prompt text
    char *user = getenv("USER");
    if (user == NULL)
    {
        perror("USER environment variable not declared");
        exit(1);
    }
    char host[INPUT_SIZE];
    int hostname = gethostname(host, sizeof(host));
    if (hostname == -1)
    {
        perror("HOST not declared");
        exit(1);
    }
    int status;
    do
    {
        printf("%s@%s~$ ", user, host);
        fflush(stdout); 
        // taking input
        char *cmd = read_user_input();
        // handling the case if the input is blank or enter key
        if (strlen(cmd) == 0 || strcmp(cmd, "\n") == 0)
        {
            status = 1;
            continue;
        }
        // removing the newline character
        cmd = strtok(cmd, "\n");
        bool isInvalidcmd = validate_cmd(cmd);
        char *tmp = strdup(cmd);
        if (tmp == NULL)
        {
            perror("Error in strdup");
            exit(EXIT_FAILURE);
        }
        if (isInvalidcmd)
        {
            status = 1;
            strcpy(history.record[history.histCount].cmd, tmp);
            history.record[history.histCount].start_time = time(NULL);
            history.record[history.histCount].end_time = time(NULL);
            history.record[history.histCount].duration = difftime(
                history.record[history.histCount].end_time,
                history.record[history.histCount].start_time);
            history.histCount++;
            printf("Invalid cmd : includes quotes/backslash\n");
            continue;
        }
        // checking if the input is "history"
        if (strstr(cmd, "history"))
        {
            if (history.histCount > 0)
            {
                strcpy(history.record[history.histCount].cmd, tmp);
                history.record[history.histCount].start_time = time(NULL);
                disHist();
                history.record[history.histCount].end_time = time(NULL);
                history.record[history.histCount].duration = difftime(
                    history.record[history.histCount].end_time,
                    history.record[history.histCount].start_time);
                history.histCount++;
            }
            else
            {
                status = 1;
                printf("No cmd in the history\n");
                continue;
            }
        }
        else if (strcmp(cmd, "run") == 0)
        {
            // Starting the timer before executing the process
            if (timer_handler(TSLICE * 1e6) == -1)
            {
                perror("Timer handler");
                exit(1);
            }
            // Sending a signal to the child process to start the scheduler in the background
            kill(child_pid, SIGCONT);
        }
        else
        {
            char **args = tokenize(cmd, " ");
            strcpy(history.record[history.histCount].cmd, tmp);
            history.record[history.histCount].start_time = time(NULL);
            if (strcmp(args[0], "submit") == 0)
            {

                pid_t pid = fork();
                if (pid == 0) { // Child process
                    // Waiting for the scheduler signal before starting execution
                    printf("Process continued: %s\n", args[1]);
                    char *temp[2] = {args[1], NULL};
                    if (execvp(args[1], temp) == -1)
                    {
                        printf("Error executing %s\n", args[1]);
                        exit(1);
                    }
                }
                else if (pid > 0)
                { // Parent process
                    // Pausing the execution of the child process to where it was
                    if (kill(pid, SIGSTOP) == -1)
                    {
                        perror("shell.c: SIGSTOP signal failed\n");
                        exit(1);
                    }
                    process p;
                    // priority set
                    if (args[2] != NULL)
                    {
                        int pr = atoi(args[2]);
                        p = create_process(args[1], pr);
                        if (&p == NULL)
                        {
                            perror("Allocation failed\n");
                            exit(1);
                        }
                    }
                    else
                    {
                        // default priority = 1
                        p = create_process(args[1], 1);
                        if (&p == NULL)
                        {
                            perror("Allocation failed\n");
                            exit(1);
                        }
                    }
                    p.pid = pid;
                    // Adding the process to the ready queue and the process table
                    add_process(p);
                    add_process_table(p);
                    num_processes++;
                    // Resume the scheduler if it’s paused
                    if (kill(child_pid, SIGCONT) == -1) {
                        perror("Failed to resume scheduler");
                        exit(1);}
                }
                else
                {
                    perror("shell.c: Fork failed\n");
                    exit(1);
                }
                status = pid;
                if (status > 0)
                {
                    history.record[history.histCount].proc_pid = status;
                }
                else
                {
                    history.record[history.histCount].proc_pid = 0;
                }
            }
            else
            {
                status = launch(args);
            }
            history.record[history.histCount].end_time = time(NULL);
            history.record[history.histCount].duration = difftime(
                history.record[history.histCount].end_time,
                history.record[history.histCount].start_time);
            history.histCount++;

            free(args);
        }

        while (waitpid(-1, &status, WNOHANG) > 0) {
            printf("Child process terminated with status %d\n", status);
        }
    } while (status);
}

// Main function
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <NCPU> <TSLICE>\n", argv[0]);
        exit(1);
    }
    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);
    // Mapping the memory to create a shared memory for both parent and child
    process_table = mmap(NULL, sizeof(process) * MAX_PROCESSES, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ready_queue1 = mmap(NULL, sizeof(process) * MAX_PROCESSES, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ready_queue2 = mmap(NULL, sizeof(process) * MAX_PROCESSES, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ready_queue3 = mmap(NULL, sizeof(process) * MAX_PROCESSES, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ready_queue4 = mmap(NULL, sizeof(process) * MAX_PROCESSES, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    running_queue = mmap(NULL, sizeof(process) * MAX_PROCESSES, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    front1 = mmap(NULL, sizeof(front1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    front2 = mmap(NULL, sizeof(front1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    front3 = mmap(NULL, sizeof(front1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    front4 = mmap(NULL, sizeof(front1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    front_r = mmap(NULL, sizeof(front1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    rear1 = mmap(NULL, sizeof(rear1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    rear2 = mmap(NULL, sizeof(rear1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    rear3 = mmap(NULL, sizeof(rear1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    rear4 = mmap(NULL, sizeof(rear1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    rear_r = mmap(NULL, sizeof(rear1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    total_processes = mmap(NULL, sizeof(total_processes), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // initializing count for elements in history
    history.histCount = 0;
    int pid = fork();
    if (pid < 0)
    {
        perror("shell.c: Fork failed\n");
        exit(1);
    }
    else if (pid == 0)
    {
        // Child process: Running the scheduler
        printf("Starting the scheduler\n");
        scheduler();
        exit(0);
    }
    else
    {
        // Parent process: Running the shell
        // Initialising the values
        *front1 = 0;
        *rear1 = -1;
        *front2 = 0;
        *rear2 = -1;
        *front3 = 0;
        *rear3 = -1;
        *front4 = 0;
        *rear4 = -1;
        *front_r = 0;
        *rear_r = -1;
        *total_processes = 0;
        child_pid = pid;
        // Pausing the child
        if (kill(pid, SIGSTOP) == -1){
            perror("shell.c: SIGSTOP failed\n");
            exit(1);
        }
        shell_loop();
    }
    return 0;
}