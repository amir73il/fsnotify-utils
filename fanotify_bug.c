#include <err.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fanotify.h>
#include <sys/inotify.h>
#include <sys/wait.h>

int main(void)
{
	int fanotify_fd, inotify_fd;
	int ret, pid1, pid2;
	const char *testfile = "t";

	ret = unlink(testfile);
	if (ret == -1 && errno != ENOENT)
		err(1, "unlink()");

	ret = mknod(testfile, S_IFREG | 0666, 0);
	if (ret == -1)
		err(1, "mknod()");

	fanotify_fd = fanotify_init(FAN_CLASS_PRE_CONTENT, O_RDONLY);
	if (fanotify_fd == -1)
		err(1, "fanotify_init()");

	ret = fanotify_mark(fanotify_fd, FAN_MARK_ADD, FAN_OPEN_PERM, AT_FDCWD, testfile);
	if (ret == -1)
		err(1, "fanotify_mark()");

	pid1 = fork();
	if (pid1 == -1)
		err(1, "fork()");

	if (pid1 == 0) {
		close(fanotify_fd);
		ret = open(testfile, O_RDONLY);
		if (ret == -1)
			err(1, "open()");
		fprintf(stderr, "something went wrong: open succeeded\n");
		exit(1);
	}
	sleep(1);

	pid2 = fork();
	if (pid2 == -1)
		err(1, "fork()");

	if (pid2 == 0) {
		close(fanotify_fd);
		inotify_fd = inotify_init();
		if (inotify_fd == -1)
			err(1, "inotify_init()");
		ret = inotify_add_watch(inotify_fd, testfile, IN_ALL_EVENTS);
		if (ret == -1)
			err(1, "inotify_add_watch()");
		close(inotify_fd);
		fprintf(stderr, "close(inotify_fd): success\n");
		exit(0);
	}

	sleep(1);
	kill(pid1, SIGKILL);
	kill(pid2, SIGKILL);
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);

	return 0;
}
