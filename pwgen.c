#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define NUM "0123456789"
#define ALL "abcdefghijklmnopqrstuvwxyz"
#define ALU "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define SYM ",.;:=%*+-_/@!$&|\?\\(){}[]^ '\""
#define SL(X) sizeof(X) -1
#define ENTRY(X, Y, Z) {X, Y, SL(Z)}

static const struct {char name[6]; unsigned char idx, len;} md[] = {
	ENTRY("all", 0, NUM ALL ALU SYM),
	ENTRY("num", 0, NUM),
	ENTRY("lower", SL(NUM), ALL),
	ENTRY("upper", SL(NUM ALL), ALU),
	ENTRY("sym", SL(NUM ALL ALU), SYM),
	ENTRY("alnum", 0, NUM ALL ALU),
	ENTRY("alnuml", 0, NUM ALL),
};

static int set(const char *name) {
	int i; size_t l=strlen(name);
	if (l>6) return -1;
	for(i=0;i<sizeof(md)/sizeof(md[0]);i++)
		if(memcmp(md[i].name, name, l)==0) return i;
	return -1;
}
static int usage(const char *b0) {
	dprintf(2,
	       "usage: SET=name %s [number]\n"
	       "generates a random password.\n"
	       "SET is an environment variable and can be one of:\n"
	       "all num lower upper sym alnum alnuml\n"
	       "it denotes the set of characters we pick characters from.\n"
	       "SET can also be a negative number\n"
	       "(shrinks number of available chars in the 'all' set)\n"
	       "default: all\n"
	       "number denotes the desired length of the password\n"
	       "default: 16\n", b0);
	return 1;
}

int main(int a, char**b) {
	static const char alphabet[] = NUM ALL ALU SYM;
	if(a == 2 && !strcmp(b[1], "--help")) return usage(b[0]);
	const char *myset = getenv("SET");
	int reduce = 0, setidx = myset ? set(myset) : 0;
	if(setidx == -1) {
		if(!myset || myset[0] != '-') {
			dprintf(2, "set %s not found\nexecute %s --help for a list of sets\n", myset, b[0]);
			return 1;
		}
		reduce = atoi(myset+1);
		setidx = 0;
		if(reduce >= SL(NUM ALL ALU SYM)) {
			dprintf(2, "SET reduction greater than available chars\n");
			return 1;
		}
	}
	const char* ch = alphabet + md[setidx].idx;
	const size_t chl = md[setidx].len-reduce;
	int fd = open("/dev/urandom", O_RDONLY);
	if(fd == -1) {
		dprintf(2, "open /dev/urandom failed\n");
		return 1;
	}
	int i, pwlen = a > 1 ? atoi(b[1]) : 16;
	for(i=0;i<pwlen;i++) {
		unsigned char rnd;
		if(1!=read(fd, &rnd, 1)) {
			dprintf(2, "read error\n");
			return 1;
		}
		dprintf(1, "%c", ch[rnd%chl]);
	}
	dprintf(1, "\n");
	close(fd);
	return 0;
}
