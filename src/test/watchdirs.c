/*
 * watchdirs - setup fanotify marks on pre defined directory tree
 *
 * Copyright (C) 2016 CTERA Network by Amir Goldstein <amir73il@gmail.com>
 */

#define _GNU_SOURCE     /* Needed to get O_LARGEFILE definition */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fanotify.h>
#include "iter.h"


static int nmarks = 0;
static int nopen = 0;
static int nclose = 0;
static int nremoved = 0;
static int ndeleted = 0;
static int verbose = 0;

static int fanotify_fd = -1;

#define EVENT_MASK (FAN_OPEN_PERM | FAN_OPEN | FAN_CLOSE | FAN_ONDIR)

static int do_add_mark(const char *name, int depth)
{
	if (!depth)
		return 0;

	/* Mark non leaf directory for:
	   - permission events before opening dirs
	   - notification events after closing a dir
	   file descriptor */

	if (fanotify_mark(fanotify_fd, FAN_MARK_ADD, EVENT_MASK,
			  AT_FDCWD, name) != 0)
		return -1;

	nmarks++;
	return 0;
}

static int do_rm_mark(const char *name, int depth)
{
	if (!depth)
		return 0;

	return fanotify_mark(fanotify_fd, FAN_MARK_REMOVE, EVENT_MASK,
			  AT_FDCWD, name);
}

static void
handle_events(int fd)
{
    const struct fanotify_event_metadata *metadata;
    struct fanotify_event_metadata buf[200];
    ssize_t len;
    char path[PATH_MAX];
    ssize_t path_len;
    char procfd_path[PATH_MAX];
    struct fanotify_response response;
    struct stat st;

    /* Loop while events can be read from fanotify file descriptor */

    for(;;) {

        /* Read some events */

        len = read(fd, (void *) &buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Check if end of available data reached */

        if (len <= 0)
            break;

        /* Point to the first event in the buffer */

        metadata = buf;

        /* Loop over all events in the buffer */

        while (FAN_EVENT_OK(metadata, len)) {

            /* Check that run-time and compile-time structures match */

            if (metadata->vers != FANOTIFY_METADATA_VERSION) {
                fprintf(stderr,
                        "Mismatch of fanotify metadata version.\n");
                exit(EXIT_FAILURE);
            }

            /* metadata->fd contains either FAN_NOFD, indicating a
               queue overflow, or a file descriptor (a nonnegative
               integer). Here, we simply ignore queue overflow. */

            if (metadata->fd >= 0) {

                /* Handle open permission event */
                if (metadata->mask & FAN_OPEN_PERM) {
                    if (verbose) printf("FAN_OPEN_PERM: ");

                    /* Allow file to be opened */
                    response.fd = metadata->fd;
                    response.response = FAN_ALLOW;
                    write(fd, &response,
                          sizeof(struct fanotify_response));
                }

                /* Handle open of dir event */
                if (metadata->mask & FAN_OPEN) {
                    if (verbose) printf("FAN_OPEN: ");
		    nopen++;
		    if ((nopen % 7) == 0)
			    sleep(1);
		}

                /* Handle closing of dir event */
                if (metadata->mask & FAN_CLOSE) {
                    if (verbose) printf("FAN_CLOSE: ");
		    nclose++;
		    if (fstat(metadata->fd, &st)) {
		        perror("fstat");
		        exit(EXIT_FAILURE);
		    }
		    if (st.st_nlink == 0) {
			    /* Deleted dir */
			    ndeleted++;
		    } else {
			    /* Remove mark */
			    if (fanotify_mark(fanotify_fd, FAN_MARK_REMOVE, EVENT_MASK,
					metadata->fd, ".") == 0)
				    nremoved++;
		    }

		}

		if (!verbose)
			goto next;

                /* Retrieve and print pathname of the accessed file */
                snprintf(procfd_path, sizeof(procfd_path),
                         "/proc/self/fd/%d", metadata->fd);
                path_len = readlink(procfd_path, path,
                                    sizeof(path) - 1);
                if (path_len == -1) {
                    perror("readlink");
                    exit(EXIT_FAILURE);
                }

                path[path_len] = '\0';
                printf("File %s, mask=0x%llx\n", path, metadata->mask);

                /* Close the file descriptor of the event */
next:
                close(metadata->fd);
            }

            /* Advance to next event */

            metadata = FAN_EVENT_NEXT(metadata, len);
        }
    }
}

static void listen_events(int fd)
{
    char buf;
    int poll_num;
    nfds_t nfds;
    struct pollfd fds[2];

    printf("Press enter key to terminate.\n");

    /* Prepare for polling */

    nfds = 2;

    /* Console input */

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    /* Fanotify input */

    fds[1].fd = fd;
    fds[1].events = POLLIN;

    /* This is the loop to wait for incoming events */

    printf("Listening for events. (nmarks=%d)\n", nmarks);

    while (1) {
        poll_num = poll(fds, nfds, -1);
        if (poll_num == -1) {
            if (errno == EINTR)     /* Interrupted by a signal */
                continue;           /* Restart poll() */

            perror("poll");         /* Unexpected error */
            exit(EXIT_FAILURE);
        }

        if (poll_num > 0) {
            if (fds[0].revents & POLLIN) {

                /* Console input is available: empty stdin and quit */

                while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
                    continue;
                break;
            }

            if (fds[1].revents & POLLIN) {

                /* Fanotify events are available */

                handle_events(fd);
            }
        }
    }

    printf("Listening for events stopped. (nopen=%d, nclose=%d, nremoved=%d, ndeleted=%d)\n",
		    nopen, nclose, nremoved, ndeleted);
    exit(EXIT_SUCCESS);
}

void main(int argc, char *argv[])
{
	const char *progname = basename(argv[0]);
	const char *path = argv[1];
	int depth = 0;

	if (argc < 3) {
		printf("usage: %s <root of directory tree> <directory tree depth> [-v]\n", progname);
		exit(1);
	}

	depth = atoi(argv[2]);

	if (chdir(path)) {
		perror(path);
		exit(1);
	}

	if (argc > 3 && !strcmp(argv[3], "-v"))
		verbose = 1;

	printf("%s %s tree_depth=%d, verbose=%d\n", progname, path, depth, verbose);

	fanotify_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT | FAN_NONBLOCK,
				    O_RDONLY | O_LARGEFILE);
	if (fanotify_fd == -1) {
		perror("fanotify_init");
		exit(EXIT_FAILURE);
	}

	/* Add marks in DFS so we won't trigger our own open permission events */
	iter_tree(do_add_mark, -depth);

	/* Listen until user input - remove marks on dir close events */
	listen_events(fanotify_fd);

	/* Flush remaning marks */
	if (fanotify_mark(fanotify_fd, FAN_MARK_FLUSH, 0, AT_FDCWD, path) != 0) {
		perror("fanotify_mark flush");
		exit(EXIT_FAILURE);
	}
}
