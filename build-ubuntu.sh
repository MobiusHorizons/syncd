#!/bin/bash

VERSION="0.8.9"
MANTAINER="Brian Cole <bec8@students.calvin.edu>"

if [ ! -d "syncd" ]; then
	git clone https://github.com/Mobiushorizons/syncd.git	
	cd syncd
else
	cd syncd
	git pull
fi


if [ ! -e "configure" ]; then
	./autogen.sh
else
	autoreconf
fi

./configure --prefix=/usr
make
mkdir -p /tmp/syncd
make DESTDIR=/tmp/syncd install
cd ..

if [ ! -d "syncd-rules" ]; then
	git clone https://github.com/yjftsjthsd-g/syncd-rules.git	
	cd syncd-rules
else
	cd syncd-rules
	git pull
fi

cp syncd-add /tmp/syncd/usr/bin/
cp syncd-remove /tmp/syncd/usr/bin/
cp syncd-restart /tmp/syncd/usr/bin/
cd ..

rm *.deb

fpm -s dir 		\
    -t deb 		\
    -n "syncd"  	\
    -v $VERSION 	\
    -d libjson-c2 	\
    -d libcurl3 	\
    -d python3		\
    -m "$MANTAINER" 	\
    -C /tmp/syncd 	\
    --url "http://mobiushorizons.github.io/syncd" 
