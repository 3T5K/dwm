# tilemove

Adds a client moving system for the tile layout. A client can be moved in the
four cardinal directions.

Arguments for `tilemove`:
- `w` or `W` -> West
- `s` or `S` -> South
- `n` or `N` -> North
- `e` or `E` -> East

The configuration is done through the following variables:
```c
static const TileMoveCallback tilemovefallback;
static const int tilemovecfg;
```

When the layout isn't `tile` and `tilemovefallback` isn't `NULL`,
`tilemovefallback` will be called with the argument passed to `tilemove`. The
argument is not validated when passed to the callback. Therefore, it might be
`NULL` or not be one of the permitted values above.

The `tilemovecfg` variable controls the general behavior of `tilemove`. It's
expected to be a bitwise OR (`|`) of the following enumerators:
```c
enum {
    TileMoveVertOverflow = 1 << 0,
    TileMoveHorzOverflow = 1 << 1,
    TileMoveMasterTop    = 1 << 2,
    TileMoveMasterBottom = 1 << 3,
    TileMoveStackTop     = 1 << 4,
    TileMoveStackBottom  = 1 << 5,
};
```

`TileMoveMasterTop` and `TileMoveMasterBottom` are mutually exclusive and at
least one of them must be set. Otherwise `tilemove` will return early. The same
applies to `TileMoveStackTop` and `TileMoveStackBottom`.

Example bindings in `keys`:
```c
{ MODKEY|ShiftMask, XK_h, tilemove, {.i = 'W'} },
{ MODKEY|ShiftMask, XK_j, tilemove, {.i = 'S'} },
{ MODKEY|ShiftMask, XK_k, tilemove, {.i = 'N'} },
{ MODKEY|ShiftMask, XK_l, tilemove, {.i = 'E'} },
```

When using exstacks, it might not be desirable to move a client vertically
when its area is expanded. The exstacks compatibility patches introduce new
enumerators which deny vertical movement when the respective area is expanded.
```c
enum {
    TileMoveMasterXDeny  = 1 << 6,
    TileMoveStackXDeny   = 1 << 7,
};
```

## patch files

The following diffs do not include the other patches. Those must be applied
separately.

### tilemove-6.8 + cutils-6.8 + tileinfo-6.8
```sh
git diff tilemove-6.8^..tilemove-6.8 > dwm-tilemove-6.8.diff
```

### tilemove-6.8 + cutils-6.8 + tileinfo-6.8 + pertag-61bb8b2
```sh
git diff tilemove-pertag-6.8^^..tilemove-pertag-6.8 > dwm-tilemove-pertag-6.8.diff
```

### tilemove-6.8 + cutils-6.8 + tileinfo-exstacks-xray-6.8 + exstacks-xray-6.8
```sh
git diff tilemove-exstacks-xray-6.8^^..tilemove-exstacks-xray-6.8 > dwm-tilemove-exstacks-xray-6.8.diff
```

### tilemove-6.8 + cutils-6.8 + tileinfo-exstacks-xray-6.8 + exstacks-pertag-xray-6.8
```sh
git diff tilemove-exstacks-pertag-xray-6.8^^^...tilemove-exstacks-pertag-xray-6.8 > dwm-tilemove-exstacks-pertag-xray-6.8.diff
```
