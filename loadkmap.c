#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#define KDGKBTYPE 0x4B33
#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])
static int console_fd(int mode) {
	const char devs[][16] = { "/dev/tty", "/dev/vc/0", "/dev/console" };
	size_t i = 0;
	int fd = -1;
	while(i < ARRAYSIZE(devs) && fd == -1) {
		fd = open(devs[i++], mode);
		if(fd != -1 && ioctl(fd, KDGKBTYPE, &(char){0})) {
			close(fd);
			fd = -1;
		}
	}
	return fd;
}

struct kbentry {
        unsigned char kb_table;
        unsigned char kb_index;
        unsigned short kb_value;
};

#define KDSKBENT 0x4B47
#define NR_KEYS 128
#define MAX_NR_KEYMAPS 256

#include <stdio.h>
#include <stdlib.h>
#ifdef USE_LIBULZ
#include <ulz/stdio-repl.h>
#endif

#define noret __attribute__((noreturn))

noret static void die_perror(const char* msg) {
	perror(msg);
	exit(1);
}

noret static void usage(const char *arg0) {
	dprintf(2, "usage: %s < my.kmap\n"
	           "loads kmap from stdin.\n", arg0);
	exit(1);
}

static void fetch(int fd, void* buf, size_t cnt) {
	if(read(fd, buf, cnt) != (ssize_t) cnt) die_perror("read");
}

#include <errno.h>
#include <string.h>
int main(int argc, char** argv) {
	(void) argc;
	if(argv[1] || isatty(0)) usage(argv[0]);
	char flags[MAX_NR_KEYMAPS];
	fetch(0, flags, 7);
	if(memcmp(flags, "hcukmap", 7)) { errno = EINVAL; die_perror("invalid magic"); }
	fetch(0, flags, MAX_NR_KEYMAPS);
	int cfd = console_fd(O_WRONLY);
	if(cfd == -1) die_perror("could not get console fd");
	size_t i = 0, j;
	for(;i < MAX_NR_KEYMAPS; i++) if(flags[i]) {
		short kmap[NR_KEYS];
		fetch(0, kmap, sizeof kmap);
		for(j=0; j < NR_KEYS; j++) {
			struct kbentry ke = { .kb_index = j, .kb_table = i };
			ke.kb_value = ntohs(kmap[j]);
			if(ioctl(cfd, KDSKBENT, &ke))
				;
				// some keymaps contain the value 638 in the first entry, which means K_ALLOCATED.
				// not sure how to deal with this best; it seems it can be ignored.
				/* dprintf(2, "warning: failed to set key slot %zu,%zu to %d, reason: %s\n",
				        i, j, ke.kb_value, strerror(errno)); */
		}
	}
	return 0;
}
