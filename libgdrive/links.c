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

/*******************
 * links.c -- This is a cross platform file link library.
 *    * Linux   -- Create .desktop entries
 *    * Windows -- Create .url entries
*/

#include "links.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#define DESKTOP_FILE_FORMAT "[Desktop Entry]\n\
Encoding=UTF-8\n\
Name=%s\n\
Type=Link\n\
URL=%s"

#define  URL_FILE_FORMAT "[InternetShortcut]\n\
URL=%s\n\
IconFile=%s\n"


#define LINK_FORMAT DESKTOP
FILE * getLink(const char * title, const char * url, const char * icon){
    //char * file = malloc(DESKTOP_FILE_LEN + strlen(title) + strlen(url));
    //sprintf(file, DESKTOP_FILE_FORMAT, title, url);
#ifdef LINK_TYPE_URL
    const char * format = URL_FILE_FORMAT;
#endif
#ifdef LINK_TYPE_DESKTOP
	const char * format = DESKTOP_FILE_FORMAT;
#endif
    FILE * file = tmpfile();
    fprintf(file, format, url, icon);
    printf(format, title, url);
	rewind(file);
    return file;
}
