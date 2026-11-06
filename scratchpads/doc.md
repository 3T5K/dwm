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
following macros should be used to specify this. Do note that no arguments of
these macros are checked and/or clamped automatically. Unreasonable values will
therefore be accepted.

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
This makes the width of the scratchpad relative to the monitor's `mfact`,
similarly to clients in the `tile` layout. The `d` parameter can be one of
`SP_R_MFACT_M` and `SP_R_MFACT_S`. It specifies whether the scratchpad will
appear in the master or the stack area. The `o` parameter specifies the outer
gap in pixels around the scratchpad. That's the gap on every side except the one
where the split between the master and stack area is. The `i` parameter
specifies that gap.

### `SP_R_XYWH(x, y, w, h)`
All parameters are `float` factors of the available width and height
respectively. If `ww` is maximum width of a client in pixels and `wh` is the
height, then a scratchpad's `(x, y, width, height)` will be
`(ww * x, wh * y, ww * w, wh * h)`.

The `match` field specifies how to identify a client to associate with a
scratchpad. This happens when a scratchpad hasn't been opened yet, or it was
killed. It must be one of the following macros:
  - `SP_M_PID`
  - `SP_M_CLASS(class)`
  - `SP_M_INSTANCE(instance)`
  - `SP_M_TITLE(title)`
`class`, `instance`, and `title` must be non-empty strings.

The `cmd` field specifies the command `sp_toggle` will use to spawn a scratchpad
client if it wasn't yet, or it has been killed. It should be the `SP_CMD` macro.
To specify arguments of the command, do so as separate string arguments of the
macro, i.e. `SP_CMD("command", "--some-flag", "arg")`.

## functions

## appearence

## other configuration

## patch files

### scratchpads-6.8

```sh
git diff 6.8..scratchpads-6.8 > dwm-scratchpads-6.8.diff
```
