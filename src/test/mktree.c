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
#include <attr/xattr.h>
#include "iter.h"
#include "xorshift.h"


static off64_t file_size;

#define KB (1024)
#define MB (KB * KB)

static uint32_t state[4];
static char data[MB];
static off_t block_size = 1;

struct xattr_t {
	const char *name;
	char *val;
	size_t len;
};

// From: source3/include/smb.h:
/* EA names used internally in Samba. KEEP UP TO DATE with prohibited_ea_names in trans2.c !. */
#define SAMBA_POSIX_INHERITANCE_EA_NAME "user.SAMBA_PAI"
/* EA to use for DOS attributes */
#define SAMBA_XATTR_DOS_ATTRIB "user.DOSATTRIB"

// From: libcli/smb/smb_constants.h
/* FileAttributes (search attributes) field */
#define FILE_ATTRIBUTE_READONLY         0x0001L
#define FILE_ATTRIBUTE_HIDDEN           0x0002L
#define FILE_ATTRIBUTE_SYSTEM           0x0004L
#define FILE_ATTRIBUTE_VOLUME           0x0008L
#define FILE_ATTRIBUTE_DIRECTORY        0x0010L
#define FILE_ATTRIBUTE_ARCHIVE          0x0020L
#define FILE_ATTRIBUTE_DEVICE           0x0040L
#define FILE_ATTRIBUTE_NORMAL           0x0080L

/*
 * Dos attribute structure which is binary compatible with samba's DosInfo4.
 * Storing it into the xattr named "DOSATTRIB" separately from inode
 * allows ksmbd to faithfully reproduce windows filesystem semantics
 * on top of a POSIX filesystem.
 */
struct xattr_dos_attrib {
	uint16_t   pad;            /* zeroed */
	uint16_t   version;        /* version 4 */
	uint32_t   version2;       /* version 4 */
	uint32_t   flags;          /* valid flags */
	uint32_t   attr;           /* Dos attribute */
	uint64_t   itime;          /* Invented/Initial time */
	uint64_t   create_time;    /* File creation time */
};

static struct xattr_t all_xattrs[] = {
	/* Directory-only xattrs: */
	{ "system.posix_acl_default" },
	/* Directory/File xattrs: */
	{ "system.posix_acl_access" },
	{ "security.NTACL" },
	{ SAMBA_POSIX_INHERITANCE_EA_NAME },
	/* Keep this last - it may be removed from array */
	{ SAMBA_XATTR_DOS_ATTRIB },
	{ NULL, NULL, 0 }
};

/* Skip directory-only xattrs */
static struct xattr_t *file_xattrs = all_xattrs + 1;

static mode_t dir_mode = 0777;
static mode_t file_mode = 0666;

static struct timespec times[2];

/* Fix DOSATTRIB of dir so they could be applied to files */
static int fix_dos_attrib(char *buf, size_t len)
{
	struct xattr_dos_attrib *dos = (struct xattr_dos_attrib *)buf;

	if (len != sizeof(struct xattr_dos_attrib) || dos->version != 4)
		return -1;

	if (!(dos->attr & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	/*
	 * Samba stores this flag for some reason and displays it also for
	 * non-dirs if the flag is stored, but we can safely remove the flag
	 * because Samba will automatically sets it for dirs if it is missing.
	 */
	dos->attr &= ~FILE_ATTRIBUTE_DIRECTORY;
	return 1;
}

static int read_acls(const char *name)
{
	struct xattr_t *xattr;
	ssize_t res;
	int ret = 0;
	int fd;

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		perror("open file");
		return fd;
	}

	for (xattr = all_xattrs; xattr->name; xattr++) {
		if (xattr->val) {
			free(xattr->val);
			xattr->val = NULL;
		}
		res = fgetxattr(fd, xattr->name, NULL, 0);
		if (res < 0 && errno != ENODATA)
			perror(xattr->name);
		if (res <= 0)
			continue;

		xattr->val = malloc(res);
		if (!xattr->val) {
			perror("alloc xattr");
			goto out;
		}
		res = fgetxattr(fd, xattr->name, xattr->val, res);
		if (res <= 0) {
			perror(xattr->name);
			goto out;
		}
		xattr->len = res;

		if (strcmp(xattr->name, SAMBA_XATTR_DOS_ATTRIB) == 0) {
			res = fix_dos_attrib(xattr->val, xattr->len);
			if (res < 0) {
				/* Do not set DOSATTRIB on files unless we managed to fix it */
				fprintf(stderr, "failed to read valid DOSATTRIB from '%s' - ignoring it.\n", name);
				xattr->name = NULL;
			} else if (res > 0) {
				/* Try to set the fixed DOSATTRIB on rootdir as well */
				fprintf(stderr, "removed directory attribute from '%s'.\n", name);
				(void)fsetxattr(fd, xattr->name, xattr->val, xattr->len, 0);
			}
		}

		ret++;
	}
out:
	close(fd);
	return ret;
}

static int write_xattrs(int fd, xid_t id, struct xattr_t *xattrs)
{
	struct xattr_t *xattr;
	int ret;

	if (id) {
		ret = fsetxattr(fd, XATTR_XID, &id, sizeof(id), 0);
		if (ret) {
			perror("fsetxattr xid");
			return ret;
		}
	}
	for (xattr = xattrs; copy_root_acls && xattr->name; xattr++) {
		if (!xattr->val)
			continue;
		ret = fsetxattr(fd, xattr->name, xattr->val, xattr->len, 0);
		if (ret) {
			perror(xattr->name);
			return ret;
		}
	}

	return 0;
}

static int write_random_block(int fd)
{
	int i;
	uint32_t *p = (uint32_t *)data;

	for (i = 0; i < block_size >> 2; i++)
		*p++ = xorshift128(state);

	return write(fd, data, block_size);
}

static int set_times(int fd)
{
	if (copy_root_mtime && futimens(fd, times) < 0) {
		perror("set mtime");
		return -1;
	}
	return 0;
}

static int create_file(const char *name, xid_t id)
{
	int i, ret = 0;
	int flags = O_CREAT|O_WRONLY|(keep_data ? 0 : O_TRUNC);
	int fd = open(name, flags, file_mode);

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
	if (ret >= 0)
		ret = write_xattrs(fd, id, file_xattrs);
	if (ret >= 0)
		ret = set_times(fd);
out:
	close(fd);
	return ret;
}

static int create_dir(const char *name, xid_t id)
{
	int fd;
	int ret = mkdir(name, dir_mode);

	if (ret < 0 && errno != EEXIST)
		return ret;

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		perror("open dir");
		return fd;
	}

	ret = write_xattrs(fd, id, all_xattrs);
	close(fd);
	return ret;
}

static int do_rm(const char *name, int depth, xid_t __attribute__((__unused__)) id)
{
	return depth ? rmdir(name) : unlink(name);
}

static int do_create(const char *name, int depth, xid_t id)
{
	return depth ? create_dir(name, id) : create_file(name, id);
}

static int do_print(const char *name, int depth, xid_t id)
{
	printf("%s%s%s\n", rel_path, name, depth ? "/" : "");
	return 0;
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

int main(int argc, char *argv[])
{
	char *path = argv[1];
	char *size_unit;
	struct stat stbuf;
	int ret;

	umask(0);
	progname = basename(argv[0]);
	if (argc < 4)
		usage();

	tree_depth = atoi(argv[2]);

	file_size = strtol(argv[3], &size_unit, 10);
	file_blocks = file_size ? 1 : 0;

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
			file_blocks = file_size * BLOCKS_PER_MB;
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
			file_blocks = file_size * BLOCKS_PER_MB;
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

	if (chdir(path) && !dry_run) {
		perror(path);
		exit(1);
	}

	if (stat(".", &stbuf) < 0) {
		perror(path);
		exit(1);
	}

	/*
	 * Use root dir permissions with stripped SUID bits for all dir creations
	 * and with stripped execute bits for all file creations.
	 */
	dir_mode = stbuf.st_mode & 0777;
	file_mode = dir_mode & 0666;

	if (copy_root_acls) {
		if (read_acls(".") <= 0) {
			fprintf(stderr, "failed to read ACLs from '%s' - ignoring -A flag.\n", path);
			copy_root_acls = 0;
		}
	}

	if (copy_root_mtime) {
		/* Will set files mtime/atime to root dir mtime */
		times[0] = times[1] = stbuf.st_mtim;
	}

	printf("%s %s\n", progname, path);
	// Print parameters that are mixed into random seed
	printf("tree_depth=%d\nfile_size=%ld%s\n"
		"tree_id=%x\ntree_width=%d\nleaf_start=%d\nleaf_count=%d\nnode_count=%d\n"
		"file_prefix='%s'\ndir_prefix='%s'\ndata_seed=%d\n",
		tree_depth, *size_unit ? file_size : file_size*block_size, size_unit,
		tree_id, tree_width, leaf_start, leaf_count, node_count,
		file_prefix, dir_prefix, data_seed);

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
		// For compatibility with deterministic data set created
		// before adding this parameter, do not mix the default 0
		// value into the random seed
		if (leaf_start)
			mixseed(state, leaf_start);
		if (tree_id)
			mixseed(state, tree_id);
		printf("mixed_seed=%u\n", state[0]);
	}

	// Print parameters that are not mixed into random seed
	printf("keep_data=%d\ncopy_root_acls=%d\ncopy_root_mtime=%d\n",
		keep_data, copy_root_acls, copy_root_mtime);

	if (dry_run)
		ret = iter_tree(do_print, tree_depth);
	else if (strcmp(progname, "rmtree") == 0)
		ret = iter_tree(do_rm, -tree_depth);
	else
		ret = iter_tree(do_create, tree_depth);

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}
