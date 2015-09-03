#!/bin/sh
set -e
if [ ! -d "$HOME/pcc/bin/pcc" ]; then
  cd ..
  wget ftp://pcc.ludd.ltu.se/pub/pcc-releases/pcc-1.1.0.tgz
  wget ftp://pcc.ludd.ltu.se/pub/pcc-releases/pcc-libs-1.1.0.tgz
  tar -xzvf pcc-1.1.0.tgz
  tar -xzvf pcc-libs-1.1.0.tgz
  CC="gcc"
  (
	  cd pcc-1.1.0
	  ./configure --prefix=$HOME/pcc
	  make
	  make install
  )
  (
	  cd pcc-libs-1.1.0
	  ./configure --prefix=$HOME/pcc
	  make
	  make install
  )
else
  echo 'Using cached copy of pcc.';
fi
