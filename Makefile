PROGS=fanotify_demo inotify_demo fanotify_bug fanotify_example mkdirs rmdirs ioloop
SCRIPTS=test_demo.sh

COMMON=error_functions.c

all: $(PROGS)

clean:
	rm $(PROGS)

install:
	./install-fanotify.sh $(PROGS) $(SCRIPTS)

fanotify_demo: fanotify_demo.c $(COMMON)
	gcc -g -o $@ fanotify_demo.c $(COMMON)

inotify_demo: inotify_demo.c $(COMMON)
	gcc -g -o $@ inotify_demo.c $(COMMON)

rmdirs: mkdirs
	ln -s mkdirs rmdirs
