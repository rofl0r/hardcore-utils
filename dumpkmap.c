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
		char arg = 0;
		if(fd != -1 && ioctl(fd, KDGKBTYPE, &arg)) {
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

#define KDGKBENT 0x4B46
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
	dprintf(2, "usage: %s > backup.kmap\n"
	           "dumps current keymap to stdout.\n"
	           "redirect to file as output contains binary data\n", arg0);
	exit(1);
}

int main(int argc, char** argv) {
	(void) argc;
	if(argv[1] || isatty(1)) usage(argv[0]);
	const char flags[MAX_NR_KEYMAPS] = {
		[0] = 1, [1] = 1, [2] = 1,
		[4] = 1, [5] = 1, [6] = 1,
		[8] = 1, [9] = 1, [10] = 1,
		[12] = 1,
	};
	int cfd = console_fd(O_RDONLY);
	if(cfd == -1) die_perror("could not get console fd");
	write(1, "hcukmap", 7);
	write(1, flags, MAX_NR_KEYMAPS);
	size_t i = 0, j;
	for(;i < 13; i++) if(flags[i]) for(j=0; j < NR_KEYS; j++) {
		struct kbentry ke = { .kb_index = j, .kb_table = i };
		if(ioctl(cfd, KDGKBENT, &ke)) die_perror("failed to get key setting");
		unsigned short s;
		s = htons(ke.kb_value);
		write(1, &s, 2);
	}
	return 0;
}
