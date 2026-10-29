#define UNICODE
#define _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <stdio.h>

#define BAR_HEIGHT 44
#define MAX_TASKS 40
#define MAX_PROGRAMS 40

#define ID_START 100
#define ID_TASK_BASE 1000
#define ID_PROG_BASE 2000
#define ID_TASKMGR 3000
#define ID_QUIT 3001
#define TIMER_REFRESH 1

static HWND g_bar;
static HWND g_start;
static HWND g_tasks[MAX_TASKS];
static HWND g_taskHwnd[MAX_TASKS];
static int g_taskCount;
static WCHAR g_progPaths[MAX_PROGRAMS][MAX_PATH];
static WCHAR g_progNames[MAX_PROGRAMS][64];
static int g_progCount;
static HFONT g_font;
static HBRUSH g_bg;
static HBRUSH g_hot;
static DWORD g_selfPid;

static const WCHAR *kSkipClasses[] = {
    L"WinOsBar", L"Default IME", L"MSCTFIME UI", L"IME", L"GDI+ Hook Window Class", NULL
};

static void bar_log(const char *text) {
    FILE *f = NULL;
    if (fopen_s(&f, "Z:\\storage\\emulated\\0\\Download\\WinOs-Bar\\bar.log", "a") == 0 && f) {
        fprintf(f, "%s\n", text);
        fclose(f);
    }
}

static BOOL class_is_skipped(const WCHAR *cls) {
    for (int i = 0; kSkipClasses[i]; i++) {
        if (lstrcmpiW(cls, kSkipClasses[i]) == 0) return TRUE;
    }
    return FALSE;
}

typedef struct {
    HWND hwnd;
    WCHAR title[160];
} TaskEntry;

static TaskEntry g_found[MAX_TASKS * 2];
static int g_foundCount;

static BOOL CALLBACK enum_proc(HWND hwnd, LPARAM lp) {
    (void)lp;
    if (g_foundCount >= (int)(sizeof(g_found) / sizeof(g_found[0]))) return FALSE;
    if (!IsWindowVisible(hwnd)) return TRUE;
    if (GetWindow(hwnd, GW_OWNER) != NULL) return TRUE;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == g_selfPid) return TRUE;
    LONG ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (ex & WS_EX_TOOLWINDOW) return TRUE;
    WCHAR cls[128] = {0};
    GetClassNameW(hwnd, cls, 128);
    if (class_is_skipped(cls)) return TRUE;
    WCHAR title[160] = {0};
    GetWindowTextW(hwnd, title, 160);
    if (title[0] == 0) return TRUE;
    if (wcscmp(title, L"Program Manager") == 0) return TRUE;
    g_found[g_foundCount].hwnd = hwnd;
    lstrcpynW(g_found[g_foundCount].title, title, 160);
    g_foundCount++;
    return TRUE;
}

static void add_program(const WCHAR *path, const WCHAR *name) {
    if (g_progCount >= MAX_PROGRAMS) return;
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) return;
    for (int i = 0; i < g_progCount; i++) {
        if (lstrcmpiW(g_progPaths[i], path) == 0) return;
    }
    lstrcpynW(g_progPaths[g_progCount], path, MAX_PATH);
    lstrcpynW(g_progNames[g_progCount], name, 64);
    g_progCount++;
}

static void add_exes_in(const WCHAR *dir, int limit) {
    WCHAR pattern[MAX_PATH];
    lstrcpynW(pattern, dir, MAX_PATH);
    if (pattern[0] && pattern[lstrlenW(pattern) - 1] != L'\\') lstrcatW(pattern, L"\\");
    lstrcatW(pattern, L"*.exe");

    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    int n = 0;
    do {
        if (n++ >= limit) break;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        WCHAR full[MAX_PATH];
        lstrcpynW(full, dir, MAX_PATH);
        if (full[0] && full[lstrlenW(full) - 1] != L'\\') lstrcatW(full, L"\\");
        lstrcatW(full, fd.cFileName);
        WCHAR lower[64];
        lstrcpynW(lower, fd.cFileName, 64);
        CharLowerW(lower);
        if (wcsstr(lower, L"unins") || wcsstr(lower, L"setup") || wcsstr(lower, L"vc_redist")) continue;
        add_program(full, fd.cFileName);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

static void scan_programs(void) {
    g_progCount = 0;
    WCHAR download[MAX_PATH] = L"Z:\\storage\\emulated\\0\\Download";
    add_exes_in(download, 30);

    WCHAR pattern[MAX_PATH];
    lstrcpynW(pattern, download, MAX_PATH);
    lstrcatW(pattern, L"\\*");
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        int dirs = 0;
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            if (fd.cFileName[0] == L'.') continue;
            if (++dirs > 25) break;
            WCHAR sub[MAX_PATH];
            lstrcpynW(sub, download, MAX_PATH);
            lstrcatW(sub, L"\\");
            lstrcatW(sub, fd.cFileName);
            add_exes_in(sub, 6);
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    add_program(L"Z:\\storage\\emulated\\0\\Download\\npp\\notepad++.exe", L"notepad++.exe");
    add_program(L"Z:\\storage\\emulated\\0\\Download\\qBittorrentPortable\\App\\qBittorrent\\qbittorrent.exe", L"qbittorrent.exe");
}

static void layout(void) {
    RECT rc;
    GetClientRect(g_bar, &rc);
    int h = rc.bottom - rc.top;
    int y = 4;
    int bh = h - 8;
    MoveWindow(g_start, 4, y, 92, bh, TRUE);
    int x = 102;
    for (int i = 0; i < g_taskCount; i++) {
        WCHAR text[80];
        GetWindowTextW(g_tasks[i], text, 80);
        HDC dc = GetDC(g_bar);
        HGDIOBJ old = SelectObject(dc, g_font);
        SIZE sz = {0};
        GetTextExtentPoint32W(dc, text, lstrlenW(text), &sz);
        SelectObject(dc, old);
        ReleaseDC(g_bar, dc);
        int w = sz.cx + 24;
        if (w < 60) w = 60;
        if (w > 220) w = 220;
        if (x + w > rc.right - 6) break;
        MoveWindow(g_tasks[i], x, y, w, bh, TRUE);
        x += w + 4;
    }
}

static void rebuild_tasks(void) {
    for (int i = 0; i < g_taskCount; i++) {
        if (g_tasks[i]) DestroyWindow(g_tasks[i]);
        g_tasks[i] = NULL;
    }
    g_taskCount = 0;

    for (int i = 0; i < g_foundCount && g_taskCount < MAX_TASKS; i++) {
        HWND b = CreateWindowExW(0, L"BUTTON", g_found[i].title,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
            0, 0, 10, 10, g_bar, (HMENU)(INT_PTR)(ID_TASK_BASE + g_taskCount), NULL, NULL);
        if (!b) continue;
        SendMessageW(b, WM_SETFONT, (WPARAM)g_font, TRUE);
        g_tasks[g_taskCount] = b;
        g_taskHwnd[g_taskCount] = g_found[i].hwnd;
        g_taskCount++;
    }
    layout();
    InvalidateRect(g_bar, NULL, TRUE);
}

static void refresh_tasks(void) {
    g_foundCount = 0;
    EnumWindows(enum_proc, 0);
    rebuild_tasks();
}

static void activate(HWND h) {
    if (!IsWindow(h)) return;
    if (IsIconic(h)) ShowWindow(h, SW_RESTORE);
    SetForegroundWindow(h);
    BringWindowToTop(h);
}

static void launch_program(int index) {
    if (index < 0 || index >= g_progCount) return;
    WCHAR dir[MAX_PATH];
    lstrcpynW(dir, g_progPaths[index], MAX_PATH);
    WCHAR *slash = wcsrchr(dir, L'\\');
    if (slash) *slash = 0;
    ShellExecuteW(NULL, L"open", g_progPaths[index], NULL, dir, SW_SHOWNORMAL);
}

static void show_start_menu(void) {
    scan_programs();
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, ID_TASKMGR, L"Görev Yöneticisi");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    for (int i = 0; i < g_progCount; i++) {
        AppendMenuW(menu, MF_STRING, ID_PROG_BASE + i, g_progNames[i]);
    }
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_QUIT, L"Görev çubuğunu kapat");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(g_bar);
    int cmd = (int)TrackPopupMenu(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_BOTTOMALIGN,
        pt.x, pt.y, 0, g_bar, NULL);
    DestroyMenu(menu);

    if (cmd >= ID_PROG_BASE) {
        launch_program(cmd - ID_PROG_BASE);
    } else if (cmd == ID_TASKMGR) {
        ShellExecuteW(NULL, L"open", L"taskmgr.exe", NULL, NULL, SW_SHOWNORMAL);
    } else if (cmd == ID_QUIT) {
        DestroyWindow(g_bar);
    }
    SetTimer(g_bar, TIMER_REFRESH, 700, NULL);
}

static LRESULT CALLBACK bar_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        g_selfPid = GetCurrentProcessId();
        g_font = CreateFontW(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        g_bg = CreateSolidBrush(RGB(24, 28, 34));
        g_hot = CreateSolidBrush(RGB(48, 56, 68));
        g_start = CreateWindowExW(0, L"BUTTON", L"Başlat",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
            0, 0, 10, 10, hwnd, (HMENU)ID_START, NULL, NULL);
        SendMessageW(g_start, WM_SETFONT, (WPARAM)g_font, TRUE);
        refresh_tasks();
        SetTimer(hwnd, TIMER_REFRESH, 700, NULL);
        return 0;
    }
    case WM_TIMER:
        if (wp == TIMER_REFRESH) refresh_tasks();
        return 0;
    case WM_MOUSEMOVE: {
        static int lastX = -1, lastY = -1;
        int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
        if (x != lastX || y != lastY) {
            char buf[64];
            sprintf_s(buf, sizeof(buf), "move %d %d", x, y);
            bar_log(buf);
            lastX = x;
            lastY = y;
        }
        return 0;
    }
    case WM_LBUTTONDOWN: {
        char buf[64];
        sprintf_s(buf, sizeof(buf), "down %d %d", (int)GET_X_LPARAM(lp), (int)GET_Y_LPARAM(lp));
        bar_log(buf);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        char buf[64];
        sprintf_s(buf, sizeof(buf), "cmd %d", id);
        bar_log(buf);
        if (id == ID_START) {
            KillTimer(hwnd, TIMER_REFRESH);
            show_start_menu();
        } else if (id >= ID_TASK_BASE && id < ID_TASK_BASE + MAX_TASKS) {
            activate(g_taskHwnd[id - ID_TASK_BASE]);
            SetTimer(hwnd, TIMER_REFRESH, 700, NULL);
        }
        return 0;
    }
    case WM_CTLCOLORBTN: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, RGB(230, 234, 240));
        SetBkColor(dc, RGB(40, 46, 56));
        return (LRESULT)g_hot;
    }
    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wp, &rc, g_bg);
        return 1;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(dc, &rc, g_bg);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(70, 80, 96));
        HGDIOBJ old = SelectObject(dc, pen);
        MoveToEx(dc, 0, 0, NULL);
        LineTo(dc, rc.right, 0);
        SelectObject(dc, old);
        DeleteObject(pen);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, TIMER_REFRESH);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, LPWSTR cmdline, int show) {
    (void)prev; (void)cmdline; (void)show;
    SetProcessDPIAware();

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = bar_proc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"WinOsBar";
    if (!RegisterClassExW(&wc)) return 1;

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    g_bar = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"WinOsBar", L"WinOsBar",
        WS_POPUP | WS_VISIBLE, 0, sh - BAR_HEIGHT, sw, BAR_HEIGHT, NULL, NULL, inst, NULL);
    if (!g_bar) return 1;

    ShowWindow(g_bar, SW_SHOW);
    UpdateWindow(g_bar);
    bar_log("bar started");

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
