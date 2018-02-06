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
#include <ctype.h>
#include "iter.h"
#include "xorshift.h"


static off64_t file_size;

#define MB (1024 * 1024)

static uint32_t state[4];
static char data[MB];
static off_t block_size = MB;

static int write_random_block(int fd)
{
	int i;
	uint32_t *p = (uint32_t *)data;

	for (i = 0; i < block_size >> 2; i++)
		*p++ = xorshift128(state);

	return write(fd, data, block_size);
}

static int create_file(const char *name)
{
	int i, ret = 0;
	int fd = creat(name, 0644);

	if (fd < 0) {
		perror("create file");
		return fd;
	}

	if (!file_size || data_seed < 0) {
		ret = ftruncate64(fd, file_size * block_size);
	} else if (data_seed == 0) {
		ret = fallocate64(fd, 0, 0, file_size * block_size);
	} else {
		for (i = 0; i < file_size; i++) {
			ret = write_random_block(fd);
			if (ret < block_size) {
				perror("write_random_block");
				goto out;
			}
		}
		ret = 0;
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
	fprintf(stderr, "usage: %s <root of dirtree> <dirtree depth> <file size> [options]\n", progname);
	fprintf(stderr, "file size suffix may be 'k', 'K', 'm', 'M', 'g', 'G' (no suffix for mb).\n");
	fprintf(stderr, "options:\n");
	iter_usage();
	exit(1);
}

void main(int argc, char *argv[])
{
	char *path = argv[1];
	char *size_unit;

	progname = basename(argv[0]);
	if (argc < 4)
		usage();

	tree_depth = atoi(argv[2]);

	if (chdir(path)) {
		perror(path);
		exit(1);
	}

	file_size = strtol(argv[3], &size_unit, 10);

	switch (*size_unit) {
		case 0:
		case 'm':
		case 'M':
			break;
		case 'k':
		case 'K':
			block_size = 1024;
			break;
		case 'g':
		case 'G':
			file_size *= 1024;
			break;
		default:
			fprintf(stderr, "illegal size unit '%s'\n", size_unit);
			usage();
	}

	printf("%s %s\ntree_depth=%d\nfile_size=%d(%s)\n",
		progname, path, tree_depth, (int)file_size, size_unit);

	if (iter_parseopt(argc, argv) == -1)
		usage();

	printf("tree_width=%d\nleaf_count=%d\nnode_count=%d\nfile_prefix='%s'\ndir_prefix='%s'\ndata_seed=%d\n",
		tree_width, leaf_count, node_count, file_prefix, dir_prefix, data_seed);

	if (data_seed > 0) {
		// mix params with random seed
		mixseed(state, strhash(basename(path)));
		mixseed(state, tree_depth);
		mixseed(state, file_size);
		mixseed(state, tree_width);
		mixseed(state, leaf_count);
		mixseed(state, node_count);
		mixseed(state, strhash(file_prefix));
		mixseed(state, strhash(dir_prefix));
		mixseed(state, data_seed);
		printf("mixed_seed=%u\n", state[0]);
	}

	if (strcmp(progname, "mktree") == 0)
		iter_tree(do_create, tree_depth);
	else if (strcmp(progname, "rmtree") == 0)
		iter_tree(do_rm, -tree_depth);
}
