#!/usr/bin/env python

import cmd
import locale
import os
import json
import pprint
import shlex
import sys
import time
import requests
import hashlib

from dropbox import client, rest, session

from libc.stdio cimport *
from libc.stdlib cimport malloc, free
from libc.string cimport *
from cpython.ref cimport PyObject
cdef extern from "fileobject.h":
    ctypedef class __builtin__.file [object PyFileObject]:
        pass

cdef extern from "Python.h":
    object PyFile_FromFile(FILE *fp, char *name, char *mode, int (*close)(FILE*))
    FILE * PyFile_AsFile(object)
    void PyFile_IncUseCount( file)

cdef extern from "dropbox_c.h" nogil:
    char * curl_get(char*)

# XXX Fill in your consumer key and secret below
# You can find these at http://www.dropbox.com/developers/apps
APP_KEY = 'gmq6fs74fuw1ead'
APP_SECRET = 'ia87pt0ep6dvb7y'
STATE_FILE = 'state.json'

class Node(object):
    def __init__(self, path, content):
        # The "original" path (i.e. not the lower-case path)
        self.path = path
        # For files, content is a pair (size, modified)
        # For folders, content is a dict of children Nodes, keyed by lower-case file names.
        self.content = content
    def is_folder(self):
        return isinstance(self.content, dict)
    def to_json(self):
        return (self.path, Node.to_json_content(self.content))
    @staticmethod
    def from_json(jnode):
        path, jcontent = jnode
        return Node(path, Node.from_json_content(jcontent))
    @staticmethod
    def to_json_content(content):
        if isinstance(content, dict):
            return dict([(name_lc, node.to_json()) for name_lc, node in content.iteritems()])
        else:
            return content
    @staticmethod
    def from_json_content(jcontent):
        if isinstance(jcontent, dict):
            return dict([(name_lc, Node.from_json(jnode)) for name_lc, jnode in jcontent.iteritems()])
        else:
            return jcontent

def apply_delta(root, e):
    path, metadata = e
    branch, leaf = split_path(path)

    if metadata is not None:
        sys.stdout.write('+ %s\n' % path)
        # Traverse down the tree until we find the parent folder of the entry
        # we want to add.  Create any missing folders along the way.
        children = root
        for part in branch:
            node = get_or_create_child(children, part)
            # If there's no folder here, make an empty one.
            if not node.is_folder():
                node.content = {}
            children = node.content

        # Create the file/folder.
        node = get_or_create_child(children, leaf)
        node.path = metadata['path']  # Save the un-lower-cased path.
        if metadata['is_dir']:
            # Only create an empty folder if there isn't one there already.
            if not node.is_folder():
                node.content = {}
        else:
            node.content = metadata['size'], metadata['modified']
    else:
        sys.stdout.write('- %s\n' % path)
        # Traverse down the tree until we find the parent of the entry we
        # want to delete.
        children = root
        for part in branch:
            node = children.get(part)
            # If one of the parent folders is missing, then we're done.
            if node is None or not node.is_folder(): break
            children = node.content
        else:
            # If we made it all the way, delete the file/folder (if it exists).
            if leaf in children:
                del children[leaf]

def get_or_create_child(children, name):
    child = children.get(name)
    if child is None:
        children[name] = child = Node(None, None)
    return child

def split_path(path):
    assert path[0] == '/', path
    assert path != '/', path
    parts = path[1:].split('/')
    return parts[0:-1], parts[-1]

# Recursively search 'tree' for files that contain the string in 'term'.
# Print out any matches.
def search_tree(results, tree, term):
    for name_lc, node in tree.iteritems():
        path = node.path
        if (path is not None) and term in path:
            if node.is_folder():
                results.append('%s' % (path,))
            else:
                size, modified = node.content
                results.append('%s  (%s, %s)' % (path, size, modified))
        # Recurse on children.
        if node.is_folder():
            search_tree(results, node.content, term)

def load_state():
    if not os.path.exists(STATE_FILE):
        sys.stderr.write("ERROR: Couldn't find state file %r.  Run the \"link\" subcommand first.\n" % (STATE_FILE))
        sys.exit(1)
    f = open(STATE_FILE, 'r')
    state = json.load(f)
    state['tree'] = Node.from_json_content(state['tree'])
    f.close()
    return state

def save_state(state):
    f = open(STATE_FILE, 'w')
    state['tree'] = Node.to_json_content(state['tree'])
    json.dump(state, f, indent=4)
    f.close()

def command(login_required=True):
    """a decorator for handling authentication and exceptions"""
    def decorate(f):
        def wrapper(self, args):
            if login_required and self.api_client is None:
                self.stdout.write("Please 'login' to execute this command\n")
                return

            try:
                return f(self, *args)
            except TypeError, e:
                self.stdout.write(str(e) + '\n')
            except rest.ErrorResponse, e:
                msg = e.user_error_msg or str(e)
                self.stdout.write('Error: %s\n' % msg)

        wrapper.__doc__ = f.__doc__
        return wrapper
    return decorate

class DropboxTerm(cmd.Cmd):
    TOKEN_FILE = "token_store.txt"

    def __init__(self, app_key, app_secret):
        cmd.Cmd.__init__(self)
        self.app_key = app_key
        self.app_secret = app_secret
        self.current_path = ''
        self.prompt = "Dropbox> "

        self.api_client = None
        try:
            serialized_token = open(self.TOKEN_FILE).read()
            if serialized_token.startswith('oauth1:'):
                access_key, access_secret = serialized_token[len('oauth1:'):].split(':', 1)
                sess = session.DropboxSession(self.app_key, self.app_secret)
                sess.set_token(access_key, access_secret)
                self.api_client = client.DropboxClient(sess)
                print "[loaded OAuth 1 access token]"
            elif serialized_token.startswith('oauth2:'):
                access_token = serialized_token[len('oauth2:'):]
                self.api_client = client.DropboxClient(access_token)
                print "[loaded OAuth 2 access token]"
            else:
                print "Malformed access token in %r." % (self.TOKEN_FILE,)
        except IOError:
            pass # don't worry if it's not there

    @command()
    def do_ls(self):
        """list files in current remote directory"""
        resp = self.api_client.metadata(self.current_path)

        if 'contents' in resp:
            for f in resp['contents']:
                name = os.path.basename(f['path'])
                encoding = locale.getdefaultlocale()[1]
                self.stdout.write(('%s\n' % name).encode(encoding))

    @command()
    def do_cd(self, path):
        """change current working directory"""
        if path == "..":
            self.current_path = "/".join(self.current_path.split("/")[0:-1])
        else:
            self.current_path += "/" + path

    @command(login_required=False)
    def do_login(self):
        """log in to a Dropbox account"""
        flow = client.DropboxOAuth2FlowNoRedirect(self.app_key, self.app_secret)
        authorize_url = flow.start()
        sys.stdout.write("1. Go to: " + authorize_url + "\n")
        sys.stdout.write("2. Click \"Allow\" (you might have to log in first).\n")
        sys.stdout.write("3. Copy the authorization code.\n")
        code = raw_input("Enter the authorization code here: ").strip()

        try:
            access_token, user_id = flow.finish(code)
        except rest.ErrorResponse, e:
            self.stdout.write('Error: %s\n' % str(e))
            return

        with open(self.TOKEN_FILE, 'w') as f:
            f.write('oauth2:' + access_token)
        self.api_client = client.DropboxClient(access_token)

    @command(login_required=False)
    def do_login_oauth1(self):
        """log in to a Dropbox account"""
        sess = session.DropboxSession(self.app_key, self.app_secret)
        request_token = sess.obtain_request_token()
        authorize_url = sess.build_authorize_url(request_token)
        sys.stdout.write("1. Go to: " + authorize_url + "\n")
        sys.stdout.write("2. Click \"Allow\" (you might have to log in first).\n")
        sys.stdout.write("3. Press ENTER.\n")
        raw_input()

        try:
            access_token = sess.obtain_access_token()
        except rest.ErrorResponse, e:
            self.stdout.write('Error: %s\n' % str(e))
            return

        with open(self.TOKEN_FILE, 'w') as f:
            f.write('oauth1:' + access_token.key + ':' + access_token.secret)
        self.api_client = client.DropboxClient(sess)

    @command()
    def do_logout(self):
        """log out of the current Dropbox account"""
        self.api_client = None
        os.unlink(self.TOKEN_FILE)
        self.current_path = ''

    @command()
    def do_cat(self, path):
        """display the contents of a file"""
        f, metadata = self.api_client.get_file_and_metadata(self.current_path + "/" + path)
        self.stdout.write(f.read())
        self.stdout.write("\n")

    @command()
    def do_mkdir(self, path):
        """create a new directory"""
        self.api_client.file_create_folder(self.current_path + "/" + path)

    @command()
    def do_rm(self, path):
        """delete a file or directory"""
        self.api_client.file_delete(self.current_path + "/" + path)

    @command()
    def do_mv(self, from_path, to_path):
        """move/rename a file or directory"""
        self.api_client.file_move(self.current_path + "/" + from_path,
                                  self.current_path + "/" + to_path)

    @command()
    def do_share(self, path):
        print self.api_client.share(path)['url']

    @command()
    def do_account_info(self):
        """display account information"""
        f = self.api_client.account_info()
        pprint.PrettyPrinter(indent=2).pprint(f)

    @command(login_required=False)
    def do_exit(self):
        """exit"""
        return True

    @command()
    def do_get(self, from_path, to_path):
        """
        Copy file from Dropbox to local file and print out the metadata.

        Examples:
        Dropbox> get file.txt ~/dropbox-file.txt
        """
        to_file = open(os.path.expanduser(to_path), "wb")

        f, metadata = self.api_client.get_file_and_metadata(self.current_path + "/" + from_path)
        print 'Metadata:', metadata
        to_file.write(f.read())

    @command()
    def do_thumbnail(self, from_path, to_path, size='large', format='JPEG'):
        """
        Copy an image file's thumbnail to a local file and print out the
        file's metadata.

        Examples:
        Dropbox> thumbnail file.txt ~/dropbox-file.txt medium PNG
        """
        to_file = open(os.path.expanduser(to_path), "wb")

        f, metadata = self.api_client.thumbnail_and_metadata(
                self.current_path + "/" + from_path, size, format)
        print 'Metadata:', metadata
        to_file.write(f.read())

    @command()
    def do_put(self, from_path, to_path):
        """
        Copy local file to Dropbox

        Examples:
        Dropbox> put ~/test.txt dropbox-copy-test.txt
        """
        from_file = open(os.path.expanduser(from_path), "rb")

        self.api_client.put_file(self.current_path + "/" + to_path, from_file)

    @command()
    def do_search(self, string):
        """Search Dropbox for filenames containing the given string."""
        results = self.api_client.search(self.current_path, string)
        for r in results:
            self.stdout.write("%s\n" % r['path'])

    @command(login_required=False)
    def do_help(self):
        # Find every "do_" attribute with a non-empty docstring and print
        # out the docstring.
        all_names = dir(self)
        cmd_names = []
        for name in all_names:
            if name[:3] == 'do_':
                cmd_names.append(name[3:])
        cmd_names.sort()
        for cmd_name in cmd_names:
            f = getattr(self, 'do_' + cmd_name)
            if f.__doc__:
                self.stdout.write('%s: %s\n' % (cmd_name, f.__doc__))

    # the following are for command line magic and aren't Dropbox-related
    def emptyline(self):
        pass

    def do_EOF(self, line):
        self.stdout.write('\n')
        return True

    def parseline(self, line):
        parts = shlex.split(line)
        if len(parts) == 0:
            return None, None, line
        else:
            return parts[0], parts[1:], line


cdef public void py_main():
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY, APP_SECRET)
    term.cmdloop()

cdef public void update():
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY,APP_SECRET)
    page_limit = None
    # Load state
    state = load_state()
    cursor = state.get('cursor')
    tree = state['tree']

    # Connect to Dropbox
    c = term.api_client

    page = 0
    changed = False
    while (page_limit is None) or (page < page_limit):
        # Get /delta results from Dropbox
        result = c.delta(cursor)
        page += 1
        if result['reset'] == True:
            sys.stdout.write('reset\n')
            changed = True
            tree = {}
        cursor = result['cursor']
        # Apply the entries one by one to our cached tree.
        for delta_entry in result['entries']:
            changed = True
            apply_delta(tree, delta_entry)
        cursor = result['cursor']
        if not result['has_more']: break

    if not changed:
        sys.stdout.write('No updates.\n')
    else:
        # Save state
        state['cursor'] = cursor
        state['tree'] = tree
        save_state(state)



cdef public longpoll( int (*cb)(char*,int) ) with gil:
    cdef char* args
    cdef char* responsea
    cdef char* c_cursor
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY,APP_SECRET)

    # Load state
    state = load_state()
    cursor = state.get('cursor')
    while True: 
        result = term.api_client.delta(cursor)
        cursor = result['cursor']
        for path, metadata in result['entries']:
            m = term.api_client.metadata(path)
            if metadata is not None and m is not None:
                pfx_path = "dropbox://" + m['path']
                print '%s was created/updated' %path
                if m['is_dir']:
                    cb(pfx_path,0x40000100)
                else :
                    cb(pfx_path,0x08)
            else:
                if m['is_dir']:
                    cb(pfx_path,0x40000200);
                else:
                    cb(pfx_path,0x200)
                print '%s was deleted' % path

        if not result['has_more']:

            changes = False
            # poll until there are changes
            while not changes:
                url = 'https://api-notify.dropbox.com/1/longpoll_delta'
#                response = requests.get('https://api-notify.dropbox.com/1/longpoll_delta',
#                params={
#                    'cursor': cursor,  # latest cursor from delta call
#                   'timeout': 120     # default is 30 seconds
#                })
                c_cursor = <char*>malloc(cursor.len+1);
                strcpy(c_cursor,cursor);
                args = <char*> malloc(cursor.len + url.len + 47)
                sprintf(args,"curl -s -G -d 'cursor=%s' -d 'timeout=120' 'https://api-notify.dropbox.com/1/longpoll_delta'",c_cursor);
                with nogil:
                    response = curl_get(args);
		
                data = json.loads(response)
                free(args)
                free(c_cursor)
#                data = response.json()

                print data
                changes = data['changes']
                if not changes:
                    print 'Timeout, polling again...'
    
                backoff = None
                if backoff is not None:
                    print 'Backoff requested. Sleeping for %d seconds...' % backoff
                    time.sleep(backoff)
                print 'Resuming polling...'


cdef public char * py_open(char* path) :
    if APP_KEY == '' or APP_SECRET == '':
       exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY,APP_SECRET)
    f = term.api_client.get_file(path)
    temp = "/tmp/%d-%d" %(os.getpid(),f.fileno()+1)
    print temp
    r = open(temp,'wb');
    print r.fileno()
    r.write(f.read())
    r.close();
    f.close();
    return temp

cdef public int py_write(char * path,FILE * fd) :
    printf("py_write \n");
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY,APP_SECRET)
    printf("converting file\n")
    f = PyFile_FromFile(fd,path,'rb',NULL);
    printf("file converted\n")
    retn = term.api_client.put_file(path,f)
    print retn
    return retn['bytes']

cdef public int py_cpy(char * fr, char* to) with gil:
    if APP_KEY == '' or APP_SECRET == '':
        exit("You need to set your APP_KEY and APP_SECRET!")
    term = DropboxTerm(APP_KEY,APP_SECRET)
    c = term.api_client
    with c.get_file(fr) as f:
        print c.put_file('/test.txt',f)

	
