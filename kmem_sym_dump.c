#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

static int open_kmem(int rdwr) {
	int fd = open("/dev/kmem", rdwr ? O_RDWR : O_RDONLY);
	return fd;
}

static int seek_kmem(int fd, off_t offset) {
	if(-1==lseek(fd, offset, SEEK_SET)) {
		perror("lseek");
		return 0;
	}
	return 1;
}

static int find_sym(const char *symname, unsigned long *off, unsigned long *end) {
	FILE *f = fopen("/proc/kallsyms", "r");
	if(!f) return 0;
	char buf[256];
	size_t l = strlen(symname);
	int succ = 0;
	while(fgets(buf, sizeof buf, f)) {
		char* p = buf;
		if(succ) {
			if(1!=sscanf(p, "%lx", end)) succ = 0;
			goto ret;
		}
		while(*p && *p != ' ') p++;
		p+=1;
		if(*p != 'D' && *p != 'T') continue;
		p+=2;
		if(!strncmp(p, symname, l)) {
			p=buf;
			while(isspace(*p))p++;
			if(1!=sscanf(p, "%lx", off)) goto ret;
			succ=1;
		}
	}
	ret:
	fclose(f);
	return succ;
}

int main(int argc, char **argv){
	unsigned long off, end;
	if(argc != 2) return 1;

	FILE *f = fopen("dump.bin", "w");
	if(!f) {
		perror("fopen");
		return 1;
	}

	if(!find_sym(argv[1], &off, &end)) {
		printf("couldnt find offsets\n");
		return 1;
	}

	int fd = open_kmem(0);
	if(!seek_kmem(fd, off)) {
		printf("kmem seek failed\n");
		return 1;
	}
	char buf[4096];
	size_t left, toread, tot = end - off;
	while(off<end) {
		left = end - off;
		toread = left >= sizeof(buf) ? sizeof(buf) : left;
		ssize_t n;
		if(toread != (n = read(fd, buf, toread))) {
			puts("read error");
			break;
		}
		fwrite(buf, toread, 1, f);
		off+=toread;
	}
	printf("dumped %zu bytes to dump.bin\n", tot);
	close(fd);
	fclose(f);

	return 0;
}
