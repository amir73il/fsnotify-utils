PROGS= fanotify_demo inotify_demo fanotify_bug
PROGS+= sbwatch addmarks rmmarks mkdirs rmdirs ioloop

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

rmdirs: mkdirs
	ln -s mkdirs rmdirs

rmmarks: addmarks
	ln -s addmarks rmmarks
