/*
 * mkdirs/rmdirs - fast create/remove pre defined directory tree
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


/* -1 indicates no files only dirs */
static off_t file_size = -1;

static int do_mk(const char *name, int depth)
{
	if (depth || file_size < 0)
		return mkdir(name, 0751);
	if (mknod(name, 0644, 0) != 0)
		return -1;
	return truncate(name, file_size);
}

static int do_rm(const char *name, int depth)
{
	return (depth || file_size < 0) ? rmdir(name) : unlink(name);
}

static int do_create(const char *name, int depth)
{
	return do_mk(name, depth) || chmod(name, 0750) || chown(name, 1000, 1000);
}

void main(int argc, char *argv[])
{
	const char *progname = basename(argv[0]);
	const char *path = argv[1];
	int depth = 0;

	if (argc < 3) {
		printf("usage: %s <root of directory tree> <directory tree depth> [file size in MB]\n", progname);
		exit(1);
	}

	depth = atoi(argv[2]);

	if (chdir(path)) {
		perror(path);
		exit(1);
	}

	if (argc > 3)
		file_size = atoi(argv[3]) * 1024 * 1024;

	printf("%s %s tree_depth=%d, file_size=%d\n", progname, path, depth, (int)(file_size >> 20));

	if (strcmp(progname, "mkdirs") == 0)
		iter_dirs(do_create, depth);
	else if (strcmp(progname, "rmdirs") == 0)
		iter_dirs(do_rm, -depth);
}
