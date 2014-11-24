#ifndef __JSON_HELPER_H_
#define __JSON_HELPER_H_
#include <json-c/json.h>

struct JSON_HELPER;

typedef struct JSON_HELPER (*JSON_HELPER_GET)       (struct JSON_HELPER, ...);
typedef struct JSON_HELPER (*JSON_HELPER_STRINGIFY) (struct JSON_HELPER);
typedef struct JSON_HELPER (*JSON_HELPER_PRETTY)    (struct JSON_HELPER);
typedef struct JSON_HELPER (*JSON_HELPER_PARSE)     (const char *);
typedef struct JSON_HELPER (*JSON_HELPER_GET_INT)   (struct JSON_HELPER, int);
typedef struct JSON_HELPER (*JSON_HELPER_GET_FLOAT) (struct JSON_HELPER);
typedef struct JSON_HELPER (*JSON_HELPER_GET_STRING)(struct JSON_HELPER);

struct JSON_HELPER {
  int type;
  void * data;
  JSON_HELPER_GET         get;
  JSON_HELPER_GET_INT     get_int;
  JSON_HELPER_GET_STRING  get_string;
  JSON_HELPER_GET_FLOAT   get_float;
  JSON_HELPER_PRETTY      pretty;
  JSON_HELPER_STRINGIFY   stringify;
  JSON_HELPER_PARSE       parse;
};
typedef struct JSON_HELPER JSON_HELPER;

json_object * json_copy(json_object **, const char *, json_object *, json_object *);

const char * json_get_string(json_object * obj, char * name);
long long int json_get_int(json_object * obj, char * name, long long int def);
double json_get_double(json_object * obj, char * name);

#endif
