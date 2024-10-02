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
int parseCommand(char *command, struct Command *obj, int isBegin, int isEnd) {
    // REMOVE AFTER MINI DEADLINE 1
    // printf("%s\n", command);

    char **args = (char **) malloc(BUFSIZE * sizeof(char *));
    size_t tokenLength = 0;
    enum tokenType token;

    int idx = 0;

    // print command with leading and trailing spaces stripped
    // printf("%s\n", command + strspn(command, WHITESPACE));
    // fflush(stdout);

    // used only for printing out command, remove after mini deadline
    // size_t tempLen;
    // enum tokenType tempToken;


    while ((token = getTokenType(command, &tokenLength)) != T_NONE) {
        // if quote, skip quote
        if (token == T_QUOTE) {
            command += tokenLength;
            continue;
        }
        // if at the first or last command in pipeline, allow for redirection
        else if (token == T_LEFT_ARROW) {
            if (!isBegin && !(isEnd && isBegin)) {
                // fprintf(stderr, "ERROR: Invalid command. Cannot redirect input\n");
                return -1;
            }

            // skip over whitespace, token, then whitespace to next token
            command += strspn(command, WHITESPACE);
            command += tokenLength;
            command += strspn(command, WHITESPACE);

            token = getTokenType(command, &tokenLength);   
            if (token != T_WORD) {
                // fprintf(stderr, "ERROR: Invalid command. Expected source for redirect\n");
                return -1;
            }
            obj->inPath = strndup(command, tokenLength);
            command += tokenLength;
        } else if (token == T_RIGHT_ARROW) {
            if (!isEnd && !(isEnd && isBegin)) {
                // fprintf(stderr, "ERROR: Invalid command. Cannot redirect output\n");
                return -1;
            }
            command += strspn(command, WHITESPACE);
            command += tokenLength;
            command += strspn(command, WHITESPACE);

            token = getTokenType(command, &tokenLength);
            if (token != T_WORD) {
                // fprintf(stderr, "ERROR: Invalid command. Expected destination for redirect\n");
                return -1;
            }
            obj->outPath = strndup(command, tokenLength);
            command += tokenLength;
        }

        else {

            // get rid of leading whitespace
            command += strspn(command, WHITESPACE);

            char *temp = strndup(command,tokenLength);

            // to handle quotes that wrap around a word
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


    }
    args[idx] = NULL;
    obj->args = args;

    return 0;
}

// splits by |. Returns head of linked list of tokenized commands
struct Command *parseLine(char *buffer, int *background) {
    struct Command *head = NULL;
    struct Command *current = NULL;
    struct Command *prev = NULL;

    // Used to keep track of the start of the next command
    char *bufferStart = buffer + strspn(buffer, WHITESPACE);

    size_t tokenLength = 0;
    int len = 0;
    enum tokenType token;

    // keep track of first or last command in pipeline
    int first = 1;
    int last = 0;

    while (1) {
        token = getTokenType(buffer, &tokenLength);
                    // printf("Buffer: %s\n", buffer);

        if (token == T_NONE) {
            break;
        }
        if (token == T_AMP) {
            // check if we are the end of the line
            buffer += strspn(buffer, WHITESPACE);
            buffer += tokenLength;
            if (getTokenType(buffer, &tokenLength) != T_NONE) {
                fprintf(stderr, "ERROR: Invalid command. & must be at the end of the line\n");
                return NULL;
            }

            *background = 1;

            // len--;
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

            // Create a new command for command up to | 
            current = initCommand();
            int res = parseCommand(strndup(bufferStart, len), current, first, last);
            current->next = NULL;

            if (res == -1) {
                fprintf(stderr, "ERROR: Invalid command. Could not parse command\n");
                cleanCommand(current);
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
            first = 0;
        } else {
            buffer += tokenLength;
            len += tokenLength;
        }


    }

    // Handle the last command
    if (len > 0) {
        last = 1;
        current = initCommand();
        // current->args = parseCommand(strndup(bufferStart, len), current);
        int res = parseCommand(strndup(bufferStart, len), current, first, last);
        current->next = NULL;

        if (res == -1) {
            fprintf(stderr, "ERROR: Invalid command. Could not parse command\n");
            cleanCommand(current);
            return NULL;
        }

        if (prev) {
            prev->next = current;
        } else {
            head = current;
        }
        last = 0;
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


