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

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN	(1024 * (EVENT_SIZE + 16))
FILE ** open_files;
int num_open_files;
int first_open_files;

static int inotify_fd;
static char* watchpoints[255]; // should use dynamic memory, but oh well
static int num_watchpoints;

void init();
void add_watch(char *);
void watch_dir(char *);
void listen(int(*)(char*,int));

void init(){
	num_watchpoints = 0;
	num_open_files = 0;
	num_open_files = 0;
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
				char * fp  = (char * ) malloc(event->len + strlen(watchpoints[event->wd])+1);
				sprintf(fp,"%s/%s",watchpoints[event->wd],event->name);
				if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)){ // new directory created
					printf("%.4x = %.4x\n",event->mask, (IN_CREATE|IN_ISDIR));
					add_watch(fp);
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


int sync_open (char * path, const char * specifier ){
	FILE * fp = fopen(path,specifier);
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
	}
}

void sync_close ( unsigned int id ){
	if (id < num_open_files){
		fclose(open_files[id]);
		open_files[id] = NULL;
		if (first_open_files > id){
			if (first_open_files == num_open_files -1 && first_open_files != id)
				open_files = realloc(open_files, sizeof(FILE*) * --num_open_files);
			first_open_files = id;
		}
	}
}

int sync_read (int id, char * buffer, int len){
	if (id >= num_open_files) return 0;
	FILE* fp = open_files[id];
	printf("retrieving fp#%d, it is %p; len = %d\n",id,fp,len);
	if (fp != NULL){
		return fread(buffer,sizeof(char),len,fp);
	} else {
		return 0;
	}
}
  
int sync_write(char * path, int in_fp, int (*s_r)(int,char *,int)){
	FILE * fp = fopen(path, "wb");	
	printf("opening '%s' fp#%d for write\n",path,in_fp);
	char data[1024];
	int out=0, in;
	if (fp != NULL){
		while ( (in = s_r(in_fp,data,1024) ) > 0){
			printf("read %d bytes\n");
			out += fwrite(data,sizeof(char),in,fp);
		}
		fclose(fp);
		return out;
	} else {
		printf("error opening %s\n",path);
		return -1;
	}
}

int sync_mkdir(char * path){
	printf("mkdir %s\n",path);
	mkdir(path, 0777); // TODO: this should 
}

int sync_rm(char * path){
	printf("trying to delete %s\n",path);
	return remove(path);
}

/*int main ()
{
    init();
    watch_dir ("/home/paul/.cache");
    listen(cb);
    return 0;
}*/
