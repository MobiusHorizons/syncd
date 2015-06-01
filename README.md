syncd
=====

Synchronization service for linux with plugins for various endpoints hopefully dropbox, Google Drive, etc

more info can be found in the [wiki](https://github.com/MobiusHorizons/syncd/wiki "Wiki")

Build/installation instructions can be found in the INSTALL file or on the wiki
# Building

You will need `libcurl` and `json-c` development libraries installed. 

Then run these commands from the checked out folder.

    ./autogen.sh #build the configure script.
    ./configure
    make
    sudo make install.


# Running

Once you have built and installed **syncd**, you will need to create a rules file.

This file should be located in the syncd config directory at `$HOME/.config/syncd/rules.json`.
This is a json formatted file specifying where to copy files to/from.

The format of this file is as follows:

    { 
        "plugin:///src/folder/" : [ "plugin:///dest/1/", "plugin:///dest/2/"],
        "dropbox:///Documents/" : [ "file:///home/user/Documents/"]
    }

The current plugin prifixes are 
- `dropbox://` for dropbox
- `file://` for the filesystem
- `gdrive://` for Google Drive

You can also use the python utilities in [syncd-rules](https://github.com/yjftsjthsd-g/syncd-rules).
These are included in the 
[Ubuntu](https://github.com/MobiusHorizons/syncd/releases/download/v0.9/syncd_0.9_amd64.deb) and 
[Fedora](https://github.com/MobiusHorizons/syncd/releases/download/v0.9/syncd-0.9-1.fedora.x86_64.rpm)
packages as part of [release v0.9](https://github.com/MobiusHorizons/syncd/releases/tag/v0.9)

To add a rule run:

    syncd-add "file:///home/user/Documents/" "dropbox:///Documents"

this will add full synchronization both to and from your filesystem and dropbox.
