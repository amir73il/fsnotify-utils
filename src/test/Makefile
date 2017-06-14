PROGS= fanotify_demo inotify_demo fanotify_bug
PROGS+= sbwatch watchdirs mktree rmtree ioloop

TLPI=../lib/error_functions.c

COMMON=iter.c

CFLAGS= -I../lib -g

all: $(PROGS)

$(PROGS): $(COMMON)

clean:
	rm $(PROGS)

install:
	../install.sh $(PROGS)

fanotify_demo: fanotify_demo.c $(TLPI)

inotify_demo: inotify_demo.c $(TLPI)

rmtree: mktree
	ln -s mktree rmtree
