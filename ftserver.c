
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
void sendToClient(char* charsToSend, int establishedConnectionFD);
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
    portNumber = 50023;
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

        printf("Connected to client.\n"); fflush(stdout);

        validUserPass = validateUserPass(establishedConnectionFD);

        // do {
        //     validUserPass = validateUserPass(establishedConnectionFD);
        // } while (!validUserPass);

        
        // getCommand(establishedConnectionFD);

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

    printf("FinaluName: %s, %d\n", nameIn, strlen(nameInput)); fflush(stdout);
    printf("FinalPass: %s, %d\n", passIn, strlen(passInput)); fflush(stdout);

    int charsSent;

    if ((strcmp(nameIn, uName) != 0) || (strcmp(passIn, uPass) != 0)) {
        // charsSent = send(establishedConnectionFD, invalidUserPass, sizeof(invalidUserPass), 0);
        return 0;
    }
    else {
        // charsSent = send(establishedConnectionFD, proceedConnection, sizeof(proceedConnection), 0);
    }

    return 1;
}

void getCommand(int establishedConnectionFD) {
    char* argArray[MAXARG];
    printf("Retrieving client command...\n"); fflush(stdout);

    // char *clientIn = getClientInput(establishedConnectionFD);
    // int argCount = parseInput(argArray, clientIn);

    // if (strcmp(argArray[0], "-l") == 0) {
    //     char *cwd = getCWD();
    //     strcat(cwd, "\n");
    //     sendToClient(cwd, establishedConnectionFD);
    // }
    // else if (strcmp(argArray[0], "-g") == 0) {
    //     printf("Begin file transfer of: %s\n", argArray[1]); fflush(stdout);
    //     transferFile(argArray, establishedConnectionFD);
    // }
    // else if (strstr(argArray[0], "cd")) {
    //     printf("Changing to directory: %s\n", argArray[1]); fflush(stdout);
    //     changeDir(argArray, argCount);
    // }
    // else {
    //     printf("Invalid command.\n"); fflush(stdout);
    // }
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
        if (strcmp(receiveChar, ",") == 0) {
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

    // printf("ReceiveString: %s\n", newString); fflush(stdout);
}

int inStreamLength(int establishedConnectionFD) {
    char intBufferCoupReq[1];
    memset(intBufferCoupReq, '\0', sizeof(intBufferCoupReq));
    int *charsToReceive;
    int clientInput = recv(establishedConnectionFD, &intBufferCoupReq, 1, 0);

    charsToReceive = (int*) intBufferCoupReq;

    return charsToReceive[0];
}

void sendToClient(char *charsToSend, int establishedConnectionFD) {
    int charsSent = 0;

    charsSent = send(establishedConnectionFD, charsToSend, strlen(charsToSend), 0);
    if (charsSent != strlen(charsToSend)) {
        printf("Not all data delivered.\n"); fflush(stdout);
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

// Changes working directory to home or to specified path, if provided
void changeDir(char* charArray[MAXARG], int numArgs) {
    if (numArgs > 1) {
        if (chdir(charArray[1]) != 0) {
            perror("changeDir() failed.");
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
    // Open text file and check for open success
	plaintext = fopen(charArray[1], "r");
	if (plaintext == NULL) {
		printf("Error: file not found\n"); fflush(stdout);
	}
    else {
        printf("File found\n"); fflush(stdout);
        //Find size of file
        //Send file size to client
        //Send line-by-line, using newline as end marker

        // nread = getline(&b, &len, plaintext); // Read from plaintext file into buffer variable
        // strcpy(plainText, buffer); // Save buffer value to plaintext variable
        // memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

        fclose(plaintext);
    }
    
    
    
}