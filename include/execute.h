#ifndef EXECUTE_SHELL
#define EXECUTE_SHELL

#define BUILD_IN 0
#define NON_BUILD_IN 1
#define NO_FORK 2
#define ERROR 3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parse.h"

void Execute(command_t *cmd, int numIN);

void initenv();
int  find_env(char *name);
void myprintenv(void *par);
void mysetenv(void *par);
void myquit(void *par);
void myhelp(void *par);

int  builtin(command_t *cmd);
int  nonbuiltin(command_t *cmd);

void ExeSingle(command_t *cmd, int numIN);
void singlePar(command_t *cmd, int numIN);
void singleChild(command_t *cmd, int numIN);
int  ExePipe(command_t *cmd, int numIN);
void pipePar(command_t *cmd, int *fd, int RW, int numIN);
int  pipeChild(command_t *cmd, int *fd, int RW, int numIN);
int  Pipe(int *fd, int RW, int numIN);

#endif