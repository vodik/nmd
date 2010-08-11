#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#define _PATH_PROCNET_DEV "/proc/net/dev"

int skfd;

struct aftype {
    const char *name;
    const char *title;
    int af;
    int alen;
    /*char *(*print) (unsigned char *);
    char *(*sprint) (struct sockaddr *, int numeric);
    int (*input) (int type, char *bufp, struct sockaddr *);
    void (*herror) (char *text);
    int (*rprint) (int options);
    int (*rinput) (int typ, int ext, char **argv);*/

    /* may modify src */
    //int (*getmask) (char *src, struct sockaddr * mask, char *name);

    int fd;
    char *flag_file;
};

struct aftype unspec_aftype = {
	.name = "unspec",
	.title = "UNSPEC",
	.af = AF_UNSPEC,
	.alen = 0,
	.fd = -1,
	.flag_file = NULL,
};

struct aftype unix_aftype = {
	.name = "unix",
	.title = "UNIX Domain",
	.af = AF_UNIX,
	.alen = sizeof(unsigned long),
	.fd = -1,
	.flag_file = "/proc/net/unix"
};

struct aftype inet_aftype = {
	.name = "inet",
	.title = "DARPA Internet",
	.af = AF_INET,
	.alen = sizeof(unsigned long),
	.fd = -1,
	.flag_file = NULL,
};

struct aftype inet6_aftype = {
	.name = "inet6",
	.title = "IPv6",
	.af = AF_INET6,
	.alen = sizeof(struct in6_addr),
	.fd = -1,
	.flag_file = "/proc/net/if_inet6",
};

struct aftype *aftypes[] = {
	&unix_aftype,
	&inet_aftype,
	&inet6_aftype,
	&unspec_aftype,
	NULL
};

/*static int if_print(char *ifname)
{
	int res;

	if (!ifname) {
		int all = 1;
		res = for_all_interfaces(do_if_print, &all);
	} else {
		struct interface *ife;

		ife = lookup_interface(ifname);
		res = do_if_fetch(ife);
		if (res >= 0)
			ife_print(ife);
	}
	return res;
}*/

static char *get_name(char *name, char *p)
{
	while (isspace(*p))
		++p;

	while (*p) {
		if (isspace(*p))
			break;

		if (*p == ':') {	/* could be an alias */
			char *dot = p, *dotname = name;
			*name++ = *p++;
			while (isdigit(*p))
				*name++ = *p++;
			if (*p != ':') {	/* it wasn't, backup */
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

int
lookup2()
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
		//ife = add_interface(name);
		printf("found: %s\n", name);
		//get_dev_fields(s, ife);
		//ife->statistics_valid = 1;
		/*if (target && !strcmp(target,name))
			break;*/
	}
	if (ferror(fh))
		perror(_PATH_PROCNET_DEV);

	fclose(fh);
}

int
sockets_open(int family)
{
	struct aftype **aft;
	int sfd = -1;
	static int force = -1;

	if (force < 0) {
		force = 0;
		if (access("/proc/net", R_OK))
			force = 1;
	}

	for (aft = aftypes; *aft; ++aft) {
		struct aftype *af = *aft;
		int type = SOCK_DGRAM;

		printf("%s\n", af->title);

		if (af->af == AF_UNSPEC)
			continue;
		if (family && family != af->af)
			continue;
		if (af->fd != -1) {
			sfd = af->fd;
			continue;
		}
		af->fd = socket(af->af, type, 0);
		if (af->fd >= 0) {
			sfd = af->fd;
			printf("sfd: %d\n", sfd);
		}
		else
			fprintf(stderr, "fuck?\n");
	}
	if (sfd < 0)
		perror("fuck!");
	return sfd;
}

int get_socket_for_af(int af)
{
	struct aftype **afp;

	afp = aftypes;
	while (*afp != NULL) {
		printf("^^%s\n", (*afp)->name);
		if ((*afp)->af == af)
			return (*afp)->fd;
		afp++;
	}
	return -1;
}

void
lookup()
{
	int numreqs = 30;
	struct ifconf ifc;
	struct ifreq *ifr;
	int n;
	int len;
	int skfd;

	skfd = sockets_open(0);
	skfd = get_socket_for_af(AF_INET);
	if (skfd < 0) {
		perror("warning: no inet socket available");
		skfd = sockets_open(0);
	}
	//skfd = sockets_open(0);
	if (skfd < 0)
		exit(EXIT_FAILURE);

	ifc.ifc_buf = NULL;
realloc:
	len = ifc.ifc_len = sizeof(struct ifreq) * numreqs;
	ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len);

	if (ioctl(skfd, SIOCGIFCONF, &ifc) < 0) {
		perror("ioctl SIOCGIFCONF");
		exit(EXIT_FAILURE);
	}
	if (ifc.ifc_len == len) {
		numreqs += 10;
		goto realloc;
	}

	ifr = ifc.ifc_req;
	for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {
		printf("found: %s\n", ifr->ifr_name);
		++ifr;
	}

	free(ifc.ifc_buf);
}

int
main(int argc, char *argv[])
{
	/*if ((skfd = sockets_open(0)) < 0) {
		perror("sockets_open");
		exit(EXIT_FAILURE);
	}

	if (argc == 2)
		return if_print(argv[1]);
	else
		return if_print(NULL);*/
	lookup2();
}
