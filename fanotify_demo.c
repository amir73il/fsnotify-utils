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

#ifndef XFS_FILEID_TYPE_64FLAG
#define XFS_FILEID_TYPE_64FLAG  0x80

#define FILEID_INO32_GEN        0x1
#define FILEID_INO32_GEN_PARENT 0x2

#define FILEID_INO64_GEN        (FILEID_INO32_GEN|XFS_FILEID_TYPE_64FLAG)
#define FILEID_INO64_GEN_PARENT (FILEID_INO32_GEN_PARENT|XFS_FILEID_TYPE_64FLAG)
#endif

#define FAN_EVENT_ON_SB         0x01000000
#define FAN_EVENT_ON_DESCENDANT (FAN_EVENT_ON_CHILD | FAN_EVENT_ON_SB)

#define FAN_EVENT_INFO_PARENT   0x100
#define FAN_EVENT_INFO_NAME     0x200
#define FAN_EVENT_INFO_FH       0x400

#define FAN_DENTRY_EVENTS (IN_ATTRIB |\
		IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF |\
		IN_CREATE | IN_DELETE)

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
    char path[256];
    char filename[256];
    ssize_t len = 0;
    struct file_handle *fh = (struct file_handle *)(i+1);
    unsigned *fid = (unsigned *)fh->f_handle;

    printf("    fd =%d; ", i->fd);
    printf("mask = ");
    if (i->mask & IN_ACCESS)        printf("FAN_ACCESS ");
    if (i->mask & IN_ATTRIB)        printf("FAN_ATTRIB ");
    if (i->mask & IN_CLOSE_NOWRITE) printf("FAN_CLOSE_NOWRITE ");
    if (i->mask & IN_CLOSE_WRITE)   printf("FAN_CLOSE_WRITE ");
    if (i->mask & IN_CREATE)        printf("FAN_CREATE ");
    if (i->mask & IN_DELETE)        printf("FAN_DELETE ");
    if (i->mask & IN_DELETE_SELF)   printf("FAN_DELETE_SELF ");
    if (i->mask & IN_IGNORED)       printf("FAN_IGNORED ");
    if (i->mask & IN_ISDIR)         printf("FAN_ISDIR ");
    if (i->mask & IN_MODIFY)        printf("FAN_MODIFY ");
    if (i->mask & IN_MOVE_SELF)     printf("FAN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM)    printf("FAN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      printf("FAN_MOVED_TO ");
    if (i->mask & IN_OPEN)          printf("FAN_OPEN ");
    if (i->mask & IN_Q_OVERFLOW)    printf("FAN_Q_OVERFLOW ");
    if (i->mask & IN_UNMOUNT)       printf("FAN_UNMOUNT ");
    printf("\n");

    if (i->fd > 0) {
	len = snprintf(procfile, 255, "/proc/self/fd/%d", i->fd);
	if (len > 0)
		len = readlink(procfile, path, 255);
	if (len > 0)
		path[len] = 0;
	close(i->fd);
    }

    if (len > 0)
        printf("        path = %s\n", path);

    if (i->event_len <= FAN_EVENT_METADATA_LEN)
	    return;

    printf("    type =0x%x; ", fh->handle_type);
    printf("    bytes =0x%x; ", fh->handle_bytes);
    switch (fh->handle_type) {
	    case FILEID_INO32_GEN_PARENT:
		    printf("    parent ino =%u; ", fid[2]);
		    printf("    parent gen =%u; ", fid[3]);
	    case FILEID_INO32_GEN:
		    printf("    ino =%u; ", fid[0]);
		    printf("    gen =%u; ", fid[1]);
		    break;
	    case FILEID_INO64_GEN_PARENT:
		    printf("    parent ino =%llu; ", *(unsigned long long *)(fid+3));
		    printf("    parent gen =%u; ", fid[5]);
	    case FILEID_INO64_GEN:
		    printf("    ino =%llu; ", *(unsigned long long *)fid);
		    printf("    gen =%u; ", fid[2]);
		    break;
    }

    if (i->event_len > FAN_EVENT_METADATA_LEN + sizeof(*fh) + fh->handle_bytes)
	    printf("        name = %s\n", fh->f_handle + fh->handle_bytes);
    else
	    printf("\n");
}

#define BUF_LEN (10 * (sizeof(struct fanotify_event_metadata) + NAME_MAX + 1))

static int add_watch(int notifyFd, const char *path)
{
	int wd;

	/* Mark the mount for:
	 * - notification events after closing a write-enabled
	 * file descriptor
	 */
	wd = fanotify_mark(notifyFd, FAN_MARK_ADD,
				FAN_ALL_EVENTS|FAN_DENTRY_EVENTS|
				FAN_EVENT_ON_DESCENDANT|FAN_ONDIR,
				AT_FDCWD, path);
	if (wd == -1) {
		fprintf(stderr, "fanotify sb watch not supported\n");
		wd = fanotify_mark(notifyFd, FAN_MARK_ADD,
				FAN_ALL_EVENTS|FAN_EVENT_ON_CHILD,
				AT_FDCWD, path);
		if (!wd)
			wd = fanotify_mark(notifyFd, FAN_MARK_ADD|FAN_MARK_MOUNT,
					FAN_ALL_EVENTS,
					AT_FDCWD, path);
	}
	if (wd == -1) {
		int err = errno;
		printf("Failed adding watch on %s: %s\n", path, strerror(err));
		return wd;
	}

	printf("Watching %s using wd %d\n", path, wd);
	return wd;
}

int
main(int argc, char *argv[])
{
    int notifyFd, wd, j;
    char buf[BUF_LEN] __attribute__ ((aligned(8)));
    ssize_t numRead, len;
    struct fanotify_event_metadata *event;
    int countdown = 5;
    int maxevents = 100;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname...\n", argv[0]);

    notifyFd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_NOTIF |
				FAN_EVENT_INFO_PARENT | FAN_EVENT_INFO_NAME| FAN_EVENT_INFO_FH,
				O_RDONLY);
    if (notifyFd == -1) {
	fprintf(stderr, "fanotify filename events not supported\n");
	notifyFd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_NOTIF, O_RDONLY);
    }
    if (notifyFd == -1) {
	    errExit("fanotify_init");
    }

    for (j = 1; j < argc; j++) {
	wd = add_watch(notifyFd, argv[j]);
	if (wd == -1)
		errExit("notify_add_watch");
    }

    printf("Waiting for events...\n");
    while (--countdown > 0) {
	printf("%d...\n",countdown);
	sleep(1);
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
	    if (maxevents-- <= 0)
		    exit(EXIT_SUCCESS);
	    event = FAN_EVENT_NEXT(event, len);
        }
    }

    exit(EXIT_SUCCESS);
}
