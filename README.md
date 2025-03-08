# Wow

## Show

![Show](./show.gif)

## Description

Wow is a wallpaper player Demo(Support X11 and Wayland). It can play Static and Dynamic wallpaper.

It can use "example.png/jpg/webp/..."(Picture) and "example.mp4/webm/..."(Video) as your wallpaper.

(I wrote it for myself, So it only can run on Linux. [Why I wrote it?](#why-i-wrote-it))

## How to use

1. Run command `mkdir -p $HOME/.config/Wow/{static, dynamic}`
2. Run command `./Wow -t`

## How to get

You can download the executable file in [release](https://github.com/dty2/Wow/releases).

Or you can [build it by your self](#build).

> [!TIP]
> Don't forget add execute permissions when you install it.

## Commands

Just a simple wallpaper player, so the command is simple.

``` bash
## These are all the commands

./Wow -t # run it. "-t" is [T]oggle.
./Wow -m # change to another [M]ode
./Wow -n # [N]ext wallpaper
./Wow -p # [P]revious wallpaper
./Wow -h # [H]elp
```

The default mode is static, and it will change the wallpaper every 60 seconds.

Dynamic mode will not auto change to next wallpaper, if you want to play next/previous, you need use command "Wow -n/p".

> [!CAUTION]
>
> PS: If you want to run Wow, use "-t", and if you want to stop Wow, use "-t" too.
> When you run "./Wow -t" for the first time without create Wow dir in "$HOME/.config/".
> You will see output below, then just follow the instructions.
>
> (Shell) ./Wow -t
> No wallpaper to play.
> Please place your wallpaper "example.png/jpg/webp/..." on $HOME/.config/Wow/static.
> Or place your wallpaper "example.mp4/webm/..." on $HOME/.config/Wow/dynamic.
> Then run "./Wow -t" again.

## Dependence

That's all the libs which I used.

``` bash
sudo pacman -S ffmpeg sdl2 libx11 google-glog wayland-protocols wlroots
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

## Why I wrote it

* I don't want to use desktop environment. So I can't use the wallpaper player supplied by the desktop environment.

* I don't want to buy Wallpaper Engine on steam or use [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine).

* I don't want to use Feh to show only static wallpaper. So I don't use it or variety.

* I don't want to use Xwinwrap to show only dynamic wallpaper either.

So, I try to write a shell ~~or python~~(I don't know how to use Python) to call the Feh or Xwinwrap to implement it.

But soon, I realized, "Why not write a wallpaper player by myself?". And this crazy idea occupied my mind.

Then I implement Wow in Cpp.
