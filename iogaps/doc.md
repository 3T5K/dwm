# iogaps

Adds inner and outer window gaps for the tile layout. Based on tilegap (6.4).

```c
static const unsigned int ogap = 30; /* gap on outer edges  */
static const unsigned int igap = 10; /* gap between windows */
```

## tilegap

https://dwm.suckless.org/patches/tilegap/

## patch files

The following diffs do not include the other patches. Those must be applied
separately.

### iogaps-6.8
```sh
git diff 6.8..iogaps-6.8 > dwm-iogaps-6.8.diff
```

### iogaps-6.8 + exstacks-6.8
```sh
git diff exstacks-6.8..iogaps-exstacks-6.8 > dwm-iogaps-exstacks-6.8.diff
```

### iogaps-6.8 + exstacks-pertag-6.8
```sh
git diff exstacks-pertag-6.8..iogaps-exstacks-pertag-6.8 > dwm-iogaps-exstacks-pertag-6.8.diff
```

### iogaps-6.8 + exstacks-xray-6.8
```sh
git diff exstacks-xray-6.8..iogaps-exstacks-xray-6.8 > dwm-iogaps-exstacks-xray-6.8.diff
```

### iogaps-6.8 + exstacks-pertag-xray-6.8
```sh
git diff exstacks-pertag-xray-6.8..iogaps-exstacks-pertag-xray-6.8 > dwm-iogaps-exstacks-pertag-xray-6.8.diff
```
