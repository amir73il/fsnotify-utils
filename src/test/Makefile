PROGS= fanotify_bug fanotify_example
TLPI_PROGS= fanotify_demo inotify_demo dnotify
ITER_PROGS= sbwatch watchdirs mktree rmtree ioloop

TLPI=../lib/error_functions.c

ITER=iter.c

CFLAGS= -I../lib -g

all: $(PROGS) $(TLPI_PROGS) $(ITER_PROGS)

$(TLPI_PROGS): $(TPLI)

$(ITER_PROGS): $(ITER)

clean:
	rm $(PROGS)

install:
	../install.sh $(PROGS)

fanotify_demo: fanotify_demo.c $(TLPI)

inotify_demo: inotify_demo.c $(TLPI)

dnotify: dnotify.c $(TLPI)

rmtree: mktree
	ln -s mktree rmtree
