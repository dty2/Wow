# Wow

## Description

Wow is a wallpaper player. It can play static and dynamic wallpaper.

It can use any "example.webp" and "example.webm" as your wallpaper.

(I wrote it for myself, So it just can run in Linux. [Why I wrote it?](#why-i-wrote-it))

## How to get

You can download the executable file in [release](https://github.com/dty2/Wow/releases).

Or you can [build it by your self](#build).

> [!TIP]
> Don't forget add execute permissions when you install it.

## How to use

Just a wallpaper player, so the command is simple.

``` bash
## These are all the cmd

./Wow -t # run it. "-t" is [T]oggle.
# PS: If you want to run Wow, use "-t", and if you want to stop Wow, use "-t" too.
# When you run "./Wow -t" for the first time, you will see below, then just follow the instructions.
#
# (Shell) ./Wow -t
# No wallpaper to play.
# Please place your wallpaper "example.webp" on $HOME/.config/Wow/static.
# Or place your wallpaper "example.webm" on $HOME/.config/Wow/dynamic.
# Then run "./Wow -t" again.

./Wow -m # change to another [M]ode
./Wow -n # [N]ext wallpaper
./Wow -p # [P]revious wallpaper
./Wow -k # pause it. "-k" is [K]eep
./Wow -h # [H]elp
```

The default mode is static, and it will change the wallpaper every 50 seconds.

Dynamic mode will not auto change to next wallpaper, if you want to play next, you need use command "./Wow -n".

## Dependence

That's all the libs which I used.

``` bash
sudo pacman -S ffmpeg sdl2 libx11 google-glog
```

## Build

Build is very simple too.

``` bash
mkdir build
cd build
cmake ..
make
```

## Develop

I will continue to improve it slowly...

## Why I wrote it

* I don't want to use desktop environment.
So I can't use the wallpaper player supplied by the desktop environment.
* I don't want to buy Wallpaper Engine on steam
or use [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine) because I'm as stingy as Mr.Krabs.
* I don't want to use feh to show only static wallpaper. So I don't use it or variety.
* I don't want to use xwinwrap to show only dynamic wallpaper either.

So, I try to write a shell or python to call the feh or xwinwrap to implement it.

But soon, I realized, "why not write a wallpaper player by yourself?".

So this crazy idea occupied my mind. Then I implement it in cpp.
