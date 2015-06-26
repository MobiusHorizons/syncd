#include "log.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>


FILE *__logfile;
FILE *__stderr;
FILE *__stdout;

int __stderr_fd;
int __stdout_fd;


void logging_init(const char *filename)
{

	// backup stdout and stderr
	__stderr_fd = dup(fileno(stderr));
	__stdout_fd = dup(fileno(stdout));

	__stdout = fdopen(__stdout_fd, "a");
	__stderr = fdopen(__stderr_fd, "a");

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
	//vdprintf (__stdout_fd, fmt, args);
	vfprintf (__stdout, fmt, args);
	fflush   (__stdout);
	va_end(args);
}

void logging_stderr(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	//vdprintf (__stdout_fd, fmt, args);
	vfprintf (__stderr, fmt, args);
	fflush   (__stderr);
	va_end(args);
}

void logging_close()
{

	dup2(__stdout_fd, fileno(stdout));
	dup2(__stderr_fd, fileno(stderr));

	fclose(__stderr);
	fclose(__stdout);
}
