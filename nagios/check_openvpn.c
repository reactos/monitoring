/**
 * File:	check_openvpn.c
 * Author:	Pierre Schweitzer <pierre@reactos.org>
 * Created:	24 Mar 2013
 * Licence:	GNU GPL v2 or any later version
 * Purpose:	Nagios plugin to check for OpenVPN client status
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef enum {
	OK,
	WARNING,
	CRITICAL,
	UNKNOWN
} state_t;

const char STATUS_FILE[] = "/var/run/openvpn.client.status";

#define unused(a) (void)a

int main(int argc, char *argv[]) {
	struct stat stbuf;

	unused(argc);
	unused(argv);

	/* Try to stat() the status file */
	if (stat(STATUS_FILE, &stbuf) == 0) {
		printf("OK: OpenVPN client is connected\n");
		return OK;
	/* Check if we failed because it doesn't exist */
	} else if (errno == ENOENT || errno == ENOTDIR) {
		printf("CRITICAL: OpenVPN client is not connected!\n");
		return CRITICAL;
	}

	/* Otherwise we failed for some other reason */
	printf("UNKNOWN: Can't get status of OpenVPN client!\n");
	return UNKNOWN;
}
