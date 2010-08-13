#include <stdlib.h>
#include <stdio.h>

#include "interfaces.h"
#include "scanning.h"

int
main(int argc, char *argv[])
{
	const struct interface *ife;
	const struct wi_network *ntwk;

	for (ife = get_interfaces(); ife; ife = ife->next) {
		printf("found dev %s\n", ife->name);
		if (ife->wi) {
			printf(" - is wireless");
			if (ife->wi->has_essid)
				printf("; essid: %s\n", ife->wi->essid);
			else
				printf("; not connected\n");

			printf(" - scanning for networks on %s...\n", ife->name);
			ntwk = scan(ife->name);
			for (; ntwk; ntwk = ntwk->next)
				printf("     %s -> %s\n", ntwk->bssid, ntwk->essid);
		}
	}
	return 0;
}
