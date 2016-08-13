#include <stdio.h>
static int usage(const char *b0) {
	dprintf(2, "usage: %s oldname newname\n"
	           "renames oldname to newname.\n"
	           "there are no options, so any file, even if starting "
	           "with a dash, can be renamed.\n", b0);
	return 1;
}

int main(int a, char** b) {
	if(a != 3) return usage(b[0]);
	int ret = rename(b[1], b[2]);
	if(ret) perror("rename");
	return ret *-1;
}
