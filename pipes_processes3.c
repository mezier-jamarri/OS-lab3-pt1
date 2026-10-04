#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) 
{
    // Ensure the user provided exactly one argument for grep (e.g., "28")
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        return 1;
    }

    int pipefd1[2]; // Pipe 1: cat -> grep
    int pipefd2[2]; // Pipe 2: grep -> sort
    pid_t pid1, pid2;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL}; // argv[1] is the command line argument
    char *sort_args[] = {"sort", NULL};

    // Create the first pipe
    if (pipe(pipefd1) == -1) {
        perror("pipe1 failed");
        return 1;
    }

    // Fork the first child (P2 - grep)
    pid1 = fork();

    if (pid1 < 0) {
        perror("fork1 failed");
        return 1;
    }

    if (pid1 == 0) 
    {
        // ----------------------------------------------------
        // Inside P2 (Child - executes grep)
        // ----------------------------------------------------
        
        // Create the second pipe for grep -> sort
        if (pipe(pipefd2) == -1) {
            perror("pipe2 failed");
            return 1;
        }

        // Fork the second child (P3 - sort, Child's Child)
        pid2 = fork();

        if (pid2 < 0) {
            perror("fork2 failed");
            return 1;
        }

        if (pid2 == 0) 
        {
            // ------------------------------------------------
            // Inside P3 (Child's Child - executes sort)
            // ------------------------------------------------
            
            // Replace standard input with input end of pipefd2
            dup2(pipefd2[0], 0);

            // Close all unused pipe ends (including those inherited from P2)
            close(pipefd2[0]);
            close(pipefd2[1]);
            close(pipefd1[0]); 
            close(pipefd1[1]); 

            // Execute sort
            execvp("sort", sort_args);
            perror("execvp sort failed");
            exit(1);
        } 
        else 
        {
            // ------------------------------------------------
            // Back in P2 (Child - executes grep)
            // ------------------------------------------------
            
            // Replace standard input with input end of pipefd1
            dup2(pipefd1[0], 0);

            // Replace standard output with output end of pipefd2
            dup2(pipefd2[1], 1);

            // Close unused pipe ends
            close(pipefd1[0]);
            close(pipefd1[1]);
            close(pipefd2[0]);
            close(pipefd2[1]);

            // Execute grep
            execvp("grep", grep_args);
            perror("execvp grep failed");
            exit(1);
        }
    } 
    else 
    {
        // ----------------------------------------------------
        // Inside P1 (Parent - executes cat)
        // ----------------------------------------------------
        
        // Replace standard output with output end of pipefd1
        dup2(pipefd1[1], 1);

        // Close unused pipe ends
        close(pipefd1[0]);
        close(pipefd1[1]); // Safe to close after dup2

        // Execute cat
        execvp("cat", cat_args);
        perror("execvp cat failed");
        exit(1);
    }

    return 0;
}