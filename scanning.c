#include "scanning.h"

#include <stdlib.h>
#include <stdio.h>
#include <iwlib.h>

struct wi_network *head = NULL, *tail = NULL;

/*struct iw_networks {
};*/

struct wi_scanning_data {
	struct stream_descr stream;
	struct iw_event event;

	struct iw_range *iw_range;
	int has_range;
};

////////////////////////////////////////////////////////////////////////////////

void
add_network(struct wi_network *node)
{
	if (head == NULL) {
		head = node;
		node->prev = NULL;
	} else {
		tail->next = node;
		node->prev = tail;
	}

	tail = node;
	node->next = NULL;
}

struct wi_network *
collect_scanning_token(struct wi_network *node, struct wi_scanning_data *d)
{
	char buffer[128];
	double freq;

	switch (d->event.cmd) {
		case SIOCGIWAP:
			if (node)
				add_network(node);
			node = malloc(sizeof(struct wi_network));

			iw_saether_ntop(&d->event.u.ap_addr, buffer);
			node->bssid = strdup(buffer);
			break;
		case SIOCGIWNWID:
			break;
		case SIOCGIWFREQ:
			freq = iw_freq2float(&d->event.u.freq);
			if (d->has_range) {
				node->channel = iw_freq_to_channel(freq, d->iw_range);
				iw_print_freq(buffer, sizeof buffer, freq, node->channel, d->event.u.freq.flags);
				node->freq = strdup(buffer);
			}
		case SIOCGIWMODE:
			break;
		case SIOCGIWNAME:
			break;
		case SIOCGIWESSID:
			memset(node->essid, '\0', sizeof(node->essid));
			if (d->event.u.essid.pointer && d->event.u.essid.length)
				memcpy(node->essid, d->event.u.essid.pointer, d->event.u.essid.length);
			if (d->event.u.essid.flags) {
				/* TODO: complete */
			} else {
				/* TODO: hidden */
			}
			break;
		case SIOCGIWENCODE:
			break;
		case SIOCGIWRATE:
			break;
		case SIOCGIWMODUL:
			break;
		case IWEVQUAL:
			break;
		case IWEVCUSTOM:
			break;
		default:
			break;
	}
	return node;
}

const struct wi_network *
scan(const char *ifname)
{
	struct iwreq wrq;
	struct iw_range range;
	unsigned char *buffer = NULL;
	unsigned char *newbuf;
	int buflen = IW_SCAN_MAX_DATA;
	int has_range;
	int wait = 100000;
	int timeout = 15000000;

	int skfd = iw_sockets_open();
	has_range = iw_get_range_info(skfd, ifname, &range) >= 0;

	if(!has_range || range.we_version_compiled < 14) {
		fprintf(stderr, "Interface doesn't support scanning.\n");
		exit(EXIT_FAILURE);
	}

	wrq.u.data.pointer = NULL;
	wrq.u.data.flags = 0;
	wrq.u.data.length = 0;

	if (iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0) {
		fprintf(stderr, "Interface doesn't support scanning.\n");
		exit(EXIT_FAILURE);
	}

realloc:
	newbuf = realloc(buffer, buflen);
	if (newbuf == NULL) {
		if (buffer)
			free(buffer);
		perror("realloc");
		exit(EXIT_FAILURE);
	}
	buffer = newbuf;

	wrq.u.data.pointer = buffer;
	wrq.u.data.flags = 0;
	wrq.u.data.length = buflen;

iw_get_ext:
	if (iw_get_ext(skfd, ifname, SIOCGIWSCAN, &wrq) < 0) {
		if (errno == EAGAIN) {
			timeout -= wait;
			if (timeout > 0) {
				usleep(wait);
				goto iw_get_ext;
			}
		}
		else if (errno == E2BIG && range.we_version_compiled > 16) {
			if (wrq.u.data.length > buflen)
				buflen = wrq.u.data.length;
			else
				buflen *= 2;
			goto realloc;
		}

		free(buffer);
		perror("iw_get_ext");
		exit(EXIT_FAILURE);
	}

	if (wrq.u.data.length) {
		struct wi_scanning_data data = {
			.iw_range = &range,
			.has_range = has_range
		};
		struct wi_network *node = NULL;
		int ret;

		iw_init_event_stream(&data.stream, (char *)buffer, wrq.u.data.length);
		while ((ret = iw_extract_event_stream(&data.stream, &data.event, range.we_version_compiled)) > 0) {
			node = collect_scanning_token(node, &data);
		}
		add_network(node);
	}

	free(buffer);
	return head;
}
