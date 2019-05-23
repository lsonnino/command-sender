#ifndef DATABASE_HELPER_H
#define DATABASE_HELPER_H

#include <sqlite3.h>
#include "generalInclude.h"
#include <pwd.h>
#include <unistd.h>

typedef struct message{
    char *message;
    char *sender;
    struct message *next;
} message_t;

message_t *receivedMessages;

void freeMemory();

void createDatabase();

bool getProofOfLife();
int retreiveProof(void *NotUsed, int argc, char **argv, char **azColName);

int openDatabase();

void closeDatabase();

char *getUsername();

int send(char *receiver, char* message);

int receiveCallback(void *NotUsed, int argc, char **argv, char **azColName);
int receive(void (*onReceive)(message_t *));

#endif
