CC := clang
CFLAGS := -g

all: storage client

clean: 
        rm -rf storage client

storage: storage.c message.h message.c socket.h hashmap.c hashmap.h 
        $(CC) $(CFLAGS) -o storage storage.c message.c hashmap.c -lcrypto -lpthread

client: client.c message.h message.c 
        $(CC) $(CFLAGS) -o client client.c message.c