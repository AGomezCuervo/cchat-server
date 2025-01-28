#include "libxml/tree.h"
#include <sys/socket.h>
#include <stdarg.h>
#include <stdbool.h>

#define MAX_VALUE 256

struct initial_stream  {
  char from[MAX_VALUE];
  char to[MAX_VALUE];
  char version[MAX_VALUE];
  char xml_lang[MAX_VALUE];
  char xmlns[MAX_VALUE];
  char xmlns_stream[MAX_VALUE];
};


int parse_stream_attributes (xmlNode *node, struct initial_stream *stream_attr);
void err_doit(bool errnoflag, int level, const char *fmt, va_list ap);
void err_sys(const char *fmt, ...);
void err_buf(const char*fmt, ...);

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Socket(int domain, int type, int protocol);
int Listen(int fd, int backlog);
