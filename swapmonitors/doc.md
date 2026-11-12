# swapmonitors

Swaps the states of two monitors. This is an extension of swapmonitors (4cd2832)
with support for pertag and swapping some additional fields.

Swapped fields:
- `lt`
- `tagset`
- `clients`
- `stack`
- `pertag`
- `sel`
- `showbar`
- `topbar`
- `mfact`
- `nmaster`
- `sellt`

Example binding in `keys`:
```c
{ MODKEY|ShiftMask, XK_grave, swapmon, {.i = +1} },
```

When the argument to `swapmon` is an integer greater than 0, the focused monitor
is swapped with the next monitor. If there is not one, the previous one is used
instead. If the argument is less than, or equal to 0, the focused monitor is
swapped with the previous monitor. If there is not one, the next one is used
instead.

## swapmonitors

https://dwm.suckless.org/patches/swapmonitors/

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### swapmonitors-pertag-6.8

**Dependencies:**
- pertag-61bb8b2

```sh
git diff swapmonitors-pertag-6.8^^..swapmonitors-pertag-6.8 > dwm-swapmonitors-pertag-6.8.diff
```

### swapmonitors-pertag-barpadding-6.8

**Dependencies:**
- pertag-61bb8b2
- barpadding-6.6

```sh
git diff swapmonitors-pertag-barpadding-6.8^^^..swapmonitors-pertag-barpadding-6.8 > dwm-swapmonitors-pertag-barpadding-6.8.diff
```
