# For Develop

### Add this line to feeds.conf - root directory of openwrt.
```conf
src-link Rj45LessWrt /home/pmnxis/Develop/Rj45LessWrt
```

### Applying and Combine
```shell
./scripts/feeds update Rj45LessWrt && ./scripts/feeds install -a -p Rj45LessWrt
```

### For Test
```shell
./scripts/feeds update Rj45LessWrt && ./scripts/feeds install -a -p Rj45LessWrt && make package/mabase62/compile V=s 
./scripts/feeds update Rj45LessWrt && ./scripts/feeds install -a -p Rj45LessWrt && make package/rlprofile/compile V=s 
```

### ssh connecting tip
```shell
ssh -o StrictHostKeyChecking=no -l "root" "192.168.68.231"
```
