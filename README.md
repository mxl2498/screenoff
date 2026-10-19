# ScreenOff —— 极轻量 Windows 关屏小工具

一个 **4.6 KB 的单文件 exe**，双击即可关闭显示器。绿色、免安装、无托盘常驻、无注册表、无配置文件。

## 下载

- 可执行文件：<https://github.com/mxl2498/screenoff/releases/latest/download/ScreenOff.exe>
- 仓库地址：<https://github.com/mxl2498/screenoff>
- 克隆：`git clone https://github.com/mxl2498/screenoff.git`

## 快速开始

| 想做的事 | 做法 |
|---|---|
| 立刻关屏 | 双击 `ScreenOff.exe` |
| 一键关屏（推荐） | 双击 `install-hotkey.bat`，之后按 `Ctrl + Alt + Q` |
| 查看参数 | `ScreenOff.exe -h` |

屏幕关闭后，**移动鼠标或按任意键即可重新点亮**。

## 命令行参数

```
ScreenOff.exe              立即关闭显示器
ScreenOff.exe -t 60        60 秒后关闭显示器
ScreenOff.exe -l           锁定计算机并关闭显示器
ScreenOff.exe -s           让系统进入睡眠
ScreenOff.exe -k           关屏并短暂屏蔽输入，防止误触立刻唤醒
ScreenOff.exe -h           显示帮助
```

参数可组合，例如 `ScreenOff.exe -t 600 -k`（10 分钟后关屏，并防误触唤醒）。

## 快捷键设置

`install-hotkey.bat` 会做三件事：

1. 把 `ScreenOff.exe` 复制到 `%LOCALAPPDATA%\ScreenOff\`（这样即使移动/删除本目录，快捷键也不会失效）；
2. 在桌面创建快捷方式；
3. 给快捷方式绑定全局热键，默认 `Ctrl + Alt + Q`。

自定义热键：

```bat
install-hotkey.bat -Hotkey "CTRL+ALT+F12"
```

> Windows 的快捷方式热键只在快捷方式位于**桌面或开始菜单**时才生效，这是系统限制。

## 文件说明

| 文件 | 说明 |
|---|---|
| `ScreenOff.exe` | 主程序，4.6 KB，x64，GUI 子系统（不会闪黑窗） |
| `screenoff.c` | 源码，纯 C + 原生 Win32 API |
| `build.bat` | 重新编译脚本（首次运行会自动下载 TinyCC） |
| `install-hotkey.bat / .ps1` | 一键绑定全局热键 |
| `tools/gen_help.py` | 辅助脚本：把中文帮助文本转成 C 里的 UTF-16 转义 |

不需要重编译的话，**只保留 `ScreenOff.exe` 一个文件就够了**，其余都可以删掉。

## 实现原理

核心只有一行：向所有顶层窗口广播系统命令，请求显示器进入断电状态。

```c
SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
```

- `2` = 关闭显示器（`-1` 点亮，`1` 进入低功耗）；
- 这是 Windows 自带的电源管理行为，和"屏幕保护 → 关闭显示器"走的是同一条路，不需要驱动或管理员权限；
- `SC_MONITORPOWER` 作用于系统电源策略，因此**所有显示器会一起关闭**；
- 程序发完消息立即退出，不占用内存、不驻留后台。

`-k` 参数额外调用了 `BlockInput()`，在关屏瞬间屏蔽约 0.6 秒的鼠标/键盘输入，避免"手刚松开鼠标就把屏幕碰亮"。进程退出时系统会自动解除屏蔽，不会有卡住输入的风险。

## 常见问题

**Q：能关屏后彻底不被鼠标唤醒吗？**
不能，至少不能用这种"发完就退出"的方式做到。Windows 的设计就是任意输入唤醒显示器；要强制保持黑屏，必须有一个常驻进程持续拦截输入，那就不"轻量"了。`-k` 只是挡住关屏瞬间那一下。

**Q：为什么双击没有黑窗口闪一下？**
程序用 GUI 子系统编译，没有控制台窗口，运行后直接安静退出。

**Q：支持 32 位 Windows 吗？**
当前 exe 是 x64。需要 x86 版本的话，在 `build.bat` 的编译命令里加 `-m32` 即可。

**Q：会不会被杀毒软件报毒？**
程序只调用了 `SendMessage`、`MessageBox`、`Sleep`、`BlockInput` 等系统 API，不涉及注入、联网、自启动、文件改写。个别启发式引擎仍可能误报，加入白名单即可，源码可自行审查。

## 重新编译

```
build.bat
```

首次运行会自动下载 TinyCC（约 480 KB）到 `.build` 目录。也可以手动指定已安装的编译器：

```bat
gcc screenoff.c -o ScreenOff.exe -mwindows -luser32
```

## 许可

[MIT License](LICENSE)。源码很短，可以随意审查、修改和分发。
