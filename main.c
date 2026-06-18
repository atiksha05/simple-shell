/**************************************************************
 * Simple Shell
 *
 * A Unix-style command-line shell implemented in C.
 * Supports command execution, process creation, and pipelines.
 *
 * Author: Atiksha Fnu
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 85
#define MAX_ARGS 10
#define MAX_PIPES 10

int parseCommand(char *cmd, char **args) {
    int count = 0;
    char *token = strtok(cmd, " \t");

    while (token != NULL && count < MAX_ARGS) {
        args[count] = token;
        count++;
        token = strtok(NULL, " \t");
    }

    args[count] = NULL;
    return count;
}

void executeSingle(char **args) {
    int status;
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        execvp(args[0], args);
        perror(args[0]);
        exit(1);
    }

    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("Child %d finished with status: %d\n",
               pid, WEXITSTATUS(status));
    }
}

void executePiped(char ***commands, int numCommands) {
    int pipes[MAX_PIPES][2];
    pid_t pids[MAX_PIPES];
    int status;

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
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            if (i < numCommands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < numCommands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(commands[i][0], commands[i]);
            perror(commands[i][0]);
            exit(1);
        }
    }

    for (int i = 0; i < numCommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < numCommands; i++) {
        waitpid(pids[i], &status, 0);

        if (WIFEXITED(status)) {
            printf("Child %d finished with status: %d\n",
                   pids[i], WEXITSTATUS(status));
        }
    }
}

int main(int argc, char *argv[]) {
    char *prompt = (argc > 1) ? argv[1] : "> ";

    char input[BUFFER_SIZE];
    char bufferCopy[BUFFER_SIZE];
    char *commands[MAX_PIPES][MAX_ARGS + 1];
    char **commandPtrs[MAX_PIPES];

    while (1) {
        int numCommands = 0;

        printf("%s", prompt);
        fflush(stdout);

        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            if (feof(stdin)) {
                printf("\n");
                exit(0);
            }

            perror("error reading input");
            exit(1);
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            fprintf(stderr, "Error: no command entered\n");
            continue;
        }

        if (strcmp(input, "exit") == 0) {
            exit(0);
        }

        strncpy(bufferCopy, input, BUFFER_SIZE);
        bufferCopy[BUFFER_SIZE - 1] = '\0';

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

        if (numCommands == 1) {
            executeSingle(commands[0]);
        } else if (numCommands > 1) {
            executePiped(commandPtrs, numCommands);
        }
    }

    return 0;
}