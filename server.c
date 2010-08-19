#include "server.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "epoll.h"

struct server {
	int fd;
	struct sockaddr_un addr;

	struct polling *poll;
} *s = NULL;

void
poll_cb(int fd)
{
	int cfd;
	size_t addr_len = sizeof(s->addr);

	if (fd == s->fd) {
		if ((cfd = accept(s->fd, (struct sockaddr *)&s->addr, &addr_len)) > -1)
			s->poll->add(cfd);
	} else {
		char buf[1024];
		int ret = read(fd, buf, 1024);
		if (ret == 0) {
			s->poll->rem(fd);
			close(fd);
		} else {
			buf[ret] = '\0';
			printf("read: [%d:%d] %s\n", fd, ret, buf);
		}
	}
}

struct server *server_open()
{
	s = malloc(sizeof(struct server));
	int buf_len;/*, ret;*/

	if (s == NULL)
		return NULL;

	s->fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (s->fd < 0) {
		free(s);
		return NULL;
	}

	buf_len = sizeof(s->addr.sun_path);
	s->addr.sun_family = AF_UNIX;
	/*ret = snprintf(s->addr.sun_path, buf_len, "/tmp/nmd_ctrl");*/
	strncpy(s->addr.sun_path, "/tmp/nmd_ctrl", buf_len);
	/*if (ret < 0 || (size_t)ret >= buf_len) {
		close(s->fd);
		free(s);
		return NULL;
	}*/

	unlink(s->addr.sun_path);
	if (bind(s->fd, (struct sockaddr *)&s->addr, sizeof(s->addr)) < 0) {
		close(s->fd);
		free(s);
		return NULL;
	}

	if (listen(s->fd, 5) != 0) {
		close(s->fd);
		free(s);
		return NULL;
	}

	s->poll = epoll_get(poll_cb);
	s->poll->add(s->fd);
	return s;
}

void server_close(struct server *s)
{
	unlink(s->addr.sun_path);
	close(s->fd);
	free(s);
}

void server_loop(struct server *s)
{
	while (1) {
		s->poll->loop();
	}
}
