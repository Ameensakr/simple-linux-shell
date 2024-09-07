#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX 1024
#define MAX2 64
int current_pid = -1;
static volatile int keepRunning = 1;
int running_background_process[MAX2];
int number_of_active_background = 0;


void put_in_running_background_process(int pid)
{
    for (int i = 0; i < MAX2; ++i) {
        if(running_background_process[i] == -1)
        {
            running_background_process[i] = pid;
            return;
        }
    }
    printf("Memory is full with process\n");
}
void kill_all()
{
    for (int i = 0; i < MAX2; ++i) {
        if(running_background_process[i] != -1)
        {
            kill(running_background_process[i] , SIGKILL);
        }
    }
}

void intHandler(int dummy) {
    if (current_pid != -1) {
        kill(current_pid, SIGKILL);
    }
}

bool StringPrefixCompare(char arr[], char *target, int sz) {
    int idx = 0;
    while (idx < sz) {
        if (arr[idx] != target[idx])return false;
        idx++;
    }
    return true;
}

char command[MAX];
int number_of_words_in_command = 0;

bool is_background_command(char *argv[]) {
    // -1 because it is 0-based
    if (argv[number_of_words_in_command - 1][0] == '&') return true;
    return false;
}


void reap_background_processes() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("%d \n" , pid);
        for (int i = 0; i < MAX2; ++i) {
            if(running_background_process[i] == pid)
            {
                running_background_process[i] = -1;
                break;
            }
        }
        printf("Shell: Background process finished\n");
    }
}

int main() {
    memset(running_background_process , -1 , sizeof running_background_process);
    signal(SIGINT, intHandler);

    char *argv[10];
    while (keepRunning) {

        printf("My_Shell> ");
        fgets(command, sizeof(command), stdin);
        int idx = strcspn(command, "\n");
        number_of_words_in_command = 0;

        command[idx] = 0;

        if (strcmp(command, "exit") == 0)
        {
            kill_all();
            break;
        }
        if (strcmp(command, "return") == 0 || strcmp(command, "\n") == 0)continue;


        if (StringPrefixCompare(command, "cd", 2)) {
            char temp[98];
            for (int i = 3; i < 100; ++i) {
                temp[i - 3] = command[i];
            }
            if (chdir(temp) != 0) {
                printf("error in changing directory\n");
            }
            continue;
        }

        if (strcmp(command, "cwd") == 0) {
            char cwd[MAX];
            printf("%s \n", getcwd(cwd, MAX));
            continue;
        }


        argv[number_of_words_in_command] = strtok(command, " ");
        while (argv[number_of_words_in_command] != NULL) {
            number_of_words_in_command++;
            argv[number_of_words_in_command] = strtok(NULL, " ");
        }


        bool is_background_command_v = false;
        if (is_background_command(argv)) {
            // here I will erase the '&' from the command
            argv[number_of_words_in_command - 1] = NULL;
            number_of_words_in_command--;
            is_background_command_v = true;
        }



        int child_id = fork();
        if (child_id == 0) {
            // child process
            put_in_running_background_process(getpid());
            current_pid = getpid();
            execvp(argv[0], argv);
            printf("execution failed :<\n");
        } else if (child_id > 0) {
            // parent
            if (!is_background_command_v) {
                wait(NULL);
            }

            printf("\n");

        } else {
            printf("fork failed :<");
        }

        reap_background_processes();

    }

    return 0;
}
