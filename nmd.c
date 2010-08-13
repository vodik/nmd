#include <stdlib.h>
#include <stdio.h>

#include "nmd.h"

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
			struct wi_network *sndcup = NULL;
			ntwk = scan(ife->name);
			for (; ntwk; ntwk = ntwk->next) {
				if (strcmp(ntwk->essid, "Second Cup") == 0)
					sndcup = ntwk;
				printf("     %s -> %s\n", ntwk->bssid, ntwk->essid);
			}

			if (sndcup) {
				printf("attempting to connect to %s!\n", sndcup->essid);
				network_disconnect(ife);
				network_connect(ife, sndcup);
			}
		}
	}
	return 0;
}
