# #!/bin/bash

# this file will be remove later commit.

project_path=$PWD
patch_name="Rj45LessWrt-v0.1-Rj45Less.patch"
#src-link Rj45LessWrt /home/pmnxis/Develop/Rj45LessWrt

# Clone specific version (v19.07.8) of openwrt
# https://git.openwrt.org/?p=openwrt/openwrt.git;a=commit;h=cfc1602a1e4e013959997d15bcfa89583e9ae53f
git clone https://git.openwrt.org/openwrt/openwrt.git --recursive -b v19.07.10 openwrt
cd $project_path/openwrt
# git checkout cfc1602a1e4e013959997d15bcfa89583e9ae53f
# git checkout 83b0e20711ee4a927634b3c2a018c93527e84a2b
git submodule update --init --recursive

# patch a Rj45LessWrt
git am $project_path/openwrt_patch/$patch_name
./scripts/feeds update -a
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

ls -al ../build
make -j8
rm -rf ../build
mkdir -p ../build
cp ./bin/targets/ramips/mt76x8/*iptime*.bin ../build
ls -al ../build