#ifndef NMD_EPOLL
#define NMD_EPOLL

#include "server.h"

struct polling *epoll_get(void (*event)(int fd));

#endif
