/*
 * fanotify_example.c - from man fanotify(7)
 */
#define _GNU_SOURCE     /* Needed to get O_LARGEFILE definition */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

static int nrequests;
static int fail_every;

#ifndef FAN_REPORT_FD_ERROR
#define FAN_REPORT_FD_ERROR    0x00002000      /* event->fd can report error */
#endif

#ifndef FAN_PRE_ACCESS
#define FAN_PRE_ACCESS		0x00100000	/* Pre-content access hook */
#endif

#ifndef FAN_PRE_DIR_ACCESS
#define FAN_PRE_DIR_ACCESS	0x00200000	/* Pre-content dir access hook */
#endif

#ifndef FAN_REPORT_DFID_NAME_TARGET
#define FAN_REPORT_DFID_NAME_TARGET	(FAN_REPORT_DFID_NAME | FAN_REPORT_FID | \
					 0x00001000 /* FAN_REPORT_TARGET_FID */)
#endif

#ifndef FAN_CLASS_PRE_CONTENT_FID
#define FAN_CLASS_PRE_CONTENT_FID	(FAN_CLASS_PRE_CONTENT | \
					 FAN_REPORT_DFID_NAME_TARGET)
#endif

#ifndef FAN_EVENT_INFO_TYPE_RANGE
#define FAN_EVENT_INFO_TYPE_RANGE       6

struct fanotify_event_info_range {
	struct fanotify_event_info_header hdr;
	uint32_t pad;
	uint64_t offset;
	uint64_t count;
};
#endif

/*
 * Return the info record of type info_type, or NULL if the event has no such
 * info record.
 */
static const struct fanotify_event_info_header *
event_info(const struct fanotify_event_metadata *metadata, int info_type)
{
    const char *rec = (const char *)metadata + metadata->metadata_len;
    const char *end = (const char *)metadata + metadata->event_len;

    while (rec + sizeof(struct fanotify_event_info_header) <= end) {
        const struct fanotify_event_info_header *hdr = (const void *)rec;

        if (!hdr->len || rec + hdr->len > end)
            break;
        if (hdr->info_type == info_type)
            return hdr;
        rec += hdr->len;
    }
    return NULL;
}

/*
 * Return the name from the dir fid info record of a FAN_PRE_DIR_ACCESS event,
 * which is empty for readdir and the looked up name for lookup.
 */
static const char *
event_name(const struct fanotify_event_metadata *metadata)
{
    const struct fanotify_event_info_fid *fid = (const void *)
        event_info(metadata, FAN_EVENT_INFO_TYPE_DFID_NAME);
    const struct file_handle *fh;
    const char *name;

    if (!fid)
        return "";
    fh = (const void *)fid->handle;
    name = (const char *)fh->f_handle + fh->handle_bytes;
    return name < (const char *)fid + fid->hdr.len ? name : "";
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

    /* Loop while events can be read from fanotify file descriptor. */

    for (;;) {

        /* Read some events. */

        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Check if end of available data reached. */

        if (len <= 0)
            break;

        /* Point to the first event in the buffer. */

        metadata = buf;

        /* Loop over all events in the buffer. */

        while (FAN_EVENT_OK(metadata, len)) {

            if (metadata->vers != FANOTIFY_METADATA_VERSION) {
                fprintf(stderr,
                        "Mismatch of fanotify metadata version.\n");
                exit(EXIT_FAILURE);
            }

            /* metadata->fd contains either FAN_NOFD, indicating a
               queue overflow, or a file descriptor (a nonnegative
               integer). Here, we simply ignore queue overflow. */


                /* Handle open/opendir permission event. */

	    if (metadata->mask & FAN_OPEN_PERM) {
		printf("FAN_OPEN_PERM: ");

                    /* Allow file to be opened. */

		if (metadata->fd >= 0) {
                    response.fd = metadata->fd;
                    response.response = FAN_ALLOW;
                    write(fd, &response, sizeof(response));
                }
	    }

                /* Handle access/readdir permission event. */

	    if (metadata->mask & (FAN_ACCESS_PERM | FAN_PRE_ACCESS)) {
		if (metadata->mask & FAN_ACCESS_PERM)
		    printf("FAN_ACCESS_PERM: ");
		if (metadata->mask & FAN_PRE_ACCESS) {
		    const struct fanotify_event_info_range *range = (const void *)
		        event_info(metadata, FAN_EVENT_INFO_TYPE_RANGE);

		    printf("FAN_PRE_ACCESS: ");
		    if (range)
		        printf("[%llu..%llu] ", (unsigned long long)range->offset,
			       (unsigned long long)(range->offset + range->count));
		}
		if (metadata->fd >= 0) {
		    nrequests++;
                    response.fd = metadata->fd;
                    response.response = !fail_every || (nrequests % fail_every) ? FAN_ALLOW : FAN_DENY;
                    write(fd, &response, sizeof(response));
		    if (response.response == FAN_DENY)
                        printf("DENIED! ");
                }
	    }

                /* Handle directory pre-content (readdir/lookup) event. */

	    if (metadata->mask & FAN_PRE_DIR_ACCESS) {
		const char *name = event_name(metadata);

		if (*name)
		    printf("FAN_PRE_DIR_ACCESS: lookup '%s' in ", name);
		else
		    printf("FAN_PRE_DIR_ACCESS: readdir ");
		if (metadata->fd >= 0) {
		    nrequests++;
                    response.fd = metadata->fd;
                    response.response = !fail_every || (nrequests % fail_every) ? FAN_ALLOW : FAN_DENY;
                    write(fd, &response, sizeof(response));
		    if (response.response == FAN_DENY)
                        printf("DENIED! ");
                }
	    }

                /* Handle closing of writable file event. */

	    if (metadata->mask & FAN_CLOSE_WRITE)
                printf("FAN_CLOSE_WRITE: ");

	    if (metadata->fd < 0) {
		printf("fd open failed: %s\n", strerror(-metadata->fd));
	    } else {
                /* Retrieve and print pathname of the accessed file. */

                snprintf(procfd_path, sizeof(procfd_path),
                         "/proc/self/fd/%d", metadata->fd);
                path_len = readlink(procfd_path, path,
                                    sizeof(path) - 1);
                if (path_len == -1) {
                    perror("readlink");
                    exit(EXIT_FAILURE);
                }

                path[path_len] = 0;
                printf("File %s\n", path);

                /* Close the file descriptor of the event. */

                close(metadata->fd);
            }

            /* Advance to next event. */

            metadata = FAN_EVENT_NEXT(metadata, len);
        }
    }
}

int
main(int argc, char *argv[])
{
    char buf;
    int fd, pre_fd, poll_num;
    nfds_t nfds;
    struct pollfd fds[3];

    /* Check mount point is supplied. */

    if (argc < 2) {
        fprintf(stderr, "Usage: %s MOUNT [fail every]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc > 2)
        fail_every = atoi(argv[2]);

    printf("Press enter key to terminate.\n");

    /* Create the file descriptor for accessing the fanotify API. */

    fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT | FAN_NONBLOCK |
		       FAN_REPORT_FD_ERROR, O_RDONLY | O_LARGEFILE);
    if (fd == -1) {
        perror("fanotify_init");
        exit(EXIT_FAILURE);
    }

    /* Mark the mount for:
       - permission events before opening files and directories
       - permission events before reading files and directories
       - notification events after closing a write-enabled
         file descriptor. */

    if (fanotify_mark(fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                      FAN_OPEN_PERM | FAN_ACCESS_PERM |
                      FAN_CLOSE_WRITE | FAN_ONDIR, AT_FDCWD,
                      argv[1]) == -1) {
        perror("fanotify_mark");
        exit(EXIT_FAILURE);
    }

    /* Create a second group for pre-content events before accessing file
       content.  Pre-content events do not support event flags, not even
       in the combined mask of a mark, so they cannot be watched together
       with the FAN_ONDIR events above.
       With a group of class FAN_CLASS_PRE_CONTENT_FID, which reports fid
       info and only allows pre-content events, also watch directory
       pre-content events (readdir/lookup), which need no FAN_ONDIR flag.
       Fall back to FAN_CLASS_PRE_CONTENT for file pre-content events. */

    uint64_t pre_mask = FAN_PRE_ACCESS | FAN_PRE_DIR_ACCESS;

    pre_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_PRE_CONTENT_FID | FAN_NONBLOCK |
		           FAN_REPORT_FD_ERROR, O_RDONLY | O_LARGEFILE);
    if (pre_fd == -1) {
        perror("fanotify_init FAN_CLASS_PRE_CONTENT_FID (not supported?)");
        pre_mask = FAN_PRE_ACCESS;
        pre_fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_PRE_CONTENT | FAN_NONBLOCK |
		               FAN_REPORT_FD_ERROR, O_RDONLY | O_LARGEFILE);
    }
    if (pre_fd == -1) {
        perror("fanotify_init FAN_CLASS_PRE_CONTENT (not supported?)");
    } else if (fanotify_mark(pre_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                             pre_mask, AT_FDCWD, argv[1]) == -1) {
        perror("fanotify_mark pre-content events");
        exit(EXIT_FAILURE);
    }

    /* Prepare for polling. */

    nfds = pre_fd == -1 ? 2 : 3;

    fds[0].fd = STDIN_FILENO;       /* Console input */
    fds[0].events = POLLIN;

    fds[1].fd = fd;                 /* Fanotify input */
    fds[1].events = POLLIN;

    fds[2].fd = pre_fd;             /* Fanotify pre-content events input */
    fds[2].events = POLLIN;

    /* This is the loop to wait for incoming events. */

    printf("Listening for events.\n");

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

                /* Console input is available: empty stdin and quit. */

                while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
                    continue;
                break;
            }

            if (fds[1].revents & POLLIN) {

                /* Fanotify events are available. */

                handle_events(fd);
            }

            if (nfds > 2 && fds[2].revents & POLLIN) {

                /* Fanotify pre-content events are available. */

                handle_events(pre_fd);
            }
        }
    }

    printf("Listening for events stopped.\n");
    exit(EXIT_SUCCESS);
}

