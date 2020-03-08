# file-transfer-system
1. While in the file-transfer-system folder, type "compileall" using Bash to compile the files.
    Note: "chmod 777 compileall" may be required after first downloading the program
2. To run the Server, type "Server PORT-NUM"
3. To run the Client, type "java FTClient HOST-NAME PORT-NUM"
4. You will first be prompted for a username and password. Type the username "Admin" and the password "yaynetworking".
5. You have the option to change the directory of the server ("cd new-path", or "cd home" to return to the original folder), list current working directory ("-l"), 
    or retrieve a specified file ("-g test.txt" - or another valid text file). When requesting a file, you will be prompted until you submit a unique file name for saving on the client side.
    
    Note: The cd and -l commands will loop back for user input. The only way to terminate the program from the client side is with a file transfer or ctrl+c. 
    This made the most sense to me, as "-l" is helpful to determine where to "cd" to and "cd" is only useful if the user is able to get files from different places.
6. The Server can be terminated using ctrl+c.

Note: I was unable to test this program properly using the Jane Austen text, due to the special characters (double quotes specifically) not translating well in Linux (dos2unix didn't fix this).
    I have attached a text file with more total characters than the Austen text, which has transfered properly during testing.
Note: I prepended the data length to each transmission, using '|' as a separator. Any file that has this character will break the program.

Testing Machine: I SSH'd to the flip servers using Visual Studio Code for testing.

Extra Credit:
* Implement username/password access
* Allow client to change directory on the server
* Additional functionality: 
    1. Client is able to cd to original folder (rather than a hardcoded path) by typing "cd home"
    2. Program loops to allow for multiple "-l" and "cd" commands

Citations: 
* The server program uses code from CS 344, Programs 3 and 4
* The client and server programs use code from our previous assignment in CS 372
