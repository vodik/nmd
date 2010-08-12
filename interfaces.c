#include "interfaces.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <net/if.h>
#include <iwlib.h>

#define _PATH_PROCNET_DEV       "/proc/net/dev"
#define _PATH_PROCNET_WIRELESS  "/proc/net/wireless"

/*struct wireless_info {
	bool has_range;
};*/

static struct interface *head = NULL, *tail = NULL;

static struct interface *
add_interface(const char *name)
{
	struct interface *node;

	for (node = head; node; node = node->next) {
		if (strcmp(node->name, name) == 0)
			return node;
	}

	node = malloc(sizeof(struct interface));
	strncpy(node->name, name, IFNAMSIZ);

	if (head == NULL) {
		head = node;
		node->prev = NULL;
	} else {
		tail->next = node;
		node->prev = tail;
	}

	tail = node;
	node->next = NULL;
	return node;
}

static char *
get_name(char *name, char *p)
{
	while (isspace(*p))
		++p;

	while (*p) {
		if (isspace(*p))
			break;

		if (*p == ':') {
			char *dot = p, *dotname = name;
			*name++ = *p++;
			while (isdigit(*p))
				*name++ = *p++;
			if (*p != ':') {
				p = dot;
				name = dotname;
			}
			if (*p == '\0')
				return NULL;
			p++;
			break;
		}
		*name++ = *p++;
	}
	*name++ = '\0';
	return p;
}

static int
get_info(int skfd, char *ifname)
{
	struct wireless_info *info = malloc(sizeof(struct wireless_info));

	struct iwreq wrq;
	memset(info, 0, sizeof(struct wireless_info));

	if (iw_get_basic_config(skfd, ifname, &(info->b)) < 0) {
		struct ifreq ifr;
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
			return -ENODEV;
		else
			return -ENOTSUP;
	}

	printf("wireless extension on %s!\n", ifname);

	if (iw_get_range_info(skfd, ifname, &(info->range)) >= 0)
		info->has_range = 1;
	
	if (iw_get_ext(skfd, ifname, SIOCGIWAP, &wrq) >= 0) {
		info->has_ap_addr = 1;
		memcpy(&info->ap_addr, &wrq.u.ap_addr, sizeof(sockaddr));
	}

	if (iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0) {
		info->has_bitrate = 1;
		memcpy(&info->bitrate, &wrq.u.bitrate, sizeof(iwparam));
	}

	wrq.u.power.flags = 0;
	if (iw_get_ext(skfd, ifname, SIOCGIWPOWER, &wrq) >= 0) {
		info->has_power = 1;
		memcpy(&info->power, &wrq.u.power, sizeof(iwparam));
	}

	if (iw_get_stats(skfd, ifname, &info->stats, &info->range, info->has_range) >= 0)
		info->has_stats = 1;
	
	return 0;
}

void
detect_interfaces()
{
	FILE *fh;
	char buf[512];

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		perror("cannot open");
		exit(EXIT_FAILURE);
	}
	fgets(buf, sizeof buf, fh);
	fgets(buf, sizeof buf, fh);

	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[IFNAMSIZ];

		s = get_name(name, buf);
		add_interface(name);

		/* TODO: consoldate: */
		int skfd = iw_sockets_open();
		get_info(skfd, name);
		//get_dev_fields(s, ife);
		//ife->statistics_valid = 1;
	}
	if (ferror(fh))
		perror(_PATH_PROCNET_DEV);

	fclose(fh);
}

const struct interface *
get_interfaces()
{
	if (!head)
		detect_interfaces();
	return head;
}
