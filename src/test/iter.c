/*
 * iter - fast iterate pre defined directory tree
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
#include "iter.h"

const char *names = "0\0!\0@\0#\0_\0+\0a\0b\0c\0d\0e\0f\0g\0h\0i\0j\0k\0l\0m\0n\0o\0p\0q\0r\0s\0t\0u\0v\0w\0x\0y\0z\0\0";

static int iter_names(iter_op op, int depth)
{
	const char *name = names;
	int ret;

	while (*name) {
		ret = op(name, depth);
	       	if (ret && errno != EEXIST && errno != ENOENT) {
			perror(name);
			return ret;
		}
		name += 2;
	}

	return 0;
}

#ifndef O_PATH
#define O_PATH 010000000
#endif

int iter_dirs(iter_op op, int depth)
{
	const char *name = names;
	int ret = 0;
	int fd = open(".", O_PATH);

	if (fd < 0) {
		perror("open(.)");
		return fd;
	}

	if (depth >= 0) // BFS
		ret = iter_names(op, depth);
	
	if (ret || depth == 0)
		goto out;

	while (*name) {
		if (ret = chdir(name) && errno != ENOENT) {
			perror("chdir");
			goto out;
		}
		if (depth > 0) // BFS
			ret = iter_dirs(op, depth - 1);
		if (depth < 0) // DFS
			ret = iter_dirs(op, depth + 1);
		if (ret)
			goto out;
		if (ret = fchdir(fd)) {
			perror("fchdir");
			goto out;
		}
		name += 2;
	}
	if (depth < 0) // DFS
		ret = iter_names(op, depth);

out:
	if (ret)
		fprintf(stderr, "name=%s depth=%d ret=%d\n", name, depth, ret);
	close(fd);
	return ret;
}


