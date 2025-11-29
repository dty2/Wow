# Wow!

[中文](/doc/README-CN.md)

## Showcase

[Dynamic](/show/dynamic.gif)

[Static](/show/static.png)

## Introduction

Wow is a wallpaper player for the Wayland environment.

It supports images as static wallpapers and videos as dynamic wallpapers.

## Install

[Release](https://github.com/dty2/Wow/releases)

For building from source, see [Build Documentation](/doc/构建.md).

## Quick Start

1. Add a wallpaper list, see [Wallpaper List Setup](#wallpaper-list-setup)
2. Add a configuration file (`config.toml`), see [Configuration File](#configuration-file)
3. ~~Genshin Impact~~ Launch! Command: `wow-daemon`
4. Control playback, see [Control Commands](#control-commands)

### Wallpaper List Setup

By default, wallpaper lists are stored in `$HOME/.config/wow`.

1. In `$HOME/.config/wow`, create a wallpaper list (you can also place it elsewhere, just configure it in `config.toml`).

   ```bash
   mkdir $HOME/.config/wow/your_wallpaper_list_name
   ```

   You can have multiple wallpaper lists:

   ```bash
   mkdir $HOME/.config/wow/girl $HOME/.config/wow/animal
   ```

2. Move your downloaded wallpapers into the wallpaper list directory:
   ```bash
   mv your_wallpaper_dir $HOME/.config/wow/your_wallpaper_list_name/your_wallpaper_dir
   ```

### Configuration File

[Configuration File Template](/config.toml)

```toml
startlist = "" # <- Fill in the wallpaper list you want to start playing first

[""] # <- Fill in the name of your wallpaper list

dir = "" # <- Fill in the directory of your wallpaper list. Both relative and absolute paths are supported.
          # Relative paths must be relative to "$HOME/.config/wow/"

# -1 is the default value, meaning manual control of wallpaper switching (next/previous).
# If set to 60, it means automatic switching, and each wallpaper will play for 60 seconds before switching to the next.
time = -1

# order can be empty, meaning no specific order is required. In this case, wallpapers are played in the loading order.
# If order is not empty, wallpapers will be played in the sequence defined in the order list.
order = []

# You can have multiple wallpaper lists
["xxx"]
dir = ""
time = -1
order = []
```

### Control Commands

```bash
# --------------------------------------------------------------
wow-cli -h                   # [H]elp     Show help
wow-cli -v                   # [V]ersion  Show version
wow-cli -i                   # [I]nfo     Show all information (wallpaper lists, current playback status)
# --------------------------------------------------------------
wow-cli -n                   # [N]ext     Next wallpaper
wow-cli -p                   # [P]revious Previous wallpaper
wow-cli -m                   # [M]anual   Manual playback
wow-cli -r                   # [R]efresh  Refresh
wow-cli -q                   # [Q]uit     Quit (daemon)
# --------------------------------------------------------------
wow-cli -t [wallpaper]       # [T]est     Preview a wallpaper
wow-cli -l [list]            # [L]ist     Play a specified list
```

Usage scenario for `wow-cli -t [wallpaper]`:  
When you obtain an image or video wallpaper and want to quickly preview its effect, you can use this command.
It allows you to check whether the wallpaper meets your expectations without formally setting it. The preview lasts for 30 seconds.

## Others

Design: see [Design Documentation](/doc/design.md)  
Testing: see [Testing Documentation](/doc/test.md)
