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

#ifndef __BUFFER_H_
#define __BUFFER_H_
#include <stdlib.h>
#ifdef WIN32
#define EXPORT_SHARED __declspec(dllexport)
#else
#define EXPORT_SHARED
#endif
typedef struct{
	char * data;
	size_t size;
} buffer;

buffer EXPORT_SHARED buffer_init(size_t size);
buffer EXPORT_SHARED buffer_append(buffer b, char* data, size_t length);
buffer EXPORT_SHARED buffer_clone(buffer b);
int    EXPORT_SHARED buffer_read(buffer *b, char *data, size_t length);
buffer EXPORT_SHARED buffer_from_string(char * string);
buffer EXPORT_SHARED buffer_free(buffer b);
#endif
