#ifndef NMD_INTERFACES_H
#define NMD_INTERFACES_H

#include <net/if.h>
#include "wireless.h"

struct interface {
	struct interface *next, *prev;

	char name[IFNAMSIZ];           /* interface name */

	struct wi_info *wi;
};

struct interface *get_interfaces();

void iface_up(struct interface *iface);
void iface_down(struct interface *iface);

#endif
