#!/bin/sh
set -e
if [ ! -d "$HOME/jsonc/lib" ]; then
  cd ..
  wget https://github.com/json-c/json-c/archive/json-c-0.12-20140410.tar.gz
  tar -xzvf json-c-0.12-20140410.tar.gz
  cd json-c-json-c-0.12-20140410
  CC="clang"
  export CFLAGS=""
  ./configure --prefix=$HOME/jsonc
  make
  make install
else
  echo 'Using cached copy of json-c.';
fi
