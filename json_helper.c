#include "json_helper.h"
#include <math.h>

const char * json_get_string(json_object * obj, char * name){
    json_object * out;
    if (json_object_object_get_ex(obj,name,&out)){
        return json_object_get_string(out);
    }
    return NULL;
}

long long int json_get_int(json_object * obj, char * name,long long int def){
    json_object * out;
    if (json_object_object_get_ex(obj,name,&out)){
        return json_object_get_int64(out);
    }
    return def;
}

double json_get_double(json_object * obj, char * name){
    json_object * out;
    if (json_object_object_get_ex(obj,name,&out)){
        return json_object_get_double(out);
    }
    return NAN;
}

