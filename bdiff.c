#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef USE_LIBULZ
#include <ulz/stdio-repl.h>
#endif

static int fetch(FILE *f, off_t *cnt) {
	int c = fgetc(f);
	if(c >= 0) *cnt = *cnt + 1;
	return c;
}

static int diff(FILE *f1, FILE* f2) {
	off_t min;
	size_t diffs = 0;
	struct stat st1, st2;
	if(fstat(fileno(f1), &st1)) return 1;
	if(fstat(fileno(f2), &st2)) return 1;
	min = st1.st_size;
	off_t p1, p2, l;
	if (st2.st_size != min) {
		printf("sizes differ! %llu, %llu\n",
		       (long long) st1.st_size, (long long) st2.st_size);
		if (st2.st_size < min) min = st2.st_size;
	}
	p1 = 0;
	p2 = 0;
	while(p1 < min) {
		l = 0;
		while(p1 < min && fetch(f1, &p1) != fetch(f2, &p2)) l++;
		if(l) {
			printf("difference at offset %llu of size %llu\n",
			       (long long) p1-l-1,  (long long) l);
			diffs++;
		}
	}
	printf("%zu differences in %llu bytes detected.\n",
	        diffs, (long long) min);
	return diffs != 0;
}


static void syntax(void) {
	printf("syntax: filename1 filename2\nlist differences in binaries\n");
	exit(1);
}

static int canread(char* fn) {
	return access(fn, R_OK) != -1;
}

int main(int argc, char** argv) {
	if(argc < 3) syntax();
	if(!canread(argv[1]) || !canread(argv[2])) {
		perror("can not read");
		return 1;
	}
	return diff(fopen(argv[1], "r"), fopen(argv[2], "r"));
}
