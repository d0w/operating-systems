#include "parser.h"
#include "shell.h"
#define BUFSIZE 512
#define WORDSIZE 32
#define DELIMITERS "|&<>\""
#define WHITESPACE " \t\n"

enum tokenType {
    T_NONE,
    T_AMP,
    T_PIPE,
    T_WORD,
    T_LEFT_ARROW,
    T_RIGHT_ARROW,
    T_NEXT,
    T_QUOTE
};

// separates command into array of strings (cd /here -> ["cd", "/here"])
char **parseCommand(char *command) {
    // REMOVE AFTER MINI DEADLINE 1
    // printf("%s\n", command);

    char **args = (char **) malloc(BUFSIZE * sizeof(char *));
    size_t tokenLength = 0;
    enum tokenType token;

    int idx = 0;

    // print command with leading and trailing spaces stripped
    printf("%s\n", command + strspn(command, WHITESPACE));
    fflush(stdout);

    // used only for printing out command, remove after mini deadline
    // size_t tempLen;
    // enum tokenType tempToken;


    // loop through command and separate by special characters, checking validity
    while ((token = getTokenType(command, &tokenLength)) != T_NONE) {
        if (token == T_QUOTE) {
            command += tokenLength;
            continue;
        }

        // get rid of leading whitespace
        command += strspn(command, WHITESPACE);

        // 
        char *temp = strndup(command,tokenLength);

        if (temp[0] == '"' && temp[strlen(temp)-1] == '"') {
            if (temp[0] == '"') {
                args[idx] = strndup(command+1, tokenLength-1);
            } else if (temp[strlen(temp)-1] == '"') {
                args[idx] = strndup(command, tokenLength-1);
            }
           // args[idx] = strndup(command+1, tokenLength-2);
        } else {
            args[idx] = strndup(command, tokenLength);
        }
        
        command += tokenLength;
        idx++;

        // remove after mini deadline
        // if ((tempToken = getTokenType(command, &tempLen)) == T_NONE) {
        //     printf("%s", temp);
        // } else {
        //     printf("%s ", temp);
        // }

        free(temp);


    }
    // remove after mini deadline
    // printf("\n");

    args[idx] = NULL;

    return args;
}

// splits by |. Returns head of linked list of tokenized commands
struct Command *parseLine(char *buffer) {
    struct Command *head = NULL;
    struct Command *current = NULL;
    struct Command *prev = NULL;

    char *bufferStart = buffer + strspn(buffer, WHITESPACE);
    size_t tokenLength = 0;
    int len = 0;
    enum tokenType token;

    while (1) {
        token = getTokenType(buffer, &tokenLength);
                    // printf("Buffer: %s\n", buffer);

        if (token == T_NONE) {
            break;
        }
        //
        if (token == T_PIPE) {
            size_t temp = 0;
            buffer += strspn(buffer, WHITESPACE);

            // checking for pipe at beginning of command and if pipe does not have command before
            if (len == 0) {
                fprintf(stderr,"ERROR: Invalid command. Pipe has no command before\n");
                return NULL;
            } else if (getTokenType(buffer+tokenLength, &temp) == T_PIPE) {
                fprintf(stderr, "ERROR: Invalid command. Pipe has no command before\n");
                return NULL;
            }

            // Create a new command for previous part
            // current = (struct Command *)malloc(sizeof(struct Command));
            current = initCommand();
            current->args = parseCommand(strndup(bufferStart, len));
            current->next = NULL;

            if (current->args == NULL) {
                return NULL;
            }

            // Attach previous command to the current command
            if (prev) {
                prev->next = current;
            } else {
                head = current;
            }

            // Update pointers
            prev = current;
            buffer += tokenLength;

            bufferStart = buffer;
            len = 0;
        } else {
            buffer += tokenLength;
            len += tokenLength;
        }
    }

    // Handle the last command
    if (len > 0) {


        // current = (struct Command *)malloc(sizeof(struct Command));
        current = initCommand();
        current->args = parseCommand(strndup(bufferStart, len));
        current->next = NULL;

        if (prev) {
            prev->next = current;
        } else {
            head = current;
        }
    }

    return head;
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
            return T_PIPE;
        case '<':
            *tokenLength = 1;
            return T_LEFT_ARROW;
        case '>':
            *tokenLength = 1;
            return T_RIGHT_ARROW;
        case '\0':
            *tokenLength = 0;
            return T_NONE;
        case '\n':
            *tokenLength = 0;
            return T_NEXT;
        case '"':
            *tokenLength = 1;
            return T_QUOTE;
        default:
            // if no special characters, set tokenLength to length of token (up to next whitespace or special char)
            *tokenLength = strcspn(cursor, DELIMITERS WHITESPACE);
            return T_WORD;
    }
    return T_NONE;
}


