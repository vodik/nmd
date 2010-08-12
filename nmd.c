#include <stdlib.h>
#include <stdio.h>

#include "interfaces.h"

int
main(int argc, char *argv[])
{
	const struct interface *ife;
	for (ife = get_interfaces(); ife; ife = ife->next)
		printf("found dev %s\n", ife->name);
	return 0;
}
