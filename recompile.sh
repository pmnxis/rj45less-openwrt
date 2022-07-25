#!/bin/bash

# this file will be remove later commit.

project_path=$PWD
cd openwrt

# remove existing file
rm -rf $project_path/bin/targets/ramips/mt76x8

# compile at AMD 16c32t workstation (1950x, 3950x)
make -j8

# copy built data to build dir
cd $project_path
mkdir -p $project_path/build
cp $project_path/openwrt/bin/targets/ramips/mt76x8/*.bin $project_path/build