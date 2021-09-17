#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int run(char** argv) {
	pid_t child, ret;
	int stat_loc;
	if((child = fork()) == 0) {
		execvp(argv[0], argv);
		perror("execvp");
		_exit(1);
	} else {
		ret = waitpid(child, &stat_loc, 0);
		assert(ret == child);
		return WIFEXITED(stat_loc) ? 0 : WTERMSIG(stat_loc);
	}
	return 0;
}

static void usage() {
	printf(
		"benchmark N COMMAND [ARGS...]\n"
		"runs COMMAND (with ARGS) N times and prints timings.\n"
	);
	exit(1);
}

#define NANOSECS 1000000000LL

long long timespectoll(struct timespec *ts) {
	return ts->tv_sec*NANOSECS + ts->tv_nsec;
}

char *fmt(long long n) {
	char buf[256];
	sprintf(buf, "%lld.%04lld", n/NANOSECS, (n%NANOSECS)/(NANOSECS/1000));
	return strdup(buf);
}

int main(int argc, char** argv) {
	if(argc < 3 || !isdigit(argv[1][0])) usage();
	int i, n = atoi(argv[1]);
	argv++;
	argv++;
	long long *results = calloc(n, sizeof *results);
	for (i=0; i<n; ++i) {
		struct timespec b_start, b_end;
		assert(0 == clock_gettime(CLOCK_MONOTONIC, &b_start));
		run(argv);
		assert(0 == clock_gettime(CLOCK_MONOTONIC, &b_end));
		results[i] = timespectoll(&b_end) - timespectoll(&b_start);
	}
	long long best = 0x7fffffffffffffffLL, sum = 0;
	for (i=0; i<n; ++i) {
		if(results[i] < best) best = results[i];
		sum += results[i] ;
	}
	printf("called %d times, best result: %ss, avg: %ss, total: %ss\n",
		n, fmt(best), fmt(sum/(long long)n), fmt(sum));
	return 0;
}

