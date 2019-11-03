#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/types.h>
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


static sigset_t set, old_set;
static pid_t ccpid = -1;
static int bg_count = 0;

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
int cmd_cd(tok_t arg[]);
int cmd_wait(tok_t arg[]);

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
  {cmd_cd, "cd", "change current directory"},
  {cmd_wait, "wait", "wait for all background process"}
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

int cmd_wait(tok_t arg[]) {
	pid_t p;
	while((p = waitpid(-1, NULL, 0)) != 0) {
		if (errno == ECHILD) { // all waited
			break;
		} else {
			return 1;
		}
	}
	return 0;
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

char *path_resolve(char *cmd) {
	int max_len;
	char *buf = path_alloc(&max_len);
#ifdef _WIN32
	if(cmd[0] == '.' && cmd[1] == '\\') {
#else
	if(cmd[0] == '.' && cmd[1] == '/') {
#endif
		getcwd(buf, max_len);
		strcat(buf, &cmd[1]);
		return buf;
	}
	char *tmp = getenv("PATH");
	char *env_path = malloc(strlen(tmp) + 1);
	strcpy(env_path, tmp);
	char *p = strtok(env_path, ":");
	while(p) {
		size_t path_len = strlen(p);
		size_t remain_len = max_len - path_len - 1; // -1 for delim
		strcpy(buf, p);
#ifdef _WIN32
		strcat(buf, "\\");
#else
		strcat(buf, "/");
#endif
		strncat(buf, cmd, remain_len);
		struct stat st;
		if(lstat(buf, &st) == 0 && st.st_mode & S_IEXEC) {
			break;
		}
		p = strtok(NULL, ":");
	}
	free(env_path);
	if(!p) return NULL;
	char *r = malloc(strlen(buf)+1);
	strcpy(r, buf);
	free(buf);
	return r;
}

int io_redirect(tok_t *args){
	int out_index = isDirectTok(args, ">");
	int in_index = isDirectTok(args, "<");
	int fd;

	if(!out_index && !in_index) return 0;

	// assume all format correct
	if (out_index) {
		char *out_fname = args[out_index+1];
		if((fd = open(out_fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
			perror("open error:");
		}
		dup2(fd, STDOUT_FILENO);
	}
	
	if (in_index) {
		char *in_fname = args[in_index+1];
		if((fd = open(in_fname, O_RDONLY)) < 0) {
			perror("open error:");
		}
		dup2(fd, STDIN_FILENO);
	}

	if(in_index >0) {
		for(int i=in_index;i<MAXTOKS && args[i];i++) {
			args[i] = args[i+2];
			args[i+2] = NULL;
		}
	}
	if(out_index >0) {
		for(int i=out_index;i<MAXTOKS && args[i];i++) {
			args[i] = args[i+2];
			args[i+2] = NULL;
		}
	}
	
	return 0;
}

void sigint_handler(int sig) {
	if(sig == SIGINT) {
		puts("SIGINT handler");
	}
}

int run_external(char *cmd, tok_t args[], int is_bg) {
	pid_t cpid = fork();
	pid_t p;
	if(cpid > 0) { // parent
		ccpid = cpid;
		int status;
		if (is_bg) {
			fprintf(stdout, "[%d]: %d\n", ++bg_count, cpid);
			fprintf(stdout, "background work:");
			int i =0;
			tok_t t = args[i++];
			while(t) {
				fprintf(stdout, " %s", t);
				t = args[i++];
			}
			fputc('\n', stdout);
		} else if((p = wait(&status)) == cpid){
			if(WIFSIGNALED(status)) {
				putc('\n', stdout);
			}
		} 
	} else if(cpid == 0) { // child
		// struct sigaction sa;
		// sa.sa_handler = sigint_handler;
		// sigaction(SIGINT, &sa, NULL);
		//
		// signal handler not inherited after exec but mask do
		sigprocmask(SIG_SETMASK, &old_set, NULL);
		if(io_redirect(args) < 0)
			exit(1);
		execv(cmd, args);
		exit(0);
	}
	return 0;
}

int shell (int argc, char *argv[]) {
  char *s;			/* user input string */
  tok_t *t = NULL;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes pid */
  pid_t ppid = getppid();	/* get parents pid */

  printf("%s running as PID %d under %d\n",argv[0],pid,ppid);

  lineNum=0;
  fprintf(stdout,"%d: ",lineNum);
 while ((s = freadln(stdin))) {
    int is_bg;
    size_t t_len = 0;
    t = getToks(s, &t_len);		/* Break the line into tokens */
    fundex = lookup(t[0]);	/* Is first token a shell literal */
    if (fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {			/* Treat it as a file to exec */
      if(t[0] != NULL && strlen(t[0]) > 0) {
	      char *cmd = path_resolve(t[0]);
	      if(cmd == NULL) fprintf(stderr, "Command not found: %s\n", t[0]);
	      else {
		      if(strncmp(t[t_len-1], "&", 1) == 0)  {// is_bg
			      is_bg = 1;
			      t[t_len-1]=NULL;
		      } else {
			      int len = strlen(t[t_len-1]);
			      if(t[t_len-1][len-1] == '&') {
				      is_bg = 1;
				      t[t_len-1][len-1] = '\0';
			      }
		      }
		      run_external(cmd, t, is_bg);
		      free(cmd);
	      }
      }
    }
    fprintf(stdout,"%d: ",++lineNum);
  }
  if(t != NULL)
  	freeToks(t);
  return 0;
}

void setup_sig() {
	if(sigemptyset(&set) <0) {
		perror("sigemptyset err");
		exit(1);
	}
	if(sigaddset(&set, SIGINT) <0) {
		perror("sigaddset err");
		exit(1);
	}
	if(sigprocmask(SIG_BLOCK, &set, &old_set) <0) {
		perror("sigprocmask err");
		exit(1);
	}

}

int main (int argc, char *argv[]) {
	setup_sig();
  return shell(argc, argv);
}
