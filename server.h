#ifndef NMD_SERVER
#define NMD_SERVER

struct server;
typedef void (*server_cb)(int argc, char *argv[]);

struct server *server_open(const char *path);
void server_close(struct server *);

void server_loop(struct server *, server_cb cb);

#endif
