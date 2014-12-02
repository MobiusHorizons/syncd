
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "cache.h"
#include <config.h>

#if !defined(HAVE_FORK)  
    #define fork() pseudo_fork(i,argv[0])
    int pseudo_fork(int plugin_num,char * exec_name){
	char pnum[10];
        sprintf(pnum,"%d",plugin_num);
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
#if defined(__APPLE__) && defined(__MACH__)
	int pseudo_fork(int plugin_num, char* exec_name){
		char pnum[10];
		sprintf(pnum,"%d",plugin_num);
		int pid = fork();
		if (pid != 0) return pid;
		execl(exec_name, exec_name, "-p", pnum, NULL);
		return 0;
	}
	#define fork() pseudo_fork(i,argv[0])
#endif
