import java.net.*;
import java.io.*;
import java.util.Scanner;

public class FTClient {
    Socket clientSocket = null;
    PrintWriter out;
    BufferedReader in;
    
    StringBuilder buildString = new StringBuilder();
    Scanner userInput = new Scanner(System.in);
    BufferedReader serverInput;
    BufferedReader clientInput;
    PrintWriter clientOutput;
    String userMessage = "";
    String serverMessage = "";
    String userName = "";
    String userPass = "";

    public FTClient(String host, int port) {
        try {
            clientSocket = new Socket(host, port);
            clientOutput = new PrintWriter(clientSocket.getOutputStream(), true);
            serverInput = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            clientInput = new BufferedReader(new InputStreamReader(System.in));

            getUserPass();

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
        System.out.println("Connected to server.");

        do {
            System.out.println("Please enter your username and password");
            System.out.print("Username: ");
            userName = getUserInput();
            System.out.print("Password: ");
            userPass = getUserInput();

            clientOutput.println(userName);
            clientOutput.println(userPass);
            serverMessage = getServerInput();

            if (serverMessage.contains("invalid")) {
                System.out.println("Invalid username/password combination. Please try again.");
            }
            else { validUserPass = 1; }

            // Clear variables
            userName = "";
            userPass = "";
            serverMessage = "";

        } while (validUserPass == 0);
    }

    public String getUserInput() throws IOException {
        int i = 0;
        int c;
        buildString.setLength(0); 
        while (true) {
            if (clientInput.ready()) { // Client input received
                do { // Read each char and append to string builder until \n found
                    c = clientInput.read();
                    buildString.append((char) c);
                    i++;
                } while ((buildString.indexOf("\n") == -1) && (i < 500));
                return buildString.toString();
            }
        }
    }

    public String getServerInput() throws IOException {
        int i = 0;
        int c;
        buildString.setLength(0); 
        while (true) {
            if (serverInput.ready()) { // Client input received
                do { // Read each char and append to string builder until \n found
                    c = serverInput.read();
                    buildString.append((char) c);
                    i++;
                } while ((buildString.indexOf("\n") == -1) && i < 500);
                return buildString.toString();
            }
        }
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