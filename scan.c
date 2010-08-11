#include <stdlib.h>
#include <stdio.h>
#include <iwlib.h>

struct iw_mngr {
	int skfd;
	char *ifname;
};

struct iw_network {
	struct iw_network *next;

	int cell;
	char *bssid;
	int channel;
	char *freq;
	char essid[IW_ESSID_MAX_SIZE + 1];
};

struct iw_networks {
	struct iw_network *root, *tail;
};

/*struct iw_networks {
};*/

struct iw_scanning_data {
	struct stream_descr stream;
	struct iw_event event;

	struct iw_range *iw_range;
	int has_range;
};

////////////////////////////////////////////////////////////////////////////////

int
print_info(int skfd, char *ifname, char *args[], int argc)
{
	struct wireless_info info;
	int rc;

	rc = get_info(skfd, ifname, &info);
	switch (rc) {
		case 0:
			fprintf("%s: %s", ifname, info->b.name);
			//mngr->ifname = strdup(dev);
			display_info(&info, ifname);
			break;
		case -ENOTSUP:
			fprintf(stderr, "%s -> no wireless externsion detected.\n", ifname);
			break;
		default:
			errno = -errno;
			perror("get_info");
			break;
	}
}

void
iw_mngr_init(struct iw_mngr *mngr, const char *dev)
{
	if((mngr->skfd = iw_sockets_open()) < 0)
	{
		perror("iw_sockets_open");
		exit(EXIT_FAILURE);
	}
	mngr->ifname = strdup(dev);

	iw_enum_devices(skfd, &print_info, NULL, 0);
}

////////////////////////////////////////////////////////////////////////////////

struct iw_network *
print_scanning_token(struct iw_network *node, struct iw_scanning_data *d)
{
	struct iw_network *newnode;
	char buffer[128];
	double freq;

	switch (d->event.cmd) {
		case SIOCGIWAP:
			newnode = malloc(sizeof(struct iw_network));
			if (node != NULL)
				newnode->next = node;
			else
				newnode->next = NULL;
			node = newnode;

			//node->cell = state->ap_num;
			iw_saether_ntop(&d->event.u.ap_addr, buffer);
			node->bssid = strdup(buffer);
			//++state->ap_num;
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

void
scanning(struct iw_mngr *mngr)
{
	struct iwreq wrq;
	struct iw_range range;
	unsigned char *buffer = NULL;
	unsigned char *newbuf;
	int buflen = IW_SCAN_MAX_DATA;
	int has_range;
	int wait = 100000;
	int timeout = 15000000;

	has_range = iw_get_range_info(mngr->skfd, mngr->ifname, &range) >= 0;

	if(!has_range || range.we_version_compiled < 14) {
		fprintf(stderr, "Interface doesn't support scanning.\n");
		exit(EXIT_FAILURE);
	}

	wrq.u.data.pointer = NULL;
	wrq.u.data.flags = 0;
	wrq.u.data.length = 0;

	if (iw_set_ext(mngr->skfd, mngr->ifname, SIOCSIWSCAN, &wrq) < 0) {
		fprintf(stderr, "Interface doesn't support scanning.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
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
		if (iw_get_ext(mngr->skfd, mngr->ifname, SIOCGIWSCAN, &wrq) < 0) {
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
		} else
			break;
	}

	if (wrq.u.data.length) {
		struct iw_scanning_data data = {
			.iw_range = &range,
			.has_range = has_range
		};
		struct iw_network *node = NULL;
		int ret;

		iw_init_event_stream(&data.stream, (char *)buffer, wrq.u.data.length);
		while ((ret = iw_extract_event_stream(&data.stream, &data.event, range.we_version_compiled)) > 0) {
			node = print_scanning_token(node, &data);
		}

		while (node) {
			printf("%s -> %s\n", node->bssid, node->essid);
			node = node->next;
		}
	}

	free(buffer);
}

////////////////////////////////////////////////////////////////////////////////

int
main(int argc, char *argv[])
{
	struct iw_mngr iw_mngr;

	//iw_mngr_init(&iw_mngr, "wlan0");
	iw_mngr_init(&iw_mngr);
	scanning(&iw_mngr);

	return 0;
}
