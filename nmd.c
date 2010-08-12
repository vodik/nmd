#include <stdlib.h>
#include <stdio.h>

#include "interfaces.h"

int
main(int argc, char *argv[])
{
	const struct interface *ife;
	for (ife = get_interfaces(); ife; ife = ife->next) {
		printf("found dev %s\n", ife->name);
		if (ife->wi) {
			printf(" - is wireless");
			if (ife->wi->has_essid)
				printf("; essid: %s\n", ife->wi->essid);
			else
				printf("; not connected\n");
		}
	}
	return 0;
}
