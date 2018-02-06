#ifndef _ITER_H
#define _ITER_H

extern int tree_depth;
extern int tree_width;
extern int leaf_count;
extern int node_count;
extern char *file_prefix;
extern char *dir_prefix;
extern int data_seed;
extern int keep_data;

void iter_usage();
int iter_parseopt(int argc, char *argv[]);

typedef int (*iter_op)(const char *, int);

int iter_tree(iter_op op, int depth);
#endif
