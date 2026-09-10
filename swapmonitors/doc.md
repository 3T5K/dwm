# swapmonitors

Swaps states of two monitors. This is an extension of swapmonitors (4cd2832)
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

Example binds in `keys`:
```c
{ MODKEY|ShiftMask, XK_apostrophe, swapmon, {0} },
```

## swapmonitors

https://dwm.suckless.org/patches/swapmonitors/

## patch files

The diff does not include pertag. It must be patched separately.

```sh
git diff swapmonitors-pertag-6.8^^..swapmonitors-pertag-6.8 > dwm-swapmonitors-pertag-6.8.diff
```
