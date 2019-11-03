#include <stdlib.h>
#include <stdio.h>

int main() {
	char *memory = malloc(4096);
	printf("Address of memory is %p", memory);
	free(memory);
	printf("Address of memory is %p", memory);
}
