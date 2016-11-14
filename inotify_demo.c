/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2016.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

#include <sys/inotify.h>
#include <limits.h>
#include "tlpi_hdr.h"

#define FS_VOLATILE            0x01000000
#define IN_VOLATILE            0//0x08000000
#define FS_D_INSTANTIATE	(IN_CREATE|IN_ISDIR|FS_VOLATILE)

static void             /* Display information from inotify_event structure */
displayInotifyEvent(struct inotify_event *i)
{
    printf("    wd =%2d; ", i->wd);
    if (i->cookie > 0)
        printf("cookie =%4d; ", i->cookie);

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

    if (i->len > 0)
        printf("        name = %s\n", i->name);
}

static inline int is_new_dentry(struct inotify_event *i)
{
	return (i->mask & FS_D_INSTANTIATE) == FS_D_INSTANTIATE && i->len > 0;
}

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static int add_watch(int inotifyFd, const char *dir, const char *name)
{
	char path[256];
	static char last_path[256];
	snprintf(path, sizeof(path) - 1, "%s/%s", dir ? dir : last_path, name);
	int wd = inotify_add_watch(inotifyFd, path, IN_ALL_EVENTS | IN_VOLATILE);
	if (wd == -1) {
		int err = errno;
		if (dir && err == ENOENT)
			return add_watch(inotifyFd, NULL, dir);
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
    int inotifyFd, wd, j;
    char buf[BUF_LEN] __attribute__ ((aligned(8)));
    ssize_t numRead;
    char *p;
    struct inotify_event *event;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname...\n", argv[0]);

    inotifyFd = inotify_init();                 /* Create inotify instance */
    if (inotifyFd == -1)
        errExit("inotify_init");

    for (j = 1; j < argc; j++) {
	wd = add_watch(inotifyFd, argv[j], "");
	if (wd == -1)
		errExit("inotify_add_watch");
    }

    for (;;) {                                  /* Read events forever */
        numRead = read(inotifyFd, buf, BUF_LEN);
        if (numRead == 0)
            fatal("read() from inotify fd returned 0!");

        if (numRead == -1)
            errExit("read");

        printf("Read %ld bytes from inotify fd\n", (long) numRead);

        /* Process all of the events in buffer returned by read() */

        for (p = buf; p < buf + numRead; ) {
            event = (struct inotify_event *) p;
            displayInotifyEvent(event);

	    if (is_new_dentry(event))
		    add_watch(inotifyFd, NULL, event->name);

            p += sizeof(struct inotify_event) + event->len;
        }
    }

    exit(EXIT_SUCCESS);
}
