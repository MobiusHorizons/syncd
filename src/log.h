#ifndef __LOGGING_H__
#define __LOGGING_H__


typedef enum {
  Debug = 3,
  Info = 2,
  Error = 1,
} logging_level;

#define LOGARGS __FILE__,__LINE__

void logging_init(const char *filename);
void logging_log(const char * file, int line, const char *fmt, ...);
void logging_close();

typedef void (*LOGGING_LOG)(const char *,int,const char*,...);

#endif
