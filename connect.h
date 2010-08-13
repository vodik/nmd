#ifndef NMD_CONNECT
#define NMD_CONNECT

#include "interfaces.h"
#include "scanning.h"

void network_disconnect(struct interface *iface);
void network_connect(struct interface *iface, struct wi_network *network);

#endif
