#include "log.h"
#include <unistd.h>
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>


FILE *__logfile;
int __stderr;
int __stdout;

void logging_init(const char *filename)
{

	// backup stdout and stderr
	__stderr = dup(fileno(stderr));
	__stdout = dup(fileno(stdout));

	// open the logfile under the filedescriptor of stdout
	__logfile = freopen(filename, "a", stdout);
	dup2(fileno(stdout), fileno(stderr));
}

void logging_log(const char * file, int line, const char *fmt, ...)
{
	int header_len;
	fprintf(__logfile, "LOG  %s:%d%n",file,line,&header_len);
	fprintf(__logfile, "%*s -- ", (header_len > 20) ? 0 : (20 - header_len), " ");
	va_list args;
	va_start(args, fmt);
	vfprintf(__logfile, fmt, args);
	fflush(__logfile);
	va_end(args);
}

void logging_stdout(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vdprintf (__stdout, fmt, args);
	va_end(args);
}

void logging_stderr(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vdprintf (__stdout, fmt, args);
	va_end(args);
}

void logging_close()
{

	dup2(__stdout, fileno(stdout));
	dup2(__stderr, fileno(stderr));

	close(__stderr);
	close(__stdout);
}
