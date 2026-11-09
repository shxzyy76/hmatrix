# HMatrix

A lightweight terminal digital-rain animation in C17 and ncurses. It shows
only the characters `0` and `1` by default, falling in green streams over
your terminal's own background, so transparency and wallpapers stay visible.

## Features

- Vertical streams with random speed, length and starting position
- Bright head, fading green trail, drawn directly over the terminal background
- Fully configurable through a documented TOML file (colors, characters, speed, density, FPS, ...)
- Configurable frame rate for low CPU use; no allocation inside the animation loop
- Handles terminal resizing, tiny terminals, and terminals with few or no colors
- Clean terminal restore on quit, `Ctrl-C` and `SIGTERM`

## Dependencies

- GCC (or any C17 compiler), CMake >= 3.10
- ncurses development files (`ncurses` on Arch, `libncurses-dev` on Debian/Ubuntu, `ncurses-devel` on Fedora)
- [tomlc17](https://github.com/cktan/tomlc17) (MIT) for TOML parsing. It is vendored in `third_party/tomlc17/`, so there is nothing extra to install.

## Installation (Linux)

### 1. Install the build tools and ncurses

| Distribution | Command |
|--------------|---------|
| Arch / CachyOS / Manjaro | `sudo pacman -S --needed base-devel cmake ncurses` |
| Debian / Ubuntu / Mint | `sudo apt install build-essential cmake libncurses-dev` |
| Fedora / RHEL | `sudo dnf install gcc cmake ncurses-devel` |
| openSUSE | `sudo zypper install gcc cmake ncurses-devel` |

You need a C17 compiler (GCC), CMake 3.10 or newer, and the ncurses
(wide-character) development files.

### 2. Get the source and build

    git clone https://github.com/<your-username>/hmatrix.git
    cd hmatrix
    cmake -B build
    cmake --build build

### 3. Run it

    ./build/hmatrix

This works straight from the source tree and uses the `config/hmatrix.toml`
shipped with it.

### 4. Install (optional)

System-wide, to `/usr/local`:

    sudo cmake --install build

Or only for your user, to `~/.local` (make sure `~/.local/bin` is in your `PATH`):

    cmake --install build --prefix ~/.local

This installs the `hmatrix` binary to `bin/` and the sample config to
`share/hmatrix/`. Then run `hmatrix` from anywhere.

To uninstall, remove the installed files (use `sudo` for a system-wide install):

    xargs rm < build/install_manifest.txt

### 5. Personal config (optional)

The config in the source tree (or in `share/hmatrix/` once installed) is used
by default. To keep your own settings, which take priority over it, copy it to
your home directory and edit the copy:

    mkdir -p ~/.config/hmatrix
    cp config/hmatrix.toml ~/.config/hmatrix/
    $EDITOR ~/.config/hmatrix/hmatrix.toml

See the [configuration reference](#configuration-reference) below.

### Troubleshooting

- **`Could NOT find Curses`**: the ncurses development package is missing;
  install it as in step 1.
- **Non-ASCII characters show as `?`**: your terminal and locale must be UTF-8.
  Check with `locale charmap`, which should print `UTF-8`.
- **Colors look wrong**: use a 256-color terminal (`echo $TERM` should contain
  `256color`, for example `xterm-256color`).

## Usage

    hmatrix                          # uses the default lookup described below
    hmatrix --config ./my.toml       # use a specific file
    hmatrix --help
    hmatrix --version

### Command-line options

| Option | Description |
|--------|-------------|
| `--config PATH` | Read configuration from `PATH` (`--config=PATH` also works). It is an error if the file cannot be opened. |
| `--help`, `-h` | Show usage and exit. |
| `--version` | Show the version and exit. |

If `--config` is not given, the first file found among these is used:

1. `~/.config/hmatrix/hmatrix.toml`
2. `config/hmatrix.toml` next to the build (relative to the executable, so
   `./build/hmatrix` uses the project's own file from any directory), or
   `share/hmatrix/hmatrix.toml` after `cmake --install`

If none exists, built-in defaults apply (the classic green binary rain).

### Controls

| Key | Action |
|-----|--------|
| `Q` or `Esc` | Quit |
| `P` | Pause / resume |
| `R` | Restart the animation |

Resizing the terminal restarts the animation at the new size.

## Configuration reference

The file is TOML. Every option is optional; a missing option uses its default.
An invalid value, unknown option or unknown section prints a warning to
stderr and the default is used instead. A file that is not valid TOML is
ignored entirely (with a warning). The fully commented file is
[`config/hmatrix.toml`](config/hmatrix.toml).

### `[general]`

| Option | Accepted | Default | Description |
|--------|----------|---------|-------------|
| `fps` | integer 1-240 | `30` | Target frames per second. Lower = less CPU. |
| `transparent_background` | `true` / `false` | `true` | `true` keeps the terminal's own background and transparency. `false` fills the screen with solid black. No effect on terminals without color. |

### `[characters]`

| Option | Accepted | Default | Description |
|--------|----------|---------|-------------|
| `chars` | any 1-256 single-width characters (UTF-8) | `"01"` | Your own character set, e.g. `"ｱｲｳｴｵ"`, `"░▒▓"`, `"0123456789ABCDEF"`. Repeating one makes it more likely (`"0001"`). Double-width characters (CJK, emoji) are rejected. |
| `change_rate` | number 0-20 | `0.6` | Changes per on-screen character per second (shimmer). `0` = characters never change. |

### `[appearance]`

Colors are a name (`black`, `red`, `green`, `yellow`, `blue`, `magenta`,
`cyan`, `white`; these follow your terminal theme), hex `"#rrggbb"`, or a 256-color palette number `0`-`255` (unquoted, e.g.
`201`). Hex
values are snapped to the nearest color of the 256-color palette. With fewer
than 256 colors they fall back to the nearest of the 8 basic colors, and the
trail is always dimmed.

| Option | Accepted | Default | Description |
|--------|----------|---------|-------------|
| `head_color` | name, hex or 0-255 | `"#00ff00"` | Leading character of each stream. |
| `color` | name, hex or 0-255 | `"#00af00"` | Front half of the trail. |
| `trail_color` | name, hex or 0-255 | `"#005f00"` | Back (fading) half of the trail. |
| `bold_head` | `true` / `false` | `true` | Draw the head in bold. |
| `dim_trail` | `true` / `false` | `false` | Also apply the terminal "dim" attribute to the trail (always on with fewer than 256 colors). |

### `[animation]`

| Option | Accepted | Default | Description |
|--------|----------|---------|-------------|
| `speed_min` | number 0.5-200 | `6` | Slowest fall speed, rows per second. |
| `speed_max` | number 0.5-200 | `30` | Fastest fall speed. Must be >= `speed_min`, else both revert to defaults. |
| `length_min` | integer 1-1000 | `6` | Shortest stream, in characters. |
| `length_max` | integer 0-1000 | `0` | Longest stream. `0` = terminal height. If non-zero it must be >= `length_min`, else both revert to defaults. |
| `density` | number 0.05-1.0 | `1.0` | Fraction of columns carrying a stream at any moment. |

### `[behavior]`

| Option | Accepted | Default | Description |
|--------|----------|---------|-------------|
| `initial_scatter` | `true` / `false` | `true` | `true`: streams start spread over the screen. `false`: all start above the screen and fall in. |
| `start_paused` | `true` / `false` | `false` | Start paused; press `P` to begin. |

### Examples

Cyan, sparse, slow rain over a transparent terminal:

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

Hex digits at 15 FPS on a solid black background:

```toml
[general]
fps = 15
transparent_background = false

[characters]
chars = "0123456789ABCDEF"
```

The default file (every option at its default) is
[`config/hmatrix.toml`](config/hmatrix.toml).

## Project layout

    src/main.c        main loop, input, frame limiting
    src/animation.c   stream and character state (no ncurses)
    src/renderer.c    ncurses setup and drawing
    src/config.c      TOML loading, validation, defaults
    src/cli.c         command-line parsing
    config/hmatrix.toml  documented default configuration
    third_party/tomlc17  vendored TOML parser

## License

HMatrix is released under the MIT License (see `LICENSE`). The vendored
tomlc17 library is also MIT licensed (see `third_party/tomlc17/LICENSE`).
