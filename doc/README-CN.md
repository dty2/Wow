# Wow!

## 展示

[动态](/show/dynamic.gif)

[静态](/show/static.png)

## 简介

Wow 是一个运行在 Wayland 环境下的壁纸播放器

它支持图片作为静态壁纸, 视频作为动态壁纸

## 获取

[发布版本](https://github.com/dty2/Wow/releases)

如果需要自行构建，请参阅 [构建文档](/doc/构建.md)。

## 快速开始

1. 添加壁纸列表，详见 [壁纸列表设置](#壁纸列表设置)
2. 添加配置文件 (`config.toml`)，详见 [配置文件](#配置文件)
3. ~~原神~~ 启动！命令：`wow-daemon`
4. 控制播放，详见 [控制命令](#控制命令)

### 壁纸列表设置

默认情况下，壁纸列表存放在 `$HOME/.config/wow` 目录下。

1. 在 `$HOME/.config/wow` 下创建一个壁纸列表（当然也可以放在其他位置，只需在 `config.toml` 中配置即可）。

   ```bash
   mkdir $HOME/.config/wow/your_wallpaper_list_name
   ```

   你可以创建多个壁纸列表：

   ```bash
   mkdir $HOME/.config/wow/girl $HOME/.config/wow/animal
   ```

2. 将你下载的壁纸放入壁纸列表目录：

   ```bash
   mv your_wallpaper_dir $HOME/.config/wow/your_wallpaper_list_name/your_wallpaper_dir
   ```

### 配置文件

[配置文件模板](/config.toml)

```toml
startlist = "" # <- 填入你最开始播放的壁纸列表

[""] # <- 填入你的壁纸列表名称

dir = "" # <- 填入你的壁纸列表目录。支持相对路径和绝对路径。
          # 相对路径必须相对于 "$HOME/.config/wow/"

# -1 是默认值，表示手动控制壁纸切换（next/previous）。
# 如果设置为 60，表示自动切换，每个壁纸播放 60 秒后切换到下一个。
time = -1

# order 可以为空，表示没有顺序要求，此时按照加载顺序播放壁纸。
# 如果 order 不为空，则按照 order 列表中的顺序播放壁纸。
order = []

# 可以有多个壁纸列表
["xxx"]
dir = ""
time = -1
order = []
```

### 控制命令

```bash
# --------------------------------------------------------------
wow-cli -h                   # [H]elp     帮助
wow-cli -v                   # [V]ersion  显示版本
wow-cli -i                   # [I]nfo     显示所有信息（壁纸列表、当前播放状态）
# --------------------------------------------------------------
wow-cli -n                   # [N]ext     下一个壁纸
wow-cli -p                   # [P]revious 上一个壁纸
wow-cli -m                   # [M]anual   手动播放
wow-cli -r                   # [R]efresh  刷新
wow-cli -q                   # [Q]uit     退出（守护进程）
# --------------------------------------------------------------
wow-cli -t [wallpaper]       # [T]est     预览一个壁纸
wow-cli -l [list]            # [L]ist     播放指定的列表
```

> [!NOTE]
> `wow-cli -t [wallpaper]` 的使用场景:
> 当你获得一张图片或视频壁纸, 想要快速预览它的效果时, 可以使用该命令
> 它能让你方便地检查壁纸是否符合预期, 而无需正式设置. 预览时间为 30 秒

## 其他

设计: 详见 [设计文档](/doc/设计.md)  
测试: 详见 [测试文档](/doc/测试.md)
