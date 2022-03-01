/*
 * ioloop - benchmark small reads/writes
 *
 * Copyright (C) 2016 CTERA Network by Amir Goldstein <amir73il@gmail.com>
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#define MAX_LEN 1024

static char buf[MAX_LEN];

typedef ssize_t (*file_op)(int, void *, size_t);

static ssize_t do_write(int fd, void *buf, size_t len)
{
	return write(fd, buf, len);
}

static int io_loop(int fd, file_op op, size_t len, int count)
{
	while (count--) {
		if (op(fd, buf, len) < 1 && errno != EINTR)
			return -1;
	}

	return 0;
}

void main(int argc, char *argv[])
{
	const char *progname = basename(argv[0]);
	int len = 1, count = 0;
	const char *mode = "read";
	int flags = O_RDONLY;
	file_op op = read;
	char val = 0;
	int fd;

	if (argc < 2)
		exit(1);

	if (argc > 2)
		count = atoi(argv[2]);

	if (argc > 3 && argv[3][0] == 'w') {
		mode = "write";
		flags = O_WRONLY|O_CREAT;
		op = do_write;
	}

	if (argc > 4)
		len = atoi(argv[4]);

	if (len > MAX_LEN)
		len = MAX_LEN;

	fd = open(argv[1], flags, 0640);
	if (fd < 0) {
		perror(argv[1]);
		exit(1);
	}

	printf("%s count=%d len=%d op=%s\n", progname, count, len, mode);

	if (io_loop(fd, op, len, count))
		perror(mode);

	close(fd);
}
