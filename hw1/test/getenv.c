#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

int main() {
	char *env = getenv("PATH");
	char *env1 = getenv("PATH");

	puts(env);

	int i=0;
	char *ptr = environ[i];
	while(ptr) {
		puts(ptr);
		ptr=environ[++i];
	}
}
