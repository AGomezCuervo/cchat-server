CC = gcc
OP = -O0
FLAGS = -std=c11 -Wall -Wextra -pedantic -ggdb -Wunused-function -Wmissing-prototypes -Wunreachable-code -Wmissing-declarations -Wshadow -Wcast-align -I/usr/include/libxml2 -Iinclude/
OBJECTS = build/main.o build/err.o build/utils.o
CFILES = src/main.c src/err.c src/utils.c
BINARY = server
LIBS = -lxml2
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
