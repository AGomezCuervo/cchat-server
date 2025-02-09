#ifndef MAIN_H_
#define MAIN_H_

#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/un.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_VALUE 256
#define BUFF_LEN 1024
#define SERVER_ADDR "home.com"
#define MAX_CLIENTS 200

typedef void Sigfunc(int);

struct stream_conf  {
  char from[MAX_VALUE];
  char to[MAX_VALUE];
  char id[MAX_VALUE];
  char xmlns[MAX_VALUE];
  char xmlns_stream[MAX_VALUE];
  char version[16];
  char xml_lang[16];
};

void sig_child(int sig);
void sig_exit(int sig);

void err_doit(bool errnoflag, int level, const char *fmt, va_list ap);
void err_sys(const char *fmt, ...);
void err_buf(const char*fmt, ...);

int Send(int fd, char *buf, int len);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Socket(int domain, int type, int protocol);
int Listen(int fd, int backlog);
int EpollCreate(int flag);
int Accept(int fd, struct sockaddr *addr, socklen_t *len);
int Close(int fd);
Sigfunc *Signal(int sig, Sigfunc *func);
#endif
