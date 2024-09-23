#ifndef PARSER_H
#define PARSER_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"
// #include <unistd.h> 



// TRY USING FSM
/* 
If normal character, just add to array
if special charadcter, switch state
    - state 0 - add char to array
    - state 1 (|) - take output from previous state and pipe to next command
    - state 2 (<) - take input from file, go back to state 0
    - state 3 (>) - output to file, go back to state 0
    - state 4 (&) - run in background, go back to state 0
*/

// parse command and separate into string array
int parseCommand(char *buffer, struct Command *command, int first, int last);

enum tokenType getTokenType(const char *token, size_t *tokenLength);
// 
struct Command *parseLine(char *buffer, int *background);


#endif

