# Rj45LessWrt Package

## rlprofile
Access to non-volatile address for writing or reading rj45less profile.

rj45less profile contains **MID**(ShopID) that's 6digit of integer.

The profile have a hash together, thus when flash got pollution, It will fail to read.

## mabase62
Custom base62 algorithm for rj45less.