#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>

static int urandfd;

static off_t getfs(int fd) {
	struct stat st;
	if (fstat(fd, &st) != -1)
		return st.st_size;
	return (off_t)-1;
}

static int shred(char *fn) {
	int fd, ret = 1;
	off_t fs, curr = 0;
	unsigned char buf[16384];
	if((fd = open(fn, O_RDWR)) == -1) {
		perror(fn);
		return 1;
	}
	if((fs = getfs(fd)) == (off_t)-1) {
		fputs(fn, stderr);
		perror(": failed to get filesize");
		goto out;
	}
	ret = 0;
	while(curr < fs) {
		off_t left = fs - curr;
		if(left > 16384) left = 16384;
		read(urandfd, buf, left);
		if(left != write(fd, buf, left)) ret = 1;
		curr += left;
	}
out:
	close(fd);
	return ret;
}


static int usage() {
	fputs(
		"shred FILE1 [FILE2...]\n\n"
		"overwrites contents of FILEs with random garbage\n"
		, stderr
	);
	return 1;
}

int main(int argc, char **argv) {
	if(argc < 2) return usage();
	if((urandfd = open("/dev/urandom", O_RDONLY)) == -1) {
		perror("failed to open /dev/urandom");
		return 1;
	}
	int i, f = 0;
	for(i=1; i<argc; ++i) {
		f += shred(argv[i]);
	}
	close(urandfd);
	return f;
}
