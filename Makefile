CC=gcc
CFLAGS=-Wall -Werror

all: out/sendTo out/receive

out/init-message: out/sendTo out/receive build/initMessage.o
	$(CC) -o $@ $^ $(CFLAGS)

build/initMessage.o: src/initMessage.c src/generalInclude.h
	$(CC) -o $@ -c $< $(CFLAGS)

out/sendTo: build/sendTo.o build/databaseHelper.o
	$(CC) -o $@ $^ $(CFLAGS) -lsqlite3

build/sendTo.o: src/sendTo.c build/databaseHelper.o src/generalInclude.h
	$(CC) -o $@ -c $< $(CFLAGS)

out/receive: build/receive.o build/databaseHelper.o
	$(CC) -o $@ $^ $(CFLAGS) -lsqlite3

build/receive.o: src/receive.c build/databaseHelper.o src/generalInclude.h
	$(CC) -o $@ -c $< $(CFLAGS)

build/databaseHelper.o: src/databaseHelper.c src/databaseHelper.h src/generalInclude.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm build/*.o
	rm out/*
