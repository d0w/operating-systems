// loop through string and separate
#include "parser.h"
#include "shell.h"
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
    T_RIGHT_ARROW,
    T_NEXT
};

// separates command into array of strings (cd /here -> ["cd", "/here"])
char **parseCommand(char *command) {
    char **args = (char **) malloc(BUFSIZE * sizeof(char *));
    size_t tokenLength = 0;
    enum tokenType token;

    int idx = 0;

    // printf("Command: %s\n", command);

    // loop through command and separate by special characters, checking validity
    while ((token = getTokenType(command, &tokenLength)) != T_NONE) {
        // error handle for incorrect pipe
        if (token == T_PIPE) {
            fprintf(stderr, "ERROR: Unexpected pipe is not preceded by a command.\n");
            return NULL;
        }

        command += strspn(command, WHITESPACE);
        args[idx] = strndup(command, tokenLength);
        
        command += tokenLength;
        idx++;
    }
    args[idx] = NULL;

    return args;
}

// splits by | 
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
            buffer += strspn(buffer, WHITESPACE);

            // Create a new command for previous part
            current = (struct Command *)malloc(sizeof(struct Command));
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


        current = (struct Command *)malloc(sizeof(struct Command));
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
        default:
            // if no special characters, set tokenLength to length of token (up to next whitespace or special char)
            *tokenLength = strcspn(cursor, DELIMITERS WHITESPACE);
            return T_WORD;
    }
    return T_NONE;
}


