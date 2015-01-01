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
#include <string.h>

typedef struct{
	char * data;
	size_t size;
} buffer;

buffer buffer_init(buffer b, size_t size){
/*	if (b.data != NULL && b.size != 0){
		free(b.data);
	}*/
	b.data = (char*)malloc(size);
	b.size = size;
	return b;
}

buffer buffer_append(buffer b, char* data, size_t length){
	size_t newLength = b.size + length;
	b.data = (char*) realloc(b.data, newLength+1);
	//printf("appending %d bytes ...",length);
	//fflush(stdout);
	memcpy(b.data + b.size,data, length);
	//printf("DONE\n");
	b.size = newLength;
	b.data[newLength] = '\0';
	return b;
}

buffer buffer_free(buffer b){
	free(b.data);
	b.data = NULL;
	b.size = 0;
	return b;
}

buffer buffer_clone(buffer b){
	buffer n;
	n = buffer_init(n,b.size);
	memcpy(n.data,b.data,b.size);
	return n;
}
#endif
