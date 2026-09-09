# cutils

Provides some basic singly-linked list operations for the `selmon->clients`.

## cmove

Moves `what` before `where` if `dir` is 0, otherwise it moves it after `where`.
Automatically adjusts `selmon->clients` if needed. It is null-safe, but
expects `what` and `where` to be in the same list.
```c
static void cmove(Client *what, Client *where, int dir);
```

## cprev

Finds the client before `c`. Returns `NULL` if the argument is `NULL`, there
is no client before `c`, or `c` is not in `selmon->clients`.
```c
static Client *cprev(Client *c);
```

## cprevtiled

Finds the first tiled client before `c`. Returns `NULL` if the argument is
`NULL`, there is no tiled client before `c`, or `c` is not in `selmon->clients`.
```c
static Client *cprevtiled(Client *c);
```

## cswap

Swaps `x` and `y`. Automatically adjusts `selmon->clients` if needed. It is
null-safe, but expects `x` and `y` to be in the same list.
```c
static void cswap(Client *x, Client *y);
```

## patch files

```sh
git diff 6.8..cutils-6.8 > dwm-cutils-6.8.diff
```
