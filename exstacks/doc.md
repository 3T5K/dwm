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
The same behavior applies to the master area when a client in it is focused and
`exstacks` is called.

By default, the clients are stacked in front of each other. When using
transparent windows, other clients from that area can be seen underneath them
instead of the wallpaper, for example. The exstacks xray integration patch
should be used if sending every window except the "expanded" one off-screen is
desired.

With xray, adding a window to an expanded area will not make it the expanded
one unless it is focused. This can occur when invoking `incnmaster` while not
focused on the client that would be moved to the other area.

Example binding in `keys`:
```c
{ MODKEY, XK_x, exstack, {0} },
```

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
