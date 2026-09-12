# rstacks

Automatically unexpands stacks after all clients within them are removed.

The configuration is done through the following variable:
```c
static const int rstackscfg;
```

The configuration must be a bitwise OR (`|`) of the following enumerators:
```c
enum {
    RStacksMasterReset = 1 << 0,
    RStacksMasterKeep  = 1 << 1,
    RStacksStackReset  = 1 << 2,
    RStacksStackKeep   = 1 << 3,
};
```

One, but not both, of `RStacksMasterReset` and `RStacksMasterKeep` must be set.
The same applies to `RStacksStackReset` and `RStacksStackKeep`.

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### rstacks-6.8

**Dependencies:**
- exstacks-6.8 or exstacks-xray-6.8
- xray-6.8 (required when using exstacks-xray-6.8)

```sh
git diff exstacks-6.8..rstacks-6.8 > dwm-rstacks-6.8.diff
```

### rstacks-pertag-6.8

**Dependencies:**
- exstacks-pertag-6.8 or exstacks-pertag-xray-6.8
- pertag-61bb8b2
- xray-6.8 (required when using exstacks-pertag-xray-6.8)

```sh
git diff exstacks-pertag-6.8..rstacks-pertag-6.8 > dwm-rstacks-pertag-6.8.diff
```
