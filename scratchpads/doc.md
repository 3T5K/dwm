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
specifies that gap. If there are no tiled clients yet, or they are all in one
area, the rule will behave like `SP_R_GAP(g)`, where `g` is `o`.

Layouts like `monocle` don't use `mfact` meaningfully. To specify a constant for
such layouts, use the `sp_constant_mfacts` array. If the provided mfact is 1,
the behavior is akin to `SP_R_GAP(g)` as specified above. Similarly to the macro
arguments, unreasonable `mfact`s will be blindly accepted.
```c
static SpConstMfact sp_constant_mfacts[] = {
    { monocle, 1.0 },
    { NULL,    1.0 },
};
```

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
client if it wasn't yet, or it has been killed. The `SP_CMD` macro should be
used. To specify arguments of the command, do so as separate string arguments of
the macro, i.e. `SP_CMD("command", "--some-flag", "arg")`.

## functions

```c
static void sp_toggle(const Arg *arg);
static void sp_focus(const Arg *arg);
```

`sp_toggle` toggles the visibility of a scratchpad. It expects a string argument
corresponding to a name of one of the scratchpads. If the scratchpad is toggled
for the first time, or it was previously killed, the command defined in the
respective scratchpad is executed.

`sp_focus` focuses a scratchpad if one is visible and not in focus. If there is
no visible scratchpad, focus doesn't change. If the focused client is a
scratchpad, it focuses the last focused non-scratchpad client. It does not
process the parameter.

Example bindings in keys:
```c
{ MODKEY, XK_z, sp_toggle, { .v = "st1" } },
{ MODKEY, XK_x, sp_toggle, { .v = "st2" } },
{ MODKEY, XK_v, sp_focus,  {0} },
```

## appearence

The border size of scratchpads is controlled by the following variable:
```c
static const unsigned int sp_borderpx = 1;
```

The border color can be specified in the `colors` array. The first field is the
active color, second is the inactive color, and the third field is ignored.
Despite that, it **must** be set a to valid color string.
```c
static const char *colors[][3] = {
    [SchemeSpad] = { "#ff0000", "#00ff00", "#000000" },
};
```


## other configuration

```c
static const int sp_exclusive_across_mons = 1;
static const int sp_permit_fullscreen = 1;
```

If monitor A has scratchpad X visible on it, toggling scratchpad Y on monitor B
will cause X to be hidden on A if `sp_exclusive_across_mons` is non-zero.

## patch files

### scratchpads-6.8

```sh
git diff 6.8..scratchpads-6.8 > dwm-scratchpads-6.8.diff
```
