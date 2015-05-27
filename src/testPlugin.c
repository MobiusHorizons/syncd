#include <stdio.h>
#include <ltdl.h>
#include "plugin.h"
#include "cache.h"

int cb(const char * name, int mode){
	printf("%s %d\n", name, mode);
}

int main(int argc, char ** argv){
	if (argc < 2) return 1;
	S_INIT init;
	S_UNLOAD unload;
	char * plugin_name = argv[1];
	char * prefix;
	logging_init("plugin_log.txt");
	cache_init();
	lt_dlinit();

	lt_dlhandle plugin = lt_dlopen(plugin_name);
	if (plugin == NULL){
		logging_stdout("Error loading '%s': %s\n", plugin_name, lt_dlerror());
		return 1;
	}
	
	init    = (S_INIT  ) lt_dlsym(plugin, "init");
	if (init == NULL){
		logging_stdout("Error loading function 'init': %s\n", lt_dlerror());
		return 1;
	}
	
	unload	= (S_UNLOAD) lt_dlsym(plugin, "sync_unload");
	if (unload == NULL){
		logging_stdout("Error loading function 'sync_unload': %s\n", lt_dlerror());
		return 1;
	}
		
	init_args args;
	args.event_callback = cb;
	args.log = logging_log;
	args.printf = logging_stdout;
	args.error = logging_stderr;
	args.utils = get_utility_functions();
	prefix = init(args);
	logging_close();
	cache_clear();
	puts(prefix);
}
