typedef struct{
  /* functions */
  S_INIT      init;
  S_LISTEN    listen;
  S_WATCH_DIR watch_dir;
  S_OPEN_FILE open;
  S_WRITE     write;
  S_RM        rm;
  S_MV        mv;
  S_MKDIR     mkdir;
  S_UNLOAD    unload;
  /* properties */
  char *      prefix;
  int         prefix_len;
  lt_dlhandle dlhandle;
} plugin;