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

#define KB (1024)
#define MB (KB * KB)

static uint32_t state[4];
static char data[MB];
static off_t block_size = 1;

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
	int flags = O_CREAT|O_WRONLY|(keep_data ? 0 : O_TRUNC);
	int fd = open(name, flags, 0644);

	if (fd < 0) {
		perror("create file");
		return fd;
	}

	if (!file_size || data_seed < 0) {
		ret = ftruncate64(fd, file_size * block_size);
		if (ret)
			perror("ftruncate64");
	} else if (data_seed == 0) {
		ret = fallocate64(fd, 0, 0, file_size * block_size);
		if (ret)
			perror("fallocate64");
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
			if (file_size > MB) {
				fprintf(stderr, "illegal file size '%ld'. try specifying size in K or M units\n", file_size);
				usage();
			}
			break;
		case 'm':
		case 'M':
			block_size = MB;
			break;
		case 'k':
		case 'K':
			block_size = KB;
			break;
		case 'g':
		case 'G':
			block_size = MB;
			size_unit = "m";
			file_size *= KB;
			break;
		default:
			fprintf(stderr, "illegal size unit '%s'\n", size_unit);
			usage();
	}

	if (iter_parseopt(argc, argv) == -1)
		usage();

	if (file_size && block_size == 1) {
		block_size = file_size;
		file_size = 1;
		/* random data block size must be 4 bytes aligned */
		if (data_seed > 0)
			block_size = (block_size >> 2) << 2;
	}

	printf("%s %s\ntree_depth=%d\nfile_size=%ld%s\n",
		progname, path, tree_depth, *size_unit ? file_size : block_size, size_unit);
	printf("tree_width=%d\nleaf_count=%d\nnode_count=%d\nfile_prefix='%s'\ndir_prefix='%s'\ndata_seed=%d\nkeep_data=%d\n",
		tree_width, leaf_count, node_count, file_prefix, dir_prefix, data_seed, keep_data);

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
