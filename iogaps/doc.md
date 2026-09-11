# iogaps

Adds inner and outer window gaps for the tile layout. Based on tilegap (6.4).

```c
static const unsigned int ogap = 30; /* gap on outer edges  */
static const unsigned int igap = 10; /* gap between windows */
```

## tilegap

https://dwm.suckless.org/patches/tilegap/

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### iogaps-6.8
```sh
git diff 6.8..iogaps-6.8 > dwm-iogaps-6.8.diff
```

### iogaps-exstacks-6.8
**Dependencies:**
- exstacks-6.8

```sh
git diff exstacks-6.8..iogaps-exstacks-6.8 > dwm-iogaps-exstacks-6.8.diff
```

### iogaps-exstacks-pertag-6.8
**Dependencies:**
- exstacks-pertag-6.8
- pertag-61bb8b2

```sh
git diff exstacks-pertag-6.8..iogaps-exstacks-pertag-6.8 > dwm-iogaps-exstacks-pertag-6.8.diff
```

### iogaps-exstacks-xray-6.8
**Dependencies:**
- exstacks-xray-6.8
- xray-6.8

```sh
git diff exstacks-xray-6.8..iogaps-exstacks-xray-6.8 > dwm-iogaps-exstacks-xray-6.8.diff
```

### iogaps-exstacks-pertag-xray-6.8
**Dependencies:**
- exstacks-pertag-xray-6.8
- pertag-61bb8b2
- xray-6.8

```sh
git diff exstacks-pertag-xray-6.8..iogaps-exstacks-pertag-xray-6.8 > dwm-iogaps-exstacks-pertag-xray-6.8.diff
```
