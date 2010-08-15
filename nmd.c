#include <stdlib.h>
#include <stdio.h>

#include "nmd.h"
#include "server.h"

void
server_run(int argc, char *argv[])
{
	char buf[1024];
	int ret = read(argc, buf, 1024);
	buf[ret] = '\0';

	printf("got: %s\n", buf);
}

int
main(int argc, char *argv[])
{
	struct server *server = server_open();
	server_loop(server, server_run);
	server_close(server);
}
