/* released under the public domain, as the content is too trivial to even claim a copyright */

#include <stdlib.h>
#include <stdio.h>

__attribute__((noreturn))
void usage(void) {
	printf("usage: swapon path-to-blockdevice\n");
	exit(1);
}

extern int swapon(const char *path, int swapflags);

int main(int argc, char** argv) {
	int ret;
	if (argc != 2) usage();
	if((ret = swapon(argv[1], 0)) == -1) perror("swapon");
	return ret != 0;
}
