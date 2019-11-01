#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/wait.h>

#define FALSE 0
#define TRUE 1
#include "io.h"
#include "parse.h"


#define _FB_PATH_MAX 1024
#ifdef PATH_MAX
static int _path_max = PATH_MAX;
#else
static int _path_max = 0;
#endif

char *path_alloc(int *s) {
	if(_path_max == 0) {
		_path_max = _FB_PATH_MAX;
	} 
	char *buf = malloc(_path_max * sizeof(char));
	*s = _path_max;
	return buf;

}

int cmd_quit(tok_t arg[]) {
  printf("Bye\n");
  exit(0);
  return 1;
}

int cmd_help(tok_t arg[]);
int cmd_pwd(tok_t arg[]);
int cmd_cd(tok_t argp[]);

/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token arra and rtn int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_pwd, "pwd", "get current directory"},
  {cmd_cd, "cd", "change current directory"}
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

// pwd
int cmd_pwd(tok_t arg[]) {
	int s = 0;
	char *buf = path_alloc(&s);
	if(getcwd(buf, s) != NULL) {
		puts(buf);
	} else {
		free(buf);
		perror("error: getcwd");
		return 1;
	}
	free(buf);
	return 0;
} 

// cd
int cmd_cd(tok_t arg[]) {
	int err = chdir(arg[0]);
	if(err < 0) {
		perror("error: chdir");
		return 1;
	}
	return 0;
}


int lookup(char cmd[]) {
  if(cmd == NULL || strlen(cmd) == 0)
	  return -1;
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (strcmp(cmd_table[i].cmd, cmd) == 0) return i;
  }
  return -1;
}

int run_external(char *args[]) {
	pid_t cpid = fork();
	int status;
	if(cpid > 0) { // parent
		if(wait(&status) != cpid) {
			fprintf(stderr, "%s", "wait error");
		}
	} else if(cpid == 0) { // child
		execv(args[0], args);
		exit(0);
	}
	return 0;
}

int shell (int argc, char *argv[]) {
  char *s;			/* user input string */
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes pid */
  pid_t ppid = getppid();	/* get parents pid */

  printf("%s running as PID %d under %d\n",argv[0],pid,ppid);

  lineNum=0;
  fprintf(stdout,"%d: ",lineNum);
 while ((s = freadln(stdin))) {
    t = getToks(s);		/* Break the line into tokens */
    fundex = lookup(t[0]);	/* Is first token a shell literal */
    if (fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {			/* Treat it as a file to exec */
      // fprintf(stdout,"This shell currently supports only built-ins.  Replace this to run programs as commands.\n");
      if(t[0] != NULL && strlen(t[0]) > 0)
	      run_external(t);
    }
    fprintf(stdout,"%d: ",++lineNum);
  }
  return 0;
}

int main (int argc, char *argv[]) {
  return shell(argc, argv);
}
