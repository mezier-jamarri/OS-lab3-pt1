// C program to demonstrate use of fork() and pipe() 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main() 
{ 
    int fd1[2]; 
    int fd2[2]; 

    char fixed_str1[] = "howard.edu"; 
    char fixed_str2[] = "gobison.org"; 
    char input_str[100]; 
    char second_input[100]; 

    pid_t p; 

    if (pipe(fd1) == -1) 
    { 
        fprintf(stderr, "Pipe 1 Failed\n"); 
        return 1; 
    } 
    if (pipe(fd2) == -1) 
    { 
        fprintf(stderr, "Pipe 2 Failed\n"); 
        return 1; 
    } 

    // FIXED: Changed prompt to exactly "Input : " to match sample output
    printf("Input : ");
    // FIXED: Used fgets instead of scanf for buffer safety
    if (fgets(input_str, sizeof(input_str), stdin) != NULL) {
        input_str[strcspn(input_str, "\n")] = '\0'; // Strip the newline character
    }

    p = fork(); 

    if (p < 0) 
    { 
        fprintf(stderr, "Fork Failed\n"); 
        return 1; 
    } 

    // Parent process (P1)
    else if (p > 0) 
    { 
        char concat_str[200]; 

        close(fd1[0]); // Close reading end of first pipe 
        close(fd2[1]); // Close writing end of second pipe

        // Write input string to child
        write(fd1[1], input_str, strlen(input_str) + 1); 
        close(fd1[1]); 

        // Wait for child to finish execution
        wait(NULL); 

        // Read concatenated string from child
        read(fd2[0], concat_str, 200); 
        
        // FIXED: Used strncat for bounds-checked string concatenation
        strncat(concat_str, fixed_str2, sizeof(concat_str) - strlen(concat_str) - 1); 

        // Final output from P1 only
        printf("Output : %s\n", concat_str); 

        close(fd2[0]); 
    } 

    // Child process (P2)
    else
    { 
        char concat_str[200]; 

        close(fd1[1]); // Close writing end of first pipe 
        close(fd2[0]); // Close reading end of second pipe

        // Read string from parent
        read(fd1[0], concat_str, 200); 
        close(fd1[0]); 

        // Append "howard.edu" safely
        strncat(concat_str, fixed_str1, sizeof(concat_str) - strlen(concat_str) - 1); 

        // First output from P2 only
        printf("Output : %s\n", concat_str); 

        // FIXED: Used exactly "Input : " for the second prompt
        printf("Input : ");
        if (fgets(second_input, sizeof(second_input), stdin) != NULL) {
            second_input[strcspn(second_input, "\n")] = '\0'; // Strip the newline character
        }

        // Append second input safely
        strncat(concat_str, second_input, sizeof(concat_str) - strlen(concat_str) - 1);

        // Send back to parent
        write(fd2[1], concat_str, strlen(concat_str) + 1); 
        close(fd2[1]); 

        exit(0); 
    } 
    return 0; 
}