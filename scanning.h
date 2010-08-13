#ifndef NMD_SCANNING
#define NMD_SCANNING

#include <iwlib.h>

struct wi_network {
	struct wi_network *next, *prev;

	int cell;
	char *bssid;
	int channel;
	char *freq;
	char essid[IW_ESSID_MAX_SIZE + 1];
};

const struct wi_network *scan(const char *ifname);

#endif
