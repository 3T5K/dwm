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
{ MODKEY|ShiftMask, XK_apostrophe, swapmon, {0} },
```

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
