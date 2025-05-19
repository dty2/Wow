# Wow

## Show

## Description

Wow is a wallpaper player(Support X11 and Wayland).

It can use "example.png/jpg/webp/..."(Picture) and "example.mp4/webm/..."(Video) as your wallpaper.

(I wrote it for myself, So it only can run on Linux.)

## How to use

### Initlization

First of all, run command `./Wow -i` to initlization

It will create `Wow` directory in your `~/.config`.

``` bash
(bash) ls
Wow
(bash) ./Wow -i
Configuration File create successful: /home/xxx/.config/Wow/Wow.json
Wow initlization success.
(bash) cd ~/.config/Wow
(bash) tree -L1
.
├── dynamic   <--- Video files are placed here
├── static    <--- Picture files are placed here
└── Wow.json  <--- configuation file

3 directories, 1 file
```

The new Wow.json like:

``` json
{
    "Version": "1.2.1"
    "StartMode": "Static",
    "Dynamic": {
        "AutoChange": false,
        "Default": [],
        "Frequency": 20,
        "Listname": [
            "Default"
        ],
        "StartList": "Default"
    },
    "Static": {
        "AutoChange": true,
        "Default": [],
        "IntervalTime": 60,
        "Listname": [
            "Default"
        ],
        "StartList": "Default"
    },
}

```

### Configuation

Second, config it.

When you have completed the initialization, you should add some photos and videos to the corresponding directories and add the file names of the photos and videos you added to the configuration file.

``` json
{
    "Version": "1.2.1"
    "StartMode": "Dynamic",
    "Dynamic": {
        "AutoChange": false,
        "Default": [
            "1.webm",
            "3.webm",
            "4.webm",
            "6.webm"
        ],
        "Girl": [
            "7.webm",
            "9.webm"
        ],
        "Frequency": 20,
        "Listname": [
            "Default",
            "Girl"
        ],
        "StartList": "Default"
    },
    "Static": {
        "AutoChange": true,
        "Default": [
            "1.webp",
            "4.webp",
            "5.webp"
        ],
        "Car": [
            "2.jpg",
            "4.webp",
            "8.webp",
            "9.webp"
        ],
        "IntervalTime": 60,
        "Listname": [
            "Default",
            "Cat"
        ],
        "StartList": "Default"
    },
}
```

This is the modified configuration file. You can see that new photos, videos and playlists have been added here.

If IntervalTime is set to 60, the static wallpaper will be switched every 60 seconds.

And if Frequency is set to 20, the dynamic wallpaper will be switched to the next one after playing the dynamic wallpaper 20 times.

If StartMode is set to static, it means that the startup mode is set to static, that is, when running Wow, the static StartList will be played first.

Conversely, if it is set to dynamic, it means that the mode is set to dynamic, that is, the default dynamic StartList will be played.

### Command

Third, you can start up Wow by command `./Wow -t`

Other command is:

``` bash
Wow -h           # [H]elp
Wow -v           # [V]ersion
Wow -i           # [I]nitlization Wow
Wow -t           # [T]oggle wallpaper
--------------------------------------------------------------
Wow -s           # Toggle of [S]top automatic play wallpaper
Wow -n           # [N]ext wallpaper
Wow -p           # [P]revious wallpaper
--------------------------------------------------------------
Wow -ls ListName # Change to "[L]istname" of [S]tatic list
Wow -ld ListName # Change to "[L]istname" of [D]ynamic list
Wow -a           # Show [A]ll your list
```

The option`-s` is a switch. When you execute the option `-s` for the first time, it will stop automatic playback

but when you execute the option `-s` again, it will start automatic playback.

## How to get

You can download the executable file in [release](https://github.com/dty2/Wow/releases).

Or you can [build it by your self](#build).

> [!TIP]
> Don't forget add execute permissions when you install it.

## Dependence

That's all the libs which I used.

``` bash
sudo pacman -S pkgconf ffmpeg sdl2 libx11 google-glog wayland wayland-protocols nlohmann-json
```

## How to build

Build is very simple too.

``` bash
mkdir build
cd build
cmake -DWAYLAND=ON .. # or cmake -DX11=ON ..
make -j4
```

Add Wayland support, you need generate "xdg-shell-client-protocol.h" by your self

``` bash
wayland-scanner client-header wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1-client-protocol.h
wayland-scanner private-code wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1-client-protocol.c
wayland-scanner client-header xdg-shell.xml xdg-shell-client-protocol.h
wayland-scanner private-code xdg-shell.xml xdg-shell-protocol.c
```

## Develop

Its current functions have met my needs. If you want some other functions, please raise an issue.
