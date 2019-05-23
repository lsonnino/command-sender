#include "databaseHelper.h"

char *databaseName = "messages";
char *database = "/etc/message-sender/messages.db";
sqlite3 *db;
void (*onReceive);

void freeMemory(){
    message_t *last = NULL;
    while(receivedMessages != NULL){
        free(receivedMessages->message);
        free(receivedMessages->sender);
        last = receivedMessages;
        receivedMessages = receivedMessages->next;
        free(last);
    }
}

void createDatabase(){
    char *sql = "CREATE TABLE IF NOT EXISTS Database(Proof INTEGER PRIMARY KEY NOT NULL);";

    char *err_msg;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to create table Database\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);

        return;
    }

    sql = "CREATE TABLE IF NOT EXISTS Messages("
            "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "Sender TEXT NOT NULL,"
            "Receiver TEXT NOT NULL,"
            "Message TEXT NOT NULL,"
            "Read INTEGER DEFAULT 0 NOT NULL,"
            "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to create table Messages\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);

        return;
    }

    sql = "INSERT INTO Database(Proof) VALUES (1);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed inserting into Database\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);

        return;
    }
}

bool getProofOfLife() {
    char *err_msg;
    int rc = sqlite3_exec(db, "SELECT Proof FROM Database;", retreiveProof, NULL, &err_msg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);

        return false;
    }
    return true;
}
int retreiveProof(void *NotUsed, int argc, char **argv, char **azColName) {
    return argc > 0 ? 0 : 1;
}

int openDatabase(){
    int rc = sqlite3_open_v2(database, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));

        sqlite3_close(db);

        return EXIT_FAILURE;
    }

    if(!getProofOfLife()){
        // Database does not exists (yet)
        createDatabase();

        if(!getProofOfLife()){
            printf("Could not create database\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void closeDatabase(){
    sqlite3_close(db);
}

char *getUsername(){
    /*register struct passwd *pass;
    register uid_t uid;

    uid = geteuid();
    pass = getpwuid(uid);
    char *username = pass->pw_name;*/
    char *username = getlogin();

    char *ret = (char*) malloc( (strlen(username) + 1) * sizeof(char));
    strcpy(ret, username);

    return ret;
}

char *concat(char *str1, const char *str2, bool freed){
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char *result = (char *) malloc( (len1 + len2 + 1) * sizeof(char));
    int i;
    for (i = 0; i < len1; i++) {
        *(result + i) = *(str1 + i);
    }
    for (i = 0; i <= len2; i++) { // take the \0 as well
        *(result + len1 + i) = *(str2 + i);
    }

    if(freed){
        free(str1);
    }

    return result;
}

int send(char *receiver, char* message){
    char *sender = getUsername();

    char *err_msg;
    char *sql = "INSERT INTO Messages(Sender, Receiver, Message) VALUES (\'";
    sql = concat(sql, sender, false);
    sql = concat(sql, "\', \'", true);
    sql = concat(sql, receiver, true);
    sql = concat(sql, "\', \'", true);
    sql = concat(sql, message, true);
    sql = concat(sql, "\');", true);

    free(sender);

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    free(sql);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to send message\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int receiveCallback(void *NotUsed, int argc, char **argv, char **azColName) {
    message_t *current = receivedMessages;

    if(receivedMessages == NULL){
        receivedMessages = (message_t*) malloc(sizeof(message_t));
        current = receivedMessages;
    }
    else {
        while(current->next != NULL){
            current = current->next;
        }
        current->next = (message_t*) malloc(sizeof(message_t));
        current = current->next;
    }

    int messageOffset = 0;
    int senderOffset = 1;
    if(strcmp(azColName[0], "Sender") == 0){
        messageOffset = 1;
        senderOffset = 0;
    }

    current->message = (char*) malloc( (strlen(argv[messageOffset])+1) * sizeof(char) );
    strcpy(current->message, argv[messageOffset]);

    current->sender = (char*) malloc( (strlen(argv[senderOffset])+1) * sizeof(char) );
    strcpy(current->sender, argv[senderOffset]);

    current->next = NULL;

    return 0;
}
int receive(void (*onReceive)(message_t *)){
    char *sql = "SELECT Message, Sender FROM Messages WHERE Receiver = \'";

    char *username = getUsername();
    sql = concat(sql, username, false);
    sql = concat(sql, "\' AND Read = \'0\' ORDER BY Timestamp;", true);

    char *err_msg;
    int rc = sqlite3_exec(db, sql, &receiveCallback, NULL, &err_msg);
    free(sql);
    if (rc != SQLITE_OK ) {
        free(username);

        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);

        return EXIT_FAILURE;
    }

    onReceive(receivedMessages);
    freeMemory();


    sql = "UPDATE Messages SET Read = \'1\' WHERE Receiver = \'";

    sql = concat(sql, username, false);
    free(username);
    sql = concat(sql, "\' AND Read = \'0\';", true);

    err_msg = 0;
    free(sql);
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to set data to read\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);

        return EXIT_FAILURE;
    }




    return EXIT_SUCCESS;
}
