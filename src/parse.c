#include "parse.h"
#include "execute.h"

command_t *Parser(char* buffer) {
	command_t *cmd = (command_t *)malloc(sizeof(command_t));
	int pipidx = IsPipe(buffer);
	int numpipidx = IsNumberPipe(buffer);
	if (pipidx == -1) {    // single command
		cmd = parseCommand(buffer);
		cmd -> number = numpipidx;
		cmd -> next = NULL;
	}
	else { 				   // pipe 
		cmd = parseCommand(buffer);
		cmd -> next = parseCommand(buffer + pipidx + 1);
		cmd -> next -> next = NULL;
	}
	type(cmd);
	return cmd;
}

command_t *parseCommand(char *input) {
	command_t *cmd = (command_t *)malloc(sizeof(command_t));
	cmd -> tokenNumber = 0;
	while (input[0] == ' ') input ++;
	char *str = strsep(&input, " ");
	strncpy(cmd->command, str, strlen(str));
	while (1) {
		str = strsep(&input, " ");
		if (str == NULL)
			break;
		if (strlen(str) != 0){
			strncpy(cmd->token[cmd -> tokenNumber], str, strlen(str));
			cmd -> tokenNumber++;
		}
	}
	return cmd;
}

void type(command_t *cmd) {
	int builtin1 = builtin(cmd);
	int builtin2 = builtin(cmd -> next);
	int nonbuiltin1 = (builtin1 != -1) ? -1 : nonbuiltin(cmd);
	int nonbuiltin2 = (builtin2 != -1) ? -1 : nonbuiltin(cmd -> next);
	if (builtin1 == -1 && nonbuiltin1 == -1)
		cmd -> type = ERROR;
	if (cmd -> next && builtin2 == -1 && nonbuiltin2 == -1)
		cmd -> next -> type = ERROR;
}

int IsPipe(char *input) {
	int len = strlen(input)-1;
	for (int i=0; i<len; i++) {
		if (input[i] == '|' && input[i+1] == ' ') {
			input[i] = '\0';
			return i;
		}
	}
	return -1;
}

int IsNumberPipe(char *input)
{
	int len = strlen(input)-1;
	for (int i=0; i<len; i++) {
		if (input[i] == '|' && isdigit(input[i+1])) {
			input[i] = '\0';
			return atoi(input + i + 1); //number pipe number
		}
	}
	return -1;
}