#define UNICODE
#define _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <stdio.h>

#define BAR_HEIGHT 56
#define MAX_TASKS 40
#define MAX_PROGRAMS 40
#define MAX_ITEMS 48

#define ID_START 100
#define ID_TASK_BASE 1000
#define TIMER_REFRESH 1

#define MENU_ROW 56
#define MENU_WIDTH 460

static HWND g_bar;
static HWND g_start;
static HWND g_tasks[MAX_TASKS];
static HWND g_taskHwnd[MAX_TASKS];
static int g_taskCount;
static WCHAR g_progPaths[MAX_PROGRAMS][MAX_PATH];
static WCHAR g_progNames[MAX_PROGRAMS][64];
static int g_progCount;
static HFONT g_font;
static HFONT g_menuFont;
static HBRUSH g_bg;
static HBRUSH g_hot;
static DWORD g_selfPid;

static HWND g_menu;
static int g_hover = -1;

typedef struct {
    WCHAR label[80];
    int kind; /* 0 = gorev yoneticisi, 1 = program, 2 = kapat */
    int index;
} MenuItem;

static MenuItem g_items[MAX_ITEMS];
static int g_itemCount;

static const WCHAR *kSkipClasses[] = {
    L"WinOsBar", L"WinOsMenu", L"Default IME", L"MSCTFIME UI", L"IME",
    L"GDI+ Hook Window Class", NULL
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
        if (wcsstr(lower, L"unins") || wcsstr(lower, L"vc_redist")) continue;
        add_program(full, fd.cFileName);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

static void scan_programs(void) {
    g_progCount = 0;
    WCHAR download[MAX_PATH] = L"Z:\\storage\\emulated\\0\\Download";
    add_exes_in(download, 24);

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
            if (++dirs > 20) break;
            WCHAR sub[MAX_PATH];
            lstrcpynW(sub, download, MAX_PATH);
            lstrcatW(sub, L"\\");
            lstrcatW(sub, fd.cFileName);
            add_exes_in(sub, 5);
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
    int bh = h - 16;
    MoveWindow(g_start, 8, 8, 132, bh, TRUE);
    int x = 148;
    for (int i = 0; i < g_taskCount; i++) {
        WCHAR text[80];
        GetWindowTextW(g_tasks[i], text, 80);
        HDC dc = GetDC(g_bar);
        HGDIOBJ old = SelectObject(dc, g_font);
        SIZE sz = {0};
        GetTextExtentPoint32W(dc, text, lstrlenW(text), &sz);
        SelectObject(dc, old);
        ReleaseDC(g_bar, dc);
        int w = sz.cx + 28;
        if (w < 90) w = 90;
        if (w > 340) w = 340;
        if (x + w > rc.right - 8) break;
        MoveWindow(g_tasks[i], x, 8, w, bh, TRUE);
        x += w + 6;
    }
}

static void rebuild_tasks(void) {
    for (int i = 0; i < g_taskCount; i++) {
        if (g_tasks[i]) DestroyWindow(g_tasks[i]);
        g_tasks[i] = NULL;
    }
    g_taskCount = 0;

    for (int i = 0; i < g_foundCount && g_taskCount < MAX_TASKS; i++) {
        WCHAR label[80];
        lstrcpynW(label, g_found[i].title, 40);
        if (lstrlenW(g_found[i].title) > 39) lstrcatW(label, L"...");
        HWND b = CreateWindowExW(0, L"BUTTON", label,
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

static void keep_windows_off_bar(void) {
    static DWORD lastLog;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    for (int i = 0; i < g_foundCount; i++) {
        HWND h = g_found[i].hwnd;
        if (!IsWindow(h) || !IsWindowVisible(h)) continue;
        RECT r;
        if (!GetWindowRect(h, &r)) continue;
        int w = r.right - r.left;
        int hgt = r.bottom - r.top;
        char dbg[160];
        sprintf_s(dbg, sizeof(dbg), "win %p rect %d,%d %dx%d", (void *)h, (int)r.left, (int)r.top, w, hgt);
        bar_log(dbg);
        if (w >= sw - 8 && hgt >= sh - 8) {
            MoveWindow(h, 0, 0, sw, sh - BAR_HEIGHT, TRUE);
            bar_log("resized a full window");
        }
    }
}

static void refresh_tasks(void) {
    g_foundCount = 0;
    EnumWindows(enum_proc, 0);
    keep_windows_off_bar();
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
    HINSTANCE r = ShellExecuteW(NULL, L"open", g_progPaths[index], NULL, dir, SW_SHOWNORMAL);
    SetWindowPos(g_bar, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW); BringWindowToTop(g_bar);
    char buf[256];
    sprintf_s(buf, sizeof(buf), "launch %d -> %p", index, (void *)r);
    bar_log(buf);
}

/* ---- buyuk dokunma menüsü ---- */

static void build_menu_items(void) {
    scan_programs();
    g_itemCount = 0;
    g_items[g_itemCount].kind = 0;
    g_items[g_itemCount].index = 0;
    lstrcpynW(g_items[g_itemCount].label, L"Görev Yöneticisi", 80);
    g_itemCount++;
    for (int i = 0; i < g_progCount && g_itemCount < MAX_ITEMS - 1; i++) {
        g_items[g_itemCount].kind = 1;
        g_items[g_itemCount].index = i;
        lstrcpynW(g_items[g_itemCount].label, g_progNames[i], 80);
        g_itemCount++;
    }
    g_items[g_itemCount].kind = 2;
    g_items[g_itemCount].index = 0;
    lstrcpynW(g_items[g_itemCount].label, L"Görev çubuğunu kapat", 80);
    g_itemCount++;
}

static LRESULT CALLBACK menu_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH bg = CreateSolidBrush(RGB(26, 30, 38));
        FillRect(dc, &rc, bg);
        DeleteObject(bg);
        SetBkMode(dc, TRANSPARENT);
        SelectObject(dc, g_menuFont);
        for (int i = 0; i < g_itemCount; i++) {
            RECT row = {0, i * MENU_ROW, rc.right, (i + 1) * MENU_ROW};
            if (i == g_hover) {
                HBRUSH hot = CreateSolidBrush(RGB(58, 70, 90));
                FillRect(dc, &row, hot);
                DeleteObject(hot);
            }
            SetTextColor(dc, RGB(232, 236, 242));
            RECT text = {18, row.top, rc.right - 12, row.bottom};
            DrawTextW(dc, g_items[i].label, -1, &text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(48, 54, 66));
            HGDIOBJ old = SelectObject(dc, pen);
            MoveToEx(dc, 12, row.bottom - 1, NULL);
            LineTo(dc, rc.right - 12, row.bottom - 1);
            SelectObject(dc, old);
            DeleteObject(pen);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_MOUSEMOVE: {
        int y = GET_Y_LPARAM(lp);
        int i = y / MENU_ROW;
        if (i < 0 || i >= g_itemCount) i = -1;
        if (i != g_hover) {
            g_hover = i;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        int y = GET_Y_LPARAM(lp);
        int x = GET_X_LPARAM(lp);
        RECT rc;
        GetClientRect(hwnd, &rc);
        if (x >= 0 && x < rc.right) {
            int i = y / MENU_ROW;
            if (i >= 0 && i < g_itemCount) {
                char buf[64];
                sprintf_s(buf, sizeof(buf), "pick %d", i);
                bar_log(buf);
                MenuItem item = g_items[i];
                DestroyWindow(hwnd);
                if (item.kind == 1) launch_program(item.index);
                else if (item.kind == 0) ShellExecuteW(NULL, L"open", L"taskmgr.exe", NULL, NULL, SW_SHOWNORMAL);
                else if (item.kind == 2) DestroyWindow(g_bar);
                return 0;
            }
        }
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) DestroyWindow(hwnd);
        return 0;
    case WM_KILLFOCUS:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_menu = NULL;
        g_hover = -1;
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static void open_menu(void) {
    if (g_menu) {
        DestroyWindow(g_menu);
        return;
    }
    build_menu_items();
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int h = g_itemCount * MENU_ROW + 4;
    int maxH = sh - BAR_HEIGHT - 16;
    int rows = g_itemCount;
    if (h > maxH) {
        rows = (maxH - 4) / MENU_ROW;
        if (rows < 1) rows = 1;
        h = rows * MENU_ROW + 4;
        g_itemCount = rows;
    }
    (void)sw;
    g_menu = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"WinOsMenu", L"WinOsMenu",
        WS_POPUP, 8, sh - BAR_HEIGHT - 8 - h, MENU_WIDTH, h, NULL, NULL, GetModuleHandleW(NULL), NULL);
    if (!g_menu) return;
    ShowWindow(g_menu, SW_SHOW);
    SetForegroundWindow(g_menu);
    SetFocus(g_menu);
    SetTimer(g_menu, 1, 100, NULL);
}

static LRESULT CALLBACK bar_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        g_selfPid = GetCurrentProcessId();
        g_font = CreateFontW(-18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        g_menuFont = CreateFontW(-22, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        g_bg = CreateSolidBrush(RGB(24, 28, 34));
        g_hot = CreateSolidBrush(RGB(48, 56, 68));
        g_start = CreateWindowExW(0, L"BUTTON", L"Başlat",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
            0, 0, 10, 10, hwnd, (HMENU)ID_START, NULL, NULL);
        SendMessageW(g_start, WM_SETFONT, (WPARAM)g_font, TRUE);
        refresh_tasks();
        SetTimer(hwnd, TIMER_REFRESH, 800, NULL);
        return 0;
    }
    case WM_TIMER:
        if (wp == TIMER_REFRESH) {
            refresh_tasks();
            if (g_menu == NULL) {
                SetWindowPos(g_bar, HWND_TOP, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                BringWindowToTop(g_bar);
            }
        }
        return 0;
    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == ID_START) {
            open_menu();
        } else if (id >= ID_TASK_BASE && id < ID_TASK_BASE + MAX_TASKS) {
            activate(g_taskHwnd[id - ID_TASK_BASE]);
            SetTimer(hwnd, TIMER_REFRESH, 800, NULL);
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
    case WM_DESTROY: {
        KillTimer(hwnd, TIMER_REFRESH);
        RECT full = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
        SystemParametersInfoW(SPI_SETWORKAREA, 0, &full, SPIF_SENDCHANGE);
        PostQuitMessage(0);
    }
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

    WNDCLASSEXW mc = {0};
    mc.cbSize = sizeof(mc);
    mc.lpfnWndProc = menu_proc;
    mc.hInstance = inst;
    mc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    mc.hbrBackground = NULL;
    mc.lpszClassName = L"WinOsMenu";
    if (!RegisterClassExW(&mc)) return 1;

    int sh = GetSystemMetrics(SM_CYSCREEN);
    int sw = GetSystemMetrics(SM_CXSCREEN);

    g_bar = CreateWindowExW(WS_EX_TOOLWINDOW, L"WinOsBar", L"WinOsBar",
        WS_POPUP | WS_VISIBLE, 0, sh - BAR_HEIGHT, sw, BAR_HEIGHT, NULL, NULL, inst, NULL);
    if (!g_bar) return 1;

    ShowWindow(g_bar, SW_SHOW);
    UpdateWindow(g_bar);
    bar_log("bar started");
    RECT wa = {0, 0, sw, sh - BAR_HEIGHT};
    SystemParametersInfoW(SPI_SETWORKAREA, 0, &wa, SPIF_SENDCHANGE);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
