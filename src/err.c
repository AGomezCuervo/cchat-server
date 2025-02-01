#include "main.h"
#include <string.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

void err_doit(bool errnoflag, int level, const char *fmt, va_list ap)
{
  char buff[1024];
  vsnprintf(buff, sizeof(buff), fmt, ap);

  if(errnoflag)
    snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff) , ": %s", strerror(errno));

  syslog(level, "%s", buff);
  fprintf(stderr, "%s\n", buff);
}

void err_sys(const char *fmt, ...)
{
 va_list ap;
 va_start(ap, fmt);
 err_doit(true, LOG_ERR, fmt, ap );
 va_end(ap);
 exit(1);
}

void err_buf(const char *fmt, ...)
{
 va_list ap;
 va_start(ap, fmt);
 err_doit(false, LOG_ERR, fmt, ap );
 va_end(ap);
 exit(1);
}
