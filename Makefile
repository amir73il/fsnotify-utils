SRCS=demo_inotify.c error_functions.c

demo_inotify: $(SRCS)
	gcc -o $@ $(SRCS)

