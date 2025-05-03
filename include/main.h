#ifndef MAIN_H
#define MAIN_H

#include "arena.h"
#include <unistd.h>
#include <stdint.h>
#include <expat.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/un.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_VALUE 128
#define MAX_BODY 4096
#define SERVER_ADDR "home.com"
#define MAX_CLIENTS 200

typedef void Sigfunc(int);

enum tag_type {
	NONE,
	STREAM,
	MESSAGE,
	BODY,
	ERROR,
};

struct XMPP_Message {
	char from[MAX_VALUE];
	char to[MAX_VALUE];
	char id[MAX_VALUE];
	char msg_type[MAX_VALUE];
	enum tag_type tag_type;
	char body[MAX_BODY];
	int body_len;
	int depth;
};

struct XMPP_Client  {
	char from[MAX_VALUE];
	char to[MAX_VALUE];
	char id[MAX_VALUE];
	size_t buf_len;
	int fd;
	bool status;
	Arena *arena;
	XML_Parser parser;
	struct XMPP_Message message;
};

void sig_exit(int sig);

void err_doit(bool errnoflag, int level, const char *fmt, va_list ap);
void err_sys(const char *fmt, ...);
void err_buf(const char*fmt, ...);
void err_conn(const char *fmt, ...);
void send_error(int fd, const char *ms);

int Send(int fd, char *buf, int len);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Socket(int domain, int type, int protocol);
int Listen(int fd, int backlog);
int EpollCreate(int flag);
int Epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

void *Calloc(size_t nmemb, size_t size);
int Accept(int fd, struct sockaddr *addr, socklen_t *len);
int Close(int fd);
Sigfunc *Signal(int sig, Sigfunc *func);
#endif
