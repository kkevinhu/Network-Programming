#ifndef PARSE_INPUT
#define PARSE_INPUT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

typedef struct command
{
    int tokenNumber;
	char token[10][50];
	char command[100];
	int type;
	int number;
	char fullpath[64];
	void (*func_ptr)(void *);
	struct command *next;
} command_t;

typedef struct env
{
	char name[10][32];
	char value[10][10][32];
	int count[10];
	int used;
} env_t;


command_t *Parser(char* buffer);
command_t *parseCommand(char *input);
void type(command_t *cmd);
int IsPipe(char *input);
int IsNumberPipe(char *input);

#endif