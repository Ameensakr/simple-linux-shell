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
int running_in_parallel_mode[MAX_TOKEN_SIZE];
int interrupted = 0;



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
        interrupted = 1;
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

void execute_command(char **command, bool is_background_command_v)
{
    int child_id = fork();
    if (child_id == 0) {
        // child process
        setpgid(0, 0);
        execvp(command[0] , command);
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
}

char ***parse_commands(char *line, const char *delimiter) {
    char ***commands = (char ***) malloc(MAX_NUM_TOKENS * sizeof(char **));
    char **tokens = tokenize(line); // Tokenize the input line
    int commandIndex = 0;
    int tokenIndex = 0;
    commands[commandIndex] = (char **) malloc(MAX_NUM_TOKENS * sizeof(char *)); // Allocate memory for the command
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], delimiter) == 0) { // Check if the token is the delimiter
            commands[commandIndex][tokenIndex] = NULL; // Terminate the command with NULL
            commands[++commandIndex] = (char **) malloc(MAX_NUM_TOKENS * sizeof(char *));
            tokenIndex = 0;
        } else {
            commands[commandIndex][tokenIndex++] = tokens[i]; // Add the token to the current command
        }
    }
    commands[commandIndex][tokenIndex] = NULL;
    commands[commandIndex + 1] = NULL;
    free(tokens);
    return commands;
}
void wait_for_all_parallel_process()
{
    for (int i = 0; i < MAX_TOKEN_SIZE; ++i) {
        if(running_in_parallel_mode[i] != -1)
        {
            waitpid(running_in_parallel_mode[i], NULL, 0);
            running_in_parallel_mode[i] = -1;
        }
    }
}
void put_in_parallel_background_process(int pid)
{
    for (int i = 0; i < MAX_TOKEN_SIZE; ++i) {
        if(running_in_parallel_mode[i] == -1)
        {
            running_in_parallel_mode[i] = pid;
            return;
        }
    }
    printf("Memory is full with process\n");
}
void execute_parallel_process(char ***command)
{
    for (int i = 0; command[i] != NULL; ++i) {
        int child_id = fork();
        if(child_id == 0)
        {
            execvp(command[i][0] , command[i]);
            printf("execution failed\n");
        }
        else if(child_id > 0)
        {
            put_in_parallel_background_process(child_id);
        }
        else printf("fork failed\n");
    }
    wait_for_all_parallel_process();

}

int number_of_background()
{
    int cnt = 0;
    for (int i = 0; i < MAX_TOKEN_SIZE; ++i) {
        cnt += running_background_process[i] != -1;
    }
    return cnt;
}


int main(int argc, char* argv[]) {
    memset(running_background_process , -1 , sizeof running_background_process);
    memset(running_in_parallel_mode , -1 , sizeof running_in_parallel_mode);
    signal(SIGINT, intHandler);


    char  line[MAX_INPUT_SIZE];
    char  **tokens;
    int i;


    while(keepRunning) {
        if(number_of_background() > 0)
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
        bool is_serial = false , is_parallel = false;
        for (int j = 0; j < sz; ++j) {
            if(strcmp(tokens[j] , "&&") == 0)
            {
                is_serial = true;
                break;
            }
            if(strcmp(tokens[j] , "&&&") == 0)
            {
                is_parallel = true;
                break;
            }
        }
        bool is_background_command_v = false;

        if(strcmp(tokens[sz - 1] ,"&") == 0)
        {
            tokens[sz - 1] = NULL;
            sz--;
            is_background_command_v = true;
        }
        if(is_background_command_v)
        {
            execute_command(tokens , true);
            continue;
        }
        else if(is_serial)
        {
            char *** command = parse_commands(line ,"&&");
            for (int j = 0; command[j] != NULL; ++j) {
                if(interrupted)break;
                execute_command(command[j] , 0);
            }
            free(command);

        }
        else if(is_parallel)
        {
            char *** command = parse_commands(line ,"&&&");
            execute_parallel_process(command);
            free(command);
        }


        for(i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);

    }
    return 0;
}