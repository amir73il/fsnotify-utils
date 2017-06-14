/*
 * mkdirs/rmdirs - fast create/remove pre defined directory tree
 *
 * Copyright (C) 2016 CTERA Network by Amir Goldstein <amir73il@gmail.com>
 */
#define _GNU_SOURCE
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


static off_t file_size; /* < 0 indicates sparse file of size -file_size */

static int create_file(const char *name)
{
	int ret = 0, fd = creat(name, 0644);

	if (fd < 0) {
		perror("create file");
		return fd;
	}

	if (file_size > 0)
		ret = fallocate(fd, 0, 0, file_size);
	else if (file_size < 0)
		ret = ftruncate(fd, -file_size);

	close(fd);
	return ret;
}

static int do_mk(const char *name, int depth)
{
	return depth ? mkdir(name, 0751) : create_file(name);
}

static int do_rm(const char *name, int depth)
{
	return depth ? rmdir(name) : unlink(name);
}

static int do_create(const char *name, int depth)
{
	return do_mk(name, depth);
}

static const char *progname;

void usage()
{
	fprintf(stderr, "usage: %s <root of dirtree> <dirtree depth> <filesize MB> [options]\n", progname);
	fprintf(stderr, "options:\n");
	iter_usage();
	exit(1);
}

void main(int argc, char *argv[])
{
	const char *path = argv[1];
	int depth = 0;

	progname = basename(argv[0]);
	if (argc < 4)
		usage();

	depth = atoi(argv[2]);

	if (chdir(path)) {
		perror(path);
		exit(1);
	}

	file_size = atoi(argv[3]);
	printf("%s %s tree_depth=%d file_size=%dMB\n", progname, path, depth, (int)file_size);
	file_size *= 1024 * 1024;

	if (iter_parseopt(argc, argv) == -1)
		usage();

	if (strcmp(progname, "mkdirs") == 0)
		iter_dirs(do_create, depth);
	else if (strcmp(progname, "rmdirs") == 0)
		iter_dirs(do_rm, -depth);
}
