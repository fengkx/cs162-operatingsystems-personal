#include <stdio.h>
#include <string.h>

int main() {
	char s[] = "man strtok";
	char *delim = " \n";
	char *c =  strtok(s, delim);
	while(c) {
		puts(c);
		c = strtok(NULL, delim);
	}
}
