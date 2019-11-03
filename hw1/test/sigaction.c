#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static struct sigaction saign, old_sa;

void sigint_handler(int sig) {
	puts("SIGINT handler before_fork");
}

void sigint_handler2(int sig) {
	puts("SIGINT handler child");
}

void setup_sig() {

	saign.sa_handler = sigint_handler;
	sigaction(SIGINT, &saign, NULL);
}

int main() {
	setup_sig();
	// sa.sa_handler = sigint_handler;
	pid_t cpid = fork();
	if(cpid == 0) {

		struct sigaction sa;
		sa.sa_handler = sigint_handler2;
		sigaction(SIGINT, &sa, NULL);
		execv("./loop", NULL);
	} else if (cpid > 0) {
		int status;
		wait(&status);
		printf("wait: %d\n", status);
	}
	while(1);
	return 0;
}
