/*

Copyright (C) 2009 Rob Landley <rob@landley.net>
Copyright (C) 2012 rofl0r

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>


__attribute__((noreturn))
void usage(void) {
        printf("usage: mkswap path-to-blockdevice\n");
        exit(1);
}

__attribute__((noreturn))
void die(const char* msg) {
        perror(msg);
        exit(1);
}

#include <sys/ioctl.h>
#include <sys/mount.h>

// Return how long the file at fd is, if there's any way to determine it.
off_t fdlength(int fd) {
	off_t bottom = 0, top = 0, pos, old;
	int size;

	// If the ioctl works for this, return it.

	if (ioctl(fd, BLKGETSIZE, &size) >= 0) return size*512L;

	// If not, do a binary search for the last location we can read.  (Some
	// block devices don't do BLKGETSIZE right.)  This should probably have
	// a CONFIG option...

	old = lseek(fd, 0, SEEK_CUR);
	do {
		char temp;

		pos = bottom + (top - bottom) / 2;

		// If we can read from the current location, it's bigger.

		if (lseek(fd, pos, 0)>=0 && read(fd, &temp, 1)==1) {
			if (bottom == top) bottom = top = (top+1) * 2;
			else bottom = pos;

			// If we can't, it's smaller.

		} else {
			if (bottom == top) {
				if (!top) return 0;
				bottom = top/2;
			} else top = pos;
		}
	} while (bottom + 1 != top);

	lseek(fd, old, SEEK_SET);

	return pos + 1;
}

//int fstat(int fildes, struct stat *buf);
off_t fdlength2(int fd) {
	struct stat s;
	int ret;
	if((ret = fstat(fd, &s)) == -1)
		die("fstat");
	return s.st_size;
}

int main(int argc, char** argv) {
	int fd;
	if(argc != 2) usage();
	if ((fd  = open(argv[1], O_RDWR)) == -1) die("open");
	
	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	off_t len = fdlength(fd);
	if (len < pagesize) 
		die("file to small");

	size_t pages = (len/pagesize)-1;
	unsigned int swap[129] = {0};

	// Write header.  Note that older kernel versions checked signature
	// on disk (not in cache) during swapon, so sync after writing.

	swap[0] = 1;
	swap[1] = pages;
	lseek(fd, 1024, SEEK_SET);
	write(fd, swap, 129*sizeof(unsigned int));
	lseek(fd, pagesize-10, SEEK_SET);
	write(fd, "SWAPSPACE2", 10);
	fsync(fd);

	close(fd);

	printf("Swapspace size: %luk\n", pages*(unsigned long)(pagesize/1024));
	return 0;
}
