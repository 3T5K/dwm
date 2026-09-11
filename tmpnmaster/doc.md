# tmpnmaster

Automatically decrements the monitor's `nmaster` when a client leaves the
master area.

The configuration is done through the following variables:
```c
static const int tmpnmdectil;
static const int tmpnmcfg;
```

The `tmpnmdectil` variable specifies the value at which the monitor's `nmaster`
should stop being decremented. If it's less than zero, it is treated as zero.

The `tmpnmcfg` variable is expected to be a bitwise OR (`|`) of the following
enumerators:
```c
enum {
    TmpNmRespectNmaster     = 1 << 0,
    TmpNmHookUnmanage       = 1 << 1,
    TmpNmHookToggleFloating = 1 << 2,
    TmpNmHookSetFullscreen  = 1 << 3,
};
```

If `tmpnmdectil` is not equal to the configuration's `nmaster`, then
`TmpNmRespectNmaster` can be used to set the monitor's `nmaster` to the
configuration's `nmaster` when the client leaving the master area is the last
tiled client.

The `Hook*` enumerators specify functions for which to enable the tmpnmaster
behavior.

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### tmpnmaster-6.8
```sh
git diff 6.8..tmpnmaster-6.8 > dwm-tmpnmaster-6.8.diff
```

### tmpnmaster-pertag-6.8
**Dependencies:**
- pertag-61bb8b2

```sh
git diff tmpnmaster-pertag-6.8^^..tmpnmaster-pertag-6.8 > dwm-tmpnmaster-pertag-6.8.diff
```
