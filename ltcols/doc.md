# ltcols

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### ltcols-6.8

```sh
git diff 6.8..ltcols-6.8 > dwm-ltcols-6.8.diff
```

### noborderflicker-ltcols-6.8

**Dependencies:**
- ltcols-6.8
```sh
git diff ltcols-6.8..noborderflicker-ltcols-6.8 > dwm-noborderflicker-ltcols-6.8.diff
```

### ltcols-floatlt-6.8

**Dependencies:**
- ltcols-6.8

```sh
git diff ltcols-6.8..ltcols-floatlt-6.8 > dwm-ltcols-floatlt-6.8.diff
```

### ltcols-floatwin-6.8

**Dependencies:**
- ltcols-6.8

```sh
git diff ltcols-6.8..ltcols-floatwin-6.8 > dwm-ltcols-floatwin-6.8.diff
```

### ltcols-monocle-6.8

**Dependencies:**
- ltcols-6.8

```sh
git diff ltcols-6.8..ltcols-monocle-6.8 > dwm-ltcols-monocle-6.8.diff
```

### ltcols-tile-6.8

**Dependencies:**
- ltcols-6.8

```sh
git diff ltcols-6.8..ltcols-tile-6.8 > dwm-ltcols-tile-6.8.diff
```

### ltcols-scratchpads-6.8

**Dependencies:**
- scratchpads-6.8
- ltcols-6.8

```sh
git diff ltcols-scratchpads-6.8^..ltcols-scratchpads-6.8 > dwm-ltcols-scratchpads-6.8.diff
```

### ltcols-xraymono-6.8

**Dependencies:**
- xraymono-6.8
- xray-6.8
- ltcols-6.8

```sh
git diff ltcols-xraymono-6.8^..ltcols-xraymono-6.8 > dwm-ltcols-xraymono-6.8.diff
```

### ltcols-exstacks-6.8

**Dependencies:**
- exstacks-xray-6.8 or exstacks-pertag-xray-6.8
- pertag-61bb8b2 (required when using exstacks-pertag-xray-6.8)
- xray-6.8
- ltcols-6.8

```sh
git diff ltcols-exstacks-6.8^..ltcols-exstacks-6.8 > dwm-ltcols-exstacks-6.8.diff
```
