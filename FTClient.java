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
            clientSocket = new Socket(host, port);
            System.out.println("Connected to server...\n");

            clientOutput = new BufferedWriter(new OutputStreamWriter(clientSocket.getOutputStream()));
            serverInput = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            clientInput = new BufferedReader(new InputStreamReader(System.in));

            getUserPass();
            executeProg();

            clientOutput.close();
            serverInput.close();
            clientInput.close();     
        }
        catch (IOException e) {
            System.out.println("Error with port or connection.");
            System.out.println(e.getMessage());
            System.exit(1);
        }
    }

    public void getUserPass() throws IOException {
        int validUserPass = 0;
        int length = 0;
        int i = 0;

        do {
            System.out.println("Please enter your username and password.");
            System.out.print("Username: ");
            // userName = getUserInput();
            userName = "Admin\n";
            System.out.print("Password: ");
            // userPass = getUserInput();
            userPass = "monkeys3\n";

            length = userName.length();
            sendToServer(userName, length);

            length = userPass.length();
            sendToServer(userPass, length);

            serverMessage = getServerInput();

            if (serverMessage.contains("invalid")) {
                System.out.println("Invalid username/password combination. Please try again.");
            }
            else { validUserPass = 1; }

            // Clear variables
            userName = "";
            userPass = "";
            serverMessage = "";

            validUserPass = 1;

        } while (validUserPass == 0);
    }

    public String getUserInput() throws IOException {
        StringBuilder buildString = new StringBuilder();
        int i = 0;
        int c;
        while (true) {
            if (clientInput.ready()) {
                do { 
                    c = clientInput.read();
                    buildString.append((char) c);
                    i++;
                } while ((buildString.indexOf("\n") == -1) && (i < 500));
                return buildString.toString();
            }
        }

    }

    public void sendToServer(String message, int length) throws IOException {
        //Append length to beginning of message
        String sendString = appendStringLength(message, length);
        clientOutput.write(sendString);
        clientOutput.flush();
    }
    
    public String appendStringLength(String message, int length) throws IOException {
        StringBuilder newStr = new StringBuilder(0);
        newStr.append(length);
        newStr.append(",");
        newStr.append(message);
        return newStr.toString();
    }

    public String getServerInput() throws IOException {
        StringBuilder buildString = new StringBuilder();
        StringBuilder intString = new StringBuilder();
        int isChar = 0;
        int numChars = 1;
        int i = 0;
        int c;
        
        while (true) {
            if (serverInput.ready()) { // Client input received
                do { // Read each char and append to string builder until \n found
                    c = serverInput.read();
                    if (isChar == 0) {
                        if (((char) c != ',')) {
                            intString.append((char) c);
                        }
                        else { 
                            isChar = 1;
                            numChars = Integer.parseInt(intString.toString());
                        }
                    }
                    else {
                        buildString.append((char) c);
                        i++;
                    }
                    
                } while (i < numChars);

                return buildString.toString();
            }
        }
    }

    public void executeProg() throws IOException {
        int isValid = 0;
        int length = 0;
        int fileExists = 0;
        System.out.println("\n-------------------------------------------------------");
        System.out.println("Welcome to FTClient");
        System.out.println("Send \'cd\' followed by a new path to change directory.\nSend \'-l\' to list current working directory.\nEnter \'-g\' followed by a file to initiate file transmission.");
        System.out.println("-------------------------------------------------------\n");

        do {
            userMessage = getUserInput();
            isValid = validateCommand(userMessage);

            if (isValid != 1) {
                System.out.println("Command not found. Please try again.");
                userMessage = "";
            }
        } while (isValid == 0);

        length = userMessage.length();
        sendToServer(userMessage, length);

        if (userMessage.contains("-l")) {
            serverMessage = getServerInput();
            System.out.println("Working directory: " + serverMessage);
        }
        else if (userMessage.contains("-g")) {
            System.out.println("Retrieving file...");
            fileExists = receiveFile();
            if (fileExists == 0) {
                System.out.println("Error: File not found.");
            }
            else if (fileExists == 1) {
                System.out.println("File transfer complete.");
            }
        }

        serverMessage = "";
        userMessage = "";
    }

    public int validateCommand(String command) throws IOException {
        if (command.contains("cd") || command.contains("-l") || command.contains("-g")) {
            return 1;
        }

        return 0;
    }

    public int receiveFile() throws IOException {
        // Receive first submission, including file status or total file length
        // If file not found, return 0
        // If file found, loop until EOF (alternative to file length: EOF marker at beginning of last submission)
        return 1;
    }
        

    public static void main(String []args) throws IOException {
        // Validate argument count
        // if (args.length != 2) {
        //     System.err.println("Incorrect number of arguments");
        //     System.exit(1);
        // }

        // String hostName = args[0];
        // int serverPort = Integer.parseInt(args[1]);
        String hostName = "localhost";
        int serverPort = 50023;
        FTClient client = new FTClient(hostName, serverPort); 
    }
}