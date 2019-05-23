#include "generalInclude.h"
#include "databaseHelper.h"

char *username;
char *message;

void freeAll(){
    if(username != NULL){
        free(username);
    }
    if(message != NULL){
        free(message);
    }

    freeMemory();
}

void help(){
    printf("Send messages to other accounts\n");
    printf("\tsendTo USERNAME MESSAGE\n");
}

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Missing username\n");
        return EXIT_FAILURE;
    }

    username = malloc( (strlen(argv[1]) + 1) * sizeof(char));
    if(username == NULL){
        fprintf(stderr, "An error occurred\n");
        return EXIT_FAILURE;
    }
    strcpy(username, argv[1]);

    if(strcmp(username, "--help") == 0 || strcmp(username, "-h") == 0){
        help();
        freeAll();
        return EXIT_SUCCESS;
    }

    if(argc < 3){
        fprintf(stderr, "Reading from stdin not supported by the current version\n");
        freeAll();
        return EXIT_FAILURE;
    }

    message = malloc( (strlen(argv[2]) + 1) * sizeof(char));
    if(message == NULL){
        fprintf(stderr, "An error occurred\n");
        return EXIT_FAILURE;
    }
    strcpy(message, argv[2]);
    int i;
    for (i = 3; i < argc; i++) {
        strcat(message, " ");
        strcat(message, argv[i]);
    }

    /*
        STATUS
        username: contains to who the message needs to be sent
        message: contains the message to send

        help has already been treated
    */

    int err = openDatabase();
    if(err != 0){
        freeAll();
        return EXIT_FAILURE;
    }

    err = send(username, message);

    closeDatabase();

    freeAll();

    return EXIT_SUCCESS;
}
