#!/bin/sh
set -e
if [ ! -d "$HOME/jsonc_osx/lib" ]; then
  cd ..
  wget https://github.com/json-c/json-c/archive/json-c-0.12-20140410.tar.gz
  tar -xvf json-c-0.12-20140410.tar.gz
  cd json-c-json-c-0.12-20140410
  CC="clang"
  export CCFLAGS="-Wno-error=unused-but-set-variable"
  ./configure --prefix=$HOME/jsonc_osx
  make
  make install
else
  echo 'Using cached copy of json-c.';
fi
