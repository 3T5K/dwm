# exstacks

Allows one client in the master or stack area to occupy all screen real-estate
given to that area.

Suppose s2 is focused.
```
+----------+------+
|          |  s1  |
|          |------+
|    m1    |  s2  |
|          |------+
|          |  s3  |
+----------+------+
```
Invoking `exstack` would retile everything to:
```
+----------+------+
|          |      |
|          |      |
|    m1    |  s2  |
|          |      |
|          |      |
+----------+------+
```
The same behavior applies to the master area when a client in it is focused.

## patch files

The following diffs do not include the other patches, those must be applied
separately.

### exstacks-6.8
```sh
git diff 6.8..exstacks-6.8 > dwm-exstacks-6.8.diff
```

### exstacks-6.8 + pertag-61bb8b2
```sh
git diff exstacks-pertag-6.8^..exstacks-pertag-6.8 > dwm-exstacks-pertag-6.8.diff
```

### exstacks-6.8 + xray-6.8
```sh
git diff xray-6.8..exstacks-xray-6.8 > dwm-exstacks-xray-6.8.diff
```

### exstacks-6.8 + pertag-61bb8b2 + xray-6.8
```sh
git diff exstacks-pertag-xray-6.8^^..exstacks-pertag-xray-6.8 > dwm-exstacks-pertag-xray-6.8.diff
```
