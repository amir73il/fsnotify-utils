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
#include <limits.h>
#include "iter.h"

int tree_id = 0;
int tree_depth;
int tree_width = 32;
int leaf_start = 0;
int leaf_count = 32;
int node_count;
char *file_prefix = "f";
char *dir_prefix = "d";
int data_seed = 0;
int keep_data = 0;
int copy_root_acls = 0;
int copy_root_mtime = 0;
int dry_run = 0;
int trace_depth = 0;
int xid = 0;
int node_id_log16;
int leaf_id_log16;
int block_id_log16;
int total_id_log16;
int file_blocks;

xid_t start_id = 0;
xid_t end_id = LLONG_MAX;

char rel_path[PATH_MAX];
static char *base = rel_path;

void iter_usage()
{
	fprintf(stderr, "-w <dirtree width>    (default = 32)\n");
	fprintf(stderr, "-c <leaf files count> (default = 32)\n");
	fprintf(stderr, "-C <node files count> (default = 0, < 0 for leaf files start offset)\n");
	fprintf(stderr, "-f <filename prefix>  (default = 'f')\n");
	fprintf(stderr, "-d <dirname prefix>   (default = 'd')\n");
	fprintf(stderr, "-v <trace depth>      (default = 0, implies -x)\n");
	fprintf(stderr, "-x <start global id>  (default = 0, id in hexa as printed by -v deepest trace prints)\n");
	fprintf(stderr, "-X <end global id>  (default = MAX, id in hexa as printed by -v deepest trace prints)\n");
	fprintf(stderr, "-N <tree prefix id>   (prefix id in hexa for all global ids, implies -x)\n");
	fprintf(stderr, "-s <data type/seed> [-k] (default = 0)\n");
	fprintf(stderr, "data type/seed may be 0 (default) for fallocate, < 0 for sparse file and > 0 for seed of random data\n");
	fprintf(stderr, "By default, existing file is truncated to zero before its data is allocated and initialized.\n");
	fprintf(stderr, "With -k option, an existing file is not truncated and allocated blocks are kept in the file.\n");
	fprintf(stderr, "random data overwrites existing data, but fallocate and ftruncate do not zero out existing data.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "-A copy ACLs from dirtree root to files and dirs\n");
	fprintf(stderr, "-M copy mtime from dirtree root to files mtime/atime\n");
	fprintf(stderr, "-n dry-run\n");
}

int iter_parseopt(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "AMc:C:w:s:f:d:v:x:X:N:kn")) != -1) {
		switch (c) {
			case 'A':
				copy_root_acls = 1;
				break;
			case 'M':
				copy_root_mtime = 1;
				break;
			case 'c':
				leaf_count = atoi(optarg);
				break;
			case 'C':
				node_count = atoi(optarg);
				if (node_count < 0) {
					/* Negative node count is used to skip files in leaf */
					leaf_start = -node_count;
					node_count = 0;
				}
				break;
			case 'w':
				tree_width = atoi(optarg);
				break;
			case 's':
				data_seed = atoi(optarg);
				break;
			case 'k':
				keep_data = 1;
				break;
			case 'n':
				dry_run = 1;
				break;
			case 'f':
				file_prefix = optarg;
				break;
			case 'd':
				dir_prefix = optarg;
				break;
			case 'v':
				trace_depth = atoi(optarg);
				/* Implies -x so created dir name is a progress indicator */
				xid = 1;
				break;
			case 'x':
				start_id = strtoll(optarg, NULL, 16);
				xid = 1;
				break;
			case 'X':
				end_id = strtoll(optarg, NULL, 16);
				xid = 1;
				break;
			case 'N':
				tree_id = strtoll(optarg, NULL, 16);
				xid = 1;
				break;
			default:
				fprintf(stderr, "illegal option '%s'\n", argv[optind]);
			case 'h':
				return -1;
		}
	}

	return 0;
}

static xid_t create_name(char *buf, int len, int is_dir, xid_t parent, xid_t i)
{
	const char *prefix = is_dir ? dir_prefix : file_prefix;
	xid_t id = 0;

	if (xid) {
		id = parent + i;
		snprintf(buf, len, "%s%llx", prefix, id);
	} else {
		snprintf(buf, len, "%s%lld", prefix, i);
	}

	return id;
}

static int skip_id(int depth, xid_t id)
{
	int tabs = tree_depth - abs(depth);

	if (tabs >= trace_depth)
		return 0;

	/* Skip only trace leaf id's */
	return (((id < start_id) || id > end_id) && (tabs == (trace_depth - 1)));
}

static int iter_names(iter_op op, int depth, xid_t parent)
{
	int ret;
	int i, start = depth ? 0 : leaf_start;
	int count = depth ? tree_width : leaf_count;
	char name[NAME_MAX+1];
	xid_t id;

	name[NAME_MAX] = 0;
iter_files:
	for (i = start; i < count; i++) {
		id = create_name(name, NAME_MAX, depth, parent, i);
		if (skip_id(depth, id))
			continue;
		ret = op(name, depth, id);
		if (ret && errno != EEXIST && errno != ENOENT) {
			perror(name);
			return ret;
		}
	}

	if (depth && node_count) {
		depth = 0;
		parent += count;
		count = node_count;
		goto iter_files;
	}

	return 0;
}

#ifndef O_PATH
#define O_PATH 010000000
#endif

static void trace_print(char *name, int depth, int ret)
{
	int tabs = tree_depth - abs(depth);

	while (tabs--)
		putchar('\t');
	printf("%s%s\n", name, ret < 0 ? " ABORTED!" :
			(ret ? " SKIPPED" : "..."));
}

static int trace_begin(char *name, int depth, xid_t id)
{
	int tabs = tree_depth - abs(depth);
	int len, ret = 0;

	len = sprintf(base, "%s/", name);
	base += len;

	if (tabs >= trace_depth)
		return len;

	/* Skip only trace leaf id's */
	if (skip_id(depth, id))
		ret = 1;

	trace_print(name, depth, ret);
	return ret ? -len : len;
}

static void trace_end(char *name, int depth, int ret, int len)
{
	base -= len;
	base[0] = 0;
	if (ret < 0) trace_print(name, depth, ret);
}

int iter_dirs(iter_op op, int depth, xid_t parent)
{
	int len, ret = 0;
	int fd = open(".", O_PATH);
	int i, count = tree_width;
	char name[NAME_MAX+1];
	int next_depth = depth > 0 ? depth - 1 : depth + 1;
	int next_id_log16 = next_depth ? node_id_log16 : leaf_id_log16;
	xid_t id;

	name[NAME_MAX] = 0;

	if (fd < 0) {
		perror("open(.)");
		return fd;
	}

	if (depth >= 0) // BFS
		ret = iter_names(op, depth, parent);

	if (ret || depth == 0)
		goto out;

	for (i = 0; i < count; i++) {
		id = create_name(name, NAME_MAX, 1, parent, i);
		ret = chdir(name);
		if (ret && errno != ENOENT) {
			perror("chdir");
			goto out;
		}

		ret = 0;
		len = trace_begin(name, depth, id);
		if (len > 0)
			ret = iter_dirs(op, next_depth, id << (next_id_log16*4));
		trace_end(name, depth, ret, abs(len));
		if (ret < 0)
			goto out;

		ret = fchdir(fd);
		if (ret) {
			perror("fchdir");
			goto out;
		}
	}
	if (depth < 0) // DFS
		ret = iter_names(op, depth, parent);

out:
	if (ret)
		fprintf(stderr, "name=%s depth=%d ret=%d\n", name, depth, ret);
	close(fd);
	return ret;
}

static unsigned int log16(unsigned int x)
{
	unsigned int ans = 0;

	while (x>>=4) ans++;
	return ans;
}

int iter_tree(iter_op op, int depth)
{
	int tree_id_log16 = 0;
	xid_t root = 1;

	// Calc number of hexa digits per tree level
	if (xid) {
		// reserve space for block offset and separator char
		if (file_blocks > 1)
			block_id_log16 = log16(file_blocks - 1) + 2;
		node_id_log16 = log16(tree_width + node_count - 1) + 1;
		if (leaf_count)
			leaf_id_log16 = log16(leaf_count - 1) + 1;
		if (tree_id) {
			tree_id_log16 = log16(tree_id - 1) + 1;
			root = tree_id << (node_id_log16*4);
		}
		total_id_log16 = tree_id_log16 + node_id_log16 * tree_depth + leaf_id_log16 + block_id_log16;
		printf("tree_id_digits=%d,node_id_digits=%d*%d,leaf_id_digits=%d,block_id_digits=%d\ntotal_id_digits=%d\n",
			tree_id_log16, node_id_log16, tree_depth, leaf_id_log16, block_id_log16, total_id_log16);
		if (total_id_log16 > XID_MAX_HEX_LEN) {
			fprintf(stderr, "Maximum global id has %d digit (limit is %d digit)\n"
					"Hint: remove -x/-v or reduce tree depth, width, or file size.\n",
				total_id_log16, XID_MAX_HEX_LEN);
			return -1;
		}
	}

	printf("-----------------\n");
	return iter_dirs(op, depth, root);
}
