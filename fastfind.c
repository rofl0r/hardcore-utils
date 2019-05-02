#define _ALL_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BLOCKSIZE 64*1024
#define ATIME 1

static int usage(const char *a0) {
	dprintf(2, "usage: %s file term\n"
		   "search for term in file and print the offset if found\n\n"
		   "optimized for big block reads\n"
		   "designed to find strings of accidentally "
		   "deleted files in blockdevices.\n"
		, a0);
	return 1;
}

static off_t offset = 0;
static int sigc, neednl;
static void sigh(int nsig) {
	sigc++;
	dprintf(2, "\rcurrent offset: 0x%llx, elapsed %d", offset, sigc*ATIME);
	alarm(ATIME);
	neednl = 1;
}

int main(int argc, char **argv) {
	if(argc != 3) return usage(argv[0]);
	const char* file = argv[1], *term = argv[2];
	unsigned char *dblbuf = calloc(1, BLOCKSIZE * 2);
	unsigned char *readbuf = dblbuf + BLOCKSIZE;
	size_t termlen = strlen(term);
	unsigned char *searchbuf = readbuf - termlen;
	unsigned char *copybuf = readbuf + BLOCKSIZE - termlen;
	int fd = open(file, O_RDONLY), success=0;
	if(fd == -1) {
		perror("open");
		return 1;
	}
	signal(SIGALRM, sigh);
	alarm(ATIME);
	while(1) {
		ssize_t n = read(fd, readbuf, BLOCKSIZE);
		if(n <= 0) break;
		void* res = memmem(searchbuf, BLOCKSIZE+termlen, term, termlen);
		if(res) {
			if(neednl) dprintf(2, "\n");
			neednl = 0;
			dprintf(1, "bingo: 0x%llx\n", (unsigned long long) offset + ((uintptr_t) res - (uintptr_t)readbuf));
			fflush(stdout);
			success = 1;
		}
		memcpy(searchbuf, copybuf, termlen);
		offset += n;
	}
	return !success;
}
