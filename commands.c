#include "commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static void
run(const char *command, char *const argv[])
{
	pid_t pid;
	pid = fork();

	if (pid == -1) {
		perror("Error when forking");
		exit(EXIT_FAILURE);
	} else if (pid == 0)
		execv(command, argv);
	else
		wait(0);
}

void
dhcp_aquire(const char *dev, const char *hostname)
{
	printf("dhcp aquire on %s...\n", dev);

	char *argv[] = { (char *)dev, "-h", (char *)hostname,  NULL };
	run("/usr/sbin/dhcpcd", argv);
}

void
dhcp_release(const char *dev)
{
	printf("dhcp release on %s...\n", dev);

	char *argv[] = { "-k", (char *)dev, NULL };
	run("/usr/sbin/dhcpcd", argv);
}
