# rlayout

Automatically resets the layout when there are no tiled clients left.

The `rlcfg` variable controls which functions cause a possible reset. It must be
a bitwise OR (`|`) of the following enumerators
```c
static const unsigned int rlcfg;
```

```c
static const RLMap rlmap[] = {
    { .from = monocle, .to = tile },
    { .from = NULL,    .to = tile },
};
```

## patch files

### rlayout-6.8

```sh
git diff 6.8..rlayout-6.8 > dwm-rlayout-6.8.diff
```
