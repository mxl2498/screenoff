/*
 * ScreenOff v1.0  -  Windows 显示器开关小工具
 *
 * 特点: 单个 exe, 仅调用系统自带的 kernel32 / user32 / powrprof,
 *       无运行时库依赖, 无配置文件, 无注册表, 无托盘常驻进程。
 * 编译: tcc screenoff.c -o ScreenOff.exe -Wl,-subsystem=windows -luser32 -lkernel32
 *
 * 作者注: 中文帮助文本以 UTF-16 转义形式内嵌, 源码保持纯 ASCII,
 *         保证在任何代码页下编译与显示都不会乱码。
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>

#ifndef SC_MONITORPOWER
#define SC_MONITORPOWER 0xF170
#endif
#ifndef WM_SYSCOMMAND
#define WM_SYSCOMMAND   0x0112
#endif

/* MSVCRT 提供的命令行参数 (GUI 子系统下也有) */
extern int   __argc;
extern char **__argv;

/* tcc 自带的 winapi 头文件未声明该 API, 这里补上 */
BOOL WINAPI BlockInput(BOOL fBlockIt);

#ifndef SMTO_NOTIMEOUTIFNOTHUNG
#define SMTO_NOTIMEOUTIFNOTHUNG 0x0008
#endif

typedef BOOL    (WINAPI *PFN_LOCKWORKSTATION)(VOID);
typedef BOOLEAN (WINAPI *PFN_SETSUSPENDSTATE)(BOOLEAN, BOOLEAN, BOOLEAN);

static const wchar_t HELP_TEXT[] =
L"ScreenOff v1.0  \u663e\u793a\u5668\u5f00\u5173\u5c0f\u5de5\u5177\n\n"
L"\u7528\u6cd5:\n"
L"  ScreenOff.exe            \u7acb\u5373\u5173\u95ed\u663e\u793a\u5668\n"
L"  ScreenOff.exe -t 60      60 \u79d2\u540e\u5173\u95ed\u663e\u793a\u5668\n"
L"  ScreenOff.exe -l         \u9501\u5b9a\u8ba1\u7b97\u673a\u5e76\u5173\u95ed\u663e\u793a\u5668\n"
L"  ScreenOff.exe -s         \u8ba9\u7cfb\u7edf\u8fdb\u5165\u7761\u7720\n"
L"  ScreenOff.exe -k         \u5173\u5c4f\u5e76\u77ed\u6682\u5c4f\u853d\u8f93\u5165, \u9632\u6b62\u8bef\u89e6\u5524\u9192\n"
L"  ScreenOff.exe -h         \u663e\u793a\u672c\u5e2e\u52a9\n\n"
L"\u8bf4\u660e:\n"
L"  \u79fb\u52a8\u9f20\u6807\u6216\u6309\u4efb\u610f\u952e\u5373\u53ef\u91cd\u65b0\u70b9\u4eae\u5c4f\u5e55\u3002\n"
L"  \u5efa\u8bae\u4e3a ScreenOff.exe \u521b\u5efa\u5feb\u6377\u65b9\u5f0f, \u5e76\u5728\u5feb\u6377\u65b9\u5f0f\n"
L"  \u5c5e\u6027\u91cc\u8bbe\u7f6e\u5feb\u6377\u952e (\u5982 Ctrl+Alt+Q), \u5373\u53ef\u4e00\u952e\u5173\u5c4f\u3002";

static void showHelp(void)
{
    MessageBoxW(NULL, HELP_TEXT, L"ScreenOff",
                MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
}

/* 关屏核心: 广播 WM_SYSCOMMAND / SC_MONITORPOWER */
static void displayOff(void)
{
    DWORD_PTR res = 0;
    if (!SendMessageTimeoutW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER,
                             (LPARAM)2,
                             SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG,
                             1500, &res)) {
        /* 回退: 直接广播 */
        SendMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
    }
}

/* 系统睡眠 (powrprof.dll 动态加载, 避免额外链接依赖) */
static int systemSleep(void)
{
    HMODULE h = LoadLibraryA("powrprof.dll");
    if (h) {
        PFN_SETSUSPENDSTATE fn =
            (PFN_SETSUSPENDSTATE)(void *)GetProcAddress(h, "SetSuspendState");
        if (fn) {
            fn(FALSE, FALSE, FALSE);
            FreeLibrary(h);
            return 1;
        }
        FreeLibrary(h);
    }
    return 0;
}

/* 锁定工作站 (user32.dll 动态加载, 兼容旧版导入库) */
static void lockWorkStation(void)
{
    HMODULE h = LoadLibraryA("user32.dll");
    if (h) {
        PFN_LOCKWORKSTATION fn =
            (PFN_LOCKWORKSTATION)(void *)GetProcAddress(h, "LockWorkStation");
        if (fn) fn();
        FreeLibrary(h);
    }
}

int main(int argc, char **argv)
{
    int i, delay = 0, doLock = 0, doSuspend = 0, keepOff = 0, help = 0;

    (void)argc;
    (void)argv;

    for (i = 1; i < __argc; i++) {
        char *a = __argv[i];
        if (!strcmp(a, "-h") || !strcmp(a, "--help") || !strcmp(a, "/?")) {
            help = 1;
        } else if (!strcmp(a, "-l") || !strcmp(a, "--lock")) {
            doLock = 1;
        } else if (!strcmp(a, "-s") || !strcmp(a, "--sleep")) {
            doSuspend = 1;
        } else if (!strcmp(a, "-k") || !strcmp(a, "--keep")) {
            keepOff = 1;
        } else if (!strcmp(a, "-t") || !strcmp(a, "--time")) {
            if (i + 1 < __argc) delay = atoi(__argv[++i]);
            else help = 1;
        } else if (a[0] == '-' || a[0] == '/') {
            help = 1;
        } else {
            delay = atoi(a);
        }
    }

    if (help) {
        showHelp();
        return 0;
    }

    if (delay > 0) Sleep((DWORD)delay * 1000UL);

    if (doSuspend && systemSleep()) return 0;

    if (doLock) {
        lockWorkStation();
        Sleep(600);              /* 等锁屏界面出现再关屏 */
    }

    if (keepOff) {
        BlockInput(TRUE);        /* 挡住松手瞬间的抖动, 防止刚关就被唤醒 */
        displayOff();
        Sleep(600);
        BlockInput(FALSE);
    } else {
        displayOff();
    }
    return 0;
}
