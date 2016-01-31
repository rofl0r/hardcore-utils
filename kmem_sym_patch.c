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

static int syntax() {
	puts("syntax: kernelpatch funcname payload [offset]\n"
	     "where payload is a hexadecimal string containing the new code\n"
	     "an optional offset into the function may be given to apply a selective patch\n");
	return 1;
}

static void payload_from_hex(const char* s, unsigned char *pl) {
	static const char hx[]="0123456789abcdef";
	while(*s) {
		*pl = ((strchr(hx, tolower(s[0])) - hx) << 4) | (strchr(hx, tolower(s[1])) - hx);
		pl++;
		s+=2;
	}
}

int main(int argc, char **argv){
	unsigned long off, end, user_off = 0;
	if(argc < 3 || argc > 4) return syntax();
	if(argc == 4) user_off = atoi(argv[3]);
	size_t l = strlen(argv[2]);
	if(l & 1) return syntax();
	l=l/2;
	unsigned char payload[l];
	payload_from_hex(argv[2], payload);

	if(!find_sym(argv[1], &off, &end)) {
		puts("couldnt find offsets\n");
		return 1;
	}
	off+=user_off;
	if(l > end-off) {
		puts("error: payload greater than existing code");
		return 1;
	}

	int fd = open_kmem(1);
	if(fd == -1) {
		puts("couldnt open kmem");
		return 1;
	}
	if(!seek_kmem(fd, off)) {
		puts("kmem seek failed");
		close(fd);
		return 1;
	}
	int err = 0;
	if(l != write(fd, payload, l)) {
		err = 1;
		puts("error writing payload");
	}

	close(fd);

	return err;
}
