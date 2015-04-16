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

/* cldoc:begin-category(dropbox_api) */
#ifndef __DB_API_H__
#define __DB_API_H__
#include <curl/curl.h>
#include <json-c/json.h>
#include <stdbool.h>
#include "dropbox_urls.h"
#include "librest/rest.h"

/* Download a file from DropBox
 * @path the full path to the file to download.
 * @access_token the access token used for authorization.
 *
 * @return a C style `FILE *`, which is a pipe from which you can read the data of the file.
 */
FILE * db_files_get(char* path, const char* access_token);

/* Upload a file to DropBox
 * @path the full path to the file to download.
 * @access_token the access token used for authorization.
 * @input_file a C style `FILE *` which will be uploaded.
 *
 * @return a `json_object *` containing metadata
 */
json_object * db_files_put(char* path, const char* access_token, FILE *   input_file);

/* Get the metadata of a file stored on DropBox.
 * @path the full path to the file to download.
 * @access_token the access token used for authorization.
 * @list a `bool` value specifying whether to return metadata on files inside the directorf if `path` is a directory
 *
 * @return a `json_object *` containing the file/folder metadata
 */
json_object * db_metadata (char* path, const char* access_token, bool list);

/* Get the metadata of a file stored on DropBox.
 * @cursor a string which can be NULL specifying a position in the DropBox change stream.
 * @access_token the access token used for authorization.
 *
 * @return a `json_object *` containing the change list since `cursor`
 */
json_object * db_delta    (char* cursor, const char* access_token);

/* Get the metadata of a file stored on DropBox.
 * @cursor a string which can be NULL specifying a position in the DropBox change stream.
 * @timeout the ammount of time to wait for changes.
 *
 * @return a `json_object *` containing the property changes which will be either true or false.
 */
json_object * db_longpoll (char* cursor,int timeout);

/* Get the metadata of a file stored on DropBox.
 * @cursor a string which can be NULL specifying a position in the DropBox change stream.
 * @access_token the access token used for authorization.
 *
 * @return a `json_object *` containing the change list since `cursor`
 */
const char * db_authorize_token (char* token, char * client_id, char* client_secret);

/* Create a new directory
 * @name the full path to the new directory.
 * @access_token the access token used for authorization.
 *
 * @returns the metadata of the new directory
 *
 * This function acts like `mkdir -p` it will create the directory and any
 * intermediary derectories if they do not exist
 */
json_object * db_mkdir(char * name, const char * access_token);

/* Move or rename a file or directory
 * @from the full path to the source.
 * @to the full path to the destination.
 * @access_token the access token used for authorization.
 *
 * @returns the metadata of the file/directory moved/renamed
 *
 * This function acts like `mv`
 */
json_object * db_mv(char * from,char * to, const char * access_token);

/* Delete a file/directory
 * @name the full path to the new directory.
 * @access_token the access token used for authorization.
 *
 * @returns the metadata of the deleted file/directory
 *
 * This function acts like `rm`
 */
json_object * db_rm(char * name, const char * access_token);

/* cldoc:end-category(dropbox_api) */
 #endif

