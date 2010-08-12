#include "interfaces.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <net/if.h>

#define _PATH_PROCNET_DEV "/proc/net/dev"

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
