# rlayout

Automatically resets the layout when there are no tiled clients left.

The `rlcfg` variable controls which functions cause a possible reset.
```c
static const unsigned int rlcfg;
```

It must be one, or a bitwise OR (`|`) of the following enumerators:
```c
enum {
    RLHookNone           = 0,
    RLHookUnmanage       = 1 << 0,
    RLHookToggleFloating = 1 << 1,
    RLHookSetFullscreen  = 1 << 2,
};
```

The `rlmap` array defines layout transitions. Elements whose `.from` is equal to
their `.to` are ignored. The first element whose `.from` is the arrange function
of the current layout and its `.to` is different is chosen.
```c
static const RLPair rlmap[] = {
    { .from = monocle, .to = tile },
    { .from = NULL,    .to = tile },
};
```

## patch files

### rlayout-6.8

```sh
git diff 6.8..rlayout-6.8 > dwm-rlayout-6.8.diff
```
