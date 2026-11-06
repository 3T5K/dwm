# scratchpads

Scratchpads are clients whose visibility can be toggled from any tag. They are
shared between monitors. Only one can be visible at a time.

## definition

Scratchpads are defined in the `sp_defs` array.
```c
static const Spad sp_defs[] =
{
    // name   close      rules                          match     cmd
    {  "st1", SP_C_NONE, SP_R_XYWH(0.1, 0.1, 0.8, 0.8), SP_M_PID, SP_CMD("st") },
};
```

The `name` field is a string that identifies the scratchpad. This name is passed
to `sp_toggle`.

The `close` field specifies events which cause the hiding of a visible
scratchpad. This does not kill the scratchpad client. It must be one, or a
bitwise OR (`|`), of the following enumerators:
  - `SP_C_NONE`: Meant to be used when this behavior is not desired.
  - `SP_C_NEW`: Hides when a new client is spawned.
  - `SP_C_LAYOUT`: Hides when the layout is switched.
  - `SP_C_CLOSE`: Hides when a different client is killed.

The `rules` field specifies the position and size of the scratchpad. One of the
following macros should be used to specify this.

### `SP_R_TILE`
This causes the scratchpad to be tiled. Its dimensions are therefore dictated by
the respective layout function. All other rules make the scratchpad floating.

### `SP_R_GAP(g)`
Adds an outer gap of `g` pixels around the scratchpad.

### `SP_R_GAPS(t, b, l, r, w, h)`
`t`, `b`, `l`, and `r` define the top, bottom, left, and right gaps in pixels
respectively. The `w` and `h` parameters are `float` factors of the width and
height available for clients on the monitor. For example, making `w` 0.5 and `l`
and `r` 0, will cause the scratchpad to be half of the screen wide.

### `SP_R_MFACT(o, i, d)`

### `SP_R_XYWH(x, y, w, h)`

## functions

## appearence

## other configuration

## patch files

### scratchpads-6.8

```sh
git diff 6.8..scratchpads-6.8 > dwm-scratchpads-6.8.diff
```
