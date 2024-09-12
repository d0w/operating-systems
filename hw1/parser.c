// loop through string and separate
#include "parser.h"
#define BUFSIZE 512
#define WORDSIZE 32
#define DELIMITERS "|&<>"
#define WHITESPACE " \t\n"

enum tokenType {
    T_NONE,
    T_AMP,
    T_PIPE,
    T_WORD,
    T_LEFT_ARROW,
    T_RIGHT_ARROW
};

// splits by | 
char **parseLine(char *buffer) {
    char **commands = (char **) malloc(BUFSIZE * sizeof(char *));
    size_t tokenLength;
    enum tokenType token;

    // should be a struct of type "command" that holds the command and the next command
    char *currCommand = (char *) malloc(WORDSIZE * sizeof(char));

    int idx = 0;
    int len = 0;
    while ((token = getTokenType(buffer, &tokenLength)) != T_NONE) {

        if (token == T_PIPE) {
            // add command to array
            commands[idx] = strndup(buffer, len);

            // create new command

            // attach previous command to created command

            // set new currcommand to created command

            idx++;
            len = -1;
        }
        buffer += tokenLength;
        len++;

    }

    // the last command
    char *test = strndup(buffer, len);
    commands[idx] = strndup(buffer, len);

    return commands;
}

char **parseCommand(char *command) {
    char **args = (char **) malloc(BUFSIZE * sizeof(char *));
    size_t tokenLength;
    enum tokenType token;

    int idx = 0;

    // loop through command and separate by special characters
    while (command != T_NONE) {
        printf("command: %s\n", command);
        // error handle for incorrect pipe
        token = getTokenType(command, &tokenLength);
        if (token == T_WORD) {
            args[idx] = strndup(command, tokenLength);
            idx++;
        }
        command += tokenLength;
    }

    return args;
}

// FSM-like string parsing. Returns type of token
enum tokenType getTokenType(const char *cursor, size_t *tokenLength) {
    // skip over whitespace
    cursor += strspn(cursor, WHITESPACE);
    switch (*cursor) {
        case '&':
            *tokenLength = 1;
            return T_AMP;
        case '|':
            *tokenLength = 1;
            return T_NONE;
        case '<':
            *tokenLength = 1;
            return T_LEFT_ARROW;
        case '>':
            *tokenLength = 1;
            return T_RIGHT_ARROW;
        case '\0':
            *tokenLength = 0;
            return T_NONE;
        default:
            // if no special characters, set tokenLength to length of token (up to next whitespace or special char)
            *tokenLength = strcspn(cursor, DELIMITERS WHITESPACE);
            return T_WORD;
    }
    return T_NONE;
}


