COMMON=error_functions.c

all: fanotify_demo inotify_demo

fanotify_demo: fanotify_demo.c $(COMMON)
	gcc -g -o $@ fanotify_demo.c $(COMMON)

inotify_demo: inotify_demo.c $(COMMON)
	gcc -g -o $@ inotify_demo.c $(COMMON)

