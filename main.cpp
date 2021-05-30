#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include "tokenize.h"

using namespace std;

#define LOAD_FACTOR 20
#define BUFSIZE 1028

// Hash table
struct Node{
    int rin;
    string fname;
    string lname;
    int zip;
    int mark;
    Node* next;
};

struct Bucket{
    int key;
    int count;
    Node* voterlist;
};

struct HashTable{
    int size;
    Bucket** table;
};

// Implementing hash for storing zip codes
struct zipNode{
    Node* voter;
    zipNode* next;
};

struct zipBucket{
    int zip;
    zipNode* ziplist;
    zipBucket* next;
};

int tablesize_;
struct HashTable* CreateHashTable(int tableSize);
int GetKey(int rin, int tableSize);
bool HashSearch(struct HashTable* hTable, int rin);
void hInsert(struct HashTable* hTable, struct zipBucket* first, int rin, string fname, string lname, int zip);
void hDelete(struct HashTable* hTable, int rin);
void DeleteHashTable(struct HashTable* hTable);
void PrintHashTable(struct HashTable* hTable);
struct HashTable* ReHash(struct HashTable* hTable, struct zipBucket* first);
void readCSV(struct HashTable* hTable, struct zipBucket* first, char* filename);
void registerVoter(struct HashTable* hTable, struct zipBucket* first, int rin);
void bulkVote(struct HashTable* hTable, struct zipBucket* first ,string filename, tokenize t);
void displayYes(struct zipBucket* first);
void perc(struct HashTable* hTable, struct zipBucket* first);
void printzNodes(struct zipBucket* first, int zip);
void printAllzNodes(struct zipBucket* first);
void zInsert(struct HashTable* hTable, struct zipBucket* first, int rin, int zip);
Node* zipSearch(struct HashTable* hTable, int rin, int zip);


////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]){

    tokenize t;
    
    for (int i = 1; i < argc; ++i){
        string arg = argv[i];
        // set hash table size
        if (arg == "-m"){
            tablesize_ = stoi(argv[i+1]);
        }
    }
    
    struct HashTable* hTable = CreateHashTable(tablesize_);
    struct zipBucket* first = (struct zipBucket*)malloc(sizeof(struct zipBucket));


    for (int i = 1; i < argc; ++i){
        string arg = argv[i];
        // initial set of qualiffied participants from a CSV
        if (arg == "-f"){
            readCSV(hTable, first, argv[i+1]);
        }
    }

    while(1){
    	string command;
    	string arr[6];
    	cout << "prompt$ ";
    	getline (cin, command);

    	//source: https://stackoverflow.com/questions/16029324/splitting-a-string-into-an-array-in-c-without-using-vector
    	int i = 0;
    	stringstream ssin(command);
    	while (ssin.good() && i < 6){
	        ssin >> arr[i];
	        ++i;
    	}
        
        if (arr[0] == "l"){HashSearch(hTable, stoi(arr[1]));}
        if (arr[0] == "i"){
        	hInsert(hTable, first, stoi(arr[1]), arr[2], arr[3], stoi(arr[4]));
        	PrintHashTable(hTable);}
        if (arr[0] == "d"){hDelete(hTable, stoi(arr[1]));}
        if (arr[0] == "r"){registerVoter(hTable, first, stoi(arr[1]));}
        if (arr[0] == "bv"){bulkVote(hTable, first, arr[1], t);}
        if (arr[0] == "v"){displayYes(first);}
        if (arr[0] == "perc"){perc(hTable, first);}
        if (arr[0] == "z"){printzNodes(first, stoi(arr[1]));}
        if (arr[0] == "o"){printAllzNodes(first);}
        if (arr[0] == "exit"){break;}
        if (arr[0] == "dh"){DeleteHashTable(hTable);}
        if (arr[0] == "ph"){PrintHashTable(hTable);}
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////

void readCSV(struct HashTable* hTable, struct zipBucket* first, char* filename){
    //Citation: https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
    //Generate a file pointer
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    //define the arguments that need to be passed in the hash table
    int rin;
    string fname;
    string lname;
    int zip;
    int mark;
    //open the file 
    fp = fopen(filename, "r");
    //check if the file is empty
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
    
        //Extract the first token - the rin
        char * token = strtok(line, " ");
        rin = stoi(token);
        
        //Extract the second token - the first name
        token = strtok(NULL, " ");
        fname = token;
        
        //Extract the third token - the last name
        token = strtok(NULL, " ");
        lname = token;

        //Extract the forth token - the zip code
        token = strtok(NULL, " ");
        zip = stoi(token);

        //Insert the values into the hash table
        hInsert(hTable, first, rin, fname, lname, zip);
    }

    PrintHashTable(hTable);
    //close the pointer
    fclose(fp);
    //free the memory
    if (line)
        free(line);
} 

// To construct the hash table
struct HashTable* CreateHashTable(int tableSize){
    struct HashTable* hTable;
    hTable = (struct HashTable*)malloc(sizeof(struct HashTable));

    if (!hTable){
        cout << "Memory allocation fail" << endl;
        return NULL;
    }

    hTable -> size = tableSize;
    hTable -> table = (struct Bucket**)malloc(sizeof(struct Bucket*)*tableSize);

    cout << "Table size -> " << hTable -> size << endl;
    if (!hTable -> table){
        cout << "Memory allocation fail" << endl;
        return NULL;
    }

    // Memory allocation and reset
    for (int i = 0; i < tableSize; i++){
        hTable -> table[i] = (struct Bucket*)malloc(sizeof(struct Bucket));
        if (!hTable -> table[i]){
            cout << "Memory allocation fail" << endl;
            return NULL;
        }
        hTable -> table[i] -> key = i;
        hTable -> table[i] -> count = 0;
        hTable -> table[i] -> voterlist = NULL;
    }
    cout << "HashTable Created" << endl;
    return hTable;
}

/* Command "r": register the voter with ID <rin> as having already voted by changing her status to YES */
void registerVoter(struct HashTable* hTable, struct zipBucket* first, int rin){
    struct Bucket* hNode;
    struct Node* temp = NULL;
    // get the linked list of participants from the hash table
    hNode = hTable -> table[GetKey(rin, hTable -> size)];
    temp = hNode -> voterlist;

    if (!temp || (hNode == NULL)){
        cout << "Participant not found" << endl;
        return;
    }

    while (temp){
        if (temp -> rin == rin){
            if (temp -> mark == 1){
                cout << "Voting status already marked as YES" << endl;
                return;
            }
            else{
                temp -> mark = 1; // mark as voted "YES"
                zInsert(hTable, first, rin, temp -> zip); // insert the participant in the zipcode linked list
                cout << "Voting status of participant [" << temp -> rin 
                << " " << temp -> fname << " " << temp -> lname 
                << " " << temp -> zip << "] marked as YES" << endl;
            }
        }
        temp = temp -> next;
    }
}

// string* tokenize(string line){
//     int counter = 1;
//     int leng = line.length();
//     for (int i = 0; i < leng; i++){
//         if(line[i] == ','){
//             counter++;
//         }
//     }
//     string tokenized[counter];
//     string cur = "";
//     int counter2 = 0;
//     for (int i = 0; i < leng; i++){
//         if(line[i]==','){
//             tokenized[counter2++] = cur;
//             cur = "";
//         }
//         else{
//             cur.append(1,line[i]);
//         }
//     }
//     tokenized[counter2++] = cur;
//     return tokenized;
    
// }

// To tokenize the input from file for "bulk-vote"
/*char** splitter(char* line){
	char* buf = (char*)malloc(BUFSIZE * sizeof(char));
	char** args = (char**)malloc(BUFSIZE * sizeof(char)); 
	int argnum = 0;
	char *delim;
	strcpy(buf, line);
	buf[strlen(buf)] = ','; //making the end of line a space so that delim can end gracefully
	delim = strchr(buf, ',');
	
	while (delim){
		args[argnum++] = buf;
		*delim = '\0';
		buf = delim+1;
		while(*buf && (isspace(*buf))) buf++;
		
			delim = strchr(buf, ','); //head to the next block	
		
	}
	if (!argnum) return NULL; //if no arguments return null so that it does not clutter history

	args[argnum] = NULL;//set the end of the array
	free(delim);
	return args;
} */

/*Command "bv": bulk-vote for all the keys (i.e., <rin>s) that appears in text file <fileofkeys>*/
void bulkVote(struct HashTable* hTable, struct zipBucket* first, string filename, tokenize t){
    //Citation: https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
    //Generate a file pointer
    FILE * fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    //define the arguments that need to be passed in the hash table
    int rin;
    //open the file 
    fp = fopen(filename.c_str(), "r");

    if (fp == NULL)
        exit(EXIT_FAILURE);
        
    while ((read = getline(&line, &len, fp)) != -1) {
        //Extract token - the rin
        char** token = t.splitter(line);
        int counter = 0;
        while(token[counter++]) ;
        for (int i=0; i<counter-1;i++){
            rin = stoi(token[i]);
            registerVoter(hTable, first, rin);
        }
    }
    //close the pointer
    fclose(fp);
    //free the memory
    if (line)
        free(line);
}

/*Command "perc": display the percentage of people whose vote has been as YES 
over the number of poll participants who are part of the hast-table.*/
void perc(struct HashTable* hTable, struct zipBucket* one){
    // source: https://www.tutorialspoint.com/print-nodes-of-linked-list-at-given-indexes-in-c-language
    struct zipBucket *current;

    current = one->next;

    if (current == NULL) {
        cout << "Empty list" << endl;
        return;
    }
    cout << "% of people who voted YES : ";

    float count = 0;
    while (current != NULL) {
        struct zipNode *point;
        point = current->ziplist;
        while(point){
            count++;
            point = point->next;
        }
        current = current -> next;
    }

    float hNode_count = 0;
    for (int i = 0; i < hTable -> size; i++){
        struct Bucket* first = hTable -> table[i];
        struct Node* temp = first -> voterlist;
        while (temp){
            hNode_count++;
            temp = temp -> next;
        }
    }
    float percent = (count / hNode_count)*100;
    cout << percent << "%" << endl;
}

/* Command "v": present the number of people marked as having voted so far */
void displayYes(struct zipBucket* head){
    // source: https://www.tutorialspoint.com/print-nodes-of-linked-list-at-given-indexes-in-c-language
    struct zipBucket *current;

    current = head->next;

    if (current == NULL) {
        cout << "Empty list" << endl;
        return;
    }
    cout << "People who voted YES : ";

    while (current != NULL) {
        struct zipNode *point;
        point = current->ziplist;
        while(point){
            cout << point -> voter -> rin << " ";
            point = point->next;
        }
        current = current -> next;
    }
    cout << endl;
}

/* To get the position of the hashnode within the hash table AKA hash function*/
int GetKey(int rin, int tableSize){
    return rin % tableSize;
}

/* Command "z" : print the number of all poll participants 
marked as having voted YES and list their ids one id per line */
void printzNodes(struct zipBucket* first, int zip){
    struct zipBucket *bhead = first;
    struct zipBucket *cur = bhead->next;
    struct zipNode *head;

    if (cur == NULL){
        cout << "Empty list" << endl;
        return;
    }
    // iterate through the zipcode linked list
    while(cur != NULL){
        if(cur -> zip == zip){
            cout << "Poll participants (RINs) with zipcode [" << cur -> zip << "]:" << endl;
            head = cur-> ziplist;
            // iterate through the list of participants within the zipcode node
            while(head){
                cout << head -> voter -> rin << endl;
                head = head -> next;
            }
        }
        cur = cur -> next;
    }
}


/* Command "o" : produce a output list of zipcodes in decreasing order of 
the number of people who live in each zipcode and have marked as Y(es) */
void printAllzNodes(struct zipBucket* first){
    struct zipBucket *bhead = first;
    struct zipBucket *cur = bhead->next;
    struct zipNode *head;

    if (cur == NULL){
        cout << "Empty list" << endl;
        return;
    }

    // get the number of zipcodes
    int zipcount = 0;
    while(cur != NULL){
        cur = cur -> next;
        zipcount++;
    }

    // array of zips as elements
    int *zip = new int[zipcount];
    // array with length corresponding to the list's length
    int *arr = new int[zipcount];
    // set the head pointer back to the initial zipcode node (second node)
    cur = bhead -> next;

    // iterate through the zipcodes again
    int index = 0;
    while(cur != NULL){
        //copy zipcode into array
        zip[index] = cur -> zip;
        //proceed to the nodes that store participants
        head = cur -> ziplist; 
        // count the number of participants
        int count = 0;
        while(head){
            count++;
            // next participant
            head = head -> next;
        }
        // input the number counts in the array
        arr[index] = count;
        // next zipcode
        cur = cur -> next;
        index++;
    }

    // make a copy of the array
    int *arr_copy = new int[zipcount];
    for (int i=0; i<zipcount; i++){
        arr_copy[i] = arr[i];
    }

    //Sorting participant array copy in a descending order
    //Source: https://www.includehelp.com/cpp-programs/sort-an-array-in-descending-order.aspx
    for (int i=0; i<zipcount; i++){		
        for (int j=i+1; j<zipcount; j++){
            if (arr_copy[i]<arr_copy[j]){
                int temp = arr_copy[i];
                arr_copy[i] = arr_copy[j];
                arr_copy[j] = temp;
            }
        }
    }

    int *arr_cmp = new int[zipcount];
    int *check = new int[zipcount];
    int has_appeared; 
    int counter = 0;

    for (int i=0; i<zipcount; i++){
        has_appeared = 0;
        if (i == 0){
            check[0] = arr[i];
        }
        else {
            int loop = 0;
            counter = 0;
            while(check[loop] != 0){
                if (check[loop] == arr[i]){
                    has_appeared = 1;
                    counter = counter + 1;
                }
                loop ++;

            }
            check[loop] = arr[i];

        }

        //cout << "Check loop" << check << endl;
        for (int j=0; j<zipcount; j++){
            if (arr[i] == arr_copy[j]){

                //if it has appeared
                if(has_appeared == 1){
                    arr_cmp[i] = j + counter ; 
                }
                else{
                    arr_cmp[i] = j;
                }
                break;
            }

        }
    }

    // line up zipcode according to the desceding order of # of participants
    int *new_zip = new int[zipcount];
    for (int i=0; i<zipcount; i++){
        new_zip[arr_cmp[i]] = zip[i]; 
    }

    // Now, arranging both sorted zip and sorted count
    for (int i=0; i<zipcount; i++){
        cout << "Participants with zipcode [" << new_zip[i] << "]: ";
        cout << arr_copy[i] << endl;
    }
    free(zip);
    free(arr);
    free(arr_copy);
    free(new_zip);
}

// Insert participants into the zipcode linked list (those who voted "YES")
void zInsert(struct HashTable* hTable, struct zipBucket* first, int rin, int zip){
    struct zipBucket *bhead = first;

    struct zipNode *head;
    struct zipNode* zipnode = (struct zipNode*)malloc(sizeof(struct zipNode));

    // A linked list within a linked list
    // Linked list to store pointers to the nodes (participants) chained off of the hash table
    Node *temp = zipSearch(hTable, rin, zip);
    zipnode -> voter = temp;
    zipnode -> next = NULL;

    if (!bhead->next){
        struct zipBucket* zipbucket = (struct zipBucket*)malloc(sizeof(struct zipBucket));
        zipbucket -> zip = zip;
        zipbucket -> next = NULL;
        zipbucket -> ziplist = zipnode;
        bhead->next = zipbucket;

        head = bhead->ziplist;
        
        return;
    }
    struct zipBucket *prev = bhead;
    struct zipBucket *cur = bhead->next;

    int newly_created = 1;
    while(cur != NULL){
        if(cur->zip == zip){
            newly_created = 0;
            break;
        }
        cur = cur -> next;
        prev = prev->next;
    }

    if (newly_created){
        struct zipBucket* zipbucket = (struct zipBucket*)malloc(sizeof(struct zipBucket));
        zipbucket -> zip = zip;
        zipbucket -> next = NULL;
        zipbucket -> ziplist = zipnode;
        prev->next = zipbucket;
        return;    
    }
    head = cur->ziplist;
    while(head->next){
        head = head->next;
    }
    head->next = zipnode;
}

/* Constructing a new participant within the zipcode linked listv*/
Node* zipSearch(struct HashTable* hTable, int rin, int zip){

    struct Bucket* hNode;
    struct Node* temp;

    hNode = hTable -> table[GetKey(rin, hTable -> size)];
    temp = hNode -> voterlist;

    if (!temp){
        cout << "Participant not found" << endl;
        return NULL;
    }
    while (temp){
        if (temp -> zip == zip){
            return temp;
        }
        temp = temp -> next;
    }
    return NULL;
}

/* Command "i": inserts all information for a specific voter whose ID is <rin>, 
last and first names are <lname> and <fname> respectively, and she/he resides in <zip>. */
void hInsert(struct HashTable* hTable, struct zipBucket* first, int rin, string fname, string lname, int zip){
    struct Bucket* hNode;
    hNode = hTable -> table[GetKey(rin, hTable -> size)];

    struct Node* temp;
    temp = hNode -> voterlist;

    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    if (!node){
        cout << "Memory allocation fail..." << endl;
        return;
    }

    node -> rin = rin;
    node -> fname = fname;
    node -> lname = lname;
    node -> zip = zip;
    node -> mark = 0;
    node -> next = NULL;

    if (!temp){
        hNode -> voterlist = node;
    }

    // Exist duplication 
    else{
        struct Node* temp2;
        temp2 = hNode -> voterlist;
        hNode -> voterlist = node;
        node -> next = temp2;
    }

    hNode -> count++;

    //ReHash condition check
    if ((hNode -> count / hTable -> size) > LOAD_FACTOR){
        hTable = ReHash(hTable, first);
    }
}


/* Command "l": lookup the hash-table for a voter with id: <key>. If found, print the pertinent record out on the tty;
otherwise, print an error indication. */
bool HashSearch(struct HashTable* hTable, int rin){

    struct Bucket* hNode;
    struct Node* temp = NULL;

    hNode = hTable -> table[GetKey(rin, hTable -> size)];
    temp = hNode -> voterlist;

    if (!temp){
        cout << "Participant not found" << endl;
        return false;
    }
    while (temp){
        if (temp -> rin == rin){
            cout << "Participant found: " << " "
            << temp -> rin << " " << temp -> fname << " " << temp -> lname << " " << temp -> zip << endl;
            return true;
        }
        temp = temp -> next;
    }
    return false;
}


/* Command "d": delete the voter with ID <rin> */
void hDelete(struct HashTable* hTable, int rin){
    struct Bucket* first;
    first = hTable -> table[GetKey(rin, hTable -> size)];

    struct Node* temp;
    temp = first -> voterlist;

    struct Node* before = temp;
    if (!temp){
        cout << "Voter not founnd" << endl;
        return;
    }
    while (temp){
        if (temp -> rin == rin){
            if (before == temp){
                first -> voterlist = temp -> next;
            }
            else{
                before -> next = temp -> next;
            }
            free(temp);
            break;
        }
        before = temp;
        temp = temp -> next;
    }
    cout << "Data deletion success" << endl;
    first -> count--;
}

/* Additional command "ph": print the entire hash table*/
void PrintHashTable(struct HashTable* hTable){
    cout << "\n%%%---------- PRINT HASH TABLE ----------%%%" << endl;
    for (int i = 0; i < hTable -> size; i++){
        struct Bucket* first = hTable -> table[i];
        struct Node* temp = first -> voterlist;

        cout << "Key " << i << ": ";
        while (temp){
            cout << temp -> rin << " ";
            cout << temp -> fname << " ";
            cout << temp -> lname << " ";
            cout << temp -> zip << " ";
            temp = temp -> next;
        }
        cout << endl;
    }
    cout << endl;
}

/* Additional command "dh": delete the entire hash table*/
void DeleteHashTable(struct HashTable* hTable){
    for (int i=0; i<hTable -> size; i++){
        struct Node* temp = NULL;
        struct Node* before = NULL;
        temp = hTable -> table[i] -> voterlist;
        before = temp;
        while (temp){
            before = temp;
            temp = temp -> next;
            free(before);
        }
    }
    for (int i=0; i<hTable -> size; i++){
        free(hTable -> table[i]);
    }
    free(hTable);

    cout << "Hash table deleted" << endl;
}

/* Create a new hash table structure */
struct HashTable* ReHash(struct HashTable* hTable, struct zipBucket* one){
    struct HashTable* oldTable = hTable;
    hTable = CreateHashTable(hTable -> size * 2);
    if (!hTable){
        cout << "Memory allocation fail..." << endl;
        return NULL;
    }

    // Copying data within the old table
    for (int i = 0; i < oldTable -> size; i++){
        struct Bucket* first = oldTable -> table[i];
        struct Node* temp = first -> voterlist;

        while (temp){
            hInsert(hTable, one, temp -> rin, temp -> fname, temp -> lname, temp -> zip);
            temp = temp -> next;
        }
    }
    DeleteHashTable(oldTable);
    cout << "Data scattered according to the lOAD_FACTOR" << endl;
    return hTable;
}