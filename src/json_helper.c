#include "json_helper.h"
#include <math.h>

/* Utilities */
json_object * json_copy(json_object ** to_ptr, const char * name, json_object * from, json_object * def) {
        json_object * obj;
        json_object * to = * to_ptr;
        if (json_object_object_get_ex(from, name, &obj)){
            json_object_object_add(to,name,obj);
        } else {
            json_object_object_add(to,name,def);
        }
        return to;
}

/* Getters */
const char * json_get_string(json_object * obj, const char * name){
    json_object * out;
    if (json_object_object_get_ex(obj,name,&out)){
        return json_object_get_string(out);
    }
    return NULL;
}

long long int json_get_int(json_object * obj, const char * name,long long int def){
    json_object * out;
    if (json_object_object_get_ex(obj,name,&out)){
        return json_object_get_int64(out);
    }
    return def;
}

bool json_get_bool(json_object * obj, const char * name, bool def){
  json_object * out;
  if (json_object_object_get_ex(obj,name,&out)){
    return json_object_get_boolean(out);
  }
  return def;
}

double json_get_double(json_object * obj, const char * name){
    json_object * out;
    if (json_object_object_get_ex(obj,name,&out)){
        return json_object_get_double(out);
    }
    return NAN;
}

void json_add_string(json_object * obj, const char * name, const char * val){
  json_object * jval = (json_object *) 0;
  if (val != 0) jval = json_object_new_string(val);
  json_object_object_add(obj,name,jval);
}

void json_add_int(json_object * obj, const char * name, long long int val){
  json_object * jval = json_object_new_int64(val);
  json_object_object_add(obj,name,jval);
}

void json_add_bool(json_object * obj, const char * name, bool val){
  json_object * jval = json_object_new_boolean(val);
  json_object_object_add(obj,name,jval);
}

void json_add_double(json_object * obj, const char * name, double val){
  json_object * jval = json_object_new_double(val);
  json_object_object_add(obj,name,jval);
}

/*JSON_HELPER JSON(json_object * obj){
  JSON_HELPER jh;
  jh.type = obj->type;
  jh.data = (void * )obj;
  jh.parse = NULL;
  jh.get_int
}*/
