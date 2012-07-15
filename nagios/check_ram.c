/**
 * File:	check_ram.c
 * Author:	Pierre Schweitzer <pierre@reactos.org>
 * Created:	23 Feb 2012
 * Licence:	GNU GPL v2 or any later version
 * Purpose:	Nagios plugin to check RAM use
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef enum {
	OK,
	WARNING,
	CRITICAL,
	UNKNOWN
} state_t;

typedef struct {
	int warning;
	int critical;
} params_t;

#define LINE_SIZE 255

#define strncmp_cst(s1, s2)	strncmp(s1, s2, sizeof(s2) - sizeof(char))
#define print_usage(exe)	printf("Usage: %s -w <percent_free>%% -c <percent_free>%%\n", exe);

static int parse_command_line(int argc, char *argv[], params_t *param) {
	char *inval;
	size_t arg_len;
	int opt, err = 0;
	int *to_set = NULL;

	opterr = 0;

	while ((opt = getopt(argc, argv, "w:c:")) != -1) {
		if (opt == 'w') {
			to_set = &param->warning;
		} else if (opt == 'c') {
			to_set = &param->critical;
		} else {
			err = -1;
			break;
		}

		/* We have to have an integer (percent) */
		arg_len = strlen(optarg);
		if (optarg[arg_len - 1] != '%') {
			err = -2;
			break;
		}

		/* Remove symbol */
		optarg[arg_len - 1] = '\0';
		arg_len--;

		/* Extract int */
		*to_set = strtol(optarg, &inval, 10);
		if (inval[0] != '\0') {
			err = -2;
			break;
		}

		/* Ensure percentage looks correct */
		if (*to_set < 0 || *to_set > 100) {
			err = -3;
			break;
		}
	}

	/* Check if we had an error while parsing */
	switch (err) {
		case -3:
			printf("%s: Percentages must be positive and below 100%%\n", argv[0]);
			return err;

		case -2:
			printf("%s: Arguments must be percentage!\n", argv[0]);
			return err;
	}

	/* Now, check we received everything */
	if (param->warning == -1 || param->critical == -1 || err == -1) {
		printf("%s: Could not parse arguments!\n", argv[0]);
		return -1;
	}

	/* Check parameters consistency */
	if (param->warning <= param->critical) {
		printf("%s: Warning free space should be more than critical free space\n", argv[0]);
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	char *begin;
	FILE *meminfo;
	int found = 0;
	long long *to_set;
	char line[LINE_SIZE];
	params_t param = { -1, -1 };
	long long in_use, available, critical, warning;
	long long MemTot = 0, MemFree = 0, Buffers = 0, Cached = 0;

	/* User has to provide w/c */
	if (argc < 5) {
		print_usage(argv[0]);
		return UNKNOWN;
	}

	/* Parse command line */
	if (parse_command_line(argc, argv, &param) < 0) {
		print_usage(argv[0]);
		return UNKNOWN;
	}

	/* Here we know we have everything */
	meminfo = fopen("/proc/meminfo", "r");
	if (meminfo == NULL) {
		printf("RAM CRITICAL - Failed opening /proc/meminfo\n");
		return CRITICAL;
	}

	/* We need MemTotal, MemFree, Buffers, Cached */
	while (feof(meminfo) == 0 && found < 4) {
		if (fgets(line, (int)sizeof(line), meminfo) != line) {
			/* Workaround normal behavior of fgets - it doesn't set FEOF when reading to the end */
			if (feof(meminfo) != 0) {
				break;
			}

			(void)fclose(meminfo);
			printf("RAM CRITICAL - Failed reading /proc/meminfo\n");
			return CRITICAL;
		}

		if (line[0] == 'M') {
			if (strncmp_cst(line, "MemFree") == 0) {
				to_set = &MemFree;
				found++;
			} else if (strncmp_cst(line, "MemTot") == 0) {
				to_set = &MemTot;
				found++;
			} else {
				continue;
			}
		} else if (line[0] == 'B') {
			if (strncmp_cst(line, "Buffers") == 0) {
				to_set = &Buffers;
				found++;
			} else {
				continue;
			}
		} else if (line[0] == 'C') {
			if (strncmp_cst(line, "Cached") == 0) {
				to_set = &Cached;
				found++;
			} else {
				continue;
			}
		} else {
			/* Nothing that may interest us */
			continue;
		}

		/* Now, extract data */
		begin = strchr(line, ':');
		if (begin == NULL) {
			(void)fclose(meminfo);
			printf("RAM CRITICAL - Failed parsing /proc/meminfo data\n");
			return CRITICAL;
		}

		/* strtoll skips as many spaces as needed */
		*to_set = strtoll(begin + 1, NULL, 10);
	}

	/* Close file, not needed anymore */
	(void)fclose(meminfo);

	/* Ensure we have what we need */
	if (found != 4) {
		printf("RAM CRITICAL - Failed parsing /proc/meminfo\n");
		return CRITICAL;
	}

	/* Compute memory in use */
	in_use = MemTot - MemFree - Buffers - Cached;
	available = MemTot - in_use;

	/* Compute thresholds */
	critical = MemTot * param.critical / 100;
	warning = MemTot * param.warning / 100;

	/* Check if we are below thresholds */
	if (available <= critical) {
		printf("RAM CRITICAL - %lld%% free (%lld MB out of %lld MB)|RAM=%lldMB;%lld;%lld;0;%lld\n",
				(available * 100 / MemTot), (available / 1024), (MemTot / 1024), (available / 1024),
				(warning / 1024), (critical / 1024), (MemTot / 1024));
		return CRITICAL;
	} else if (available <= warning) {
		printf("RAM WARNING - %lld%% free (%lld MB out of %lld MB)|RAM=%lldMB;%lld;%lld;0;%lld\n",
				(available * 100 / MemTot), (available / 1024), (MemTot / 1024), (available / 1024),
				(warning / 1024), (critical / 1024), (MemTot / 1024));
		return WARNING;
	}

	printf("RAM OK - %lld%% free (%lld MB out of %lld MB)|RAM=%lldMB;%lld;%lld;0;%lld\n",
			(available * 100 / MemTot), (available / 1024), (MemTot / 1024), (available / 1024),
			(warning / 1024), (critical / 1024), (MemTot / 1024));
	return OK;
}
