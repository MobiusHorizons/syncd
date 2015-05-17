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


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "cache.h"
//#include <config.h>
#define HAVE_FORK
#ifndef HAVE_FORK 
    #define fork() pseudo_fork(i,argv[0])
    int pseudo_fork(int plugin_num,char * exec_name){
		char pnum[5];
        sprintf(&pnum,"%d",plugin_num)
        printf("running '%s %s %s'\n",exec_name,"-p",pnum);
        int error = spawnl(P_NOWAIT,exec_name,exec_name,"-p",pnum);
	}
#endif
#if defined(WIN32)
	#include<windows.h>
	#include<malloc.h>

#else
	#include <alloca.h>
	#include <unistd.h>
#endif
