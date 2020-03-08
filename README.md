# file-transfer-system
1. While in the file-transfer-system folder, type "compileall" using Bash to compile the files.
2. To run the Server, type "Server"
3. To run the Client, type "java FTClient"
4. You have the option to change the directory of the server ("cd new-path"), list CWD ("-l"), or retrieve a specified file ("-g test.txt")
    Note that a file that is transfered will have the same word count as the original

Note: I was unable to test this program properly using the Jane Austen text, due to the special characters (double quotes specifically) not translating well in Linux (dos2unix didn't fix this).
    I have attached a text file with more total characters than the Austen text, which has transfered properly during testing.

Testing Machine: I SSH'd to the flip servers using Visual Studio Code for testing.

Citations: 
* The server program uses code from CS 344, Programs 3 and 4
* The client and server programs use code from our previous assignment in CS 372
