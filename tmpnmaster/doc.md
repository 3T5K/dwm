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

When using exstacks, if the stack area is expanded, tmpnmaster would no longer
decrement, and `nmaster` is not zero, then a client from the stack area would be
moved to the master area. The exstacks compatibility patches provide additional
policies to be set in `tmpnmcfg` that define which stack client should be moved.
```c
enum {
    TmpNmPrefer  = 1 << 4,
    TmpNmDefer   = 1 << 5,
    TmpNmNone    = 1 << 6,
    TmpNmAndDeny = 1 << 7,
    TmpNmNoMove  = 1 << 8,
};
```

Exactly one of `TmpNmPrefer`, `TmpNmDefer`, and `TmpNmNone` must be set.

`TmpNmPrefer` always moves the expanded client to the master area.

`TmpNmDefer` tries to defer moving the expanded client to the master area by
moving other stack clients instead. The expanded client is moved only when there
are no other clients to move.

`TmpNmNone` provides regular tmpnmaster behavior. Whether the expanded client
is moved depends on its position in the unexpanded stack area.

`TmpNmAndDeny` is a modifier that disallows moving the expanded stack client
when it would be selected as per the policies above.

`TmpNmNoMove` can only be set when both `TmpNmAndDeny` and `TmpNmPrefer` are
set. The prefer policy works by moving the expanded client within the monitor's
`clients` list in a way that puts it at the top of the stack area, were it not
expanded. This means that after unexpanding the stack area, the just-expanded
client might have been moved within the stack. `TmpNmNoMove` prevents this
repositioning.

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

### tmpnmaster-exstacks-xray-6.8

**Dependencies:**
- cutils-6.8
- tileinfo-exstacks-xray-6.8
- exstacks-xray-6.8
- xray-6.8

```sh
git diff tmpnmaster-exstacks-xray-6.8^^..tmpnmaster-exstacks-xray-6.8 > dwm-tmpnmaster-exstacks-xray-6.8.diff
```

### tmpnmaster-exstacks-pertag-xray-6.8

**Dependencies:**
- cutils-6.8
- tileinfo-exstacks-xray-6.8
- exstacks-pertag-xray-6.8
- pertag-61bb8b2
- xray-6.8

```sh
git diff tmpnmaster-exstacks-pertag-xray-6.8^^^..tmpnmaster-exstacks-pertag-xray-6.8 > dwm-tmpnmaster-exstacks-pertag-xray-6.8.diff
```
