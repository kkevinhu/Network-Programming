CC = gcc
CFLAGS = -Wall -Wextra -g
INC = -Iinclude
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

all: obj bin myshell

obj: 
	mkdir obj

bin:
	mkdir bin

myshell: $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $@ $^

# $@: myshell
# $^: $(OBJS)

obj/%.o: src/%.c 
	$(CC) $(CFLAGS) $(INC) -c -o $@ $^

clean: 
	rm obj/*.o

number:
	g++ -o ./bin/number test_src/number.cpp