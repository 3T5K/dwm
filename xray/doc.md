# xray

A basic window X-raying framework for dwm.

The basic idea is to organize clients into groups. The `Client` struct has two
new fields: `xraygrp` and `xraystat`. Both are of type `int`. A client `c` is
considered to be part of a group `G` if `ISVISIBLE(c) && c->xraygrp == G` is
true. A group is a non-negative integer. The `xraystat` field must be either
`XrayAc` or `XrayIn` when `xraygrp` is not `XrayGrpNone`. `XrayAc` represents
the active client of a group. This is intended to be the client that is
physically visible. All other clients should have their `xraystat` set to
`XrayIn` and are referred to as inactive clients. Clients that should not
participate in X-raying should have their group set to `XrayGrpNone`.

All new clients have their group set to `XrayGrpNone` in `manage`. `focus` and
`unfocus` are instrumented with `xrayfocus` and `xrayunfocus` respectively.
Layout functions and functions that put clients in "special" states, such as
floating or fullscreen, must account for X-ray information. If they do not
intend to implement X-raying, they must set the X-ray group of the relevant
client(s) to `XrayGrpNone`.

By default, the following functions are instrumented to set the group of the
relevant clients to `XrayGrpNone`:
- `arrangemon`
- `manage`
- `monocle`
- `setfullscreen`
- `tile`
- `togglefloating`

## xraycfg

Sets the X-ray state of a client. If `grp` is `XrayGrpNone`, `stat` is ignored
and `c->xraystat` is set to zero. Does nothing if `c` is `NULL`.

```c
static void xraycfg(Client *c, int grp, int stat);
```

## xrayac

Finds the active client of a group. Does nothing if `mon` is `NULL`. Does not
treat `XrayGrpNone` as special.

```c
static Client *xrayac(Monitor *mon, int grp);
```

## xrayacs

Finds the active client of a group while skipping `skip`. Does nothing if `mon`
is `NULL`. Does not treat `XrayGrpNone` as special. If `skip` is `NULL`, it
behaves like `xrayac`.

```c
static Client *xrayacs(Monitor *mon, int grp, Client *skip);
```

## xrayapply

Accesses the array `xrayhandlers` at index `c->xraygrp`. If the value is not
`NULL`, it is invoked with `c`. Does nothing if `c` is `NULL`.

```c
static void xrayapply(Client *c);
```

## xraydefhandler

Moves an inactive client off-screen or brings an active client into view using
`XMoveWindow`. It does not modify or save `c->x` and `c->y`. When bringing a
client back into view, `c->x` and `c->y` are expected to contain the original
coordinates. It is undefined behavior if `c` is `NULL`.

```c
static void xraydefhandler(Client *c);
```

## xrayfallback

Finds the first client in the focus stack of the selected monitor
(`selmon->stack`) that belongs to `grp` and makes it active. It does not check
whether another client in that group is already active. Does nothing if `grp` is
`XrayGrpNone`.

```c
static void xrayfallback(int grp);
```

## xrayfocus

Makes `c` the active client of its group. If there are any other active clients
in the group, they are made inactive. `xrayapply` is called to propagate
changes. Does nothing if `c` is `NULL` or `c->xraygrp` is `XrayGrpNone`.

```c
static void xrayfocus(Client *c);
```

## xraymaybefallback

Checks if there is an active client within `grp`. If there is not, it invokes
`xrayfallback`. Does nothing if `grp` is `XrayGrpNone`.

```c
static void xraymaybefallback(int grp);
```

## xrayunfocus

Makes `c` inactive if another client in its group is active. Does nothing if `c`
is `NULL` or `c->xraygrp` is `XrayGrpNone`.

```c
static void xrayunfocus(Client *c);
```

## xrayhandlers

Stores callbacks for `xrayapply` corresponding to individual groups. Every
existing group must have an entry in this array at an index equal to its group
number.

```c
static void (*xrayhandlers[N])(Client *);
```

## patch files

### xray-6.8

```sh
git diff 6.8..xray-6.8 > dwm-xray-6.8.diff
```
