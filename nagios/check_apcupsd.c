/**
 * File:	check_apcupsd.c
 * Author:	Pierre Schweitzer <pierre@reactos.org>
 * Created:	23 Feb 2012
 * Licence:	GNU GPL v2 or any later version
 * Purpose:	Nagios plugin to check APC UPS status
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	OK,
	WARNING,
	CRITICAL,
	UNKNOWN
} state_t;

typedef enum {
	bcharge,
	itemp,
	loadpct,
	timeleft,
	status
} checks_t;

typedef struct {
	long int warning;
	long int critical;
	checks_t check;
	char *log_file;
} params_t;

const char *default_log_file = "/var/log/apcupsd.status";
const char *lines[] = {
	"BCHARGE",
	"ITEMP",
	"LOADPCT",
	"TIMELEFT",
	"STATUS"
};
size_t lengths[] = {
	sizeof("BCHARGE") - sizeof(char),
	sizeof("ITEMP") - sizeof(char),
	sizeof("LOADPCT") - sizeof(char),
	sizeof("TIMELEFT") - sizeof(char),
	sizeof("STATUS") - sizeof(char)
};

#define LINE_SIZE 255
#define strncmp_cst(s1, s2)	strncmp(s1, s2, sizeof(s2) - sizeof(char))

static int find_check(char *s) {
	/* Determine which test it is doing only 1 strcmp */
	switch (s[0]) {
		case 'b':
			if (strcmp(s, "bcharge") == 0) {
				return bcharge;
			}

			/* Not a valid check */
			return -1;

		case 'i':
			if (strcmp(s, "itemp") == 0) {
				return itemp;
			}

			/* Not a valid check */
			return -1;

		case 'l':
			if (s[1] == 'o' && strcmp(s, "loadpct") == 0) {
				return loadpct;
			}

			/* Not a valid check */
			return -1;

		case 't':
			if (strcmp(s, "timeleft") == 0) {
				return timeleft;
			}

			/* Not a valid check */
			return -1;

		case 's':
			if (strcmp(s, "status") == 0) {
				return status;
			}
	}

	/* Not a valid check */
	return -1;
}

static int validate_param(char *exe, params_t *param) {
	/* Validate according to check */
	switch (param->check) {
		case bcharge:
			if (param->warning == -1) {
				param->warning = 95;
			}

			if (param->critical == -1) {
				param->critical = 50;
			}

			if (param->warning <= param->critical) {
				printf("%s: Warning battery charge percentage should be more than critical battery charge percentage\n", exe);
				return -1;
			}

			if (param->warning < 0 || param->critical < 0 ||
				param->warning > 100 || param->critical > 100) {
				printf("%s: percentages must be bigger than 0 and below 100\n", exe);
				return -1;
			}

			break;

		case itemp:
			if (param->warning == -1) {
				param->warning = 35;
			}

			if (param->critical == -1) {
				param->critical = 40;
			}

			if (param->warning >= param->critical) {
				printf("%s: Warning temperature should be lower than critical temperature\n", exe);
				return -1;
			}

			break;

		case loadpct:
			if (param->warning == -1) {
				param->warning = 70;
			}

			if (param->critical == -1) {
				param->critical = 85;
			}

			if (param->warning >= param->critical) {
				printf("%s: Warning load percentage should be lower than critical load percentage\n", exe);
				return -1;
			}

			if (param->warning < 0 || param->critical < 0) {
				printf("%s: percentages must be bigger than 0\n", exe);
				return -1;
			}

			break;

		case timeleft:
			if (param->warning == -1) {
				param->warning = 10;
			}

			if (param->critical == -1) {
				param->critical = 5;
			}

			if (param->warning <= param->critical) {
				printf("%s: Warning time left should be more than critical timeleft\n", exe);
				return -1;
			}

			if (param->warning < 0 || param->critical < 0) {
				printf("%s: time left values must be bigger than 0\n", exe);
				return -1;
			}

			break;

		case status:
			/* Nothing to do for status */
			break;
	}

	return 0;
}

static int parse_command_line(int argc, char *argv[], params_t *param) {
	char *inval;
	int arg, err = 0;
	long int *to_set = NULL;

	for (arg = 1; arg < argc; arg++) {
		/* Check if we have a minus parameter */
		if (argv[arg][0] == '-') {
			/* We will need another parameter */
			if (arg + 1 == argc) {
				err = -1;
				break;
			}

			/* Select parameter to set */
			if (argv[arg][1] == 'w') {
				to_set = &param->warning;
			} else if (argv[arg][1] == 'c') {
				to_set = &param->critical;
			} else if (argv[arg][1] == 'F') {
				param->log_file = argv[arg + 1];
				continue;
			} else {
				err = -1;
				break;
			}

			/* Extract value */
			*to_set = strtol(argv[arg + 1], &inval, 10);
			if (inval[0] != '\0') {
				err = -2;
				break;
			}

			/* We're done */
			continue;
		}

		/* This is the check to perform */
		if (param->check != (checks_t)-1) {
			/* This is unfortunate */
			err = -1;
			break;
		}

		param->check = find_check(argv[arg]);
	}

	/* Check if we had an error while parsing */
	switch (err) {
		case -1:
			return err;

		case -2:
			printf("%s: Arguments must be integers!\n", argv[0]);
			return err;
	}

	/* Now, validate what we got, or complete */
	if (param->check == (checks_t)-1) {
		return -1;
	}

	/* Set default log file if required */
	if (param->log_file == NULL) {
		param->log_file = default_log_file;
	}

	/* Now, validate parameters */
	return 0;
}

static void print_usage(char *exe) {
	printf("Usage: %s [-c critical_value] [-w warning_value] [-F log_file]\n", exe);
	printf("\t<bcharge|itemp|loadpct|timeleft|status>\n");
	printf("\nchecks:\n");
	printf("\tbcharge\t\t= battery charge, measured in percent.\n");
	printf("\titemp\t\t= internal temperature, measured in degree Celcius.\n");
	printf("\tloadpct\t\t= load percent, measured in percent.\n");
	printf("\ttimeleft\t= time left with current battery charge and load, measured in minutes.\n");
	printf("\tstatus\t\t= Whether the line is OK or not\n");
}

int main(int argc, char *argv[]) {
	size_t len;
	float pct = 0.0;
	FILE *status_file;
	char *begin = NULL;
	char line[LINE_SIZE];
	params_t param = { -1, -1, -1, NULL };

	/* User has to provide, at least, a check */
	if (argc < 2) {
		print_usage(argv[0]);
		return UNKNOWN;
	}

	/* Parse command line */
	if (parse_command_line(argc, argv, &param) < 0) {
		print_usage(argv[0]);
		return UNKNOWN;
	}

	/*  Validate input */
	if (validate_param(argv[0], &param) < 0) {
		print_usage(argv[0]);
		return UNKNOWN;
	}

	/* Open status file */
	status_file = fopen(param.log_file, "r");
	if (status_file == NULL) {
		printf("CRITICAL - Failed opening %s\n", param.log_file);
		return CRITICAL;
	}

	/* Now, find a line which begins matches test */
	while (feof(status_file) == 0) {
		if (fgets(line, (int)sizeof(line), status_file) != line) {
			/* Workaround normal behavior of fgets - it doesn't set FEOF when reading to the end */
			if (feof(status_file) != 0) {
				break;
			}

			(void)fclose(status_file);
			printf("CRITICAL - Failed reading %s\n", param.log_file);
			return CRITICAL;
		}

		/* Check first letter */
		if (line[0] != lines[param.check][0]) {
			continue;
		}

		if (strncmp(line, lines[param.check], lengths[param.check]) != 0) {
			continue;
		}

		/* We have found required data */
		begin = strchr(line, ':');
		break;
	}

	/* Close file, not needed anymore */
	(void)fclose(status_file);

	/* Work according check */
	switch (param.check) {
		case bcharge:
			if (begin) {
				pct = strtof(begin + 1, NULL); 
			}

			if (pct <= param.critical) {
				printf("CRITICAL - Battery Charge: %.2f%%|'Battery Charge'=%.2f%%;%ld:;%ld:;;\n",
						pct, pct, param.warning, param.critical);
				return CRITICAL;
			} else if (pct <= param.warning) {
				printf("WARNING - Battery Charge: %.2f%%|'Battery Charge'=%.2f%%;%ld:;%ld:;;\n",
						pct, pct, param.warning, param.critical);
				return WARNING;
			} else {
				printf("OK - Battery Charge: %.2f%%|'Battery Charge'=%.2f%%;%ld:;%ld:;;\n",
						pct, pct, param.warning, param.critical);
				return OK;
			}

			break;

		case itemp:
			if (begin) {
				pct = strtof(begin + 1, NULL);
			}

			if (pct >= param.critical) {
				printf("CRITICAL - Internal Temperature: %.2fC|'Battery Temperature'=%.2fC;%ld;%ld;;\n",
						pct, pct, param.warning, param.critical);
				return CRITICAL;
			} else if (pct >= param.warning) {
				printf("WARNING - Internal Temperature: %.2fC|'Battery Temperature'=%.2fC;%ld;%ld;;\n",
						pct, pct, param.warning, param.critical);
				return WARNING;
			} else {
				printf("OK - Internal Temperature: %.2fC|'Battery Temperature'=%.2fC;%ld;%ld;;\n",
						pct, pct, param.warning, param.critical);
				return OK;
			}

			break;

		case loadpct:
			if (begin) {
				pct = strtof(begin + 1, NULL);
			}

			if (pct >= param.critical) {
				printf("CRITICAL - Load: %.2f%%|'UPS Load'=%.2f%%;%ld;%ld;;\n",
						pct, pct, param.warning, param.critical);
				return CRITICAL;
			} else if (pct >= param.warning) {
				printf("WARNING - Load: %.2f%%|'UPS Load'=%.2f%%;%ld;%ld;;\n",
						pct, pct, param.warning, param.critical);
				return WARNING;
			} else {
				printf("OK - Load: %.2f%%|'UPS Load'=%.2f%%;%ld;%ld;;\n",
						pct, pct, param.warning, param.critical);
				return OK;
			}

			break;

		case timeleft:
			if (begin) {
				pct = strtof(begin + 1, NULL);
			}

			if (pct <= param.critical) {
				printf("CRITICAL - Time Left: %.2f Minutes|'minutes left'=%.2f;%ld:;%ld:;;\n",
						pct, pct, param.warning, param.critical);
				return CRITICAL;
			} else if (pct <= param.warning) {
				printf("WARNING - Time Left: %.2f Minutes|'minutes left'=%.2f;%ld:;%ld:;;\n",
						pct, pct, param.warning, param.critical);
				return WARNING;
			} else {
				printf("OK - Time Left: %.2f Minutes|'minutes left'=%.2f;%ld:;%ld:;;\n",
						pct, pct, param.warning, param.critical);
				return OK;
			}

			break;

		case status:
			if (begin) {
				/* Skip ':' && ' ' */
				begin += 2;

				/* Skip tailing EOL & spaces if any */
				len = strlen(begin);
				if (len >= 1 && begin[len - 1] == '\n') {
					begin[len - 1] = '\0';
					len--;
				}
				while (len >= 1 && begin[len - 1] == ' ') {
					begin[len - 1] = '\0';
					len--;
				}

				if (begin[0] == 'O' && strcmp(begin, "ONLINE") == 0) {
					printf("OK - Power Line: ONLINE|'Power Line'=1\n");
					return OK;
				} else {
					printf("CRITICAL - Power Line: %s|'Power Line'=0\n", begin);
					return CRITICAL;
				}
			}

			printf("CRITICAL - Power Line: OFFLINE|'Power Line'=0\n");
			return CRITICAL;
	}

	return UNKNOWN;
}
