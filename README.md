# Rj45LessWrt

**Work in progress**

Custom AP firmware for Rj45Less.
Rj45LessWrt based on OpenWRT 19.07 with helper scripts and packages for our purpose.

First this firmware is just proof of concept.

## Support Target
 * ipTIME Extender N300
 * ipTIME Extender N3

## Build Pre-requirements
 * Latest Ubuntu Linux 20.04 - amd64
 * https://openwrt.org/docs/guide-developer/build-system/install-buildsystem

## Compile
```shell
# install prerequirements for ubuntu 20.04.
chmod +x ./setup.sh
./setup.sh
```

## Applying on real machine
When you try to upload a firmware from Stock ipTIME AP. Please upload `*-initramfs-kernel.bin`.

Or for upgrade from `TFTP Bootloader` or `working openwrt`, Please use `*-squashfs-sysupgrade.bin`.