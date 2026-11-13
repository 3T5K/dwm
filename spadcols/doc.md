# spadcols

Lets you customize the scratchpad border color.

It can be specified in the `colors` array. The first field is the active color,
second is the inactive color, and the third field is ignored. Despite that, it
**must** be set a to valid color string.

```c
static const char *colors[][3] = {
    [SchemeSpad] = { "#ff0000", "#00ff00", "#000000" },
};
```

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### spadcols-6.8

**Dependencies:**
- scratchpads-6.8

```sh
git diff scratchpads-6.8..spadcols-6.8 > dwm-spadcols-6.8.diff
```

### spadcols-noborderflicker-6.8

**Dependencies:**
- noborderflicker-scratchpads-6.8
- scratchpads-6.8

```sh
git diff noborderflicker-scratchpads-6.8..spadcols-noborderflicker-6.8 > dwm-spadcols-noborderflicker-6.8.diff
```
