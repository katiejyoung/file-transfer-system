
// Katie Young
// CS 372, Program 2: File Transport System
// Last modified: 3/8/2020

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MAXLINE 2000 // Maximum command length
#define MAXARG 500 // Maximum number of commands

void acceptedConnection(int socketFD);
int validateUserPass(int establishedConnectionFD);
void getCommand(int establishedConnectionFD);
void getClientInput(char* newString, int establishedConnectionFD);
int sendToClient(char* charsToSend, int establishedConnectionFD);
char* appendLength(char* charsToSend);
int sendFileSize(int fileSize, int establishedConnectionFD);
char* getCWD();
int changeDir(char* charArray[MAXARG], int numArgs, char* charsToSend);
int parseInput(char* charArray[MAXARG], char input[MAXLINE]);
void transferFile(char* charArray[MAXARG], int establishedConnectionFD);

int main(int argc, char *argv[]) {
    int portNumber;
    int listenSocketFD, establishedConnectionFD;
    struct sockaddr_in serverAddress;

    // Check argument length
    if (argc != 2) { fprintf(stderr,"too few arguments\n", argv[0]); exit(2); }

    // Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    // Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

    // Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to port
		perror("Error: on binding");
		exit(2);
	}

    listen(listenSocketFD, 10); // Flip the socket on - it can now receive up to 10 connections
    acceptedConnection(listenSocketFD); // Continue to program

    close(listenSocketFD); // Close port connection
    return 0;
}

// Accepts new connection and calls appropriate functions
void acceptedConnection(int socketFD) {
    socklen_t sizeOfClientInfo;
    struct sockaddr_in clientAddress;
    int establishedConnectionFD;
    int validUserPass;
    sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect

    while (1) {
        establishedConnectionFD = accept(socketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept client connection
        if (establishedConnectionFD < 0) error("ERROR on accept");
        validUserPass = 0; // Reset username/password validation tracker

        printf("Connected to client...\n\n"); fflush(stdout);

        // Validate username and password
        do {
            validUserPass = validateUserPass(establishedConnectionFD);
        } while (!validUserPass);

        getCommand(establishedConnectionFD); // Move on to command portion of program

        close(establishedConnectionFD); // Close client connection
        establishedConnectionFD = -1;
    }
}

// Validates username and password
// Returns 1 if combination is valid and 0 if not
int validateUserPass(int establishedConnectionFD) {
    char uName[] = "Admin";
    char uPass[] = "yaynetworking";
    char *invalidUserPass = "invalid\n";
    char *proceedConnection = "proceed\n";
    char nameInput[1001];
    char passInput[1001];
    char *nameIn = &nameInput[0];
    char *passIn = &passInput[0];
    int charsSent;

    // Get username and password info from client
    getClientInput(nameIn, establishedConnectionFD);
    getClientInput(passIn, establishedConnectionFD);

    // Compare new values with valid credentials and respond to client accordingly
    if ((strcmp(nameIn, uName) != 0) || (strcmp(passIn, uPass) != 0)) {
        charsSent = sendToClient(invalidUserPass, establishedConnectionFD);
        strcpy(nameIn, "");
        strcpy(passIn, "");
        return 0;
    }
    else {
        strcpy(nameIn, "");
        strcpy(passIn, "");
        charsSent = sendToClient(proceedConnection, establishedConnectionFD);
    }

    return 1;
}

// Performs logic for command portion of program (e.g. "cd", "-l", "-g")
void getCommand(int establishedConnectionFD) {
    char* argArray[MAXARG];
    char clientInput[1001];
    char *clientIn = &clientInput[0];
    int isValid = 0;
    char* invalidCommand = "Error: command not found";
    char* validCommand = "Valid command";
    char* home = getCWD();
    int argCount;

    // Loop until file transfer requested
    while (1) {
        isValid = 0; // Reset client input validation boolean

        // Gather and validate client input
        while (!isValid) {
            strcpy(clientIn, "");
            getClientInput(clientIn, establishedConnectionFD); // Get input from client

            argCount = parseInput(argArray, clientIn); // Separate input into command and context

            // Validate command and respond accordingly
            if ((strcmp(argArray[0], "-l") == 0) || (strcmp(argArray[0], "-g") == 0) || (strcmp(argArray[0], "cd") == 0)) {
                sendToClient(validCommand, establishedConnectionFD);
                isValid = 1;
            }
            else {
                sendToClient(invalidCommand, establishedConnectionFD);
                argArray[0] = '\0'; // Reset argument array if command is invalid
            }
        }

        // Perform appropriate actions based on user command
        if (strcmp(argArray[0], "-l") == 0) {
            printf("Sending current working directory...\n"); fflush(stdout);

            // Get current working directory and send to client
            char *cwd = getCWD();
            strcat(cwd, "\n");
            sendToClient(cwd, establishedConnectionFD);
            printf("Working directory sent.\n\n"); fflush(stdout);
        }
        else if (strcmp(argArray[0], "-g") == 0) {
            printf("Begin file transfer of: %s\n", argArray[1]); fflush(stdout);

            // Transfer file to client, if it exists
            transferFile(argArray, establishedConnectionFD);
            break;
        }
        else if (strcmp(argArray[0], "cd") == 0) {
            printf("Changing to directory: %s\n", argArray[1]); fflush(stdout);
            char* success = "Directory change complete.\n";
            char* failure = "Failed to change directory.\n";

            // Attempt directory change and respond with success or failure notice
            int dirChange = changeDir(argArray, argCount, home);
            if (dirChange) {
                sendToClient(success, establishedConnectionFD);
            }
            else {
                sendToClient(failure, establishedConnectionFD);
            }
        }

        strcpy(clientIn, ""); // Clear variable
    }

   
}

// Reads user input from client
// Returns input string with newline removed
void getClientInput(char* newString, int establishedConnectionFD) {
    static char receiveChar[10];
    static char charInt[10];
    int charsRead;
    int i = 0;
    int j = 0;
    int numChars = 1;
    int isChar = 0;

    do {
        // Receive data from client
        charsRead = recv(establishedConnectionFD, receiveChar, sizeof(char), 0);

        // Separate prepended int from data using '|' as separator
        if (strcmp(receiveChar, "|") == 0) {
            isChar = 1;
            numChars = atoi(charInt); // Convert final numeric char array to an int
        }
        else if (!isChar) {
            strcat(charInt, receiveChar); // Concatenate numeric values to char array
        }
        else if (isChar) {
            strcat(newString, receiveChar); // Concatenate data to char array pointer
            j = j + 1; 
        }

        if (charsRead < 0) { error("ERROR reading from socket"); };
        i = i + 1;
    } while(j < numChars); // Loop until end of data (according to passed length)

    // Remove trailing newlines and clear variables
    newString[strcspn(newString, "\n")] = 0;
    strcpy(charInt, "");
    strcpy(receiveChar, "");

}

// Send data with appended length to client
// Returns 1 when delivery successful
int sendToClient(char *charsToSend, int establishedConnectionFD) {
    int charsSent = 0;
    char bufferArray[MAXLINE];
    char* buffer = &bufferArray[0];

    // prepend length to char array
    strcpy(bufferArray, appendLength(charsToSend));

    // Send data to client and loop until all data sent
    charsSent = send(establishedConnectionFD, bufferArray, strlen(bufferArray), 0);
    while (charsSent != strlen(buffer)) {
        printf("Not all data delivered.\n"); fflush(stdout);
        charsSent = send(establishedConnectionFD, bufferArray, strlen(bufferArray), 0);
    }

    strcpy(bufferArray, ""); // Clear variable
    return 1;
}

// Prepends length of char array to data
// Returns new char array with appended length
char* appendLength(char *charsToSend) {
    size_t bufLen = MAXLINE;
    char* buffer = (char *)malloc(bufLen * sizeof(char));
    memset(buffer, '\0', MAXLINE);
    static char intChar[10];
    sprintf(intChar,"%d", strlen(charsToSend)); // Convert length from int to char array

    char newString[MAXLINE];
    strcpy(newString, intChar); // Add length
    strcat(newString, "|"); // Add separator
    strcat(newString, charsToSend); // Add data
    strcpy(buffer, newString); // Save to returned variable

    return buffer; // Return new char array
}

// Function specific to file transfer
// Sends size of entire file, rather than size of individual message
// Returns 0 if data delivery fails, and 1 if it is successful
int sendFileSize(int fileSize, int establishedConnectionFD) {
    size_t bufLen = MAXLINE;
    char* buffer = (char *)malloc(bufLen * sizeof(char));
    memset(buffer, '\0', MAXLINE);
    static char intChar[10];
    sprintf(intChar,"%d", fileSize); // Convert file length from int to char array

    char newString[MAXLINE];
    strcpy(newString, intChar); // Add length
    strcat(newString, "|"); // Add separator
    strcpy(buffer, newString); // Add data

    // Send data to client and output error message if not all data sent
    int charsSent = send(establishedConnectionFD, buffer, strlen(buffer), 0);
    if (charsSent != strlen(buffer)) {
        printf("Not all data delivered.\n"); fflush(stdout);
        return 0;
    }

    strcpy(buffer, ""); // Clear variable
    return 1;
}

// Returns string with current working directory
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

// Changes working directory to home or to specified path, if provided
// Returns 1 if successful, 0 if unsuccessful
int changeDir(char* charArray[MAXARG], int numArgs, char* charsToSend) {
    char* dirChange;
    if (numArgs > 1) { // Verify that a path is passed
        // Evaluate which path to use
        if (strcmp(charArray[1], "home") == 0) { 
            dirChange = charsToSend;
        }
        else {
            dirChange = charArray[1];
        }

        // Attempt to redirect to new path
        if (chdir(dirChange) != 0) {
            perror("Directory change failed");
        }
        else {
            printf("Directory change complete.\n\n"); fflush(stdout);
            return 1;
        }
    }
    else {
        printf("No path specified.\n"); fflush(stdout);
    }

    return 0;
}

// Parses passed input and separates commands from space and newline character(s)
// Places each command in passed array
// Returns array length
int parseInput(char* charArray[MAXARG], char input[MAXLINE]) {
    char* token; 
    char* rest = input;
    int argCount = 0; 
    int i = 0;

    // Separates char array by space and newline chars
    while ((token = strtok_r(rest, " \n", &rest))) {
        charArray[argCount] = token;
        argCount = argCount + 1;
    }

    return argCount;
}

// Sends file contents to client
void transferFile(char* charArray[MAXARG], int establishedConnectionFD) {
    FILE *plaintext;
    char *notFound = "Error: file not found\n";
    char *fileFound = "File exists\n";	
    int offset = 0;

    // Attempt to open file and validate
    plaintext = fopen(charArray[1], "r");
	if (plaintext == NULL) {
        sendToClient(notFound, establishedConnectionFD); // Notify client if file not found
        printf("Transfer unsuccessful: file not found.\n\n"); fflush(stdout);
	}
    else {
        sendToClient(fileFound, establishedConnectionFD); // Notify client if file found

        size_t bufsize = 500;
        char bufferArray[bufsize + 1];
        char* buffer = bufferArray;
        size_t characters;

        // Find file size
        fseek(plaintext, 0, SEEK_END);
        int size = ftell(plaintext);
        fseek(plaintext, 0, SEEK_SET);
        int i = 0;

        sendFileSize(size, establishedConnectionFD); // Send initial message containing file size

        // Loop until all data sent
        while (i < size) {
            memset(bufferArray, '\0', strlen(bufferArray));
            characters = getline(&buffer,&bufsize,plaintext);
            sendToClient(buffer, establishedConnectionFD);
            sleep(.25);
            i += characters;
        }

        fclose(plaintext); // Close file
        printf("Transfer complete.\n\n"); fflush(stdout);
    }
    
}