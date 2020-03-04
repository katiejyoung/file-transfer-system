
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 2000 // Maximum command length
#define MAXARG 500 // Maximum number of commands
#define uName "Admin\n" // Global variables for username and password
#define pWord "monkeys3\n"

char* getInput();
int getUserPass(char* charArray[MAXARG], char input[MAXLINE]);
int parseInput(char* charArray[MAXARG], char input[MAXLINE]);
void changeDir(char* charArray[MAXARG], int numArgs);
char* getCWD();

int main(int argc, char *argv[]) {
    char *cwd; // Save current working directory to variable
    cwd = getCWD();

    int validUserPass = 0;

    char *usrIn; // User input string to store user command
    char* argArray[MAXARG]; // Array of arguments from user input
    int argCount; // Count of arguments in argArray

    char *fileIn; // File input string to incoming data

    while (1) {

        // Initiate Server
        // Wait for connection

        // Request username and password and validate
        // validUserPass = getUserPass(argArray, usrIn); 
        // while (!validUserPass) {
        //     printf("Invalid user/password combination. Please try again.\n");
        //     fflush(stdout);
        //     validUserPass = getUserPass(argArray, usrIn);
        // }

        // If valid, request desired action
        // User may opt to change directory or send a file

        // Move this prompt to client when implemented
        // printf("Current working directory: %s\nSend \'cd\' followed by a new path to change directory, \'-l\' to list CWD, or enter \'-g\' followed by a file to initiate file transmission.\n", cwd);
        // fflush(stdout); // Flush output

        usrIn = getInput(); // Generate user input
        argCount = parseInput(argArray, usrIn); // Parse input into separate arguments
        if (strcmp(argArray[0], "cd") == 0) {
            printf("Changing directory to: %s\n", argArray[1]); fflush(stdout);
            changeDir(argArray, argCount); // Change directory
            cwd = getCWD(); // Update working directory value
        }
        else if (strcmp(argArray[0], "-l") == 0) {
            printf("Current working directory: %s\n", cwd); fflush(stdout); // Flush output
        }
        else if (strcmp(argArray[0], "-g") == 0) {
            if (argCount < 2) {
                printf("Error: invalid directory.\n"); fflush(stdout);
            }
        }
        else {
            printf("Error: Invalid command.\n"); fflush(stdout);
        }

        // Close connection
    
    }
}

// Reads user input from client
// Returns input string
char* getInput() {
    // Allocate memory for string
    size_t bufLen = MAXLINE;
    char* buffer = (char *)malloc(bufLen * sizeof(char));
    memset(buffer, '\0', MAXLINE);
    fflush(stdin);

    // Loop until valid input gathered
    do {
        getline(&buffer, &bufLen, stdin);
    } while ((strlen(buffer) > (MAXLINE)) || (strlen(buffer) == 0)); // Make sure that original input meets max character requirements

    // Return input string
    return buffer;
}

// Validates login credentials
int getUserPass(char* charArray[MAXARG], char input[MAXLINE]) {
    printf("Please enter your username: ");
    fflush(stdout); // Flush output
    memset(&charArray[0], 0, sizeof(charArray)); // Clear input argument array
    input = getInput(); // Generate user input

    // Proceed if valid uName
    if (strcmp(input, uName) == 0) {
        printf("Please enter your password: ");
        fflush(stdout); // Flush output
        memset(&charArray[0], 0, sizeof(charArray)); // Clear input argument array
        input = getInput(); // Generate user input
    }
    else {
        return 0;
    }

    // Validate pWord
    if (strcmp(input, pWord) != 0) {
        return 0;
    }
    else {
        return 1;
    }
}

// Parses passed input and separates commands from space and newline character(s)
// Places each command in passed array
// Returns array length integer
int parseInput(char* charArray[MAXARG], char input[MAXLINE]) {
    char* token; // Character array variable for chunk of text
    char* rest = input; // Copy of passed character array variable
    int argCount = 0; // Counter for array element iteration
  
    // Separate text from whitespace and newline characters
    while ((token = strtok_r(rest, " \n", &rest))) {
        // Append to array and increment array count
        charArray[argCount] = token;
        argCount = argCount + 1;
    }

    // Return array size
    return argCount;
}

// Changes working directory to home or to specified path, if provided
void changeDir(char* charArray[MAXARG], int numArgs) {
    if (numArgs > 1) {
        if (chdir(charArray[1]) != 0) { // Check for failure to change directory
            perror("changeDir() failed.");
        }
    }
    else {
        printf("No path specified.\n"); fflush(stdout); // Flush output
    }
}

char* getCWD() {
    size_t bufLen = MAXLINE;
    char* buffer = (char *)malloc(bufLen * sizeof(char));
    memset(buffer, '\0', MAXLINE);

    char newDir[MAXLINE];
    if (getcwd(newDir, sizeof(newDir)) == NULL) {
        perror("Directory error");
    }

    strcpy(buffer, newDir);

    return buffer;
}