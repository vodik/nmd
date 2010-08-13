#include "connect.h"

#include <stdlib.h>
#include <stdio.h>

void
network_disconnect(struct interface* iface)
{
	printf("disconnecting on %s...\n", iface->name);

	iface_down(iface);
	/*dhcp_release(iface);
	iface_ip_reset(iface);*/
}

void
network_connect(struct interface *iface, struct wi_network *network)
{
	printf("connecting to %s on %s...\n", network->essid, iface->name);
}
