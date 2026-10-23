# tilezoom

Adds the ability to swap the client with another as per the configured policy on
tags using the tile layout. It's a replacement for the `zoom` function.

The configuration is done through the following variables:
```c
static const TileZoomCallback tilezoomfallback;
static const int tilezoomcfg;
```

When the layout isn't `tile` and `tilezoomfallback` isn't `NULL`,
`tilezoomfallback` will be called with the argument passed to `tilezoom`. The
argument is passed without any transformation. `tilezoom` itself does not expect
its argument to be a meaningful value.

The `tilezoomcfg` variable controls the general behavior of `tilezoom`. It's
expected to be a bitwise OR (`|`) of the following enumerators:
```c
enum {
    TileZoomMtSTop = 1 << 0,
    TileZoomMtSBtm = 1 << 1,
    TileZoomMtSFoc = 1 << 2,
    TileZoomMtMTop = 1 << 3,
    TileZoomMtMBtm = 1 << 4,
    TileZoomMtXDny = 1 << 5,
    TileZoomStMTop = 1 << 6,
    TileZoomStMBtm = 1 << 7,
    TileZoomStMFoc = 1 << 8,
    TileZoomStSTop = 1 << 9,
    TileZoomStSBtm = 1 << 10,
    TileZoomStXDny = 1 << 11,
};
```

```
XtYPos
^ ^^
| |\--> Client to swap the focused client with in Y.
| |
| \---> Area to move the focused client to when it's in X.
|
\-----> Initial area of the focused client.
```

All `TileZoomMt*` enumerators are mutually exclusive, but at least one of them
must be set. The same applies to all `TileZoomSt*` enumerators.

The `*MtS*` enumerators denote which client in the stack area would the selected
client be swapped with when residing in the master area. The `*StM*` enumerators
describe the opposite situation.

The `MtM` and `StS` enumerators swap the focused client within the same area.

The `*Dny` enumerators forbid any swapping when the selected client is in their
respective areas.

The `*Foc` enumerators do not cause any changes to the focus stack. This means
that after `tilezoom` runs, the unfocused area's last focused client may be the
client the focused client was swapped with, but it also might be a different
client within that area. It depends on which of those was focused last on that
tag as a whole.

Example binding in `keys`:
```c
{ MODKEY, XK_Return, tilezoom, {0} },
```

When using exstacks, a compatibility patch is provided that adds two new
enumerators:
```c
enum {
    TileZoomMtSExp = 1 << 12,
    TileZoomStMExp = 1 << 13,
};
```

These enumerators cause the focused client to be swapped with the active client
of the other area when that area is expanded.

They are to be used in conjunction with the `*MtS*` and `*StM*` enumerators.

They are mutually exclusive with the `MtM`, `StS`, and `*Dny` enumerators
respectively.

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### tilezoom-6.8

**Dependencies:**
- cutils-6.8
- tileinfo-6.8

```sh
git diff tilezoom-6.8^..tilezoom-6.8 > dwm-tilezoom-6.8.diff
```

### tilezoom-exstacks-xray-6.8

**Dependencies:**
- cutils-6.8
- tileinfo-exstacks-xray-6.8
- exstacks-xray-6.8 or exstacks-pertag-xray-6.8
- pertag-61bb8b2 (required when using exstacks-pertag-xray-6.8)
- xray-6.8

```sh
git diff tilezoom-exstacks-xray-6.8^^..tilezoom-exstacks-xray-6.8 > dwm-tilezoom-exstacks-xray-6.8.diff
```
