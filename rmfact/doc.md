# rmfact

Resets the selected monitor's `mfact` to the the configuration's `mfact` after
either all master clients or all stack clients are killed.

The configuration is done through the following variable:
```c
static const int rmfactcfg;
```

`rmfactcfg` is expected to be a bitwise OR (`|`) of the following enumerators:
```c
enum {
    RMfactResetOnEmptyMaster = 1 << 0,
    RMfactResetOnEmptyStack  = 1 << 1,
    RMfactHookArrangeMon     = 1 << 2,
    RMfactHookTile           = 1 << 3,
};
```

`RMfactHookArrangeMon` and `RMfactHookTile` are mutually exclusive.

## patch files

### rmfact-6.8

```sh
git diff 6.8..rmfact-6.8 > dwm-rmfact-6.8.diff
```
