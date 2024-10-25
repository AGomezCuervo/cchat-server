CC = gcc 
OP =  -o0
FLAGS = -std=c11 -Wall -Wextra -pedantic -ggdb -Wunused-function -Wmissing-prototypes -Wunreachable-code -Wmissing-declarations -Wshadow -Wcast-align
OBJECTS = main.o err.o
CFILES = main.c err.c
BINARY = server
# LIBS = -luv -lpthread
INSTALL_PATH = /usr/local/bin

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^

%.o:%.c
	$(CC) $(FLAGS) -c -o $@ $<

install: $(BINARY)
	install -m 0755 $(BINARY) $(INSTALL_PATH)

clean:
	rm $(BINARY) $(OBJECTS)
