# tilenmctl

A versatile `incnmaster` replacement for the exstacks-patched tile layout.

Arguments to `tilenmctl` are expected to be a bitwise OR (`|`) of the following
enumerators:
```c
enum {
    TileNmCtlDec       = 1 << 0,
    TileNmCtlInc       = 1 << 1,
    TileNmCtlNoArrange = 1 << 2,
};
```

`TileNmCtlDec` and `TileNmCtlInc` are mutually exclusive and at least one of
them must be set. `TileNmCtlNoArrange` causes `tilenmctl` to not call `arrange`
if it otherwise would. This option can be ignored if using `tilenmctl` directly
through a binding.

The configuration is done through the following variables:
```c
static const TileNmCtlCallback tilenmctlfallback;
static const int tilenmctlcfg;
```

When the layout isn't `tile` and `tilenmctlfallback` isn't `NULL`,
`tilenmctlfallback` will be called with the argument passed to `tilenmctl`. The
argument is not validated when passed to the callback. Therefore, it might be
`NULL` or not be one of the permitted values above.

There is a function `tilenmctldefaultfallback` which acts as a default fallback
for `tilenmctl`. It maps a `tilenmctl` argument to the corresponding value for
`incnmaster` and invokes it. `TileNmCtlNoArrange` is ignored.

The `tilenmctlcfg` variable controls the general behavior of `tilenmctl`. It's
expected to be a bitwise OR (`|`) of the following enumerators:
```c
enum {
    TileNmCtlMasterDefer   = 1 << 0,
    TileNmCtlMasterPrefer  = 1 << 1,
    TileNmCtlMasterNone    = 1 << 2,
    TileNmCtlMasterAndDeny = 1 << 3,
    TileNmCtlMasterNoMove  = 1 << 4,
    TileNmCtlStackDefer    = 1 << 5,
    TileNmCtlStackPrefer   = 1 << 6,
    TileNmCtlStackNone     = 1 << 7,
    TileNmCtlStackAndDeny  = 1 << 8,
    TileNmCtlStackNoMove   = 1 << 9,
};
```

`TileNmCtlMasterDefer`, `TileNmCtlMasterPrefer`, and `TileNmCtlMasterNone` are
mutually exclusive and at least one of them must be set. The same applies to
`TileNmCtlStackDefer`, `TileNmCtlStackPrefer`, and `TileNmCtlStackNone`.

`TileNmCtlMasterAndDeny` and `TileNmCtlStackAndDeny` are modifiers to the
enumerators in the previous paragraph and can be set arbitrarily.

`TileNmCtlMasterNoMove` can only be set if `TileNmCtlMasterAndDeny` and
`TileNmCtlMasterPrefer` are set. The same applies to `TileNmCtlStackNoMove` with
the respective `Stack` enumerators.

If any of the invariants above are broken, `tilenmctl` returns early.

When the argument to `tilenmctl` is `TileNmCtlInc` and the stack area is
expanded, the `TileNmCtlStack*` options are relevant. When it's `TileNmCtlDec`
and the master area is expanded, the `TileNmCtlMaster*` options are relevant.
If the affected area isn't expanded, `tilenmctl` behaves like `incnmaster`.

The `*Defer` options will attempt to move the expanded client in the monitor's
`clients` list in a way that prevents the `tile` function from moving the
expanded client to the other area, causing it to use another client instead. If
only the expanded client is left, then it will be moved to the other area.

The `*Prefer` options will attempt to move the expanded client in the monitor's
`clients` list in a way that lets the `tile` function move that expanded client
to the other area.

The `*None` options are akin to how `incnmaster` would behave by default. The
expanded client would be moved to the other area when it would be if the stack
were not expanded. The monitor's client list is not rearranged in this case.

The `*AndDeny` options prevent the expanded client from being moved to the other
area when the policies above would otherwise move it.

The `*NoMove` options stop `tilenmctl` from moving the expanded client in the
`clients` list. Even though the expanded client would not be moved to the other
area due to `*AndDeny` being enabled, it might be moved within its own area due
to `*Prefer` being enabled. Combining `*Prefer`, `*AndDeny`, and `*NoMove` is
therefore a way to disable moving any client out of the respective area and
reorganizing the order of clients inside it.

Example bindings in `keys`:
```c
{ MODKEY, XK_i, tilenmctl, {.i = TileNmCtlInc} },
{ MODKEY, XK_d, tilenmctl, {.i = TileNmCtlDec} },
```

## patch files

### tilenmctl-6.8
**Dependencies:**
- cutils-6.8
- tileinfo-exstacks-xray-6.8
- exstacks-xray-6.8
- xray-6.8

```sh
git diff tilenmctl-6.8^..tilenmctl-6.8 > dwm-tilenmctl-6.8.diff
```

### tilenmctl-pertag-6.8
**Dependencies:**
- cutils-6.8
- tileinfo-exstacks-xray-6.8
- exstacks-pertag-xray-6.8
- xray-6.8
- pertag-61bb8b2

```sh
git diff tilenmctl-pertag-6.8^^..tilenmctl-pertag-6.8 > dwm-tilenmctl-pertag-6.8.diff
```
