#include "main.h"
#include <unistd.h>

inline int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
        int n;
        if ( (n = bind(sockfd, addr, addrlen)) < 0)
                err_sys("bind error");

        return n;
}

inline int Socket(int domain, int type, int protocol)
{
        int server_fd = socket(domain, type, protocol);
        if (server_fd == -1)
                err_sys("Creating socket failed");

        return server_fd;
}

inline int Listen(int fd, int backlog )
{
        int n;
        if ( (n = listen(fd, backlog)) < 0)
        {
                close(fd);
                err_sys("Listen failed");
        }

        return n;

}

inline int Close(int fd)
{
        int n;
        if( (n = close(fd)) < 0)
        {
                err_sys("Close failed");
        }

        return n;
}

int Send(int fd, char *buf, int len)
{
    int total = 0;
    int bytesleft = len;
    int n;

    while(total < len) {
        n = send(fd, buf+total, bytesleft, 0);
        if(n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    return n==-1 ? -1 : total;
}
