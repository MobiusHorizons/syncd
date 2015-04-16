#include "log.h"
#include <stdio.h>
#include <stdarg.h>


FILE *__logfile;

void logging_init(const char *filename)
{
	__logfile = fopen(filename, "a");
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

void logging_close()
{
	fclose(__logfile);
}
