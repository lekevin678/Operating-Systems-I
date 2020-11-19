#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


struct room {
    char* name;
    int id;

    int num_connects;
    int connections[6];

    char* type;
};

struct node{
    struct room val;
    struct node * next;
};

struct node * push(struct node * head, struct room value){
    struct node * old = head;
    head = malloc(sizeof(struct node));
    head->val = value;
    head->next = old;                                           // function to push node into linked list, from the beginning

    
    return head;
}

struct node * assign_room_type(struct node * head){
    struct node * curr;

    int s = rand() % 7;                                         // pick a random number out of 7.
    int e = 0;

    do{
        e = rand() % 7;                                         // keep picking a random number until e does not equal s
    } while (e == s);

    int i = 0;
    curr = head;
    while (curr != NULL){

        if (i == s){                                            // if i equals s, then the room will be the start room
            curr->val.type = calloc(20, sizeof(char));
            strcpy(curr->val.type, "START_ROOM" );
        }

        else if (i == e){
            curr->val.type = calloc(20, sizeof(char));          // if i equals e, then the room will be the end room
            strcpy(curr->val.type, "END_ROOM" );
        }

        else{
            curr->val.type = calloc(20, sizeof(char));          // if i does not equal s or e, then the room will be mid.
            strcpy(curr->val.type, "MID_ROOM" );
        }

        curr=curr->next;
        i = i + 1;                                              // increment i and move in linked list
    }

    return head;
}

struct node * pick_rooms(struct node * head){
    int one, two, three = 0;
    struct node * curr;
    struct node * temp;

    do{
        one = rand() % 10 + 1;
        two = rand() % 10 + 1;
        three = rand() % 10 + 1;
    } while (one == two || two == three || one == three);               // randomly choose three numbers untill all three are different numbers.

    int i = 1;

    curr = head;
    if ( one == 1 || two == 1 || three == 1 ){                          // if one of the three numbers are one, shift the head to remove the first node from linked list.
            head = curr->next;
            curr = head;
            i = i + 1;
    }

    while ( curr != NULL){
        if ( i+1 == one || i+1 == two || i+1 == three){                 // if i+1 is one of the three numbers, make i next equal i next next to remove the node from the linked list.
            temp=curr->next->next;
            curr->next = temp;
            i = i + 1;
            continue;
        }

        curr = curr->next;
        i = i + 1;
    }

    return head;
} 

struct node * create_rooms_list(struct node * head){
    struct room Red_room;
    struct room Orange_room;
    struct room Yellow_room;
    struct room Green_room;
    struct room Blue_room;
    struct room Indigo_room;
    struct room Violet_room;
    struct room Pink_room;
    struct room Black_room;
    struct room White_room;

    Red_room.name = calloc(20, sizeof(char));               // create 10 rooms 
    strcpy(Red_room.name, "Red" );
    Red_room.id = 1;
    Red_room.num_connects = 0;
    Red_room.type = calloc(20, sizeof(char));
    head = push(head, Red_room);

    Orange_room.name = calloc(20, sizeof(char));
    strcpy(Orange_room.name, "Orange" );
    Orange_room.id = 2;
    Orange_room.num_connects = 0;
    Orange_room.type = calloc(20, sizeof(char));
    head = push(head, Orange_room);

    Yellow_room.name = calloc(20, sizeof(char));
    strcpy(Yellow_room.name, "Yellow" );
    Yellow_room.id = 3;
    Yellow_room.num_connects = 0;
    Yellow_room.type = calloc(20, sizeof(char));
    head = push(head, Yellow_room);

    Green_room.name = calloc(20, sizeof(char));
    strcpy(Green_room.name, "Green" );
    Green_room.id = 4;
    Green_room.num_connects = 0;
    Green_room.type = calloc(20, sizeof(char));
    head = push(head, Green_room);

    Blue_room.name = calloc(20, sizeof(char));
    strcpy(Blue_room.name, "Blue" );
    Blue_room.id = 5;
    Blue_room.num_connects = 0;
    Blue_room.type = calloc(20, sizeof(char));
    head = push(head, Blue_room);

    Indigo_room.name = calloc(20, sizeof(char));
    strcpy(Indigo_room.name, "Indigo" );
    Indigo_room.id = 6;
    Indigo_room.num_connects = 0;
    Indigo_room.type = calloc(20, sizeof(char));
    head = push(head, Indigo_room);

    Violet_room.name = calloc(20, sizeof(char));
    strcpy(Violet_room.name, "Violet" );
    Violet_room.id = 7;
    Violet_room.num_connects = 0;
    Violet_room.type = calloc(20, sizeof(char));
    head = push(head, Violet_room);

    Pink_room.name = calloc(20, sizeof(char));
    strcpy(Pink_room.name, "Pink" );
    Pink_room.id = 8;
    Pink_room.num_connects = 0;
    Pink_room.type = calloc(20, sizeof(char));
    head = push(head, Pink_room);

    Black_room.name = calloc(20, sizeof(char));
    strcpy(Black_room.name, "Black" );
    Black_room.id = 9;
    Black_room.num_connects = 0;
    Black_room.type = calloc(20, sizeof(char));
    head = push(head, Black_room);

    White_room.name = calloc(20, sizeof(char));
    strcpy(White_room.name, "White" );
    White_room.id = 10;
    White_room.num_connects = 0;
    White_room.type = calloc(20, sizeof(char));
    head = push(head, White_room);

    head = pick_rooms(head);
    head = assign_room_type(head);

    return head;

}

int ConnectionAlreadyExists (struct room * x, struct room * y){
    int i = 0;

    for (i; i <= 6; i++){                           
        if (y->connections[i] == x->id){            // if x's id is in y's array of connections, return 1 (that means connection already exist) 
            return 1;
        }
    }

    return 0;       
}

int IsSameRoom (struct room * x, struct room * y){
    if (x->id == y->id){                        // if x's id is the same as y's id, return one (same room)
        return 1;
    }

    return 0;
}

int CanAddConnectionFrom (struct room *a){
    if (a->num_connects < 6){                       //if the numbers of connection is less than 6, return 1 (able to connect room)
        return 1;
    }

    return 0;
}

struct room * GetRandomRoom(struct node * head){
    int random = rand() % 7;                // randomly choose a number out of 7.
    struct node * curr;
    curr = head;
    int i = 0 ;
    while (curr != NULL){
        if (i == random){                   // gets to the random node in the linked list and returns it
            return (&curr->val);
        }

        else {
            curr = curr->next;
            i = i + 1;
        }
    }

}

void ConnectRoom (struct room *a, struct room *b){
    a->connections[a->num_connects] = b->id;                // adds id to connection array of nodes.
    b->connections[b->num_connects] = a->id;

    a->num_connects = a->num_connects + 1;
    b->num_connects = b->num_connects + 1;
}


void addRandomConnection(struct node * head){
    struct room *a;
    struct room *b;

    do{
        a = GetRandomRoom(head);
    } while (CanAddConnectionFrom(a) == 0);         //keeps getting a random room while the number of connections is less than 6.

    do{
        b = GetRandomRoom(head);
    } while ( CanAddConnectionFrom(b) == 0 || IsSameRoom(a,b) == 1 || ConnectionAlreadyExists(a,b) == 1 ); // keeps getting a random room while the number of connections is less than 6, it is the same rooom, or if there is a connection alrady.

    ConnectRoom(a, b);                              // connect the rooms if passes tests.
    
}

int IsGraphFull(struct node * head) {
    struct node * curr = head;

    while (curr != NULL){
        if (curr->val.num_connects < 3){            // returns false if there is a room in the linked list with less than 3 connections
            return 0;
        }

        else {
            curr = curr->next;
        }
    }

    return 1;
}

struct node * create_connections(struct node * head){
    while (IsGraphFull(head) == 0) {            // keeps adding connections while there is a rooom with less than three connections
        addRandomConnection(head);
    }

    return head;
}


void create_files(struct node * head, char * file_name){
    struct node * curr = head;

    FILE * file;
    char pathway[64];                  

    char connect_temp[64];


    struct node * find_id = head;
    char id_name [64];

    int i = 0;
    while (curr != NULL){
        sprintf(pathway, "%s/%s", file_name, curr->val.name);   // creats a file in the folder created earlier and names it with the room name
        file = fopen(pathway, "w");                             // open the file created above with writing permissions

        fprintf(file, "ROOM NAME: %s\n", curr->val.name);       // print name of room to file

        for (i = 0; i < curr->val.num_connects; i++){
            find_id = head;
            while (find_id != NULL){                                    // look through linked list for room with the correct id from connections                        
                if (curr->val.connections[i] == find_id->val.id){       // if the id matches with the id of the room we are currently at, copy name to id_name
                    strcpy(id_name, find_id->val.name);
                    break;
                }
                else {
                    find_id = find_id->next;
                }
            }            
            fprintf(file, "CONNECTION %d: %s\n", i+1, id_name);         // print connections to file
        }

        fprintf(file, "ROOM TYPE: %s\n", curr->val.type);               // prints room type

        curr = curr->next;
    }

    fclose(file);
}

int main(){
    srand(time(0));

    int pid = getpid();                                     
    char * pre = "lekevi.rooms.";
    char file_name[64] = "";
    sprintf(file_name, "%s%d", pre, pid);                   // creates directory name
    mkdir(file_name, 0755);                                 // creates directory

    DIR* directory;
    directory = opendir(file_name);
    

    struct node * head = NULL;                              // creates head for linked list          

    head = create_rooms_list(head);  
    head = create_connections(head);

    create_files(head, file_name);                          // creates files

    closedir(directory);

    return 0;
}