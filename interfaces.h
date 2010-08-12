#ifndef NMD_INTERFACES_H
#define NMD_INTERFACES_H

#include <net/if.h>
#include "wireless.h"

struct interface {
	struct interface *next, *prev;

	char name[IFNAMSIZ];           /* interface name */

	struct wi_info *wi;
};

const struct interface *get_interfaces();

#endif
