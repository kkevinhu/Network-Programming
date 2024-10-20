#include "execute.h"

env_t *env;

void initenv() {
	env = (env_t *)malloc(sizeof(env_t));
	strcpy(env -> name[0], "PATH");
	strcpy(env -> value[0][0], ".");
	strcpy(env -> value[0][1], "bin");
	strcpy(env -> value[0][2], "/bin");
	memset(env->count, 0, sizeof(env->count));
	env -> count[0] = 3;
	env -> used = 1;
}

int find_env(char *name) {
	for (int i=0; i<env->used; i++) {
		if (strncmp(env->name[i], name, strlen(env->name[i])) == 0)
			return i;
	}
	return -1;
}

void myprintenv(void *par) {
	int find = find_env(((command_t *)par)->token[0]);
	if (find != -1) {
		for (int i=0; i<env->count[find]; i++)
			printf("%s ", env->value[find][i]);
	}
	printf("\n");
}

void mysetenv(void *par) {
	int find = find_env(((command_t *)par)->token[0]);
	if (find >= 0) {
		strcpy(env -> value[find][env->count[find]], ((command_t *)par)->token[1]);
		env->count[find]++;
	}
	else if (find == -1) {
		strcpy(env -> name[env->used], ((command_t *)par)->token[0]);
		strcpy(env->value[env->used][0], ((command_t *)par)->token[1]);
		env->count[env->used]++;
		env->used++;
	}
}

void myhelp(void *par) {
	(void)par;
	printf("Help\n");
}

void myquit(void *par)
{
	(void)par;
	printf("Close Shell\n");
    exit(EXIT_SUCCESS);
}

void Execute(command_t *cmd, int numIN) {
	if (cmd -> next != NULL) {  //pipe
		ExePipe(cmd, numIN);
	}
	else {  // single
		ExeSingle(cmd, numIN);
	}
}

int builtin(command_t *cmd) {
	if (cmd == NULL) return -1;
	if (strncmp(cmd->command, "help", strlen("help")) == 0) {
		cmd -> type = BUILD_IN;
		cmd -> func_ptr = myhelp;
		return 1;
	}
	else if (strncmp(cmd->command, "quit", strlen("quit")) == 0) {
		cmd -> type = NO_FORK;
		cmd -> func_ptr = myquit;
		return 1;
	}
	else if (strncmp(cmd->command, "setenv", strlen("setenv")) == 0) {
		cmd -> type = NO_FORK;
		cmd -> func_ptr = mysetenv;
		return 1;
	}
	else if (strncmp(cmd->command, "printenv", strlen("printenv")) == 0) {
		cmd -> type = BUILD_IN;
		cmd -> func_ptr = myprintenv;
		return 1;
	}
	else
		return -1;
}

int nonbuiltin(command_t *cmd) {
	if (cmd == NULL) return -1;
	int find = find_env("PATH");
	char str[50];
	if (find == -1)
		return -1;
	for (int i=0; i<env->count[find]; i++) {
		sprintf(str, "%s/%s", env->value[find][i], cmd->command);
		if (access(str, X_OK) == 0) {
			strcpy(cmd -> fullpath, str);
			cmd->type = NON_BUILD_IN;
			return 1;
		}
	}
	return -1;
}

void ExeSingle(command_t *cmd, int numIN) {
	switch (cmd->type) {
		case NO_FORK:     //quit setenv
			cmd -> func_ptr(cmd); 
			break;
		case ERROR:      //error command
			printf("ERROR\n");
			break;
		default:		 //fork
			singlePar(cmd, numIN);
			break;
	}
}

void singlePar(command_t *cmd, int numIN) {
	pid_t pid = fork();
	switch (pid) {
		case -1:
			perror("Fork Error\n");
			break;
		case 0:
			singleChild(cmd, numIN);
			break;
		default:
			waitpid(pid, NULL, 0);
			break;
	}
}

void singleChild(command_t *cmd, int numIN) {
	if (cmd -> number != -1) {            //number pipe write
		printf("OPEN NUMPIPE\n");
		int op = open("numpipe.txt", O_WRONLY | O_TRUNC);
		if (op == -1) {
			perror("Open error");
			exit(EXIT_FAILURE);
			return;
		}
		else {
			if (dup2(op, STDOUT_FILENO) < 0) {
				perror("dup2 error");
				exit(EXIT_FAILURE);
				return;
			}
		}
		close(op);
	}
	if (numIN == 1) {                    //number pipe read
		printf("READ NUMPIPE\n");
		int op = open("numpipe.txt", O_RDONLY);
		if (op == -1) {
			perror("Open error");
			exit(EXIT_FAILURE);
			return;
		}
		else {
			if (dup2(op, STDIN_FILENO) < 0) {
				perror("dup2 error");
				exit(EXIT_FAILURE);
				return;
			}
		}
		close(op);
	}
	if (cmd->type == NON_BUILD_IN) {     //ls cat...
		char *arg[16];
		arg[0] = cmd->fullpath;
		for (int i=0; i<cmd->tokenNumber; i++) 
			arg[i+1] = cmd->token[i];
		arg[cmd->tokenNumber + 1] = NULL;
		execvp(cmd->fullpath, arg);
	}
	else {								//printenv help...
		cmd->func_ptr(cmd);
		exit(EXIT_SUCCESS);
	}
}

int ExePipe(command_t *cmd, int numIN) {
	int fd[2];
	if (pipe(fd) < 0) {
		perror("Pipe Error\n");
        return -1;
	}
	else {
		pipePar(cmd, fd, 1, numIN);    //write
		pipePar(cmd->next, fd, 0, numIN);    //read
	}
	return 1;
}

void pipePar(command_t *cmd, int *fd, int RW, int numIN) {
	if (strcmp(cmd->command, "setenv") == 0) //check whether is setenv
		return;
	if (cmd -> type == ERROR) {
		printf("Unknown command: [%s].\n", cmd -> command);
		return;
	}
	pid_t pid = fork();
	switch (pid) {
		case 0: //child
			pipeChild(cmd, fd, RW, numIN);
			break;
		case -1:
			perror("Fork error\n");
			return;
		default: //parent
			if (RW == 0) {  //read (write first, ensure )
				close(fd[0]);
				close(fd[1]);
			}
			waitpid(pid, NULL, 0);
			break;
	}
}

int pipeChild(command_t *cmd, int *fd, int RW, int numIN) {
	if (Pipe(fd, RW, numIN) == -1)
		return -1;
	if (cmd->type == BUILD_IN) {
		cmd->func_ptr(cmd);
		exit(EXIT_SUCCESS);
		return 1;
	}
	else if (cmd->type == NON_BUILD_IN) {
		char *arg[16];
		arg[0] = cmd->fullpath;
		for (int i=0; i<cmd->tokenNumber; i++) 
			arg[i+1] = cmd->token[i];
		arg[cmd->tokenNumber+1] = NULL;
		execvp(cmd->fullpath, arg);
	}
	exit(EXIT_SUCCESS);
	return 1;
} 

int Pipe(int *fd, int RW, int numIN) { // pipe fd[0] fd[1]
	if (RW == 1) {      //write
		close(fd[0]);
		if (dup2(fd[1], STDOUT_FILENO) < 0) {
			perror("W dup2 error");
			exit(EXIT_FAILURE);
			return -1;
		}
		close(fd[1]);
		if (numIN == 1) {                    //number pipe read
			int op = open("numpipe.txt", O_RDONLY);
			if (op == -1) {
				perror("Open error");
				exit(EXIT_FAILURE);
				return -1;
			}
			else {
				if (dup2(op, STDIN_FILENO) < 0) {
					perror("dup2 error");
					exit(EXIT_FAILURE);
					return -1;
				}
			}
			close(op);
		}
	}
	else if (RW == 0) { //read
		close(fd[1]);
		if (dup2(fd[0], STDIN_FILENO) < 0) {
			perror("R dup2 error");
			exit(EXIT_FAILURE);
			return -1;
		}
		close(fd[0]);
	}	
	return 1;
}