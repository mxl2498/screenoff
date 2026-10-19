# -*- coding: utf-8 -*-
# 把中文帮助文本转成 C 宽字符串可用的 \uXXXX 转义，源码保持纯 ASCII
t = """ScreenOff v1.0  显示器开关小工具

用法:
  ScreenOff.exe            立即关闭显示器
  ScreenOff.exe -t 60      60 秒后关闭显示器
  ScreenOff.exe -l         锁定计算机并关闭显示器
  ScreenOff.exe -s         让系统进入睡眠
  ScreenOff.exe -k         关屏并短暂屏蔽输入, 防止误触唤醒
  ScreenOff.exe -h         显示本帮助

说明:
  移动鼠标或按任意键即可重新点亮屏幕。
  建议为 ScreenOff.exe 创建快捷方式, 并在快捷方式
  属性里设置快捷键 (如 Ctrl+Alt+Q), 即可一键关屏。"""

BS = chr(92)
parts = []
for c in t:
    if ord(c) > 127:
        parts.append(BS + "u%04x" % ord(c))
    elif c == "\n":
        parts.append(BS + "n")
    elif c == '"':
        parts.append(BS + '"')
    else:
        parts.append(c)
print("".join(parts))
