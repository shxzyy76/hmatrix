# HMatrix configuration guide

`hmatrix.toml` in this directory is the default configuration. It lists every
supported option at its default value, with a comment above each one. This
guide explains how to configure each of them.

## Setup

HMatrix looks for its config in this order and uses the first file found:

1. `~/.config/hmatrix/hmatrix.toml`
2. `config/hmatrix.toml` in the source tree (found relative to the
   executable, so running `./build/hmatrix` uses this very file), or
   `share/hmatrix/hmatrix.toml` after `cmake --install`

So while developing you can simply edit this file and run `./build/hmatrix`.
To keep a personal config that overrides it, copy it to `~/.config`:

    mkdir -p ~/.config/hmatrix
    cp hmatrix.toml ~/.config/hmatrix/
    $EDITOR ~/.config/hmatrix/hmatrix.toml

To use a different file, pass it on the command line:

    hmatrix --config ./cyan.toml

Changes apply the next time you start hmatrix. There is no live reload.

## Rules

- Every option is optional. A missing option uses its default, so a config
  can contain only the lines you want to change.
- An invalid value, unknown option or unknown section prints a warning on
  stderr and the default is used instead. hmatrix still starts.
- A file that is not valid TOML is ignored entirely, with a warning.
- If `--config` points to a file that cannot be opened, hmatrix exits with an
  error. If none of the default files exist, built-in defaults are used.
- Options must be under their own `[section]` header.

If a setting seems to have no effect, check your terminal after quitting for
warnings.

---

## `[general]`

### `fps`

How many frames are drawn per second. Lower values use less CPU, higher
values look smoother.

- Accepted: integer 1-240
- Default: `30`

```toml
[general]
fps = 15    # light on CPU
```

### `transparent_background`

Whether hmatrix leaves your terminal's own background alone.

- `true`: only the characters are drawn. Terminal transparency, blur and
  wallpapers stay visible.
- `false`: the whole screen is filled with solid black behind the characters.
- Accepted: `true` or `false`
- Default: `true`
- Limitation: has no effect on terminals without color support.

```toml
[general]
transparent_background = false
```

---

## `[characters]`

### `chars`

The set of characters the rain is made of. Every position picks one at
random from this string.

- Accepted: any string of 1-256 characters, each exactly one terminal cell
  wide: letters, digits, symbols, half-width katakana, block shapes, Greek,
  and so on. A space is allowed and leaves gaps in the rain.
- Default: `"01"` (binary)
- Repeating a character makes it more likely, so `"0001"` shows roughly
  three times as many zeros as ones.
- Non-ASCII characters need a UTF-8 terminal and locale (`locale charmap`
  should print `UTF-8`).
- Double-width characters (CJK ideographs, emoji) and control characters are
  rejected, and the default `"01"` is used with a warning.

```toml
[characters]
chars = "0123456789ABCDEF"   # hex digits
# chars = "01"               # binary (default)
# chars = "0001"             # mostly zeros
# chars = "#*+"              # symbols
# chars = "ｱｲｳｴｵｶｷｸｹｺ"     # half-width katakana
# chars = "░▒▓"            # shades
# chars = "λπΣΩ"           # Greek
```

### `change_rate`

How often characters that are already on screen change into a new random
one, which gives the shimmer effect. It is measured in changes per
character per second.

- Accepted: number 0-20
- Default: `0.6`
- `0` means a character never changes after it is created. Higher values
  make the rain flicker more.

```toml
[characters]
change_rate = 0      # static characters
# change_rate = 3    # busy, flickering
```

---

## `[appearance]`

### Color values

The three color options below accept either:

- a basic color name: `black`, `red`, `green`, `yellow`, `blue`, `magenta`,
  `cyan`, `white`. These use your terminal theme's palette.
- a hex value in quotes: `"#rrggbb"`, for example `"#ff00ff"`.
- a 256-color palette number 0-255, unquoted, for example `201`. Numbers
  0-15 follow your terminal theme; 16-255 are fixed colors.

Hex values are snapped to the nearest color in the 256-color palette. On
terminals with fewer than 256 colors they fall back to the nearest of the 8
basic colors, and the trail is always dimmed. With no color support at all,
colors are ignored and only bold and dim are used.

A stream is drawn in three parts: the head (first character), the front half
of the trail, and the back half of the trail.

### `head_color`

Color of the leading character of each stream.

- Default: `"#00ff00"` (bright green)

### `color`

Color of the front half of each trail.

- Default: `"#00af00"` (green)

### `trail_color`

Color of the back half of each trail, the fading end.

- Default: `"#005f00"` (dark green)

```toml
[appearance]
head_color  = "#00ffff"
color       = "#00afaf"
trail_color = "#005f5f"      # cyan rain

# Using theme color names instead of hex:
# head_color  = "white"
# color       = "green"
# trail_color = "green"
```

### `bold_head`

Draw the leading character in bold, which most terminals show brighter.

- Accepted: `true` or `false`
- Default: `true`

```toml
[appearance]
bold_head = false
```

### `dim_trail`

Also apply the terminal's "dim" attribute to the trail, making it fainter
on top of its color.

- Accepted: `true` or `false`
- Default: `false`
- On terminals with fewer than 256 colors the trail is dimmed regardless of
  this setting, because dimming is the only way to darken it there.

```toml
[appearance]
dim_trail = true
```

---

## `[animation]`

### `speed_min` and `speed_max`

Each stream gets a random fall speed between these two values, in terminal
rows per second.

- Accepted: number 0.5-200 for both
- Defaults: `speed_min = 6`, `speed_max = 30`
- `speed_min` must not be greater than `speed_max`. If it is, both revert to
  their defaults and a warning is printed.
- Set both to the same value for uniform speed.

```toml
[animation]
speed_min = 3
speed_max = 10     # slow rain

# speed_min = 15
# speed_max = 15   # every stream falls at the same speed
```

### `length_min` and `length_max`

Each stream gets a random length between these two values, counted in
characters including the head.

- `length_min`: integer 1-1000, default `6`
- `length_max`: integer 0-1000, default `0`
- `length_max = 0` means "the terminal's height", so the longest streams can
  span the whole screen and adapt when you resize.
- A non-zero `length_max` must not be smaller than `length_min`. If it is,
  both revert to their defaults and a warning is printed.
- If `length_min` is larger than the terminal height, streams are simply
  longer than the screen.

```toml
[animation]
length_min = 4
length_max = 12     # short trails

# length_min = 10
# length_max = 10   # every stream the same length
```

### `density`

The fraction of columns that carry a stream at any moment. `1.0` fills every
column. Lower values leave gaps, giving a sparser rain.

- Accepted: number 0.05-1.0
- Default: `1.0`

```toml
[animation]
density = 0.3
```

---

## `[behavior]`

### `initial_scatter`

How the animation starts, including after pressing `R` or resizing the
terminal.

- `true`: streams are already spread over the whole screen.
- `false`: all streams start above the screen and fall in.
- Accepted: `true` or `false`
- Default: `true`

```toml
[behavior]
initial_scatter = false
```

### `start_paused`

Start with the animation paused. Press `P` to begin.

- Accepted: `true` or `false`
- Default: `false`

```toml
[behavior]
start_paused = true
```

---

## Complete examples

Cyan, sparse, slow rain:

```toml
[appearance]
head_color  = "#00ffff"
color       = "#00afaf"
trail_color = "#005f5f"

[animation]
density   = 0.3
speed_min = 3
speed_max = 10
```

Hex digits on a solid black background, low CPU:

```toml
[general]
fps = 15
transparent_background = false

[characters]
chars = "0123456789ABCDEF"
change_rate = 1
```

Red rain with short trails that fall in from the top:

```toml
[appearance]
head_color  = "#ff5f5f"
color       = "#af0000"
trail_color = "#5f0000"

[animation]
length_min = 4
length_max = 10

[behavior]
initial_scatter = false
```

## All options at a glance

| Section | Option | Accepted | Default |
|---------|--------|----------|---------|
| `[general]` | `fps` | integer 1-240 | `30` |
| `[general]` | `transparent_background` | `true` / `false` | `true` |
| `[characters]` | `chars` | any 1-256 single-width characters (UTF-8) | `"01"` |
| `[characters]` | `change_rate` | number 0-20 | `0.6` |
| `[appearance]` | `head_color` | color name, `"#rrggbb"` or 0-255 | `"#00ff00"` |
| `[appearance]` | `color` | color name, `"#rrggbb"` or 0-255 | `"#00af00"` |
| `[appearance]` | `trail_color` | color name, `"#rrggbb"` or 0-255 | `"#005f00"` |
| `[appearance]` | `bold_head` | `true` / `false` | `true` |
| `[appearance]` | `dim_trail` | `true` / `false` | `false` |
| `[animation]` | `speed_min` | number 0.5-200 | `6` |
| `[animation]` | `speed_max` | number 0.5-200 | `30` |
| `[animation]` | `length_min` | integer 1-1000 | `6` |
| `[animation]` | `length_max` | integer 0-1000 (0 = terminal height) | `0` |
| `[animation]` | `density` | number 0.05-1.0 | `1.0` |
| `[behavior]` | `initial_scatter` | `true` / `false` | `true` |
| `[behavior]` | `start_paused` | `true` / `false` | `false` |
