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

#include "syncfs.h"

FILE ** open_files;
int num_open_files;
int first_open_files;
utilities utils;

const char * get_prefix(){
	return PLUGIN_PREFIX;
}

int update_file_cache(char * filename, int update){
    int metadata_changed = 0;
    struct stat details;
    args.log(LOGARGS,"filename = %s\n",filename);
    json_object * cache_entry = json_copy(utils.getFileCache(PLUGIN_PREFIX,filename),false);
    if (cache_entry == NULL){
        metadata_changed = S_CREATE;
        args.log(LOGARGS,"cache was null for %s\n",filename);
        cache_entry = json_object_new_object();
    }
    if (stat(filename, &details) == -1){
        metadata_changed = S_DELETE;
        json_object_object_del(cache_entry,"size");
        json_object_object_del(cache_entry,"modified");
        args.log(LOGARGS,"cannot stat '%s'\n",filename);
    } else { // stat succeeded
        if (S_ISDIR(details.st_mode)){
          metadata_changed |= S_DIR;
        }
        args.log(LOGARGS,"size = %ld; mtime= %ld\n",details.st_size, details.st_mtime);
    }
    long long int ver = 0;
    if (update) {
        ver = json_get_int(cache_entry, "version", 0);
        if (json_get_int(cache_entry, "size", -1) != details.st_size &&
                json_get_int(cache_entry, "modified", -1) != details.st_mtime
           ) {
             if (! (metadata_changed & (S_CREATE|S_DELETE)) ){
               metadata_changed |= S_CLOSE_WRITE; // essentially a modify.
             }
            ver += update;
        } else {
            // version same as cache.
						json_object_put(cache_entry);
            return 0;
        }
    } else {
        ver = json_get_int(cache_entry, "next_version",1);
        args.log(LOGARGS,"next_Version = %lld\n",ver);
    }
    json_object_object_add(cache_entry,"size", json_object_new_int64(details.st_size));
    json_object_object_add(cache_entry,"modified", json_object_new_int64((long long int)details.st_mtime));
    json_object_object_add(cache_entry, "version", json_object_new_int64(ver));
    args.log(LOGARGS,"cache for file %s : %s\n",filename, json_object_to_json_string(cache_entry));
    utils.addCache(PLUGIN_PREFIX,filename,cache_entry);
    return metadata_changed;
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
    fs_add_watch(dir_name);
    while (1) {
        struct dirent * entry;
        const char * d_name;
        struct stat sb;
        char path[PATH_MAX];
        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir (d);
        if (! entry) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = entry->d_name;
        /* See if "entry" is a subdirectory of "d". */
        if (snprintf(path, PATH_MAX, "%s/%s", dir_name, d_name) >= PATH_MAX){
            fprintf (stderr, "Path length has got too long.\n");
            exit (EXIT_FAILURE);
        }

        if (stat(path, &sb)) {
            args.log(LOGARGS,/*loggging_level.Error,*/ "failed to stat '%s'\n", path);
            continue;
        }
        if (S_ISDIR(sb.st_mode)) {
            /* Check that the directory is not "d" or d's parent. */
            if (strcmp (d_name, "..") != 0 && strcmp (d_name, ".") != 0) {
                watch_dir_recurse (path);
            }
        }else {
            // normal file. Add to cache.
            char * fileName = (char*) malloc(PLUGIN_PREFIX_LEN + strlen(dir_name) + strlen(entry->d_name) +2);
            strcpy(fileName, PLUGIN_PREFIX);
            strcat(fileName, dir_name);
            strcat(fileName, "/");
            strcat(fileName, entry->d_name);
            args.log(LOGARGS,"filename = %s\n",fileName);
            int mask;
            if ((mask = update_file_cache(fileName + PLUGIN_PREFIX_LEN, 1)) != 0){
              update_event(fileName, mask);
            }
            free(fileName);
        }
    }
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}

void sync_watch_dir (char * dir_name){
	  args.log(LOGARGS, "syncFS:watch_dir(%s)\n", dir_name);
		sync_mkdir(dir_name);
		dir_name += PLUGIN_PREFIX_LEN;
    watch_dir_recurse(dir_name);
}

char * init(init_args a){
		args = a;
    utils = args.utils;
    update_event = args.event_callback;
    local_init();
    return PLUGIN_PREFIX;
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
 args.log(LOGARGS,"read %d bytes\n",n);
 return n - 1024;
 }*/


FILE * sync_open (char * path){
    path += PLUGIN_PREFIX_LEN;
    args.log(LOGARGS,"opening %s\n",path);
    return fopen(path, "rb");
}

void sync_close ( FILE * fd ){
    fclose(fd);
}

int mkpath(char* file_path, mode_t mode) {
    char* p;
    for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
        *p='\0';

        if (mkdir(
            file_path
#ifndef WIN32
            ,mode
#endif
            )==-1) {
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
    update_file_cache(path,0);
    if (fd != NULL && fo != NULL){
        while ( (in = fread(data,sizeof(char),1024,fo) ) > 0){
            t+= in;
            args.log(LOGARGS,"\rread %d bytes",t);
            out += fwrite(data,sizeof(char),in,fd);
        }
        args.log(LOGARGS,"\n");
        fclose(fd);
        return out;
    } else {
        args.log(LOGARGS,"error opening %s\n",path);
        return -1;
    }
}

void sync_unload(){
  local_unload();
}

int sync_mkdir(char * path){
    path += PLUGIN_PREFIX_LEN;
    args.log(LOGARGS,"mkdir %s\n",path);
    return mkpath(path,0777);
}

int remove_cb (const char * fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

int sync_rm(char * path){
    path += PLUGIN_PREFIX_LEN;
    args.log(LOGARGS,"trying to delete %s\n",path);
    // recursively deleting directory

    return nftw(path, remove_cb, 64, FTW_DEPTH |FTW_PHYS);
}

int sync_mv(char*from,char*to){
    from += PLUGIN_PREFIX_LEN;
    to += PLUGIN_PREFIX_LEN;
    args.log(LOGARGS,"trying to move %s to %s\n",from,to);
    return rename(from,to);
}
