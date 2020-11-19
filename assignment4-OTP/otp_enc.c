#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int getLength(char * fileName){
    FILE *f;
    f = fopen( fileName, "r" );         // open file

    fseek (f, 0, SEEK_END);             // move pointer to the end
    int Length__file = ftell (f);       // get pointer length (number of characters in file)
    Length__file--;
    fclose (f);

    return Length__file;
}

void getContent(char * fileName, int length, char * buffer){
    FILE *f;
    f = fopen( fileName, "r" );         // open file
    if (buffer){
        fread (buffer, 1, length, f);       // get entire content of file
    }
    fclose (f);
    buffer[strcspn(buffer, "\n")] = '\0';   // replace a new line ewith NULL

    int i=0;
    for (i=0;i < strlen(buffer);i++){
        if (buffer[i] < 65 || buffer[i] > 91){
            if (buffer[i] != 32){
                fprintf(stderr, "BAD CHARACTER!!\n");  // if its not a capital letter or a space, exit
                exit(1);
            }
        }
    }
}

int main(int argc, char *argv[]){
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[160000];
    char * position;
    
    if (argc < 3) { 
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(2); 
    } // Check usage & args

    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address

    if (serverHostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0);
    }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }
    
    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
        error("CLIENT: ERROR connecting");
    }

    char op [256];
    char fileName [70000];
    char keyFile [70000];
    memset(op, '\0', sizeof(op));
    memset(fileName, '\0', sizeof(fileName));
    memset(keyFile, '\0', sizeof(keyFile));

    strcpy(op, argv[0]);
    op[strcspn(op, "\n")] = '\0';
    strcpy(fileName, argv[1]);
    fileName[strcspn(fileName, "\n")] = '\0';
    strcpy(keyFile, argv[2]);
    keyFile[strcspn(keyFile, "\n")] = '\0';

    int length = getLength(fileName);

    char * plainContent = malloc (length);
    getContent(fileName, length, plainContent);
    char * keyContent = malloc (length);
    memset(keyContent, '\0', sizeof(keyContent));
    getContent(keyFile, length, keyContent);

    int i =0;
    int counter = 0;
    while(keyContent[i] != '\0'){
        counter++;                      // count how many characters are in key
        i++;    
    }
    if (length > counter){
        printf("key length is too short\n");            // if key is less than length of text, exit
        exit(2);
    }

    memset(buffer, '\0', sizeof(buffer));
    sprintf(buffer, "%c|%s|%s|%d", 'e' ,plainContent,keyContent,length); // create a buffer with handshake, text content, key content, and length 

    // Send message to server
    int totalSend=0;
    int bufferLenght = strlen(buffer);

    charsWritten = send(socketFD, &bufferLenght, sizeof(int), 0); // send lenegth of entire buffer
    if (charsWritten < 0){
        error("CLIENT: ERROR writing to socket");
    }

    position = buffer;
    while (bufferLenght > 0){               // keep sending until entire buffer is sent
        charsWritten = send(socketFD, position, bufferLenght, 0); // send buffer
        if (charsWritten < 0){
            error("CLIENT: ERROR writing to socket");
        }
        position += charsWritten;
        bufferLenght -= charsWritten;
    }

    if (charsWritten < strlen(buffer)){
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    // Get return message from server
    char encryption[length+1];
    memset(encryption, '\0', sizeof(encryption)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, encryption, sizeof(encryption) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0){
        error("CLIENT: ERROR reading from socket");
    }

    if (strcmp(encryption, "error") == 0){          // if error, send wrong server
        fprintf(stderr, "WRONG SERVER: %d", portNumber);
        exit(2);
    }

    printf("%s\n", encryption);  // print message from server

    close(socketFD); // Close the socket
    return 0;
}
