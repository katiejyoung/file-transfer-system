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

    public FTClient(String host, int port) {
        try {
            clientSocket = new Socket(host, port);
            clientOutput = new PrintWriter(clientSocket.getOutputStream(), true);
            serverInput = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            clientInput = new BufferedReader(new InputStreamReader(System.in));

            
        }
        catch (IOException e) {
            System.out.println("Error with port or connection.");
            System.out.println(e.getMessage());
            System.exit(1);
        }

        System.out.println("Connected to server.");

        try {
            clientOutput.close();
            serverInput.close();
            clientInput.close();
        }
        catch(IOException i) 
        { 
            System.out.println(i); 
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