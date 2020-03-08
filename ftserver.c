
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
void changeDir(char* charArray[MAXARG], int numArgs);
int parseInput(char* charArray[MAXARG], char input[MAXLINE]);
void transferFile(char* charArray[MAXARG], int establishedConnectionFD);

int main(int argc, char *argv[]) {
    int portNumber;

    int listenSocketFD, establishedConnectionFD;
    struct sockaddr_in serverAddress;

    // Check argument length
    //if (argc != 1) { fprintf(stderr,"too few arguments\n", argv[0]); exit(2); }

    // Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	// portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
    portNumber = 60124;
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

    listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
    acceptedConnection(listenSocketFD);

    close(listenSocketFD);
    return 0;
}

// Accepts new connection and calls appropriate functions based on user input
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

        do {
            validUserPass = validateUserPass(establishedConnectionFD);
        } while (!validUserPass);

        getCommand(establishedConnectionFD);

        close(establishedConnectionFD);
        establishedConnectionFD = -1;
    }
}

int validateUserPass(int establishedConnectionFD) {
    char uName[] = "Admin";
    char uPass[] = "monkeys3";
    char *invalidUserPass = "invalid\n";
    char *proceedConnection = "proceed\n";

    char nameInput[1001];
    char passInput[1001];
    char *nameIn = &nameInput[0];
    getClientInput(nameIn, establishedConnectionFD);
    char *passIn = &passInput[0];
    getClientInput(passIn, establishedConnectionFD);

    int charsSent;

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

void getCommand(int establishedConnectionFD) {
    char* argArray[MAXARG];
    char clientInput[1001];
    char *clientIn = &clientInput[0];
    int isValid = 0;
    char* invalidCommand = "Error: command not found";
    char* validCommand = "Valid command";
    int argCount;

    while (!isValid) {
        strcpy(clientIn, "");
        getClientInput(clientIn, establishedConnectionFD);

        argCount = parseInput(argArray, clientIn);

        if ((strcmp(argArray[0], "-l") == 0) || (strcmp(argArray[0], "-g") == 0) || (strstr(argArray[0], "cd") == 0)) {
            sendToClient(validCommand, establishedConnectionFD);
            isValid = 1;
            sleep(1);
        }
        else {
            sendToClient(invalidCommand, establishedConnectionFD);
            argArray[0] = '\0';
            sleep(1);
        }
    }

    if (strcmp(argArray[0], "-l") == 0) {
        printf("Sending current working directory...\n"); fflush(stdout);
        char *cwd = getCWD();
        strcat(cwd, "\n");
        sendToClient(cwd, establishedConnectionFD);
        printf("Working directory sent.\n\n"); fflush(stdout);
    }
    else if (strcmp(argArray[0], "-g") == 0) {
        printf("Begin file transfer of: %s\n", argArray[1]); fflush(stdout);
        transferFile(argArray, establishedConnectionFD);
    }
    else if (strstr(argArray[0], "cd")) {
        printf("Changing to directory: %s\n", argArray[1]); fflush(stdout);
        changeDir(argArray, argCount);
    }

    strcpy(clientIn, "");
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
        // printf("While loop... i = %d\n", i); fflush(stdout);
        charsRead = recv(establishedConnectionFD, receiveChar, sizeof(char), 0);
        // printf("Character %d: %s\n", j, receiveChar); fflush(stdout);
        if (strcmp(receiveChar, "|") == 0) {
            isChar = 1;
            numChars = atoi(charInt);
            // printf("Number of characters %d\n", numChars); fflush(stdout);
        }
        else if (!isChar) {
            strcat(charInt, receiveChar);
        }
        else if (isChar) {
            strcat(newString, receiveChar);
            j = j + 1; 
        }

        if (charsRead < 0) { error("ERROR reading from socket"); };
        i = i + 1;
    } while(j < numChars);

    newString[strcspn(newString, "\n")] = 0;
    strcpy(charInt, "");
    strcpy(receiveChar, "");

    // printf("ReceiveString: %s\n", newString); fflush(stdout);
}

int sendToClient(char *charsToSend, int establishedConnectionFD) {
    int charsSent = 0;
    char bufferArray[MAXLINE];
    char* buffer = &bufferArray[0];
    strcpy(bufferArray, appendLength(charsToSend));

    charsSent = send(establishedConnectionFD, bufferArray, strlen(bufferArray), 0);
    while (charsSent != strlen(buffer)) {
        printf("Not all data delivered.\n"); fflush(stdout);
        charsSent = send(establishedConnectionFD, bufferArray, strlen(bufferArray), 0);
    }

    strcpy(bufferArray, "");
    return 1;
}

char* appendLength(char *charsToSend) {
    size_t bufLen = MAXLINE;
    char* buffer = (char *)malloc(bufLen * sizeof(char));
    memset(buffer, '\0', MAXLINE);
    static char intChar[10];
    sprintf(intChar,"%d", strlen(charsToSend));

    char newString[MAXLINE];
    strcpy(newString, intChar);
    strcat(newString, "|");
    strcat(newString, charsToSend);
    strcpy(buffer, newString);

    return buffer;
}

int sendFileSize(int fileSize, int establishedConnectionFD) {
    size_t bufLen = MAXLINE;
    char* buffer = (char *)malloc(bufLen * sizeof(char));
    memset(buffer, '\0', MAXLINE);
    static char intChar[10];
    sprintf(intChar,"%d", fileSize);

    char newString[MAXLINE];
    strcpy(newString, intChar);
    strcat(newString, "|");
    strcpy(buffer, newString);

    int charsSent = send(establishedConnectionFD, buffer, strlen(buffer), 0);
    if (charsSent != strlen(buffer)) {
        printf("Not all data delivered.\n"); fflush(stdout);
        return 0;
    }

    strcpy(buffer, "");
    return 1;
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

// Changes working directory to home or to specified path, if provided
void changeDir(char* charArray[MAXARG], int numArgs) {
    if (numArgs > 1) {
        if (chdir(charArray[1]) != 0) {
            perror("changeDir() failed.");
        }
        else {
            printf("Directory change complete.\n"); fflush(stdout);
        }
    }
    else {
        printf("No path specified.\n"); fflush(stdout);
    }
}

// Parses passed input and separates commands from space and newline character(s)
// Places each command in passed array
// Returns array length integer
int parseInput(char* charArray[MAXARG], char input[MAXLINE]) {
    char* token; // Character array variable for chunk of text
    char* rest = input; // Copy of passed character array variable
    int argCount = 0; // Counter for array element iteration
    int i = 0;

    while ((token = strtok_r(rest, " \n", &rest))) {
        charArray[argCount] = token;
        argCount = argCount + 1;
    }

    return argCount;
}

void transferFile(char* charArray[MAXARG], int establishedConnectionFD) {
    FILE *plaintext;
    char *notFound = "Error: file not found\n";
    char *fileFound = "File exists\n";
	plaintext = fopen(charArray[1], "r");
    int offset = 0;

	if (plaintext == NULL) {
        sendToClient(notFound, establishedConnectionFD);
        printf("Transfer unsuccessful: file not found.\n\n"); fflush(stdout);
	}
    else {
        sendToClient(fileFound, establishedConnectionFD);

        size_t bufsize = 500;
        char bufferArray[bufsize + 1];
        char* buffer = bufferArray;
        size_t characters;

        fseek(plaintext, 0, SEEK_END);
        int size = ftell(plaintext);
        fseek(plaintext, 0, SEEK_SET);
        int i = 0;

        sendFileSize(size, establishedConnectionFD);

        while (i < size) {
            memset(bufferArray, '\0', strlen(bufferArray));
            characters = getline(&buffer,&bufsize,plaintext);
            sendToClient(buffer, establishedConnectionFD);
            sleep(.25);
            i += characters;
        }

        fclose(plaintext);
        printf("Transfer complete.\n\n"); fflush(stdout);
    }
    
}