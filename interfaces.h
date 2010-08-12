#ifndef NMD_INTERFACES_H
#define NMD_INTERFACES_H

#include <net/if.h>

struct interface {
	struct interface *next, *prev;

	char name[IFNAMSIZ];           /* interface name */
};

const struct interface *get_interfaces();

#endif
