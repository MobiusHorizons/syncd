#ifndef __GOOGLE_DRIVE_API_H__
#define __GOOGLE_DRIVE_API_H__
#include <string.h>
#include "librest/rest.h"

/* convenience functions */
const char * 	JSON_GET_STRING		(json_object*, char*          );
long	 	JSON_GET_INT64		(json_object*, char*          );
bool 		JSON_GET_BOOL		(json_object*, char*, bool    );

const char *	gdrive_access_token	(const char *                 );
void		gdrive_init		(const char *,json_object*    );
json_object * 	gdrive_get_metadata	(const char *                 );
FILE * 		gdrive_get 		(char *                       );
json_object * 	gdrive_get_changes	(const char*,const char*,int  );
void 		gdrive_upload		(char*,FILE*                  );
json_object * 	gdrive_files_list	(char*, int                   );
json_object * 	gdrive_files_list_children(char*,int                  );
json_object * 	gdrive_files_put	(const char*, FILE*           );
char * 		gdrive_refresh_token 	(char*                        );
json_object *	gdrive_authorize_token 	(char*,const char*,const char*);
void		gdrive_cleanup		(		              );

#endif
