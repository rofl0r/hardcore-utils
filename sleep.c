#define _POSIX_C_SOURCE 200809L
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	int i;
	char* end;
	int times;
	for(i = 1; i < argc; i++) {
		double s = strtod(argv[i], &end);
		switch(*end) {
			case 'm':
				times = 60;
				break;
			case 'h':
				times = 60 * 60;
				break;
			case 'd':
				times = 60 * 60 * 24;
				break;
			default: 
				times = 1;
		}
		while(times--) {
			long long sec = s;
			long long ns = (s - (double) sec) * 1000000000.f;
			nanosleep(&(struct timespec) { .tv_sec = sec, .tv_nsec = ns }, NULL);
		}
	}
	return 0;
}
