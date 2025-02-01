#include "main.h"
#include <wait.h>
#include <unistd.h>

extern int server_fd;

Sigfunc *signal(int sig, Sigfunc *func)
{
        struct sigaction act, oact;

        act.sa_handler = func;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

        if (sig != SIGALRM)
                act.sa_flags |= SA_RESTART;

        if (sigaction(sig, &act, &oact) < 0)
                return SIG_ERR;

        return oact.sa_handler;
}

Sigfunc *Signal(int sig, Sigfunc *func)
{
        Sigfunc *sigfunc;

        if ( (sigfunc = signal(sig, func)) == SIG_ERR)
                err_sys("signal error");

        return sigfunc;
}

void sig_child(int sig)
{
        pid_t pid;
        int status;

        while( (pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
                printf("child %d is terminated with signal %d\n", pid, sig);
        }
}


void sig_exit(int sig)
{
        printf("signal: %d\n", sig);
        if (sig == SIGINT)
        {
                printf("server_fd: %d\n", server_fd);
                printf("\nReceived SIGINT. Shutting down server....\n");
                Close(server_fd);
                exit(EXIT_SUCCESS);
        }
}
