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

#ifndef __DB_API_H__
#define __DB_API_H__
#include <curl/curl.h>
#include <json-c/json.h>
#include <stdbool.h>
#include "dropbox_urls.h"
#include "librest/rest.h"

FILE * db_files_get(const char* path, const char* access_token);
json_object * db_files_put(const char* path, const char* access_token, FILE *   input_file);
json_object * db_metadata (const char* path, const char* access_token, bool list);
json_object * db_delta    (char* cursor, const char* access_token);
json_object * db_longpoll (const char* cursor,int timeout);
const char * db_authorize_token (char* token, char * client_id, char* client_secret);
json_object * db_mkdir(const char * name, const char * access_token);
json_object * db_mv(const char * from, const char * to, const char * access_token);
json_object * db_rm(const char * name, const char * access_token);
#endif
