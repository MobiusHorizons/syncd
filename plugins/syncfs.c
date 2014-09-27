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
#include <sys/stat.h>
#include "../cache.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN	(1024 * (EVENT_SIZE + 16))
#define PLUGIN_PREFIX "fs://"
#define PLUGIN_PREFIX_LEN 5

FILE ** open_files;
int num_open_files;
int first_open_files;
utilities utils;

static int inotify_fd;
static char* watchpoints[255]; // should use dynamic memory, but oh well
static int num_watchpoints;

char* init(utilities);
void add_watch(char *);
void watch_dir(char *);
void sync_listen(int(*)(char*,int));

void update_file_cache(char * filename){
    struct stat details;
    json_object * cache_entry = utils.getFileCache(PLUGIN_PREFIX,filename);
    if (cache_entry == NULL){
        cache_entry = json_object_new_object();
    }
    if (stat(filename, &details) == -1){
        printf("cannot stat '%s'\n",filename);
    } else {
        printf("size = %d; mtime= %d\n",details.st_size, details.st_mtime);
        json_object_object_add(cache_entry,"size", json_object_new_int64(details.st_size));
        json_object_object_add(cache_entry,"modified", json_object_new_int64((long long int)details.st_mtime));
    }
    utils.addCache(PLUGIN_PREFIX,filename,cache_entry);
}




char * init(utilities u){
    utils = u;
	num_watchpoints = 0;
	num_open_files = 0;
	num_open_files = 0;
	inotify_fd = inotify_init();
	if (inotify_fd < 0){
		perror("inotify_init");
	}
	return PLUGIN_PREFIX;
	
}

void sync_listen(int (*cb)( char*,int)){
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
				char * fp  = (char * ) malloc(PLUGIN_PREFIX_LEN + event->len + strlen(watchpoints[event->wd])+1);
				sprintf(fp,"%s%s/%s",PLUGIN_PREFIX,watchpoints[event->wd],event->name);
                char * filename = fp + PLUGIN_PREFIX_LEN;
                update_file_cache(filename);
				if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)){ // new directory created
					printf("%.4x = %.4x\n",event->mask, (IN_CREATE|IN_ISDIR));
					add_watch(fp + PLUGIN_PREFIX_LEN);
				}
				if (cb != NULL) cb(fp,event->mask);
				free(fp);
			}
			i += EVENT_SIZE + event->len;
		}
	}
}

void add_watch(char * dir){
	int wp  = inotify_add_watch( inotify_fd, dir, IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVE);
	printf("adding directory: %s, wp = %d\n",dir,wp);
	char * dir_local = malloc(strlen(dir)+1);
	strcpy(dir_local,dir);
	watchpoints[wp] = dir_local;
	num_watchpoints = wp;
}

void watch_dir_recurse(char * dir_name){
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
		    watch_dir_recurse (path);
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

void watch_dir (char * dir_name)
{
	dir_name += PLUGIN_PREFIX_LEN;
	watch_dir_recurse(dir_name);
}



/*int read_file (FILE *fp, char **buf) 
{
  int n, np,r;
  char *b, *b2;

  n = 1024;
  np = n;
  b = malloc(sizeof(char)*n);
  while ((r = fread(b, sizeof(char), 1024, fp)) > 0) {
    n += r;
    if (np - n < 1024) { 
      np *= 2;                      // buffer is too small, the next read could overflow!
      b = realloc(b,np*sizeof(char));
    }
  }
  *buf = b;
  printf("read %d bytes\n",n);
  return n - 1024;
}*/


FILE * sync_open (char * path){
	path += PLUGIN_PREFIX_LEN;
	printf("opening %s\n",path);
	return fopen(path, "rb");
/*	FILE * fp = fopen(path,specifier);
	if (fp == NULL) return -1;
	if (first_open_files < num_open_files){
		int f = first_open_files++;
		open_files[f] = fp;
		while (first_open_files < num_open_files){
			if (open_files[first_open_files] == NULL) break;
			first_open_files ++;
		}
		return f;
	} else {
		open_files = realloc(open_files, sizeof(FILE*) * (num_open_files+1));
		open_files[num_open_files++] = fp;
		first_open_files = num_open_files;
		return num_open_files-1;
	}*/
}

void sync_close ( FILE * fd ){
	fclose(fd);
	/*if (id < num_open_files){
		fclose(open_files[id]);
		open_files[id] = NULL;
		if (first_open_files > id){
			if (first_open_files == num_open_files -1 && first_open_files != id)
				open_files = realloc(open_files, sizeof(FILE*) * --num_open_files);
			first_open_files = id;
		}
	}*/
}

/*int sync_read (int id, char * buffer, int len){
	if (id >= num_open_files) return 0;
	FILE* fp = open_files[id];
	printf("retrieving fp#%d, it is %p; len = %d\n",id,fp,len);
	if (fp != NULL){
		return fread(buffer,sizeof(char),len,fp);
	} else {
		return 0;
	}
}*/
 
int mkpath(char* file_path, mode_t mode) {
  char* p;
  for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
    *p='\0';
    if (mkdir(file_path, mode)==-1) {
      if (errno!=EEXIST) { *p='/'; return -1; }
    }
    *p='/';
  }
  return 0;
}
 
int sync_write(char * path, FILE * fo){
	path += PLUGIN_PREFIX_LEN;
        mkpath(path,0777);// make directory if it doesn't exist yet
	FILE * fd = fopen(path, "wb");	
	char data[1024];
	int out=0, in, t=0;
	if (fd != NULL && fo != NULL){
		while ( (in = fread(data,sizeof(char),1024,fo) ) > 0){
			t+= in;
			printf("\rread %d bytes",t); fflush(stdout);
			out += fwrite(data,sizeof(char),in,fd);
		}
		printf("\n");
		fclose(fd);
		return out;
	} else {
		printf("error opening %s\n",path);
		return -1;
	}
}

int sync_mkdir(char * path){
	path += PLUGIN_PREFIX_LEN;
	printf("mkdir %s\n",path);
	return mkpath(path,0777);
}

int sync_rm(char * path){
	path += PLUGIN_PREFIX_LEN;
	printf("trying to delete %s\n",path);
	return remove(path);
}

int sync_mv(char*from,char*to){
	from += PLUGIN_PREFIX_LEN;
	to += PLUGIN_PREFIX_LEN;
	printf("trying to move %s to %s\n",from,to);
	return rename(from,to);
}

/*int main ()
{
    init();
    watch_dir ("/home/paul/.cache");
    listen(cb);
    return 0;
}*/
