/**
 * File:	check_openvpn.c
 * Author:	Pierre Schweitzer <pierre@reactos.org>
 * Created:	24 Mar 2013
 * Licence:	GNU GPL v2 or any later version
 * Purpose:	Nagios plugin to check for OpenVPN client status
 */

#define _XOPEN_SOURCE
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef enum {
	OK,
	WARNING,
	CRITICAL,
	UNKNOWN
} state_t;

/* FIXME: Change to /run/openvpn.client.status on Debian 6 support drop */
const char STATUS_FILE[] = "/var/run/openvpn.client.status";
const double QUARTER_IN_SECS = 15. * 60.;

/* No const here, to prevent GCC interpreting this as a VLA... */
#define LINE_SIZE 255
#define unused(a) (void)a
#define strncmp_cst(s1, s2) strncmp(s1, s2, sizeof(s2) - sizeof(char))

int main(int argc, char *argv[]) {
	char *date;
	struct tm tm;
	FILE *status_file;
	time_t status_time;
	char line[LINE_SIZE];

	unused(argc);
	unused(argv);

	/* Try to open the status file */
	status_file = fopen(STATUS_FILE, "r");
	if (status_file == NULL) {
		printf("CRITICAL: Failed to open status file\n");
		return CRITICAL;
	}

	/* Now, find a line which begins matches Updated */
	date = NULL;
	while (feof(status_file) == 0) {
		if (fgets(line, (int)sizeof(line), status_file) != line) {
			/* Workaround normal behavior of fgets - it doesn't set FEOF when reading to the end */
			if (feof(status_file) != 0) {
				break;
			}

			(void)fclose(status_file);
			printf("CRITICAL: Failed to read status file\n");
			return CRITICAL;
		}

		/* Check first letter */
		if (line[0] != 'U') {
			continue;
		}

		/* Check for correct line */
		if (strncmp_cst(line, "Updated,") != 0) {
			continue;
		}

		/* We have found required data */
		date = strchr(line, ',');
		break;
	}

	(void)fclose(status_file);

	/* Check the line was properly formatted */
	if (date == NULL) {
		printf("UNKNOWN: Status file seems not to contain needed data\n");
		return UNKNOWN;
	}

	/* Skip coma */
	++date;

	/* Get date & time */
	memset(&tm, 0, sizeof(struct tm));
	if (strptime(date, "%c", &tm) != &date[strlen(date) - 1]) {
		printf("UNKNOWN: Last updated is not understandable\n");
		return UNKNOWN;
	}

	/* Validate that it is less than 15 minutes old */
	status_time = mktime(&tm);
	if (status_time == -1) {
		printf("UNKNOWN: Last updated is not valid\n");
		return UNKNOWN;
	}

	if (difftime(time(0), status_time) < QUARTER_IN_SECS) {
		printf("OK: OpenVPN client is connected\n");
		return OK;
	}

	printf("WARNING: Data outdated! Client started without connection\n");
	return WARNING;
}
