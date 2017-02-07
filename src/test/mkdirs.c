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


static off_t file_size = 1024 * 1024;

static int do_mk(const char *name, int isdir)
{
	if (isdir)
		return mkdir(name, 0751);
	if (mknod(name, 0644, 0) != 0)
		return -1;
	return truncate(name, file_size);
}

static int do_rm(const char *name, int isdir)
{
	return isdir ? rmdir(name) : unlink(name);
}

static int do_create(const char *name, int isdir)
{
	return do_mk(name, isdir) || chmod(name, 0750) || chown(name, 1000, 1000);
}

void main(int argc, char *argv[])
{
	const char *progname = basename(argv[0]);
	int depth = 0;

	if (argc == 1) {
		printf("usage: %s <directory tree depth> <root of tree> [file size in MB]\n", argv[0]);
		exit(1);
	}

	if (argc > 1)
		depth = atoi(argv[1]);

	if (argc > 2 && chdir(argv[2])) {
		perror(argv[2]);
		exit(1);
	}

	if (argc > 3)
		file_size = atoi(argv[3]) * 1024 * 1024;

	printf("%s tree_depth=%d, file_size=%dMB\n", progname, depth, (int)(file_size >> 20));

	if (strcmp(progname, "mkdirs") == 0)
		iter_dirs(do_create, depth);
	else if (strcmp(progname, "rmdirs") == 0)
		iter_dirs(do_rm, -depth);
}
