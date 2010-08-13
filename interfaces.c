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
	node->wi = NULL;

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
get_info(int skfd, struct interface *iface)
{
	struct wireless_config b;

	if (iw_get_basic_config(skfd, iface->name, &b) < 0) {
		struct ifreq ifr;
		strncpy(ifr.ifr_name, iface->name, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
			return -ENODEV;
		else
			return -ENOTSUP;
	}

	struct iwreq wrq;
	iface->wi = malloc(sizeof(struct wi_info));

	/* basic info here */
	if ((iface->wi->has_essid = b.has_essid))
		strncpy(iface->wi->essid, b.essid, IW_ESSID_MAX_SIZE + 1);

	/* extended info here */
	if (iw_get_range_info(skfd, iface->name, &iface->wi->range) >= 0)
		iface->wi->has_range = true;
	
	if (iw_get_ext(skfd, iface->name, SIOCGIWAP, &wrq) >= 0) {
		iface->wi->has_ap_addr = true;
		memcpy(&iface->wi->ap_addr, &wrq.u.ap_addr, sizeof(sockaddr));
	}

	if (iw_get_ext(skfd, iface->name, SIOCGIWRATE, &wrq) >= 0) {
		iface->wi->has_bitrate = true;
		memcpy(&iface->wi->bitrate, &wrq.u.bitrate, sizeof(iwparam));
	}

	wrq.u.power.flags = 0;
	if (iw_get_ext(skfd, iface->name, SIOCGIWPOWER, &wrq) >= 0) {
		iface->wi->has_power = true;
		memcpy(&iface->wi->power, &wrq.u.power, sizeof(iwparam));
	}

	if (iw_get_stats(skfd, iface->name, &iface->wi->stats, &iface->wi->range, iface->wi->has_range) >= 0)
		iface->wi->has_stats = true;

	wrq.u.essid.pointer = (caddr_t)iface->wi->essid;
	wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
	wrq.u.essid.flags = 0;
	if(iw_get_ext(skfd, iface->name, SIOCGIWNICKN, &wrq) >= 0)
		if(wrq.u.data.length > 1)
			iface->wi->has_essid = 1;
	
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
		struct interface *iface = add_interface(name);

		/* TODO: consoldate: */
		int skfd = iw_sockets_open();
		get_info(skfd, iface);
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
