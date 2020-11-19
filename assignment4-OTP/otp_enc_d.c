#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int encrypt(char * crypt, int length, char * buffer, char * key){
    int m=0;
    int k=0;
    int sum =0;

    char tempENC;

    int i = 0;
    for (i=0; i < length; i++){  // get ascii value of character from  plaint text file
        m = (int)buffer[i];
        if (m == 32){           // if its a space, value is 26
            m = 26;
        }
        else{               // else, subtract 65 (A) from value
            m -= 65;
        }

        k = (int)key[i]; // gets ascii value of character from key file
        if (k == 32){
            k = 26;             // if its a space, value is 26
        }
        else{
            k -= 65;            // else, subtract 65 (A) from value.
        }

        sum = m+k;              // add plaintext by key and mod 27
        sum = sum % 27;

        tempENC = (char) (sum + 65);         // add to temp decrypted string

        if (tempENC == '['){     // if its the last character, make it a space
            tempENC = ' ';
        }
        crypt[i] = tempENC;         // put it in decrypted string
    }

    return length;
}

void tokenize(char * input, char ** arr){
    char * temp = strtok(input, "|");

    int i = 0;
    while (temp != NULL){                                   // tokenize the string, delimiter is a (|)
        arr[i] = temp;

        temp = strtok(NULL, "|");
        i++;
    }
}


int checkHandshake(int establishedConnectionFD, char * handshake){
    if (strcmp(handshake,"e") == 0){    // check if client sent a "E"
        return 1;                       // True if correct
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
    socklen_t sizeOfClientInfo;
    char buffer[160000];
    struct sockaddr_in serverAddress, clientAddress;

    if (argc < 2){ 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); 
    } // Check usage & args

    // Set up the address struct for this process (the server)
    memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (listenSocketFD < 0){
        error("ERROR opening socket");
    }

    // Enable the socket to begin listening
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to port
        error("ERROR on binding");
    }
    
    listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

    // Accept a connection, blocking if one is not available until one connects
    while(1){

        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0) {
            error("ERROR on accept");
        }

        int spawnPID = fork();      // create child process
        int exitMethod = -5;

        int i=0;
        int length = 0;
        char *crypt;
        char * tokens[4];
        int cryptLength = 0;

        int handShake__check = 0;
        char * sendError = "error";
        char * position;
        int sentLength = 0;
        switch (spawnPID){
            case -1: {                                     
                exit(1);
            }

            case 0:{
                // Get the message from the client and display it
                memset(buffer, '\0', sizeof(buffer));

                charsRead = recv(establishedConnectionFD, &sentLength, sizeof(sentLength) , 0); // receive length of client buffer
                position = buffer;
                while(sentLength > 0){      // loop  until it receives the entire message from client
                    charsRead = recv(establishedConnectionFD, position, sentLength , 0); // Read the client's message from the socket
                    position += charsRead;
                    sentLength -= charsRead;
                }

                if (charsRead < 0){
                    error("ERROR reading from socket");
                }
                //printf("                SERVER: I received this from the client: \"%s\"\n", buffer);

                tokenize(buffer, tokens);
                length = atoi(tokens[3]);
                crypt = malloc(length);
                char plaintText[length];
                memset(plaintText, '\0', sizeof(plaintText));
                char key[length];
                memset(key, '\0', sizeof(key));
                char handshake[2];
                strcpy(plaintText, tokens[1]);
                strcpy(key, tokens[2]);
                strcpy(handshake, tokens[0]);

                cryptLength = length;

                handShake__check = checkHandshake(establishedConnectionFD, handshake);
                if(handShake__check == 1){                                  // if it is encrypt to encrypt do this
                    encrypt(crypt, length, tokens[1], key);
                    //Send a Success message back to the client
                    while (cryptLength > 0){            // keep sending until all encrypted message is sent to client
                        charsRead = send(establishedConnectionFD, crypt , cryptLength + 1, 0); // Send success back
                        if (charsRead < 0){
                            error("ERROR writing to socket");
                        }
                        *crypt += charsRead;
                        cryptLength -= charsRead;
                    }
                    
                }

                else{           // if it is not encrypt to encrypt do this
                    charsRead = send(establishedConnectionFD, sendError , sizeof(sendError), 0);
                    if (charsRead < 0){
                        error("ERROR writing to socket");
                    }
                }

            }

            default: {
                waitpid(spawnPID, &exitMethod, 0);  /// wait for child process to be over

                if (WIFSIGNALED(exitMethod) != 0){
                        //printf(" Terminated by signal %d\n", WTERMSIG(exitMethod)); // if exited by signal, print out signal
                }
            }
        }

        close(establishedConnectionFD); // Close the existing socket which is connected to the client

    }


    close(listenSocketFD); // Close the listening socket

    return 0; 
}
