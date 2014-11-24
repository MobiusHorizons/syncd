#include <sys/inotify.h>


static int inotify_fd;
static char** watchpoints;
static int num_watchpoints;
static int watchpoints_size;

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN	(1024 * (EVENT_SIZE + 16))

void add_watch(char * dir){
  int wp  = inotify_add_watch( inotify_fd, dir, IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVE);
  printf("adding directory: %s, wp = %d\n",dir,wp);
  char * dir_local = strdup(dir);
    while (wp >= watchpoints_size) {
        watchpoints_size *= 2;
        watchpoints = realloc(watchpoints, watchpoints_size * sizeof(char*));
    }
  watchpoints[wp] = dir_local;
  num_watchpoints = wp;
}

void local_init(){
  num_watchpoints = 0;
  watchpoints_size = 1;
  watchpoints = (char **) malloc(sizeof(char*));
  //num_open_files = 0;
  inotify_fd = inotify_init();
  if (inotify_fd < 0){
    perror("inotify_init");
  }
}

void sync_listen(int (*cb)( const char*,int)){
  update_event = cb;
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
          update_file_cache(filename,1);
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
