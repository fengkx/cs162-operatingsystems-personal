#include <stdio.h>
#include <stdlib.h>

typedef unsigned long count_t;
static count_t total_wount, total_ccount, total_lcount;
static count_t wcount, ccount, lcount;

static inline void count_c(int c) {
	if(c == '\n')
		lcount++;
	ccount++;
}

static int getword(FILE * fp) {
	int c;
	if(feof(fp))
		return 0;

	while((c = getc(fp)) != EOF) {
		if(isalpha(c)) {
			wcount++;
			break;
		}
		count_c(c);
	}
	while(c != EOF) {
		count_c(c);
		if (!isalpha(c)) break;
		c = getc(fp);
	}
	return c != EOF;
}

static void count_one(char *file) {
	FILE *fp = fopen(file, "r");
	if(fp == NULL) {
		fprintf(stderr, "can't open file %s", file);
		return;
	}
	while(getword(fp));
	printf("%s: %6lu %6lu %6lu\n", file, wcount, lcount, ccount);
	total_lcount += lcount;
	total_ccount += ccount;
	total_wount += wcount;	
}

int main(int argc, char *argv[]) {

   ccount = lcount = wcount =0;
   total_ccount = total_lcount = total_wount =0;
   if (argc < 2) {
	   fputs("Usage: wc file1 [file2 file3]", stderr);
	   exit(1);
   }
   printf("%s %6s %6s %6s\n", "file", "wcount", "lcount", "ccount");
   for (int i=1;i<argc;i++) {
   	count_one(argv[i]);
   }
   return 0;
}
