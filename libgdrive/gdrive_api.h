#ifndef __GOOGLE_DRIVE_API_H__
#define __GOOGLE_DRIVE_API_H__
#include <string.h>
#include "../librest/rest.h"

/*	convenience functions */
const char * JSON_GET_STRING (json_object*, const char* );
long JSON_GET_INT64 (json_object*, const char* );
bool JSON_GET_BOOL (json_object*, const char*, bool );


/***
 *	Set/retrieve the access token for gdrive to use.
 *	if the input is NULL the current access token is output
 *	else the new value is returned.
 */
const char * gdrive_access_token (const char * accessToken);

/***
 *	initialize the library with token and a json cache object to use.
 */
void gdrive_init (const char * accessToken, json_object * cache);

/***
 *	Retrieve file metadata either from cache or from server.
 */
json_object * gdrive_get_metadata (const char * fileID);

/***
 *	Download a file from google drive. takes a fileID not a filename.
 */
FILE * gdrive_get (const char * fileID);

/***
 *  @inputs = ( PageToken, startChangeId, maxResults )
 *	Get changes to the linked accout either starting from PageToken
 *	or a specific change entry id. If all of those are null,
 *	just retrieve (max) entries from the start of the change
 *	history.
 */
json_object * gdrive_get_changes (const char* PageToken,const char* startChangeID, int maxResults, bool includeSubscribed, bool includeDeleted);

/*
 *	Upload a file to a given google upload url. Mostly used internally
 */
json_object * gdrive_upload (char* uploadURL, FILE* file);

/****
 *	List files that match a query. optionally pass a nonzero pageToken
 *	to page through results.
 */
json_object * gdrive_files_list (char* Query, int pageToken);

/***
 *	List the children of a given folder ID (ie "root")
 *	Optionally use a page token to page through results.
 */
json_object * gdrive_files_list_children(char* fileID, int pageToken);

/***
*	List the children of a given folder ID (ie "root")
*	Optionally use a page token to page through results.
* optionally pass a query to filter by.
*/
json_object * gdrive_folder_list(char* fileID,const char * pageToken, char* query);

/**
 * Create a new folder
 *
 */
json_object * gdrive_new_folder(json_object * metadata);

/*** 
 * Upload a file with a given metadata to google drive
 *
 */
json_object * gdrive_put_file(json_object * metadata, FILE * file);

/***
 *	Upload a given FILE * to the specified location.
 *	(ex. gdrive_files_put("/Family Trip/Photo.jpg", Picture); )
 */
json_object * gdrive_files_put (const char* path, FILE* file);

/***
 *	Get a new access token from a refresh token.
 ***/
char * gdrive_refresh_token (char* refreshToken);

/***
 *	Part of the Oauth step. Get oauth credentials from a returned
 *	Token. This gives you both a refresh token and an initial
 *	Access token.
 *	@Inputs= (Token, Client_ID, Client_Seret)
 */
json_object * gdrive_authorize_token (char* token,const char* clientID, const char* clientSecret);

/***
 *	Call this to clean up any memory/connections gdrive may have
 *	allocated.
 */
void gdrive_cleanup ( );

#endif
