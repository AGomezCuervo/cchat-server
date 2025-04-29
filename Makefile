CC = gcc
FLAGS = -Wall -Wextra -pedantic -ggdb -Wunused-function -Wmissing-prototypes -Wunreachable-code -Wmissing-declarations -Wshadow -Wcast-align -I/usr/include/libxml2 -Iinclude/
OBJECTS = build/main.o build/err.o build/utils.o build/socket.o build/signal.o build/arena.o
CFILES = src/main.c src/err.c src/utils.c src/socket.c src/arena.c src/signal.c
BINARY = server
LIBS = -lexpat -lc
INSTALL_PATH = /usr/local/bin

$(shell mkdir -p build)

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

build/%.o: src/%.c
	$(CC) $(FLAGS) -c -o $@ $<

install: $(BINARY)
	install -m 0755 $(BINARY) $(INSTALL_PATH)

clean:
	rm -rf $(BINARY) build/
