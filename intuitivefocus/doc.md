# intuitivefocus

Changes focus fallback behavior after killing a client. The default dwm behavior
is to fall back on the last focused client. The new behavior depends on whether
the client killed is floating or tiled.

If it's floating, focus will fall back to the last focused floating client. If
there are none, it will fall back to the last focused tiled client.

For tiled clients, the next tiled client will be found in the relevant monitor's
`clients` list. If one can't be found, the closest tiled client before the
selected one in that list will be focused. If there are no more tiled clients,
but there are floating ones, focus will fall back to the last focused floating
client.

With the tile layout, imagine placing the master area clients on top of the
stack clients. Killing a tiled client will cause focus to fall to the client
below it. If there isn't one, it will go to the one above.

```
+----------+----------+
|    m1    |    s1    |
|----------|----------|
|    m2    |    s2    |
|----------|----------|
|    m3    |    s3    |
+----------+----------+
```

Suppose s1 is focused and clients start being killed until there aren't any
more. Focus would fall back as follows: s1 -> s2 -> s3 -> m3 -> m2 -> m1.

## patch files

```sh
git diff 6.8..intuitivefocus-6.8 > dwm-intuitivefocus-6.8.diff
```
