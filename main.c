#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>

const int MAX = 100;
int current_pid = -1;
static volatile int keepRunning = 1;

void intHandler(int dummy)
{
    if(current_pid != -1)
    {
        kill(current_pid , SIGKILL);
    }
}

bool StringPrefixCompare(char arr[] , char *target , int sz)
{
    int idx = 0;
    while(idx < sz)
    {
        if(arr[idx] != target[idx])return false;
        idx++;
    }
    return true;
}

int main() {
    signal(SIGINT , intHandler);
    char command[MAX];
    char *argv[10];
    while(keepRunning)
    {

        printf("My_Shell> ");
        fgets(command , sizeof(command) , stdin);
        int idx = strcspn(command , "\n");

        command[idx] = 0;

        if(strcmp(command , "exit") == 0)break;
        if(strcmp(command , "return") == 0 || strcmp(command , "\n") == 0)continue;


        if(StringPrefixCompare(command , "cd" , 2))
        {
            char temp[98];
            for (int i = 3; i < 100; ++i) {
                temp[i - 3] = command[i];
            }
            if(chdir(temp) != 0)
            {
                printf("error in changing directory\n");
            }
            continue;
        }

        if(StringPrefixCompare(command , "cwd" , 3))
        {
            char cwd[MAX];
            printf("%s \n" , getcwd(cwd , MAX));
            continue;
        }


        int i = 0;
        argv[i] = strtok(command, " ");
        while (argv[i] != NULL) {
            i++;
            argv[i] = strtok(NULL, " ");
        }

        int child_id = fork();
        if(child_id == 0)
        {
            // child process
            current_pid = getpid();
            execvp(argv[0] , argv);
            printf("execution failed :<");
        }
        else if(child_id > 0)
        {
            // parent
            wait(NULL);
            printf("\n");

        }
        else
        {
            printf("fork failed :<");
        }
    }

    return 0;
}
