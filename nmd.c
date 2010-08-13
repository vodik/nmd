#include <stdlib.h>
#include <stdio.h>

#include "interfaces.h"
#include "scanning.h"

int
main(int argc, char *argv[])
{
	struct interface *ife;
	struct wi_network *ntwk;

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

	if (argc == 2) {
		ife = get_interfaces();
		for (; ife; ife = ife->next)
			if (strcmp(ife->name, "eth0") == 0)
				break;

		if (strcmp(argv[1], "up") == 0) {
			printf("bringing up %s\n", ife->name);
			iface_up(ife);
		} else if (strcmp(argv[1], "down") == 0) {
			printf("bringing down %s\n", ife->name);
			iface_down(ife);
		}
	}

	return 0;
}
