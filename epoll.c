#include "epoll.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "server.h"

#define MAX_EVENTS 1

void epoll_add(int fd);
void epoll_rem(int fd);
void epoll_loop();

struct polling epoll = {
	.add = epoll_add,
	.rem = epoll_rem,
	.loop = epoll_loop
};

int epfd = 0;
struct epoll_event events[MAX_EVENTS];

static void
epoll_start()
{
	if ((epfd = epoll_create(MAX_EVENTS)) == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}
}

struct polling *
epoll_get(void (*event)(int fd))
{
	if (!epfd)
		epoll_start();

	epoll.event = event;
	return &epoll;
}

void
epoll_add(int fd)
{
	struct epoll_event ev = { .events = EPOLLIN };

	ev.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl: EPOLL_CTL_ADD");
		exit(EXIT_FAILURE);
	}
}

void
epoll_rem(int fd)
{
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		perror("epoll_ctl: EPOLL_CTL_DEL");
		exit(EXIT_FAILURE);
	}
}

void
epoll_loop()
{
	int nfds, i;

	if ((nfds = epoll_wait(epfd, events, MAX_EVENTS, -1)) == -1) {
		perror("epoll_wait");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < nfds; ++i)
		epoll.event(events[i].data.fd);
}
