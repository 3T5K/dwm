# tilefocus

Adds an alternate focus-changing system for the tile layout. The focus is
directional. The four cardinal directions are supported.

Arguments for `tilefocus`:
- `w` or `W` -> West
- `s` or `S` -> South
- `n` or `N` -> North
- `e` or `E` -> East

The configuration is done through the following variables:
```c
static const TileFocusCallback tilefocusfallback;
static const int tilefocuscfg;
```

When the layout isn't `tile` and `tilefocusfallback` isn't `NULL`,
`tilefocusfallback` will be called with the argument passed to `tilefocus`. The
argument is not validated when passed to the callback. Therefore, it might be
`NULL` or not be one of the permitted values above.

There is a function `tilefocusdefaultfallback` which acts as a default fallback
for `tilefocus`. It maps a `tilefocus` argument to the corresponding value for
`focusstack` and invokes it. Only North and South are meaningfully mapped.
Anything else does not cause a `focusstack` invocation.

The `tilefocuscfg` variable controls the general behavior of `tilefocus`. It's
expected to be a bitwise OR (`|`) of the following enumerators:
```c
enum {
    TileFocusVertOverflow = 1 << 0,
    TileFocusHorzOverflow = 1 << 1,
    TileFocusMasterTop    = 1 << 2,
    TileFocusMasterBottom = 1 << 3,
    TileFocusMasterLast   = 1 << 4,
    TileFocusStackTop     = 1 << 5,
    TileFocusStackBottom  = 1 << 6,
    TileFocusStackLast    = 1 << 7,
};
```

Top, bottom, and last refer to the top, bottom, and last focused client of
the respective area (master or stack).

`TileFocusMasterTop`, `TileFocusMasterBottom`, and `TileFocusMasterLast` are
mutually exclusive. If zero or more than one of these is active, `tilefocus`
will return early and not do anything. `TileFocusStackTop`,
`TileFocusStackBottom`, and `TileFocusStackLast` are also mutually exclusive.

Example bindings in `keys`:
```c
{ MODKEY, XK_h, tilefocus, {.i = 'W'} },
{ MODKEY, XK_j, tilefocus, {.i = 'S'} },
{ MODKEY, XK_k, tilefocus, {.i = 'N'} },
{ MODKEY, XK_l, tilefocus, {.i = 'E'} },
```

When using exstacks, the last focused client is not inherently the same one as
the expanded client of a stack. This is a quirk of `incnmaster`. That function
can place a client into the other area without it being focused. Suppose such a
client was focused more recently than the expanded one. Although exstacks would
not make this client the expanded one, changing focus to that area with the
configuration set to last focused would focus this client instead of the
expanded one. To change this behavior, use the exstacks-xray compatibility
patch. Note that this patch is pertag-agnostic and will work when applied to an
exstacks-pertag-xray patched tree.

Two new enumerators for `tilefocuscfg` are added:
```c
enum {
    TileFocusMasterXrayAc = 1 << 8,
    TileFocusStackXrayAc  = 1 << 9,
};
```

These force `tilefocus` to focus the expanded client if the respective area is
expanded. These are additive to the original options. The original options must
still be set because `tilefocus` would not know how to behave when an area isn't
expanded.

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### tilefocus-6.8

**Dependencies:**
- tileinfo-6.8

```sh
git diff tileinfo-6.8..tilefocus-6.8 > dwm-tilefocus-6.8.diff
```

### tilefocus-exstacks-xray-6.8

**Dependencies:**
- tileinfo-exstacks-xray-6.8
- exstacks-xray-6.8 or exstacks-pertag-xray-6.8
- pertag-61bb8b2 (required when using exstacks-pertag-xray-6.8)
- xray-6.8

```sh
git diff tileinfo-exstacks-xray-6.8..tilefocus-exstacks-xray-6.8 > dwm-tilefocus-exstacks-xray-6.8.diff
```
