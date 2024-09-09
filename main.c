#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char **tokenize(char *line)
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for(i =0; i < strlen(line); i++){

        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
            token[tokenIndex] = '\0';
            if (tokenIndex != 0){
                tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0;
            }
        } else {
            token[tokenIndex++] = readChar;
        }
    }

    free(token);
    tokens[tokenNo] = NULL ;
    return tokens;
}


int current_pid = -1;
static volatile int keepRunning = 1;
int running_background_process[MAX_TOKEN_SIZE];


void put_in_running_background_process(int pid)
{
    for (int i = 0; i < MAX_TOKEN_SIZE; ++i) {
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
    for (int i = 0; i < MAX_TOKEN_SIZE; ++i) {
        if(running_background_process[i] != -1)
        {
            kill(running_background_process[i] , SIGKILL);
        }
    }
}

void intHandler(int dummy) {

    if (current_pid != -1) {
        kill(-current_pid, SIGKILL);
        current_pid = -1;
    }

}

void reap_background_processes() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_TOKEN_SIZE; ++i) {
            if(running_background_process[i] == pid)
            {
                running_background_process[i] = -1;
                break;
            }
        }
        printf("Shell: Background process finished\n");
    }
}

int main(int argc, char* argv[]) {
    memset(running_background_process , -1 , sizeof running_background_process);
    signal(SIGINT, intHandler);

    char  line[MAX_INPUT_SIZE];
    char  **tokens;
    int i;


    while(keepRunning) {
        reap_background_processes();
        bzero(line, sizeof(line));
        printf("$ ");
        scanf("%[^\n]", line);
        getchar();

        line[strlen(line)] = '\n';
        tokens = tokenize(line);
        if(tokens[0] == NULL)continue;
        if (strcmp(tokens[0] , "exit") == 0)
        {
            kill_all();
            break;
        }

        if (strcmp(tokens[0], "return") == 0 || strcmp(tokens[0], "\n") == 0)continue;

        if (strcmp(tokens[0] , "cd") == 0) {
            if (tokens[1] != NULL && chdir(tokens[1]) == 0) {
            }
            else
            {
                printf("error in changing directory\n");
            }
            continue;
        }
        if (strcmp(tokens[0], "cwd") == 0) {
            char cwd[MAX_INPUT_SIZE];
            printf("%s \n", getcwd(cwd, MAX_INPUT_SIZE));
            continue;
        }


        int sz = 0;
        while(tokens[sz] != NULL)sz++;
        bool is_background_command_v = false;
        if(tokens[sz - 1][0] == '&')
        {
            tokens[sz - 1] = NULL;
            sz--;
            is_background_command_v = true;
        }

        int child_id = fork();
        if (child_id == 0) {
            // child process
            setpgid(0, 0);
            execvp(tokens[0] , tokens);
            current_pid = getpid();
            printf("execution failed :<\n");
        } else if (child_id > 0) {
            // parent
            if (!is_background_command_v) {
                current_pid = child_id;
                setpgid(child_id, child_id);
                wait(NULL);
                current_pid = -1;
            }
            else
            {
                put_in_running_background_process(getpid());
            }
        } else {
            printf("fork failed :<");
        }


        for(i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);

    }
    return 0;
}