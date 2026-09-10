# cyclebetween

Allows cycling an array of `const Layout *` like a ring. It's a generalization
of a toggle. The last member in the array must be `NULL`. Provides enumerators
representing indices for the three default layouts.
```c
static const Layout *layoutring[] = {
    &layouts[LtTile],
    &layouts[LtMonocle],
    NULL
};
```

Is null-safe, returns early if `arg` or `arg->v` is `NULL`. Passing an array
without a null pointer in it causes UB.
```c
static void cyclebetween(const Arg *arg);
```

Example binding in `keys`:
```c
{ MODKEY, XK_c, cyclebetween, {.v = layoutring } },
```

## patch files

### cyclebetween-6.8
```sh
git diff 6.8..cyclebetween-6.8 > dwm-cyclebetween-6.8.diff
```
