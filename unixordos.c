#include <stdio.h>

int is_dos_file(FILE *f) {
	int c, was_cr = 0;
	while((c = fgetc(f)) != EOF) {
		if(c == 0xd) was_cr = 1;
		else if(c == 0xa && was_cr) {
			return 1;
		} else was_cr = 0;
	}
	return 0;
}

int main(int argc, char**argv) {
	if(argc < 2) {
		dprintf(2, "need filename\n");
		return 1;
	}
	FILE *f = fopen(argv[1], "r");
	if(!f) {
		perror("fopen");
		return 2;
	}
	if(is_dos_file(f))
		dprintf(1, "DOS\n");
	else
		dprintf(1, "UNIX\n");
	fclose(f);
	return 0;
}
