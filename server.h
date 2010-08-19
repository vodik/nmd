#ifndef NMD_SERVER
#define NMD_SERVER

struct server;
typedef void (*server_cb)(int argc, char *argv[]);

struct polling {
	void (*add)(int fd);
	void (*rem)(int fd);
	void (*loop)();

	void (*event)(int fd);
};

struct server *server_open();
void server_close(struct server *);

void server_loop(struct server *);

#endif
