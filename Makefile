CC := clang
CFLAGS := -g -Wall -Wno-deprecated-declarations -Wno-unused-function -Werror

# Set compiler flags for macOS using brew, or point to Charlie's ssl library on MathLAN
SYSTEM := $(shell uname -s)
ifeq ($(SYSTEM),Darwin)
  CFLAGS += -I$(shell brew --prefix openssl)/include -L$(shell brew --prefix openssl)/lib
else
  CFLAGS += -I/home/curtsinger/.local/include -L/home/curtsinger/.local/lib
endif

all: storage client test

clean: 
	rm -rf storage client test

# storage: storage.c message.h message.c socket.h hashmap.c hashmap.h 
# 	$(CC) $(CFLAGS) -o storage storage.c message.c hashmap.c -lcrypto -lpthread 

storage: storage.c message.h message.c socket.h hashmap.c hashmap.h 
	$(CC) $(CFLAGS) -o storage storage.c message.c hashmap.c -lcrypto -lpthread -Wl,-rpath=/home/curtsinger/.local/lib


client: client.c message.h message.c 
	$(CC) $(CFLAGS) -o client client.c message.c

test: test.c message.h message.c
	$(CC) $(CFLAGS) -o test test.c message.c

# all: password-cracker

# clean:
# 	rm -rf password-cracker password-cracker.dSYM

# password-cracker: password-cracker.c
# 	$(CC) $(CFLAGS) -o password-cracker password-cracker.c -lcrypto -lpthread -lm

# zip:
# 	@echo "Generating password-cracker.zip file to submit to Gradescope..."
# 	@zip -q -r password-cracker.zip . -x .git/\* .vscode/\* inputs/\* .clang-format .gitignore password-cracker
# 	@echo "Done. Please upload password-cracker.zip to Gradescope."

# format:
# 	@echo "Reformatting source code."
# 	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
# 	@echo "Done."

# .PHONY: all clean zip format


# CC := clang
# CFLAGS := -g

# CFLAGS += -I$(shell brew --prefix openssl)/include -L$(shell brew --prefix openssl)/lib

# all: storage

# clean: 
# 	rm -rf storage client

# storage: storage.c message.h message.c socket.h hashmap.c hashmap.h 
# 	$(CC) $(CFLAGS) -o storage storage.c message.c hashmap.c -lcrypto -lpthread

# client: client.c message.h message.c 
# 	$(CC) $(CFLAGS) -o client client.c message.c