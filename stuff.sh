# #!/bin/bash

# this file will be remove later commit.

project_path=$PWD
/scripts/feeds update -a
./scripts/feeds install -a
make defconfig
# Donwload sources first
make download -j4

# feeds should be apply later
echo "src-link Rj45LessWrt $project_path/Rj45LessWrt" >> $project_path/openwrt/feeds.conf
./scripts/feeds update Rj45LessWrt && ./scripts/feeds install -a -p Rj45LessWrt && make package/mabase62/compile V=s 
./scripts/feeds update Rj45LessWrt && ./scripts/feeds install -a -p Rj45LessWrt && make package/rlprofile/compile V=s 

# !!!!!!!!!!!!!! THIS SCIRPT IS NOT RIGHT 
# make menuconfig should be handle by human hand.


# take customized .config file
cp -rf $project_path/openwrt_patch/.config $project_path/openwrt

# Get in sync modified configuration.
make defconfig

# compile at AMD 16c32t workstation (1950x, 3950x)
make -j8

# copy built data to build dir
cd $project_path
mkdir -p $project_path/build
cp $project_path/openwrt/bin/targets/ramips/mt76x8/*.bin $project_path/build
