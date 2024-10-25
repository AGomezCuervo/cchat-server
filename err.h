#include <stdarg.h>
#include <stdbool.h>
void err_doit(bool errnoflag, int level, const char *fmt, va_list ap);
void err_sys(const char *fmt, ...);
