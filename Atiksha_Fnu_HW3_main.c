/**************************************************************
* Class::  CSC-415-02 Spring 2026
* Name:: Fnu Atiksha
* Student ID:: 923641508
* GitHub-Name:: atiksha05
* Project:: Assignment 3 – Simple Shell with Pipes
*
* File:: Atiksha_Fnu_HW3_main.c
*
* Description:: A simple shell that reads commands from the user,
*               parses them and runs them using fork and execvp.
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 85 //assignment says max 85 bytes for input
#define MAX_ARGS 10   //max arguments per command (assignment says at least 4+1)
#define MAX_PIPES 10  // max commands chained together with 


/* Breaks a command string into individual argument tokens
 * because execvp needs an arrray of separate strings, not 
 * one big string
 * The NULL at the end tells execvp where the argyments stop
 */


int parseCommand(char *cmd, char **args) {
    int count = 0;

    /* get first token - spaces and tabs are both valid separators between argyments
    */
    char *token = strtok(cmd, " \t");

    while (token !=NULL && count < MAX_ARGS) {
        args[count] = token;
        count++;

        /*NULL tells strtok to continue from where it left off in the
        same string*/
        token = strtok(NULL, " \t");
    }

    /* excevp required NULL as the last element to know where the arument list ends*/
    args[count] = NULL;

    return count;
}


/* RUns a single command
 * shell itself is never replaced. parent waits and 
 * reports the result so that the user knows if the 
 * command succeeded or failed
 */
void executeSingle(char **args) {
    int status;

    /*child process gets 0, parents gets child PID,
    negative means fork itself fail*/
    pid_t pid = fork();

    if (pid < 0) {
        /*fork failing means operating system couldnt create a new 
        process*/
        perror("Fork failed");
        return;

    }

    if (pid == 0){
        /*we are in the child*/
        /*replace ourselves with the requested command. execvp searches PATH so full
        path is not required*/

        execvp(args[0], args);

        /*execvp only returns if it failed*/
        perror(args[0]);
        exit(1);
    }

    /* we are in the parent waiting for the child to finish
    before accepting the next command*/
    waitpid(pid, &status, 0);

    /* print PID and exit code so user knows what ran and whether
    it succeeded */
    printf("Child %d finished with status: %d\n", pid, 
    WEXITSTATUS(status));

}



/*we need N-1 pipes for N commands
 *Each child gets its stdin/stdout redirected through dup2
 *before exec so data flows hrough the pipe chain automatically 
 */

void executePiped(char ***commands, int numCommands) {
    int pipes[MAX_PIPES][2];
    pid_t pids[MAX_PIPES];
    int status;

    /*create all pipes upfront before forking so every child 
    can access all pipe file descriptors*/

    for (int i = 0; i < numCommands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe failed");
            return;
        }
    }

    for (int i = 0; i < numCommands; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork failed");
            return;
        }

        if (pids[i] == 0) {
            /* if not first command read from previous pipe */
            if (i>0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            /* if not last command, write to next pipe */
            if (i < numCommands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);

            }

            /* close all pipe ends in child */
            for (int j = 0; j < numCommands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);

            }

            execvp(commands[i][0], commands[i]);
            perror(commands[i][0]);
            exit(1);
        }
    }

    /* parent closes all pipes so children detect EOF
    and dont hang waiting for more inputs*/
    for (int i = 0; i < numCommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);

    }

    for (int i = 0; i < numCommands; i++) {
        waitpid(pids[i], &status, 0);
        printf("Child %d finished with status: %d\n", 
            pids[i], WEXITSTATUS(status));
    }

}



int main(int argc, char *argv[]) {
    /* if a promt was passed as argument use it, otherwise default to "> "
    as the assignment requires */

    char *prompt = (argc > 1) ? argv[1] : "> ";

    /*buffer to store the raw input - 85 bytes as required in the assignment
    to prevent the overflow*/
    char input[BUFFER_SIZE];
    char *commands[MAX_PIPES][MAX_ARGS + 1];
    char **commandPtrs[MAX_PIPES];
    int numCommands = 0;

    /* keeep the shell running until user exits*/
    while (1) {
        printf("%s", prompt);

        /* force promt to appear before user types
        printf buffers output until newline without this*/
        fflush(stdout);

        /* fgets is usede because it respects buffer size unlike gets returns
        NULL on both EOP and error so we check
        feof to tell the difference*/
        if (fgets(input, BUFFER_SIZE, stdin) ==NULL) {
        if (feof(stdin)) {

            /*EOF means exit cleanly*/
            printf("\n");
            exit(0);


        } else {

            /*Actual read failure - report and exit */
            perror("Error reading input");
            exit(1);

        }

    }


    /*remove trailing newline fgets includes so our
    string comparisons work correctly*/
    input[strcspn(input, "\n")] = '\0';

    /*Empty input is useleess to process tell user and ask again*/
    if (strlen(input) ==0) {
        fprintf(stderr, "Error: no command entered\n");
        continue;
    }

    /* user wants to leave the shell*/
    if (strcmp(input, "exit") ==0) {
        exit(0);
    }

    char bufferCopy[BUFFER_SIZE];
    numCommands = 0;
    strncpy(bufferCopy, input, BUFFER_SIZE);

    char *segment = strtok(bufferCopy, "|");
    while (segment != NULL && numCommands < MAX_PIPES) {
        int count = parseCommand(segment, commands[numCommands]);
        if (count == 0) {
            fprintf(stderr, "Error: empty command in pipe\n");
            break;
        }
        commandPtrs[numCommands] = commands[numCommands];
        numCommands++;
        segment = strtok(NULL, "|");
    }

    /* single command needs no pipe setup at all */
    if (numCommands == 1) {
        executeSingle(commands[0]);
    } else if (numCommands > 1) {
        executePiped(commandPtrs, numCommands);
    }

}
    return 0;

}

