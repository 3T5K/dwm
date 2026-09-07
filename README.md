# dwm 6.8

This is a restructured tree based on dwm-6.8 meant to serve as a base for
patched dwm trees. To use it, run `git switch -c my-dwm-tree base-6.8`.

## Adding patches

When adding a patch file for archival purposes, I recommend placing it in a
`patches` directory. After adding the file, track it with Git by running:
```sh
git add patches/somepatch-ref.diff
git commit -m 'added patch: somepatch (ref)'
```

There is a target in the Makefile which will filter the history of the current
branch for all commits whose messages start with "added patch:". If you care
for this, make sure each commit that makes Git track a patch file has a message
starting with that string and doesn't do anything else, i.e. does not apply the
patch or change configuration.
```sh
make get-added
```

## Applying patches

Apply patches as you please, but make sure to patch `src` as that's where the
source files are, obviously. All files in `src` ending with `.orig` or `.rej`
are ignored by Git.

There is a similar mechanism as for adding patches. Prefixing a commit message
with "applied patch:" will let you use a make target to filter the history for
commits beginning with it. Similarly, such commits should have the single
responsibility of applying the patch and nothing more.
```sh
make get-applied
```

## Configuring

Do not treat the `src` directory as your configuration. Create a directory named
`config` at the root of the repo, where the Makefile is. Files in this directory
should serve as your configuration.

It is searched recursively, and all files are included in lexicographical order.
The Makefile causes a rebuild if any file in the config directory changes, is
added, or is removed. Avoid putting spaces in the file or subdirectory names.

If the config directory is empty, dwm will instead fall back to
`src/config.def.h`. As such, this file should only be modified by patches adding
their default configuration. `src/config.h` is treated as a build artifact and
is ignored by Git.

For consistency, commits corresponding to configuration changes can have their
messages prefixed with "config:" and be queried with:
```sh
make get-config
```

There is also a `.nvim.lua` file which will stop the attached LSP client for
any files in `config`.

## Removing and reversing patches

If you're using the commit message prefixes, do not add commits that reverse
patches or remove archived patch files. The Makefile will **not** magically
subtract commits from the output of `get-applied` or `get-added` because you
have a commit which is meant to reverse it.

In other words, if there is a commit with a message like
"applied patch: somepatch (version)", `get-applied` would show that commit even
if there is a later commit with a message like
"reversed patch: somepatch (version)". The same goes for `get-added`.

To reverse a patch you just applied, do a Git reset. To reverse an older commit,
rebase the branch. This will keep the commit history clean and representative of
what's actually applied or archived.

## Building

This tree is now an out-of-source build. A directory called `build` will be made
at the repo's root. All object files, executables, and tarballs will be placed
there.

To build dwm:
```sh
make build
```

To install dwm and its man page into `/usr/local/*`:
```sh
make install
```
