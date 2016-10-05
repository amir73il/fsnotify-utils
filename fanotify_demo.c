/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2016.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

#define _GNU_SOURCE     /* Needed to get O_LARGEFILE definition */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fanotify.h>
#include <sys/inotify.h>
#include "tlpi_hdr.h"

#define FS_VOLATILE            0x01000000
#define IN_VOLATILE            0x08000000
#define FS_D_INSTANTIATE	(IN_CREATE|IN_ISDIR|FS_VOLATILE)

/*
 * Display information from fanotify_event_metadata structure:
struct fanotify_event_metadata {
	__u32 event_len;
	__u8 vers;
	__u8 reserved;
	__u16 metadata_len;
	__aligned_u64 mask;
	__s32 fd;
	__s32 pid;
};
*/
static void             
displayNotifyEvent(struct fanotify_event_metadata *i)
{
    char procfile[256];
    char filename[256];
    ssize_t len = 0;

    printf("    fd =%d; ", i->fd);
    printf("mask = ");
    if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
    if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
    if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
    if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
    if (i->mask & IN_CREATE)        printf("IN_CREATE ");
    if (i->mask & IN_DELETE)        printf("IN_DELETE ");
    if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
    if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
    if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
    if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
    if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
    if (i->mask & IN_OPEN)          printf("IN_OPEN ");
    if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
    if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
    if (i->mask & FS_VOLATILE)      printf("FS_VOLATILE ");
    printf("\n");

    if (i->fd > 0) {
	len = snprintf(procfile, 255, "/proc/self/fd/%d", i->fd);
	if (len > 0)
		len = readlink(procfile, filename, 255);
	if (len > 0)
		filename[len] = 0;
	close(i->fd);
    }

    if (len > 0)
        printf("        filename = %s\n", filename);
}

#define BUF_LEN (10 * (sizeof(struct fanotify_event_metadata) + NAME_MAX + 1))

static int add_watch(int notifyFd, const char *dir, const char *name)
{
	char path[256];
	static char last_path[256];
	snprintf(path, sizeof(path) - 1, "%s/%s", dir ? dir : last_path, name);

	/* Mark the mount for:
	 * - notification events after closing a write-enabled
	 * file descriptor
	 */
	int wd = fanotify_mark(notifyFd, FAN_MARK_ADD | FAN_MARK_MOUNT,
				FAN_ALL_EVENTS|FAN_EVENT_ON_CHILD|FAN_ONDIR, AT_FDCWD,
				path);
	if (wd == -1) {
		int err = errno;
		if (dir && err == ENOENT)
			return add_watch(notifyFd, NULL, dir);
		printf("Failed adding watch on %s: %s\n", path, strerror(err));
		return wd;
	}

	printf("Watching %s using wd %d\n", path, wd);
	strncpy(last_path, path, sizeof(last_path) - 1);
	return wd;
}

int
main(int argc, char *argv[])
{
    int notifyFd, wd, j;
    char buf[BUF_LEN] __attribute__ ((aligned(8)));
    ssize_t numRead, len;
    struct fanotify_event_metadata *event;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname...\n", argv[0]);

    notifyFd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_NOTIF, O_RDONLY | O_LARGEFILE);
    if (notifyFd == -1) {
	    errExit("fanotify_init");
    }

    for (j = 1; j < argc; j++) {
	wd = add_watch(notifyFd, argv[j], "");
	if (wd == -1)
		errExit("notify_add_watch");
    }

    for (;;) {                                  /* Read events forever */
        numRead = read(notifyFd, buf, BUF_LEN);
        if (numRead == 0)
            fatal("read() from notify fd returned 0!");

        if (numRead == -1)
            errExit("read");

        printf("Read %ld bytes from notify fd\n", (long) numRead);

        /* Process all of the events in buffer returned by read() */

	event = (struct fanotify_event_metadata *)buf;
	len = numRead;
        while (FAN_EVENT_OK(event, len)) {
            displayNotifyEvent(event);

	    event = FAN_EVENT_NEXT(event, len);
        }
    }

    exit(EXIT_SUCCESS);
}
