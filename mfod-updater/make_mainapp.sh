#! /bin/bash

# build number update
cd tools
./buildnum_updater ../inc/verinfo.h
cd ..

# make thetis-mainapp
echo "building amod-mainapp"
make all
