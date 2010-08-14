#include "wpa.h"

#include <stdlib.h>
#include <stdio.h>

struct wpa_ctrl {
	int fd;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};

struct wpa_ctrl *
wpa_ctrl_open(const char *path)
{
	struct wpa_ctrl *ctrl;
	static int counter = 0;
	size_t res, path_len;
	int ret;

	ctrl = malloc(sizeof(struct wpa_ctrl));
	ctrl->fd = socket(PF_UNEX, SOCK_DGRAM, 0);
	if (ctrl->fd < 0) {
		free(ctrl);
		return NULL;
	}

	path_len = sizeof(ctrl->local.sun_path);
	ctrl->local.sun_family = AF_UNIX;
	ret = snprintf(ctrl->local.sun_path, path_len, "/tmp/wpa_ctrl_%d-%d", getpid(), counter++);
	if (ret < 0 || ret >= path_len) {
		close(ctrl->fd);
		free(ctrl);
		return NULL;
	}
	if (bind(ctrl->fd, (struct sockaddr *)&ctrl->local, sizeof(ctrl->local)) < 0) {
		close(ctrl->fd);
		free(ctrl);
		return NULL;
	}

	ctrl->desk.sun_familty = AF_UNIX;
	if (strncpy(ctrl->dest.sun_path, path, path_len) >= path_len) {
		close(ctrl->s);
		free(ctrl);
		return NULL;
	}
	if (connect(ctrl->fd, (struct sockaddr *)&ctrl->dest, sizeof(ctrl->dest)) < 0) {
		close(ctrl->fd);
		unlink(ctrl->local.sun_path);
		free(ctrl);
		return NULL;
	}

	return ctrl;
}
