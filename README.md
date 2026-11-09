# HMatrix

**HMatrix** is an independent implementation inspired by [CMatrix](https://github.com/abishekvashok/cmatrix), bringing the classic terminal digital rain effect with a binary twist.

Instead of the usual streams of random characters, HMatrix displays `0`s and `1`s by default, falling in green streams over your terminal's own background.

## Features

- Independent implementation inspired by CMatrix.
- Binary digital rain using `0` and `1` by default.
- Customizable characters and color schemes.
- Fully configurable through a documented `hmatrix.toml` file.
- Adjustable animation speed, stream density, length, and FPS.
- Bright stream heads with fading trails.
- Preserves terminal transparency and wallpapers.
- Lightweight animation with efficient CPU usage.
- Handles terminal resizing and different terminal color capabilities.
- Keyboard controls for pausing, restarting, and quitting.

## Installation & Building

### Requirements

- Linux
- GCC or another C17-compatible compiler
- CMake 3.10 or newer
- ncurses development libraries

### Install dependencies

**Arch Linux / CachyOS / Manjaro**

```bash
sudo pacman -S --needed base-devel cmake ncurses
```

**Debian / Ubuntu / Linux Mint**

```bash
sudo apt install build-essential cmake libncurses-dev
```

**Fedora**

```bash
sudo dnf install gcc cmake ncurses-devel
```

**openSUSE**

```bash
sudo zypper install gcc cmake ncurses-devel
```

### Clone the repository

```bash
git clone https://github.com/Shxzyy76/hmatrix.git
cd hmatrix
```

### Build

```bash
cmake -B build
cmake --build build
```

### Run

```bash
./build/hmatrix
```

### Optional: Install system-wide

```bash
sudo cmake --install build
```

Or install locally:

```bash
cmake --install build --prefix ~/.local
```

Make sure `~/.local/bin` is in your `PATH` for a local installation.

## Usage

```bash
hmatrix
hmatrix --config ./my.toml
hmatrix --help
hmatrix --version
```

### Keyboard Controls

| Key          | Action            |
| ------------ | ----------------- |
| `Q` or `Esc` | Quit              |
| `P`          | Pause / Resume    |
| `R`          | Restart animation |

## Configuration

HMatrix uses a TOML configuration file to customize its appearance and behavior.

The default configuration is located at:

```text
config/hmatrix.toml
```

You can customize:

- Characters displayed in the rain.
- Head, stream, and trail colors.
- Animation speed and density.
- Stream length.
- Frame rate.
- Background transparency.
- Initial animation behavior.

See [`config/hmatrix.toml`](config/hmatrix.toml) for all available options and their descriptions.

## License

HMatrix is licensed under the **GNU General Public License v2.0 (GPL-2.0)**.

See the [`LICENSE`](LICENSE) file for details.

The vendored `tomlc17` library retains its own MIT license, available at [`third_party/tomlc17/LICENSE`](third_party/tomlc17/LICENSE).
