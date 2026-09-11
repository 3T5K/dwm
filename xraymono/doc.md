# xraymono

Implements window X-raying for the `monocle` layout.

The `monocle` layout normally stacks all clients in front of each other. When
using transparent windows, this means you can see other clients behind the
selected one, provided there are any. This patch moves them out of the way,
giving you an unobstructed view of the background.

## patch files

### xraymono-6.8

**Dependencies:**
- xray-6.8

```sh
git diff xray-6.8..xraymono-6.8 > dwm-xraymono-6.8.diff
```
