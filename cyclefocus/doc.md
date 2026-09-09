# cyclefocus

Cycles floating or tiled windows individually. Respects `lockfullscreen`. The
argument is expected to be `<direction> | <client-kind>`. Supports wrap-around,
just like `focusstack`. `CyclePrev` goes forward in `selmon->clients`, while
`CycleNext` goes backward. Unless changed, this means `CyclePrev` would focus a
client spawned before `selmon->sel`, while `CycleNext` would focus one spawned
after it. Attempts to focus a window of a kind different from `selmon->sel`
causes focus to fall back on the last focused window of the desired kind.

All arguments:
```c
enum {
    CycleNext     = 1 << 0,
    CyclePrev     = 1 << 1,
    CycleFloating = 1 << 2,
    CycleTiled    = 1 << 3,
};
```

Example binds in `keys`:
```c
{ MODKEY,           XK_w, cyclefocus, {.i = CyclePrev | CycleTiled    } },
{ MODKEY,           XK_e, cyclefocus, {.i = CycleNext | CycleTiled    } },
{ MODKEY|ShiftMask, XK_w, cyclefocus, {.i = CyclePrev | CycleFloating } },
{ MODKEY|ShiftMask, XK_e, cyclefocus, {.i = CycleNext | CycleFloating } },
```

## patch files

```sh
git diff 6.8..cyclefocus-6.8 > dwm-cyclefocus-6.8.diff
```
