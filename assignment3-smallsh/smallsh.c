#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>

int BACKGROUND_ALLOWED = 1;

void SIGTSTP__catch (int signo){                                                            // SIGTSTP__catch function

    if (BACKGROUND_ALLOWED == 1){                                                           // if BACKGROUND_ALLOWED is currently allowed
        char * not_allowed = " Entering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, not_allowed, 50);
        BACKGROUND_ALLOWED = 0;                                                             // make it so its not allowed then signal is caught
    }

    else if (BACKGROUND_ALLOWED == 0){                                                      // if BACKGROUND_ALLOWED is currently not allowed
        char * allowed = " Exiting foreground-only mode\n";
        write(STDOUT_FILENO, allowed, 30);
        BACKGROUND_ALLOWED = 1;                                                             // make it so its allowed
    }   

}

void tokenize(char * userInput, char ** tokens, int end){   // tokenize string
    int pid = 0;
    char * pidTemp = (char*) malloc(128);
    int i = 0;

    fflush(stdout);
    for (i; i < 512; i++){                                  // initialize token array to NULL
        tokens[i] = NULL;
    }

    if (userInput[end-1] == '\n'){                          // gets rid of newline
        userInput[end-1] = '\0';
    }  

    char * temp = strtok(userInput, " ");

    i = 0;
    while (temp != NULL){                                   // tokenize the string, delimiter is a space
        if (strcmp(temp, "$$") == 0){                       // if token is $$, then replace it with the pid
            pid = getpid();
            sprintf(pidTemp, "%d", pid );
            tokens[i] = pidTemp; 

        }

        else{
            tokens[i] = temp;
        }

        temp = strtok(NULL, " ");
        i++;
    }
}

void getInput(char * userInput, char ** tokens){    // get user input
    int end;

    char user[2048] = "";

    size_t bufsize = 0;
    fflush(stdout);
    printf(": ");
    fflush(stdout);

    fgets(user, 2048, stdin);                       // get user input

    strcpy(userInput, user);

    end = strlen(userInput);                        // get length of input
    tokenize(userInput, tokens, end);               // tokenize input
}

void doCd(char ** tokens){
    char * path;
    fflush(stdout);

    if (tokens[1] == NULL){                 // if just cd, then go to home directory (environment variable)
        path = getenv("HOME");
        chdir(path);
    }

    else {                                  // else, go to path (argument in token1)
        
        path = tokens[1];
        
        if (chdir(path) == -1){             // print error message if bad path
            printf("No such file or directory\n");
            fflush(stdout);
        }

    }
}

void doStatus(char ** tokens, int childExit){                                       // prints out status
    if (WIFEXITED(childExit) != 0){
        printf("exit status %d\n", WEXITSTATUS(childExit) );                        // if most recent foreground process exited, print exit status
        fflush(stdout);
    }

    else if (WIFSIGNALED(childExit) != 0){
        printf("terminated by signal %d\n", WTERMSIG(childExit) );        // if most recent foreground process terminated by signal, print out signal
        fflush(stdout);
    }

}

int doExit(char ** tokens, int * backgroundArr, int numBackground){  // exits shell
    int i=0;
    for (i=0 ; i < numBackground; i++){
        kill(backgroundArr[i], SIGKILL);            // for each background process, kill them
    }

    return 1;
}

void shiftTokens(char ** tokens, int  * i, char ** tempArr){  // gets rid of redirect and filename
    int a=0;
    for (a = 0; a < 512; a++){                  // initalize tempArr to NULL
        tempArr[a] = NULL;
    }

    int j = 0;
    int k = 0;
    
    while (tokens[j] != NULL){                  // if token is at the redirect or the filename, don't add it to the tempArr
        if (j == *i || j == *i+1){
            j++;
            continue;
        }

        tempArr[k] = tokens[j];                 // add token to tempArr
        k++;
        j++;
    }

    int v =0;
    for (v=0; v < 512; v++){                    // put elements in tempArr into token array (it will not contain redirect of file name)
        tokens[v] = tempArr[v]; 
    }

}

void findRedirect(char ** tokens, char * outputFile, char * inputFile, int * outFound, int * inFound){   // finds any redirects and filenames
    int i = 0;
    char * tempArr[512];
    
    int j =0;
    i=0;
    while(tokens[i] != NULL){                                                   // for each token in tokens array

        if (strcmp(tokens[i], ">") == 0 && tokens[i+1] != NULL) {               // if token is >, then the next element is the output file
            *outFound = 1;
            strcpy(outputFile, tokens[i+1]);
            shiftTokens(tokens, &i, tempArr);
        }

        else if (strcmp(tokens[i], "<") == 0 && tokens[i+1] != NULL) {          // if token is <, then the next element is the input file
            *inFound = 1;
            strcpy(inputFile, tokens[i+1]);
            shiftTokens(tokens, &i, tempArr);
            i--;
        } 
        i++;  
    }


}

int findComment(char ** tokens){                                        // finds comment (#)
    char first = tokens[0][0];

    if (first == '#'){                                                  // if the first character of the first string is and #, then return TRUE
        return 1;
    }

    return 0;
}

int findAnd(char ** tokens){                                            // finds &
    int i = 0;

    while (tokens[i] != NULL){
        if (tokens[i+1] == NULL && strcmp(tokens[i],"&") == 0){         // if its the last element in tokens array and its an &
            tokens[i] = NULL;                                           // set it to NULL and return TRUE
            return 1;
        }
        i++;
    }
    return 0;
}

void checkBackground(int * backgroundArr, int numBackground){                       // checks background processes to see if any have finished
    int exitMethod__BACK = -5;
    int backgroundPID = 0;
    int exitStatus = 0;

    int i=0;

    for (i=0 ; i < numBackground; i++){                                             // for each background PIDs
        backgroundPID = waitpid(backgroundArr[i], &exitMethod__BACK, WNOHANG);      // check to see if finished 

        if (backgroundPID > 0){                                                     // if background is greator than 0, then backgruond has finished
            printf("background pid %d is done: ", backgroundPID);
            fflush(stdout);

            if ( WIFEXITED(exitMethod__BACK) != 0){                                 // if exited normally, print exit status
                exitStatus = WEXITSTATUS(exitMethod__BACK);
                printf("exit value %d\n", exitStatus);
                fflush(stdout);
            }

            if (WIFSIGNALED(exitMethod__BACK) != 0){                                // if exited by signal, print signal
                exitStatus = WTERMSIG(exitMethod__BACK);
                printf("terminated by signal %d\n", exitStatus);
                fflush(stdout);
            }

        }
    }

}

void executeCMD(char ** tokens , int ** childExit, int ** sigExit, int * backgroundArr, int ** numBackground, struct sigaction SIGINT__action){
    pid_t spawnPid = -5;
    int exitMethod;
    int dev = -5;

    char outputFile [256] = "";
    char inputFile [256] = "";

    int outFound = 0;
    int inFound = 0;
    int out = 0;
    int in = 0;

    int exitMethod__BACK = -5;

    int commentFound = findComment(tokens);         
    if (commentFound == 1){                             // if it finds a comment, skip this whole function
        return;
    }

    findRedirect(tokens, outputFile, inputFile, &outFound, &inFound);           // finds any redirection

    int backgroundFound = findAnd(tokens);                  // if it finds and &, then execute background processes

    spawnPid = fork();                                  // fork a child process and keep PID of child in spawnPID

    switch (spawnPid) {
        case -1: {                                      // if it can't fork
            exit(1);
        }

        case 0: {                                                               // if its a child
            if (backgroundFound == 0){                                          
                SIGINT__action.sa_handler = SIG_DFL;
                sigaction(SIGINT, &SIGINT__action, NULL);                       // if its not a background process, set SIGINT to its default action
            }

            if (inputFile == "" && backgroundFound == 1){                       // if input file is blank and its a background process, set input to dev null
                dev = open("/dev/null", O_RDONLY);
                dup2(dev, 0);
                return;
            }

            if (outputFile == "" && backgroundFound == 1){                      // if output file is blank and its a background process, set output to dev null
                dev = open("/dev/null", O_WRONLY);
                dup2(dev, 1);
                return;
            }
            
            if (outFound == 1){
                out = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0600);     // if there is an output redirection, open the file, and set stdout to point to the file descriptor of file
                if (out == -1){
                    perror(outputFile);
                    exit(1);
                }
                dup2(out , 1);
                fcntl(out, F_SETFD, FD_CLOEXEC);
            }

            if (inFound == 1){
                in = open(inputFile, O_RDONLY);
                if (in == -1 && backgroundFound == 1){
                    printf("cannot open %s for input\n", inputFile);            // if there is an input redirection, open the file and set stdin to point ot the file descriptor
                    fflush(stdout);
                    exit(1);
                }
                else if (in == -1){                                             // error message if can't open input file
                    printf("cannot open %s for input\n", inputFile);
                    fflush(stdout);
                    exit(1);
                }
                else {
                    dup2(in , 0);                                               
                    fcntl(in, F_SETFD, FD_CLOEXEC);
                }   
            }

            if (execvp(tokens[0], tokens) == -1){                               // execute the command and if it can't, then error message
                perror(tokens[0]);
                exit(1);
            }             
        }

        default: {                                                              // in parent
            if (backgroundFound == 1 && BACKGROUND_ALLOWED == 1){               // if child was a background process, and if background it allowed, then don't wait for the child to finish and add pid to Background PID array.            
                printf("background PID is %d\n", spawnPid );
                fflush(stdout);
                waitpid(-1, &exitMethod__BACK, WNOHANG);
                backgroundArr[**numBackground] = spawnPid;
                **numBackground += 1;
            }

            else{
                waitpid(spawnPid, &exitMethod, 0);                              // if it is not a background process, wait for child to finish
                **childExit = exitMethod;

                if (WIFSIGNALED(exitMethod) != 0){
                    printf(" Terminated by signal %d\n", WTERMSIG(exitMethod)); // if exited by signal, print out signal
                    fflush(stdout);
                }
            }        
        }

    }

}

void startShell(int * end, int * childExit, int * sigExit, int * backgroundArr, int * numBackground, struct sigaction SIGINT__action, struct sigaction SIGTSTP__action){
    char userInput[2048];
    char * tokens[512];

    getInput(userInput, tokens);                                    // gets user input and tokenizes input.

    if (tokens[0] == NULL){
        checkBackground(backgroundArr, *numBackground);             // if user didn't enter anything check to see if any background process have finished.
        return;
    }
    if( strcmp(tokens[0], "cd") == 0){                              // if user entered cd
        doCd(tokens);
    }
    else if( strcmp(tokens[0], "status") == 0){                     // if user entered status
        doStatus(tokens, *childExit);
    }
    
    else if( strcmp(tokens[0], "exit") == 0){                       // if user entered exit
        *end = doExit(tokens, backgroundArr, *numBackground);
    }
    else {
        executeCMD(tokens, &childExit, &sigExit, backgroundArr, &numBackground, SIGINT__action);            // if user entered a non built-in command
    }   

    checkBackground(backgroundArr, *numBackground);                 // check to see if any background process have finished.
}


int main(){
    
    int end = 0;
    int childExit = -5;
    int sigExit = -5;

    int i=0;
    int backgroundArr[128];                             
    int numBackground = 0;
    for (i =0; i < 128; i++){
        backgroundArr[i] = 0;                                       // create backgrouond array that holds the pids of all the background processes.
    }

    sigset_t mySignalset;
    sigaddset(&mySignalset, SIGINT);                                // adds SIGINT and SIGTSTP to Signalset.
    sigaddset(&mySignalset, SIGTSTP);

    struct sigaction SIGINT__action = {0};                          // creates struct for SIGINT. Set the handler to ignore.
    SIGINT__action.sa_handler = SIG_IGN;
    SIGINT__action.sa_mask = mySignalset;
    SIGINT__action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT__action, NULL);

    struct sigaction SIGTSTP__action = {0};                         // creates struct for SIGTSTP . Set the handler to the catch function
    SIGTSTP__action.sa_handler = SIGTSTP__catch;
    SIGTSTP__action.sa_mask = mySignalset;
    SIGTSTP__action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP__action, NULL); 

    do{
        startShell(&end, &childExit, &sigExit, backgroundArr, &numBackground, SIGINT__action, SIGTSTP__action);                     // loop
    } while (end == 0);

    return 0;
}