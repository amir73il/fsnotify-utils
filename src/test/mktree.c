/*
 * mktree/rmtree - fast create/remove pre defined directory tree
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
#include <stdint.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include "iter.h"
#include "xorshift.h"


static off_t file_size_mb;

#define MB (1024 * 1024)

static uint32_t state[4];
static char data[MB];
static char *data_type;

static int write_random_mb(int fd)
{
	int i;
	uint32_t *p = (uint32_t *)data;

	for (i = 0; i < MB >> 2; i++)
		*p++ = xorshift128(state);

	return write(fd, data, MB);
}

static int create_file(const char *name)
{
	int i, ret = 0;
	int fd = creat(name, 0644);

	if (fd < 0) {
		perror("create file");
		return fd;
	}

	switch (*data_type) {
	case 0:
		ret = fallocate(fd, 0, 0, file_size_mb * MB);
		break;
	case 's':
		ret = ftruncate(fd, file_size_mb * MB);
		break;
	case 'r':
		for (i = 0; i < file_size_mb; i++) {
			ret = write_random_mb(fd);
			if (ret < MB) {
				perror("write_random_mb");
				goto out;
			}
		}
		ret = 0;
		break;
	}
out:
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
	fprintf(stderr, "usage: %s <root of dirtree> <dirtree depth> <filesize MB>[data type] [options]\n", progname);
	fprintf(stderr, "data type suffix:\n");
	fprintf(stderr, "no suffix for default (fallocate), 's' (sparse file), 'r' (random data)\n");
	fprintf(stderr, "options:\n");
	iter_usage();
	exit(1);
}

void main(int argc, char *argv[])
{
	char *path = argv[1];
	int depth = 0;

	progname = basename(argv[0]);
	if (argc < 4)
		usage();

	depth = atoi(argv[2]);

	if (chdir(path)) {
		perror(path);
		exit(1);
	}

	file_size_mb = strtol(argv[3], &data_type, 10);

	printf("%s %s\ntree_depth=%d\nfile_size=%dMB\ndata_type='%s'\n",
		progname, path, depth, (int)file_size_mb, data_type);

	if (iter_parseopt(argc, argv) == -1)
		usage();

	printf("tree_width=%d\nleaf_count=%d\nnode_count=%d\nfile_prefix='%s'\ndir_prefix='%s'\nrandom_seed=%u\n",
		tree_width, leaf_count, node_count, file_prefix, dir_prefix, random_seed);

	if (*data_type == 'r') {
		// mix params with random seed
		mixseed(state, strhash(basename(path)));
		mixseed(state, depth);
		mixseed(state, file_size_mb);
		mixseed(state, tree_width);
		mixseed(state, leaf_count);
		mixseed(state, node_count);
		mixseed(state, strhash(file_prefix));
		mixseed(state, strhash(dir_prefix));
		mixseed(state, random_seed);
		printf("mixed_seed=%u\n", state[0]);
	}

	if (strcmp(progname, "mktree") == 0)
		iter_dirs(do_create, depth);
	else if (strcmp(progname, "rmtree") == 0)
		iter_dirs(do_rm, -depth);
}
