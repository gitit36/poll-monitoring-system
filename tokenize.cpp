#include <iostream>
#include "tokenize.h"

using namespace std;

#define BUFSIZE 1028

char** tokenize::splitter(char* line){
	char* buf = (char*)malloc(BUFSIZE * sizeof(char));
	char** args = (char**)malloc(BUFSIZE * sizeof(char)); 
	int argnum = 0;
	char *delim; //I got the idea to use delim from shell lab
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
}