#include "generalInclude.h"
#include "databaseHelper.h"

void receiveHandler(message_t *message){
    while(message != NULL){
        printf("\033[0;31m"); //Set the text to the color red
        printf("[%s]: ", message->sender);
        printf("\033[0m"); //Resets the text to default color
        printf("%s\n", message->message);
        message = message->next;
    }
}

int main(int argc, char *argv[]){
    int err = openDatabase();
    if(err != 0){
        freeMemory();
        return EXIT_FAILURE;
    }
    err = receive(&receiveHandler);
    closeDatabase();

    freeMemory();

    return EXIT_SUCCESS;
}
