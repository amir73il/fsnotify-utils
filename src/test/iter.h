#ifndef _ITER_H
#define _ITER_H

typedef int (*iter_op)(const char *, int);

int iter_dirs(iter_op op, int depth);
#endif
