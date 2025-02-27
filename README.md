# Wow

## Description

Wow is a wallpaper player. It can play static and dynamic wallpaper.

It can use any "example.webp" and "example.webm" as your wallpaper.

## How to use

Just a wallpaper player, so the command is simple.

``` bash
## These are all the cmd

./Wow -t # run it. "-t" is [T]oggle.
# PS: If you want to run Wow, use "-t", and if you want to stop Wow, use "-t" too.

./Wow -m # change to another [M]ode
./Wow -n # [N]ext wallpaper
./Wow -p # [P]revious wallpaper
./Wow -k # pause it. "-k" is [K]eep
./Wow -h # [H]elp
```

## Dependence

That's all the libs which I used.

``` bash
sudo pacman - S ffmpeg sdl2 libx11 google-glog
```

## Build

Build is very simple too.

``` bash
mkdir build
cd build
cmake ..
make
```

## Other

Why did you make it?

I don't want to use desktop environment. So I can't use the wallpaper player supplied by the desktop environment.

I don't want to buy Wallpaper Engine on steam or use [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine) because I'm as stingy as Mr.Krabs.

I don't want to use feh to show only static wallpaper. So I don't use it or variety.

I don't want to use xwinwrap to show only dynamic wallpaper either.

So, I try to write a shell or python to call the feh or xwinwrap to implement it.

But soon, I realized, "why not write a wallpaper player by yourself?".

So this crazy idea occupied my mind. Then I implement it in cpp.
