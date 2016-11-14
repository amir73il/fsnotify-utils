COMMON=error_functions.c

all: fanotify_demo inotify_demo fanotify_bug mkdirs rmdirs ioloop

install:
	./install-fanotify.sh

fanotify_demo: fanotify_demo.c $(COMMON)
	gcc -g -o $@ fanotify_demo.c $(COMMON)

inotify_demo: inotify_demo.c $(COMMON)
	gcc -g -o $@ inotify_demo.c $(COMMON)

fanotify_bug: fanotify_bug.c

mkdirs: mkdirs.c
	gcc -g -o $@ mkdirs.c

rmdirs: mkdirs
	ln -s mkdirs rmdirs

ioloop: ioloop.c
