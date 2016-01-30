#include <stdio.h>
#include <stdlib.h>

static int syntax() {
	printf("bin2hex - converts a file into a hex string\n"
	       "he output is written to stdout\n"
	       "bin2sh filename\n");
	return 1;
}

int main(int argc, char** argv) {
	if(argc != 2) return syntax();
	FILE *f = fopen(argv[1], "r");
	if(!f) { perror("fopen"); return 1; }
	unsigned char buf[1];
	while(fread(buf, 1, 1, f)) {
		printf("%x", buf[0]);
	}
	printf("\n");
	fclose(f);
	return 0;
}
