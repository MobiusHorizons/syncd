#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
/* "readdir" etc. are defined here. */
#include <dirent.h>
/* limits.h defines "PATH_MAX". */
#include <limits.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN	(1024 * (EVENT_SIZE + 16))
static int inotify_fd;
static char* watchpoints[255]; // should use dynamic memory, but oh well
static int num_watchpoints;

void init();
void add_watch(char *);
void watch_dir(char *);
void listen(int(*)(char*,int));

void init(){
	num_watchpoints = 0;
	inotify_fd = inotify_init();
	if (inotify_fd < 0){
		perror("inotify_init");
	}
	
}

void listen(int (*cb)( char*,int)){
	int length;
	char buffer[EVENT_BUF_LEN];
	int i;
	while(1){
		i = 0;
		length = read(inotify_fd,buffer,EVENT_BUF_LEN);
		if (length < 0)
			perror("read");
		while( i < length){
			struct inotify_event * event = (struct inotify_event * ) &buffer[i];
			if (event->len){
//				printf(", file was %s/%s\n",watchpoints[event->wd],event->name);
				char * fp  = (char * ) malloc(event->len + strlen(watchpoints[event->wd])+1);
				sprintf(fp,"%s/%s",watchpoints[event->wd],event->name);
				if (event->mask & (IN_CREATE | IN_ISDIR)) // new directory created
					add_watch(fp);
				if (cb != NULL) cb(fp,event->mask);
				free(fp);
			}
			i += EVENT_SIZE + event->len;
		}
	}
}

void add_watch(char * dir){
	int wp  = inotify_add_watch( inotify_fd, dir, IN_CREATE | IN_DELETE | IN_CLOSE_WRITE |IN_MOVE);
	printf("adding directory: %s, wp = %d\n",dir,wp);
	watchpoints[wp] = dir;
	num_watchpoints = wp;
}

void watch_dir (char * dir_name)
{
	DIR * d;
	/* Open the directory specified by "dir_name". */
	d = opendir (dir_name);
	/* Check it was opened. */
	if (! d) {
	    fprintf (stderr, "Cannot open directory '%s': %s\n",
		     dir_name, strerror (errno));
	   return;
	   // exit (EXIT_FAILURE);
	}
	add_watch(dir_name);
	while (1) {
	    struct dirent * entry;
	    const char * d_name;
	    /* "Readdir" gets subsequent entries from "d". */
	    entry = readdir (d);
	    if (! entry) {
		/* There are no more entries in this directory, so break
		      out of the while loop. */
		break;
	    }
	    d_name = entry->d_name;
	    /* Print the name of the file and directory. */
	    //printf ("%s/%s\n", dir_name, d_name);
	    /* See if "entry" is a subdirectory of "d". */
	    if (entry->d_type & DT_DIR) {
		/* Check that the directory is not "d" or d's parent. */
		if (strcmp (d_name, "..") != 0 &&
		    strcmp (d_name, ".") != 0) {
		    int path_length;
		    char path[PATH_MAX];

		    path_length = snprintf (path, PATH_MAX,
					    "%s/%s", dir_name, d_name);
		    //printf ("%s\n", path);
		    if (path_length >= PATH_MAX) {
			fprintf (stderr, "Path length has got too long.\n");
			exit (EXIT_FAILURE);
		    }
		    /* Recursively call "list_dir" with the new path. */
		    watch_dir (path);
		}
	    }
	}
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}

int cb(char * path, int mask){
	printf("CALLBACK:got path '%s', mask: %d\n",path,mask);
	return 1;
}
/*int main ()
{
    init();
    watch_dir ("/home/paul/.cache");
    listen(cb);
    return 0;
}*/
