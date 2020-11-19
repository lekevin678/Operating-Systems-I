#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

struct Room {
    char name [64];
    int num_connects;
    char connections[6][64];

    char type[64];
};

struct Node{
    char val [64];
    struct Node * next;
};

struct Node * append(struct Node * head, char * value){  // function to push node into linked list, from the back;
    struct Node * curr = head;
    struct Node * newNode = malloc(sizeof(struct Node)); 
    strcpy(newNode->val, value);
    newNode->next = NULL;

    curr = head;
    if (head == NULL){          // if no nodes in linked list, then head will equal new node
        head = newNode;
    }

    else {
        while (curr != NULL){
            if (curr->next == NULL){            // go to last node
                curr->next = newNode;           // add new node to the end
                break;
            }

            curr = curr->next;
        }
    } 
                                            
    return head;
}

void no_Newline(char * string){

    while (*string){
        if (*string == '\n'){               // read each character in string. and if new line, repalce it with NULL.
            *string = '\0';
        }
        string++;
    }
}

void open_recent(char * newestDirName){
    int newestDirTime = -1;

    DIR* dirToCheck;
    DIR* tempDir;

    struct dirent *fileInDir;
    struct stat dirAttributes;

    char targetDirPrefix[64] = "lekevi.rooms.";

    dirToCheck = opendir(".");

    if (dirToCheck > 0){
        while ((fileInDir = readdir(dirToCheck)) != NULL){                  // read each file in directory
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL){        // if filel name containts the prefix
                stat(fileInDir->d_name, &dirAttributes);                    // get atrributes

                if ((int)dirAttributes.st_mtime > newestDirTime){           // if the file is newer
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);               // the newest filename is set
                }
            }
        }
    }

    closedir(dirToCheck);
}

void display_file(struct Room * arr_Rooms, struct Room * room){
    int i=0;
    int index=0;

    printf("CURRENT LOCATION: %s\n", room->name);               // basic output for room content

    printf("POSSIBLE CONNECTIONS: " );
    for (i=0; i < room->num_connects; i++){
        if (i == room->num_connects - 1){ 
            printf("%s.\n",room->connections[i]);
        }
        else{
            printf("%s, ", room->connections[i]);
        }
    }  
}

void setup_File(char * recent_Dir, struct Room * arr_Rooms){
    DIR* startDir;
    struct dirent *fileInDir;

    startDir = opendir(recent_Dir);             // opens directory

    char pathname[64];
    FILE *fp;
    
    char fromFile[128];
    char temp[128];
    int i=0;
    int j = 0;
    int connect_counter = 0;

    while ((fileInDir = readdir(startDir)) != NULL){
        sprintf( pathname, "%s/%s", recent_Dir, fileInDir->d_name );                        //gets and open files
        fp = fopen( pathname, "r" );

        while( fgets(fromFile, sizeof(fromFile), fp) != NULL){              // read each line
            if (strstr( fromFile , "ROOM NAME") != NULL){                   // if line contains room
                strcpy(temp, fromFile + 11 );
                no_Newline(temp);       
                strcpy(arr_Rooms[i-2].name , temp);                         // add to room struct array
            }

            if (strstr( fromFile , "CONNECTION") != NULL){                  // if line contains connections
                strcpy(temp, fromFile + 14 );
                no_Newline(temp);
                strcpy(arr_Rooms[i-2].connections[j] , temp);               // add ronnection to room
                connect_counter = connect_counter + 1;                      // increase connection counter
                j = j + 1;                                                  // j is index of connection array
            }

            if (strstr( fromFile , "TYPE") != NULL){                        // if one contains type
                strcpy(temp, fromFile + 11 );
                no_Newline(temp);           
                strcpy(arr_Rooms[i-2].type , temp);                         // add type to room struct array
            }

            arr_Rooms[i-2].num_connects = connect_counter;                  // i-2 because of . and .. in direcotry. add number of connections to room struct array
        }

        j = 0;
        i = i + 1;
        connect_counter = 0;
    }
}

struct Room * get_Room( struct Room * arr_Rooms,  char * input){
    int i=0;
    struct Room * room;
    for (i=0; i < 7; i ++){
        if( strcmp (arr_Rooms[i].name, input) == 0){                    // goes through each room in array
            room = &arr_Rooms[i];                                       // if name of room matches the user's input, return that room
            return room;
        }
    }
}

int check_Valid(char * input, struct Room * room){
    int i = 0;
    for (i=0; i < room->num_connects; i++){
        if ( strcmp(room->connections[i], input) == 0 ){                    // if the user input matches one of the names in the room's connections, ruturn true, else return false
            return 1;
        }
    }

    return 0;
}

int check_End(struct Room * room){
    if (strcmp(room->type, "END_ROOM") == 0){               // if the room is the ned room, return true, else, return false
        return 1;
    }

    return 0;
}

void * get_Time(void * thread_num){
    pthread_mutex_t lock; 
    FILE * file;

    char timeStamp[64];

    struct tm *tmp ;
    time_t t ;
    time( &t ); 
    tmp = localtime( &t ); 
    char * pathway = "./currentTime.txt";
    
    if ( *(int*)thread_num == 0){
        pthread_mutex_lock(&lock);

        file = fopen(pathway, "w+");

        strftime(timeStamp, sizeof(timeStamp), "%l:%M%P, %A, %B %d, %Y", tmp);
        fprintf(file, "%s\n", timeStamp);

        pthread_mutex_unlock(&lock); 
    }

    if ( *(int*)thread_num == 1){
        pthread_mutex_lock(&lock);
        file = fopen("./currentTime.txt", "r");
        fgets(timeStamp, sizeof(timeStamp), file);
        printf("\n%s\n", timeStamp );
        pthread_mutex_unlock(&lock); 

    } 

    fclose(file);
}

int start_time(){
    pthread_t thread_one [2];
    int arr[2];
    
    int i = 0; 
  
    for (i=0; i < 2; i++) { 
        arr[i]=i;
        pthread_create(&(thread_one[i]), NULL, &get_Time, &arr[i]); 
    } 
  
    pthread_join(thread_one[0], NULL); 
    pthread_join(thread_one[1], NULL); 
}

void game_loop(int * numMoves, struct Node * usersPath, struct Room * arr_Rooms, int first_index, struct Room  *room ){
    char input[64];
    int valid = 0;
    *numMoves = 0;

    struct Room * toRoom = room;
    do {
        display_file(arr_Rooms, toRoom);
        do {
            valid = 0;
            printf("WHERE TO? >"); 
            scanf("%s", input );
            valid = check_Valid(input, toRoom);                 // gets user input, and check if valid

            if (strcmp(input, "time") == 0 ){
                start_time();
                                                   // if user enters time, go to time function
            }

            else if (valid == 0) {
                printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");          //  if not valid, print out error.
                display_file(arr_Rooms, toRoom);                                        // display file contents again
            }   
        } while (valid == 0);                                           // keepe getting user input untill its valid

        toRoom = get_Room(arr_Rooms, input);                            // returns the valid room user requested

        *numMoves = *numMoves + 1;                                      // increase number of moves

        usersPath = append(usersPath, toRoom->name);                    // append new room to users path linked lest

        printf("\n");

    } while (check_End(toRoom) == 0);                               // break when end room is found

    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");

    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", *numMoves);        // print out number of moves

    struct Node * curr = usersPath;             

    while (curr != NULL){
        printf("%s\n", curr->val);                              // print out path
        curr = curr->next;
    }
}

void start_game(int * numMoves, struct Node * usersPath, struct Room * arr_Rooms ){
    int i=0;
    char * first_name;
    struct Room * room;

    int same=0;
    int first_index=0;

    for (i=0; i < 7; i++){
        same = strcmp(arr_Rooms[i].type, "START_ROOM");             // read through each room in array and if type is start, then start is found

        if (same == 0){
            room = &arr_Rooms[i];                               
            first_index = i;                                       // found first room index
            break;
        }
    }

    game_loop(numMoves, usersPath, arr_Rooms, first_index, room);   // main game loop
}

int main(){

    int numMoves;
    struct Node * usersPath = NULL;

    struct Room arr_Rooms [7];          //creates array of room structs to fill from file

    char recent_Dir[64];
    open_recent(recent_Dir);            //opens directory and finds newest file

    setup_File(recent_Dir, arr_Rooms);

    int i =0;

    start_game(&numMoves, usersPath, arr_Rooms);  // starts game

    return 0;
}