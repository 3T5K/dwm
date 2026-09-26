# tileffb

Provides more intuitive focus fallback behavior for exstacks-patched dwm. 

This patch only works for the `tile` layout. When a different layout is active,
tileffb calls `tileffbfallback` if it is not `NULL`.
```c
static const TileFFBCallback tileffbfallback;
```

If the killed client was the last client on the tag, nothing happens.

If the killed client was floating, focus falls back to the previously focused
floating client. If such a client does not exist, the focus falls to the last
focused tiled client instead.

If the killed client was the expanded client of an area, the focus falls to the
last focused client in that area.

If the killed client was the last client in the stack area and the master area
is expanded, focus falls to the expanded master client.

Otherwise focus falls to the next client. If there is no next client, focus
falls to the previous client.

When using tmpnmaster, additional behavior is required when the killed client is
in the master area because killing it can decrement `nmaster`.

If the stack area is expanded and tmpnmaster will decrement `nmaster`, either
because a deny option is set or due to regular tmpnmaster semantics, focus falls
to the previous client if the killed client is at the bottom of the master area.
If there is no previous client, focus falls to the expanded client of the stack
area. If the killed client was not at the bottom of the master area, focus falls
to the next client.

If the stack area is expanded and an `nmaster` decrement would not occur,
meaning that a client from the stack area must be moved into the master area,
then, if the killed client was at the bottom of the master area, focus falls to
the client that would be moved as per the configured tmpnmaster policy.
Otherwise, focus falls to the next client.

If the stack area is not expanded and an `nmaster` decrement would occur, focus
falls to the previous client if the killed client was at the bottom of the
master area. If there is no previous client, focus falls to the next client.
If the killed client was not at the bottom of the master area, focus falls to
the next client.

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### tileffb-6.8

**Dependencies:**
- tileinfo-exstacks-xray-6.8
- exstacks-xray-6.8 or exstacks-pertag-xray-6.8
- pertag-61bb8b2 (required when using exstacks-pertag-xray-6.8)
- xray-6.8

```sh
git diff tileinfo-exstacks-xray-6.8..tileffb-6.8 > dwm-tileffb-6.8.diff
```

### tileffb-tmpnmaster-exstacks-xray-6.8

**Dependencies:**
- tmpnmaster-exstacks-xray-6.8 or tmpnmaster-exstacks-pertag-xray-6.8
- tileinfo-exstacks-xray-6.8
- exstacks-xray-6.8 (required when using tmpnmaster-exstacks-xray-6.8)
- exstacks-pertag-xray-6.8 (required when using tmpnmaster-exstacks-pertag-xray-6.8)
- pertag-61bb8b2 (required when using exstacks-pertag-xray-6.8)
- cutils-6.8
- xray-6.8

```sh
git diff tmpnmaster-exstacks-xray-6.8..tileffb-tmpnmaster-exstacks-xray-6.8 > dwm-tileffb-tmpnmaster-exstacks-xray-6.8.diff
```
