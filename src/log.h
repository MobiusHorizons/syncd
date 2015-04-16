#ifndef __LOGGING_H__
#define __LOGGING_H__

void logging_init(const char *filename);
void logging_log(const char *fmt, ...);
void logging_close();

#endif 
