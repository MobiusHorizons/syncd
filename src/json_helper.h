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
