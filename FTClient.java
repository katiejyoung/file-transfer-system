import java.net.*;
import java.io.*;
import java.util.Scanner;

public class FTClient {
    Socket clientSocket = null;

    BufferedReader serverInput;
    BufferedWriter clientOutput;
    BufferedReader clientInput;

    String userMessage = "";
    String serverMessage = "";
    String userName = "";
    String userPass = "";

    public FTClient(String host, int port) {
        try {
            clientSocket = new Socket(host, port); // Connect to server
            System.out.println("Connected to server...\n");

            // Open for input/output
            clientOutput = new BufferedWriter(new OutputStreamWriter(clientSocket.getOutputStream()));
            serverInput = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            clientInput = new BufferedReader(new InputStreamReader(System.in));

            getUserPass(); // Perform username/password actions
            executeProg(); // Perform command portion of program

            // Close input/output streams
            clientOutput.close();
            serverInput.close();
            clientInput.close(); 

            clientSocket.close(); // Close socket    
        }
        catch (IOException e) {
            System.out.println("Error with port or connection.");
            System.out.println(e.getMessage());
            System.exit(1);
        }
    }

    // Gets username and password from user
    // Validates and loops until correct credentials entered
    public void getUserPass() throws IOException {
        int validUserPass = 0;
        int length = 0;
        int i = 0;

        System.out.println("Please enter your username and password.");

        do {
            // Gather information from client
            System.out.print("Username");
            userName = getUserInput();
            System.out.print("Password");
            userPass = getUserInput();

            // Send data to server
            length = userName.length();
            sendToServer(userName, length);
            length = userPass.length();
            sendToServer(userPass, length);

            // Receive response from server and validate accordingly
            serverMessage = getServerInput(); 
            if (serverMessage.contains("invalid")) {
                System.out.println("\nInvalid username/password combination. Please try again.");
            }
            else { validUserPass = 1; }

            // Clear variables
            userName = "";
            userPass = "";
            serverMessage = "";

        } while (validUserPass == 0);
    }

    // Gets input from user
    // Returns string containing input
    public String getUserInput() throws IOException {
        StringBuilder buildString = new StringBuilder();
        int i = 0;
        int c;
        System.out.print(": "); // Input marker
        while (true) {
            if (clientInput.ready()) { // Wait until input is entered
                do { 
                    c = clientInput.read();
                    buildString.append((char) c); // Append read char to stringbuilder
                    i++;
                } while ((buildString.indexOf("\n") == -1) && (i < 500));
                return buildString.toString(); // Convert to string and return
            }
        }

    }

    // Sends passed message with prepended length to server
    public void sendToServer(String message, int length) throws IOException {
        String sendString = appendStringLength(message, length); // Prepend length to beginning of message
        clientOutput.write(sendString); // Send prepended message to server
        clientOutput.flush(); // Flush output stream
    }
    
    // Prepends data length to message
    // Returns new message with prepended length
    public String appendStringLength(String message, int length) throws IOException {
        StringBuilder newStr = new StringBuilder(0);
        newStr.append(length); // Add length
        newStr.append("|"); // Add separator
        newStr.append(message); // Add data
        return newStr.toString(); // Convert to string and return
    }

    // Receives data from server
    // Returns string containing new data
    public String getServerInput() throws IOException {
        StringBuilder buildString = new StringBuilder();
        StringBuilder intString = new StringBuilder();
        int isChar = 0;
        int numChars = 1;
        int i = 0;
        int c;
        
        while (true) {
            if (serverInput.ready()) { // Server ready
                do {
                    c = serverInput.read(); // Read data from server
                    if (isChar == 0) {
                        if (((char) c != '|')) {
                            intString.append((char) c); // Convert numeric string to int once separator detected
                        }
                        else { 
                            isChar = 1;
                            numChars = Integer.parseInt(intString.toString()); // Append numeric values to string until separator detected
                        }
                    }
                    else {
                        buildString.append((char) c); // Append input to data string if length known and increment counter
                        i++;
                    }
                    
                } while (i < numChars); // Loop until all data received

                // Return data string
                if (i == numChars) {
                    return buildString.toString();
                }
                else { // Not all data received; prepare for re-submission
                    continue;
                }

                
            }
        }
    }

    // Method specific to file transfer
    // Reads size of incoming file, using '|' as an end notifier
    // Returns file size
    public int getFileSize() throws IOException {
        StringBuilder intString = new StringBuilder();
        int isChar = 0;
        int numChars = 1;
        int c;
        
        while (true) {
            if (serverInput.ready()) { // Server ready
                do { 
                    c = serverInput.read(); // Read data from server
                    if (isChar == 0) {
                        if (((char) c != '|')) {
                            intString.append((char) c); // Convert numeric string to int once separator detected
                        }
                        else { 
                            isChar = 1;
                            numChars = Integer.parseInt(intString.toString()); // Append numeric values to string until separator detected
                            break;
                        }
                    }
                    
                } while (isChar == 0); // Loop until all data received

                return numChars; // Return file size
            }
        }
    }

    // Logic for command portion of program
    public void executeProg() throws IOException {
        int isValid = 0;
        int length = 0;
        System.out.println("\n-------------------------------------------------------");
        System.out.println("Welcome to FTClient");
        System.out.println("Send \'cd\' followed by a new path to change directory.\nSend \'-l\' to list current working directory.\nEnter \'-g\' followed by a file to initiate file transmission.");
        System.out.println("-------------------------------------------------------\n");

        while (true) { // Loop until file transfer requested
            isValid = 0; // Reset validation tracker
            do {
                userMessage = "";

                userMessage = getUserInput(); // Gather input from user

                // Send value to server
                length = userMessage.length();
                sendToServer(userMessage, length);
                
                serverMessage = getServerInput(); // Receive server response

                // Evaluate server response
                isValid = validateCommand(serverMessage);
                if (isValid != 1) {
                    System.out.println("Command not found. Please try again.");
                }

                serverMessage = ""; // Clear variable
                
            } while (isValid == 0); // Loops until valid command entered

            if (userMessage.contains("-l")) { // User requests CWD
                // Get CWD from server and output
                serverMessage = getServerInput(); 
                System.out.println("Working directory: " + serverMessage);
                serverMessage = "";
            }
            else if (userMessage.contains("-g")) { // User requests file transfer
                System.out.println("Retrieving file...");

                // Get initial server message: file found or file not found
                serverMessage = getServerInput();
                if (serverMessage.contains("not found")) {
                    System.out.println("Error: File not found.");
                }
                else {
                    int fileSize = getFileSize(); // Get file size from server
                    int charCount = 0;
                    FileWriter fileWriter = new FileWriter("newFile.txt"); // Open file for writing

                    // Loop until EOF and save data to file
                    while (charCount < fileSize) {
                        serverMessage = getServerInput();
                        fileWriter.write(serverMessage);
                        charCount += serverMessage.length();
                        serverMessage = "";
                    }

                    System.out.println("Transfer complete.");
                    fileWriter.close(); // Close file
                }
                break; // End program
            }
            else if (userMessage.contains("cd")) { // User requests directory change
                serverMessage = getServerInput(); // Get message from server: change completed or directory not found
                System.out.println(serverMessage); // Output server message
            }

            // Clear variables
            serverMessage = "";
            userMessage = "";
        }
        
    }

    // Validates server message in response to commands
    // Returns 1 if valid and 0 if not valid
    public int validateCommand(String command) throws IOException {
        if (command.contains("Valid")) {
            return 1;
        }

        return 0;
    }

    // Main: validates arguments and creates class instance of FTClient
    public static void main(String []args) throws IOException {
        // Validate argument count
        if (args.length != 2) {
            System.err.println("Incorrect number of arguments");
            System.exit(1);
        }

        String hostName = args[0];
        int serverPort = Integer.parseInt(args[1]);
        FTClient client = new FTClient(hostName, serverPort); 
    }
}