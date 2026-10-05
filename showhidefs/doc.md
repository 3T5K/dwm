# showhidefs

The function `showhide` doesn't reposition fullscreen clients. This prevents
`sendmon` from visually moving the sent client to the respective monitor.
A Git blame reveals that this was added in 2011 in c14d293e. It appears it was
necessary for mplayer.

## patch files

### showhidefs

```sh
git diff 6.8..showhidefs-6.8 > dwm-showhidefs-6.8.diff
```
