#include <stdio.h>
#include <stdlib.h>

static int syntax() {
	printf("bin2sh - converts a file into a shellscript which recreates "
		"the file when run\nthe output is written to stdout\n"
	       "bin2sh [-c N] filename\n"
	       "where N is the number of chars per line\n");
	return 1;
}

static int is_c_arg(char* s) {
	return s[0]=='-' && s[1]=='c' &&!s[2];
}
int main(int argc, char** argv) {
	if(argc == 1 || argc > 4 || argc == 3 || 
	   (is_c_arg(argv[1]) && argc != 4)) return syntax();
	int f_arg = 1;
	unsigned cpl = 20;
	if(is_c_arg(argv[1])) {
		f_arg = 3;
		cpl = atoi(argv[2]);
		if(!cpl) return syntax();
	}
	FILE *f = fopen(argv[f_arg], "r");
	if(!f) { perror("fopen"); return 1; }
	unsigned long long cnt = 0;
	unsigned char buf[1];
	while(fread(buf, 1, 1, f)) {
		if(!(cnt % cpl)) {
			if(cnt) printf("\"");
			else printf("#!/bin/sh");
			printf("\necho -ne \"");
		}
		printf("\\x%02X", buf[0]);
		cnt++;
	}
	if(cnt % cpl) printf("\"\n");
	fclose(f);
	return 0;
}
