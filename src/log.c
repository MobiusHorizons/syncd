#include "log.h"
#include <stdio.h>
#include <stdarg.h>


FILE *__logfile;

void logging_init(const char *filename)
{
	__logfile = fopen(filename, "a");
}

void logging_log(const char *fmt, ...)
{
	fprintf(__logfile, "LOG  %s:%d -- ", __FILE__, __LINE__);
	va_list args;
	va_start(args, fmt);
	vfprintf(__logfile, fmt, args);
	va_end(args);
}

void logging_close()
{
	fclose(__logfile);
}
