#ifndef NMD_WIRELESS
#define NMD_WIRELESS

#include <stdbool.h>
#include <iwlib.h>

struct wi_info {
	bool has_sens : 1;
	bool has_essid : 1;
	bool has_nickname : 1;
	bool has_ap_addr : 1;
	bool has_bitrate : 1;
	bool has_rts : 1;
	bool has_frag : 1;
	bool has_power : 1;
	bool has_txpower : 1;
	bool has_retry : 1;
	bool has_stats : 1;
	bool has_range : 1;

	iwparam sens;
	char essid[IW_ESSID_MAX_SIZE + 1];
	char nickname[IW_ESSID_MAX_SIZE + 1];
	sockaddr ap_addr;
	iwparam bitrate;
	iwparam rts;
	iwparam frag;
	iwparam power;
	iwparam txpower;
	iwparam retry;
	iwstats stats;
	iwrange range;
};

#endif
