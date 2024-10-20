#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include "execute.h"

int counter;

void print(command_t *cmd) {
	if (cmd == NULL) return;
	for (int i=0; i<cmd->tokenNumber; i++)
            printf("token=%s\n", cmd->token[i]);
	if (cmd->next != NULL) {
		for (int i=0; i<cmd->next->tokenNumber; i++)
			printf("next token=%s\n", cmd->next->token[i]);
	}
}

void myShell() {

	initenv();
	counter = -1;

    while(1)
	{
		printf("MyShell%% ");
		
		char commandStr[256] = {0};
		command_t *cmd;
						
		fgets(commandStr, sizeof(commandStr), stdin);
		commandStr[strlen(commandStr) - 1] = 0;

		if (strlen(commandStr) == 0)
			continue;

		cmd = Parser(commandStr);	

		int numIn = (counter == 1);

		if (cmd -> number != -1)
			counter = cmd -> number;
		else if (cmd -> type != ERROR)
			counter--;

		printf("number: %d\n", cmd -> number);
		printf("numIn: %d\n", numIn);

		Execute(cmd, numIn);		
		
		free(cmd);
	}		
}

int main() {
    myShell();
    return 0;
}