/* Copyright (c) 2014 Paul Martin & Brian Cole
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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

/*JSON_HELPER JSON(json_object * obj){
  JSON_HELPER jh;
  jh.type = obj->type;
  jh.data = (void * )obj;
  jh.parse = NULL;
  jh.get_int
}*/
