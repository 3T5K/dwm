# tileinfo

Queries the current state of dwm for information about the `tile` layout.

The following struct will be populated by `tileinfo`:
```c
typedef struct {
    Client *bs[2]; /* bs[0]: bottom master client bs[1]: bottom stack client */
    Client *ts[2]; /* ts[0]: top master client    ts[1]: top stack client    */
    Client *cs[2]; /* cs[0]: previous client      cs[1]: next client         */
    int     nc   ; /* client count                                           */
    int     si   ; /* selected client index (from 0)                         */
    int     st   ; /* is selected client in stack                            */
} TileInfo;
```

The `tileinfo` function assumes the layout is `tile` and does not return
prematurely if it isn't. It will collect the information regardless of the
layout. All pointers for which it can't find a client are set to `NULL`.
```c
static TileInfo tileinfo(void);
```

If using exstacks-xray or exstacks-pertag-xray, the exstacks-xray compatibility
patch can be used to collect exstacks-specific information. The following field
is added to the `TileInfo` struct.
```c
Client *xs[2]; /* xs[0]: master XrayAc xs[1]: stack XrayAc */
```

## patch files

The following diffs do not include their dependencies. Those must be applied
separately.

### tileinfo-6.8
```sh
git diff 6.8..tileinfo-6.8 > dwm-tileinfo-6.8.diff
```

### tileinfo-exstacks-xray-6.8
**Dependencies:**
- exstacks-xray-6.8 or exstacks-pertag-xray-6.8
- pertag-61bb8b2 (required when using exstacks-pertag-xray-6.8)
- xray-6.8

```sh
git diff exstacks-xray-6.8..tileinfo-exstacks-xray-6.8 > dwm-tileinfo-exstacks-xray-6.8.diff
```
