#ifndef __JSON_HELPER_H_
#define __JSON_HELPER_H_
#include <json-c/json.h>

const char * json_get_string(json_object * obj, char * name);
long long int json_get_int(json_object * obj, char * name, long long int def);
double json_get_double(json_object * obj, char * name);

#endif
