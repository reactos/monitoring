/**
 * File:	check_mountpoints.c
 * Author:	Pierre Schweitzer <pierre@reactos.org>
 * Created:	25 Feb 2012
 * Licence:	GNU GPL v2 or any later version
 * Purpose:	Nagios plugin to check NFS mount points
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

typedef enum {
	OK,
	WARNING,
	CRITICAL,
	UNKNOWN
} state_t;

#define LINE_SIZE 255

#define unused(a) (void)a

int main(int argc, char *argv[]) {
	int fd;
	FILE *mounts, *fstab;
	char line_fs[LINE_SIZE];
	char line_mnt[LINE_SIZE];
	char *dummy, *point, *type, *mpoint;

	unused(argc);
	unused(argv);

	/* Open mounted points */
	mounts = fopen("/proc/mounts", "r");
	if (mounts == NULL) {
		printf("CRITICAL - Failed opening /proc/mounts\n");
		return CRITICAL;
	}

	/* Open mounts list */
	fstab = fopen("/etc/fstab", "r");
	if (fstab == NULL) {
		(void)fclose(mounts);
		printf("CRITICAL - Failed opening /etc/fstab\n");
		return CRITICAL;
	}

	/* Browse the whole list */
	while (feof(fstab) == 0) {
		if (fgets(line_fs, (int)sizeof(line_fs), fstab) != line_fs) {
			/* Workaround normal behavior of fgets - it doesn't set FEOF when reading to the end */
			if (feof(fstab) != 0) {
				break;
			}

			(void)fclose(mounts);
			(void)fclose(fstab);
			printf("CRITICAL - Failed reading /etc/fstab\n");
			return CRITICAL;
		}

		/* Skip commented/empty lines */
		if (line_fs[0] == '#' || line_fs[0] == '\n') {
			continue;
		}

		/* Split line */
		dummy = strtok(line_fs, " \t");
		if (dummy == NULL) {
			(void)fclose(mounts);
			(void)fclose(fstab);
			printf("CRITICAL - Failed parsing /etc/fstab\n");
			return CRITICAL;
		}

		/* Ensure it's not commented nor empty */
		if (dummy[0] == '#' || dummy[0] == '\n') {
			continue;
		}

		/* Get mount point */
		point = strtok(NULL, " \t");
		if (point == NULL) {
			(void)fclose(mounts);
			(void)fclose(fstab);
			printf("CRITICAL - Failed parsing /etc/fstab\n");
			return CRITICAL;
		}

		/* Get mount type */
		type = strtok(NULL, " \t");
		if (type == NULL) {
			(void)fclose(mounts);
			(void)fclose(fstab);
			printf("CRITICAL - Failed parsing /etc/fstab\n");
			return CRITICAL;
		}

		/* Not an NFS mount point */
		if (strlen(type) != 3 || type[0] != 'n' ||
			type[1] != 'f' || type[2] != 's') {
			continue;
		}

		// Find it in mounts
		rewind(mounts);

		while (feof(mounts) == 0) {
			if (fgets(line_mnt, (int)sizeof(line_mnt), mounts) != line_mnt) {
				/* Workaround normal behavior of fgets - it doesn't set FEOF when reading to the end */
				if (feof(mounts) != 0) {
					break;
				}

				(void)fclose(mounts);
				(void)fclose(fstab);
				printf("CRITICAL - Failed reading /proc/mounts\n");
				return CRITICAL;
			}

			/* Split line */
			dummy = strtok(line_mnt, " \t");
			if (dummy == NULL) {
				(void)fclose(mounts);
				(void)fclose(fstab);
				printf("CRITICAL - Failed parsing /proc/mounts\n");
				return CRITICAL;
			}

			mpoint = strtok(NULL, " \t");
			if (mpoint == NULL) {
				(void)fclose(mounts);
				(void)fclose(fstab);
				printf("CRITICAL - Failed parsing /proc/mounts\n");
				return CRITICAL;
			}

			if (strcmp(point, mpoint) == 0) {
				break;
			}
		}

		/* Check if mount point was found */
		if (feof(mounts) != 0) {
			(void)fclose(mounts);
			(void)fclose(fstab);
			printf("CRITICAL - Mounts not OK\n");
			return CRITICAL;
		}

		/* Finally, check that the mount point isn't stale */
		fd = open(point, O_RDONLY);
		if (fd == -1) {
			if (errno == ESTALE) {
				(void)fclose(mounts);
				(void)fclose(fstab);
				printf("CRITICAL - Mounts not OK\n");
				return CRITICAL;
			}

			/* Otherwise, ignore (is that we want?) */
		} else {
			(void)close(fd);
		}
	}

	(void)fclose(mounts);
	(void)fclose(fstab);
	printf("OK - Mounts OK\n");

	return OK;
}
