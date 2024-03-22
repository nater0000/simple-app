// simple-app.cpp : Defines the entry point for the application.
//
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "framework.h"
#include "simple-app.h"
#include "webui.hpp"
#include "html.h"
#include <chrono>
#include <format>
#include "webview.h"

using std::string;
using std::string_view;
using std::cout;
using std::endl;
using std::streambuf;
using std::ofstream;
using std::ifstream;
using std::ostringstream;
using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::chrono::milliseconds;
using std::format;

#define MAX_LOADSTRING (100)
#define MAX_LOADSTRING_300 (300)

#define WINDOW_WIDTH_DEFAULT (640)
#define WINDOW_HEIGHT_DEFAULT (480)
#define WINDOW_WIDTH_MIN (1020)
#define WINDOW_HEIGHT_MIN (400)


struct MainParams {
    string name;
    ofstream logFileH;
    streambuf* restoreCout;
    string htmlPath;
    string htmlFilename;
    HWND hWnd;
};
MainParams _gParams;

#include <map>
#include <list>
#include <queue>
using std::map;
using std::list;
using std::queue;

typedef map<string, queue<string>> ParamMap;
ParamMap _p;


string generateHtml_Main(const MainParams& m)
{
    string html = getDevHtml();
    // @TODO: Update html with params
    return html;
}


void handleRefresh(webui::window::event* e) {

    // JavaScript:
    // webui.call('MyID_One', 'Hello');

    std::string str_1 = e->get_string(); // Or e->get_string(0);
    //std::string str_2 = e->get_string(1);
    string str_2 = _gParams.name;

    cout << "my_function_string 1: " << str_1 << endl; // Hello
    cout << "my_function_string 2: " << str_2 << endl; // World
}

string getUserName()
{
#ifdef WIN32
    CHAR szBuffer[MAX_LOADSTRING];
    DWORD len = MAX_LOADSTRING;
    if (GetUserNameA(szBuffer, &len))
        return string(szBuffer);

    return "Unknown";
#else
    struct passwd* pwd;
    uid_t userid = getuid();
    pwd = getpwuid(userid);
    return pwd->pw_name;
#endif //
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
#ifdef WIN32
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
#else

#endif //
}

string GetAppDir()
{
    // Use the HTML_DIR environment variable if it is set
    //  to determine the working directory

    string appdataPath = "";
#ifdef WIN32
    size_t ret = 0;
    errno_t err = 0;
    char buffer[MAX_LOADSTRING] = { 0 };
    size_t bufferSize = sizeof(buffer);

    // Fallback to using the current directory
    DWORD result = GetCurrentDirectoryA(MAX_LOADSTRING, buffer);
    if (0 == result) {
        appdataPath = buffer;
    }

    err = getenv_s(&ret, buffer, bufferSize, "APPDATA");
    if (ret > 0 && 0 == err) {
        appdataPath = string(buffer) + "\\Simple-App";
        //appdataPath = string(buffer);
    }

    err = getenv_s(&ret, buffer, bufferSize, "HTML_PATH");
    if (ret > 0 && 0 == err) {
        appdataPath = buffer;
    }

#else
    appdataPath = getenv("HTML_PATH");
#endif // WIN32

    return appdataPath;
}

string strip_trailing(string s, string trail) {
    size_t eraseLen = 0;
    size_t strLen = s.length();
    size_t trailLen = trail.length();

    if (s.ends_with(trail.c_str())) {
        eraseLen = trailLen;
    }
    //char* end = (char*)(s.c_str() + (strLen - eraseLen));
    return string(s, 0, strLen - eraseLen);
    //long long number = strtoll(s.c_str(), &end, 10);
}

void handleIndex(webui::window::event* e) {

    // JavaScript:
    // webui.call('MyID_Four', number, 2).then(...)

    //long long number = e->get_int(0);
    size_t tailLen = 0;
    string numberMs = e->get_string(0);
    long long times = e->get_int(1);

#if 0
    numberMs = strip_trailing(numberMs, string("ms"));
    if (numberMs.ends_with("ms")) {
        tailLen = 2;
    }
    char* end = (char*)(numberMs.c_str() + (numberMs.length() - tailLen));
    long long number = strtoll(numberMs.c_str(), &end, 10);
#endif // 0
    char* end = (char*)(numberMs.c_str() + numberMs.length());
    long long number = strtoll(numberMs.c_str(), &end, 10);
    long long res = number * times;

    cout << "handleIndex: " << number << " * " << times << " = " << res << std::endl;

    // Send back the response to JavaScript
    e->return_int(res);
}

void handleWebEvents(webui::window::event* e)
{
    //
    // This function gets called every time there is an event.
    switch (e->event_type) {
    case WEBUI_EVENT_CONNECTED:
        cout << "Connected." << endl;
        break;
    case WEBUI_EVENT_DISCONNECTED:
        cout << "Disconnected" << endl;
        e->get_window().close();
        break;
    //
    // The CLICK event is received after the onClickButton()
    //  events are handled
    case WEBUI_EVENT_MOUSE_CLICK:
        cout << "Clicked element: " << e->element << endl;
        break;
    //
    // WebUI will block all `href` link clicks and sent here instead.
    // We can then control the behaviour of links as needed.
    case WEBUI_EVENT_NAVIGATION: {
            string url = e->get_string();
            cout << "Starting navigation to: " << url << endl;

            webui_navigate(e->window, url.c_str());
            break;
        }

    // Leave this undefined in Debug builds to elicit an 'Unhandled' warning
    //  when a new enum is added and missing from the switch statement
#ifdef NDEBUG
    default:
        cout << "Unhandled event type: " << e->event_type << endl;
        break;
#endif // NDEBUG
    }
}

void setupCout()
{
    // Some C++ wizardry to write cout to file because there is no console
    _gParams.logFileH.open("console.log");

    //_gParams.restoreCout = cout.rdbuf(_gParams.logFileH.rdbuf());
    _gParams.restoreCout = cout.rdbuf();
    cout.rdbuf(_gParams.logFileH.rdbuf());
    cout << std::flush;
}
void setupPrintf()
{
    if (AllocConsole())
    {
        FILE* fi = 0;
        freopen_s(&fi, "CONOUT$", "w", stdout);
    }
}

int invokeUiMain(_In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                RegisterScreen(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SIMPLEAPP, szWindowClass, MAX_LOADSTRING);
#if 0
    RegisterScreen(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }
#endif // 0

    int ret = invokeUiMain(lpCmdLine, nCmdShow);
    return ret;
}

#if 0
int APIENTRY oldMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SIMPLEAPP, szWindowClass, MAX_LOADSTRING);
    RegisterScreen(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLEAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}
#endif // 0

using webview::exception;
using webview::error_info;
using webview::detail::enable_dpi_awareness;
using webview::detail::get_window_dpi;
using webview::detail::get_default_window_dpi;
using webview::detail::enable_non_client_dpi_scaling_if_needed;
using webview::detail::apply_window_theme;
using webview::detail::make_window_frame_size;
using webview::detail::scale_size;
using webview::detail::widen_string;
using webview::detail::basic_result;
using webview::detail::com_init_wrapper;
using webview::detail::msg_cb_t;
using webview::detail::webview2_com_handler;
using webview::detail::json_parse;

using namespace webview::detail::mswebview2;
//using webview::detail::mswebview2;

template <typename T>
using result = basic_result<T, error_info, exception>;

using noresult = basic_result<void, error_info, exception>;

using dispatch_fn_t = std::function<void()>;

class win32_base
{
public:
    virtual ~win32_base() = default;

#if 0
    noresult navigate(const std::string& url) {
        if (url.empty()) {
            return navigate_impl("about:blank");
        }
        return navigate_impl(url);
    }
#endif // 0

    result<void*> window() { return window_impl(); }
    result<void*> widget() { return widget_impl(); }
    result<void*> controller() { return controller_impl(); };
    //noresult run() { return run_impl(); }
    noresult terminate() { return terminate_impl(); }
    noresult dispatch(std::function<void()> f) { return dispatch_impl(f); }
    noresult set_title(const std::string& title) { return set_title_impl(title); }

    noresult set_size(int width, int height, webview_hint_t hints) {
        return set_size_impl(width, height, hints);
    }

    noresult set_content(const std::string& content) { return set_content_impl(content); }

    noresult init(const std::string& js) {
        //add_user_script(js);
        return {};
    }

    //noresult eval(const std::string& js) { return eval_impl(js); }

protected:
    //virtual noresult navigate_impl(const std::string& url) = 0;
    virtual result<void*> window_impl() = 0;
    virtual result<void*> widget_impl() = 0;
    virtual result<void*> controller_impl() = 0;
    //virtual noresult run_impl() = 0;
    virtual noresult terminate_impl() = 0;
    virtual noresult dispatch_impl(std::function<void()> f) = 0;
    virtual noresult set_title_impl(const std::string& title) = 0;
    virtual noresult set_size_impl(int width, int height, webview_hint_t hints) = 0;
    virtual noresult set_content_impl(const std::string& content) = 0;
    //virtual noresult eval_impl(const std::string& js) = 0;

    virtual void on_window_created() { inc_window_count(); }

    virtual void on_window_destroyed(bool skip_termination = false) {
        if (dec_window_count() <= 0) {
            if (!skip_termination) {
                terminate();
            }
        }
    }

private:
    static std::atomic_uint& window_ref_count() {
        static std::atomic_uint ref_count{ 0 };
        return ref_count;
    }

    static unsigned int inc_window_count() { return ++window_ref_count(); }

    static unsigned int dec_window_count() {
        auto& count = window_ref_count();
        if (count > 0) {
            return --count;
        }
        return 0;
    }

};

class win32_screen : public webview::detail::win32_edge_engine
{
public:
    static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    win32_screen(bool debug, void* window) : win32_edge_engine{ debug, window } {
        HINSTANCE hInstance = GetModuleHandle(nullptr);
#if 0
        m_com_init = { COINIT_APARTMENTTHREADED };
        enable_dpi_awareness();

        HICON icon = (HICON)LoadImage(
            hInstance, IDI_APPLICATION, IMAGE_ICON, GetSystemMetrics(SM_CXICON),
            GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);

        // Create a top-level window.
        WNDCLASSEXW wc;
        ZeroMemory(&wc, sizeof(WNDCLASSEX));
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.hInstance = hInstance;
        wc.lpszClassName = L"webview";
        wc.hIcon = icon;
        wc.lpfnWndProc = (WNDPROC)(+[](HWND hwnd, UINT msg, WPARAM wp,
            LPARAM lp) -> LRESULT {
                win32_screen* w{};

                if (msg == WM_NCCREATE) {
                    auto* lpcs{ reinterpret_cast<LPCREATESTRUCT>(lp) };
                    w = static_cast<win32_screen*>(lpcs->lpCreateParams);
                    w->m_window = hwnd;
                    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(w));
                    enable_non_client_dpi_scaling_if_needed(hwnd);
                    apply_window_theme(hwnd);
                }
                else {
                    w = reinterpret_cast<win32_screen*>(
                        GetWindowLongPtrW(hwnd, GWLP_USERDATA));
                }

                if (!w) {
                    return DefWindowProcW(hwnd, msg, wp, lp);
                }

                switch (msg) {
                case WM_SIZE:
                    w->resize_widget();
                    break;
                case WM_CLOSE:
                    DestroyWindow(hwnd);
                    break;
                case WM_DESTROY:
                    w->m_window = nullptr;
                    SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
                    w->on_window_destroyed();
                    break;
                case WM_GETMINMAXINFO: {
                    auto lpmmi = (LPMINMAXINFO)lp;
                    if (w->m_maxsz.x > 0 && w->m_maxsz.y > 0) {
                        lpmmi->ptMaxSize = w->m_maxsz;
                        lpmmi->ptMaxTrackSize = w->m_maxsz;
                    }
                    if (w->m_minsz.x > 0 && w->m_minsz.y > 0) {
                        lpmmi->ptMinTrackSize = w->m_minsz;
                    }
                } break;
                case 0x02E4 /*WM_GETDPISCALEDSIZE*/: {
                    auto dpi = static_cast<int>(wp);
                    auto* size{ reinterpret_cast<SIZE*>(lp) };
                    *size = w->get_scaled_size(w->m_dpi, dpi);
                    return TRUE;
                }
                case 0x02E0 /*WM_DPICHANGED*/: {
                    // Windows 10: The size we get here is exactly what we supplied to WM_GETDPISCALEDSIZE.
                    // Windows 11: The size we get here is NOT what we supplied to WM_GETDPISCALEDSIZE.
                    // Due to this difference, don't use the suggested bounds.
                    auto dpi = static_cast<int>(HIWORD(wp));
                    w->on_dpi_changed(dpi);
                    break;
                }
                case WM_SETTINGCHANGE: {
                    auto* area = reinterpret_cast<const wchar_t*>(lp);
                    if (area) {
                        w->on_system_setting_change(area);
                    }
                    break;
                }
                case WM_ACTIVATE:
                    if (LOWORD(wp) != WA_INACTIVE) {
                        w->focus_webview();
                    }
                    break;
                default:
                    return DefWindowProcW(hwnd, msg, wp, lp);
                }
                return 0;
            });
        RegisterClassExW(&wc);

        CreateWindowW(L"webview", L"", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
            CW_USEDEFAULT, 0, 0, nullptr, nullptr, hInstance, this);
        if (!m_window) {
            throw exception{ WEBVIEW_ERROR_INVALID_STATE, "Window is null" };
        }
        on_window_created();

        m_dpi = get_window_dpi(m_window);
        constexpr const int initial_width = 640;
        constexpr const int initial_height = 480;
        set_size(initial_width, initial_height, WEBVIEW_HINT_NONE);

        ...
#if 0

        // See win32_edge_engine
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);

        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = hInstance;
        wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEAPP));
        wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIMPLEAPP);
        wcex.lpszClassName  = szWindowClass;
        wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
        wcex.lpfnWndProc    = &win32_screen::WndProc;

        RegisterClassExW(&wcex);

        HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

        if (!hWnd)
        {
            //return;
            assert(false);
        }
        m_window = hWnd;

        ShowWindow(hWnd, true);
        UpdateWindow(hWnd);
#endif // 0
#endif // 0
    }


    ~win32_screen() {
    }

    win32_screen(const win32_screen& other) = delete;
    win32_screen& operator=(const win32_screen& other) = delete;
    win32_screen(win32_screen&& other) = delete;
    win32_screen& operator=(win32_screen&& other) = delete;


    noresult set_position(int x, int y) {
#if 1
        SetWindowPos(m_window, nullptr, x, y, 0, 0,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE |
            SWP_FRAMECHANGED);
#endif // 0
        return {};
    }

protected:
};


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


#include <wrl.h>
//#include <wil/com.h>
#include "WebView2.h"
using Microsoft::WRL::Callback;

using webview::detail::win32_edge_engine;
class win32_window
{
public:
    bool m_init;
    win32_edge_engine* m_webview;

    // edge engine also has copies of these values
    HWND m_window;
    POINT m_minsz = POINT{ 0, 0 };
    POINT m_maxsz = POINT{ 0, 0 };
    DWORD m_main_thread = GetCurrentThreadId();
    int m_dpi{};

    win32_window()
        : m_window(nullptr)
        , m_webview(nullptr)
        , m_init(false)
    {
    }

    virtual ~win32_window()
    {
        if (m_webview) {
            delete m_webview;
        } m_webview = nullptr;
        m_window = nullptr;
        m_init = false;
    }

    bool init(HWND hWnd)
    {
        if (m_init) {
            return true;
        }
        if (!hWnd) {
            return false;
        }

        m_webview = new win32_edge_engine(false, hWnd);
        if (!m_webview) {
            return false;
        }

        m_window = hWnd;
        m_init = true;
        return true;
    }

    bool navigate(string url)
    {
        //ICoreWebView2* webview
        if (!m_webview) { return false; }
        return m_webview->navigate(url).ok();
    }

    bool set_title(string title)
    {
        if (!m_window) { return false; }
        bool bResult = SetWindowTextW(m_window, widen_string(title).c_str());
        return bResult;
    }

    bool set_position(int x, int y)
    {
        return false;
    }

    bool set_size(int width, int height, webview_hint_t hints)
    {
        if (!m_window) { return false; }

        auto style = GetWindowLong(m_window, GWL_STYLE);
        if (hints == WEBVIEW_HINT_FIXED) {
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        }
        else {
            style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
        }
        SetWindowLong(m_window, GWL_STYLE, style);

        if (hints == WEBVIEW_HINT_MAX) {
            m_maxsz.x = width;
            m_maxsz.y = height;
        }
        else if (hints == WEBVIEW_HINT_MIN) {
            m_minsz.x = width;
            m_minsz.y = height;
        }
        else {
            auto dpi = get_window_dpi(m_window);
            m_dpi = dpi;
            auto scaled_size =
                scale_size(width, height, get_default_window_dpi(), dpi);
            auto frame_size =
                make_window_frame_size(m_window, scaled_size.cx, scaled_size.cy, dpi);
            SetWindowPos(m_window, nullptr, 0, 0, frame_size.cx, frame_size.cy,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE |
                SWP_FRAMECHANGED);
        }
        return {};
    }

    bool eval(string js)
    {
        if (!m_webview) { return false; }
        return m_webview->eval(js).ok();
    }

    static LRESULT CALLBACK WindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        win32_window* w{};

        if (message == WM_NCCREATE) {
            auto* lpcs{ reinterpret_cast<LPCREATESTRUCT>(lParam) };
            w = static_cast<win32_window*>(lpcs->lpCreateParams);
            w->m_window = hWnd;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(w));
            enable_non_client_dpi_scaling_if_needed(hWnd);
            apply_window_theme(hWnd);
        }
        else {
            w = reinterpret_cast<win32_window*>(
                GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }
        if (!w) {
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }

        switch (message)
        {
        case WM_SIZE:
            w->resize_widget();
            break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            w->m_window = nullptr;
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
            w->on_window_destroyed(true);
            PostQuitMessage(0);
            break;

        case WM_GETMINMAXINFO: {
            auto lpmmi = (LPMINMAXINFO)lParam;
            if (w->m_maxsz.x > 0 && w->m_maxsz.y > 0) {
                lpmmi->ptMaxSize = w->m_maxsz;
                lpmmi->ptMaxTrackSize = w->m_maxsz;
            }
            if (w->m_minsz.x > 0 && w->m_minsz.y > 0) {
                lpmmi->ptMinTrackSize = w->m_minsz;
            }
            break;
        }

        case 0x02E4 /*WM_GETDPISCALEDSIZE*/: {
            auto dpi = static_cast<int>(wParam);
            auto* size{ reinterpret_cast<SIZE*>(lParam) };
            *size = w->get_scaled_size(w->m_dpi, dpi);
            return TRUE;
        }

        case 0x02E0 /*WM_DPICHANGED*/: {
            // Windows 10: The size we get here is exactly what we supplied to WM_GETDPISCALEDSIZE.
            // Windows 11: The size we get here is NOT what we supplied to WM_GETDPISCALEDSIZE.
            // Due to this difference, don't use the suggested bounds.
            auto dpi = static_cast<int>(HIWORD(wParam));
            w->on_dpi_changed(dpi);
            break;
        }

        case WM_SETTINGCHANGE: {
            auto* area = reinterpret_cast<const wchar_t*>(lParam);
            if (area) {
                w->on_system_setting_change(area);
            }
            break;
        }

        case WM_ACTIVATE:
            if (LOWORD(wParam) != WA_INACTIVE) {
                w->focus_webview();
            }
            break;

        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    void focus_webview() {
        if (!m_webview) { return; }

        //ICoreWebView2Controller* controller
        ICoreWebView2Controller* controller = (ICoreWebView2Controller*)m_webview->browser_controller().value();

        // get webview controller
        if (m_webview->browser_controller().ok()) {
            controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
        }
    }

    void resize_widget() {
        if (!m_webview) { return; }

        HWND widget = (HWND)m_webview->widget().value();

        // get webview widget
        if (m_webview->widget().ok()) {
            RECT r{};
            if (GetClientRect(GetParent(widget), &r)) {
                MoveWindow(widget, r.left, r.top, r.right - r.left, r.bottom - r.top,
                    TRUE);
            }
        }
    }

    void on_dpi_changed(int dpi) {
        auto scaled_size = get_scaled_size(m_dpi, dpi);
        auto frame_size =
            make_window_frame_size(m_window, scaled_size.cx, scaled_size.cy, dpi);
        SetWindowPos(m_window, nullptr, 0, 0, frame_size.cx, frame_size.cy,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
        m_dpi = dpi;
    }

    SIZE get_size() const {
        RECT bounds;
        GetClientRect(m_window, &bounds);
        auto width = bounds.right - bounds.left;
        auto height = bounds.bottom - bounds.top;
        return { width, height };
    }

    SIZE get_scaled_size(int from_dpi, int to_dpi) const {
        auto size = get_size();
        return scale_size(size.cx, size.cy, from_dpi, to_dpi);
    }

    void on_system_setting_change(const wchar_t* area) {
        // Detect light/dark mode change in system.
        if (lstrcmpW(area, L"ImmersiveColorSet") == 0) {
            apply_window_theme(m_window);
        }
    }

    virtual void on_window_created() { inc_window_count(); }

    virtual void on_window_destroyed(bool skip_termination = false) {
        if (dec_window_count() <= 0) {
            if (!skip_termination) {
                terminate();
            }
        }
    }

private:
    static std::atomic_uint& window_ref_count() {
        static std::atomic_uint ref_count{ 0 };
        return ref_count;
    }

    static unsigned int inc_window_count() { return ++window_ref_count(); }

    static unsigned int dec_window_count() {
        auto& count = window_ref_count();
        if (count > 0) {
            return --count;
        }
        return 0;
    }

};

HWND CreateScreen(win32_window* w)
{
    enable_dpi_awareness();

    WNDCLASSEX wc = { };
    HWND hwnd;

    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.lpfnWndProc = &win32_window::WindowWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    //wc.lpszClassName = "webview";
    wc.lpszClassName = szWindowClass;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, TEXT("Could not register window class"),
            NULL, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindowExW(WS_EX_LEFT,
        szWindowClass,
        NULL,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInst,
        w);
    if (!hwnd) {
        MessageBox(NULL, TEXT("Could not create window"), NULL, MB_ICONERROR);
        return hwnd;
    }

    w->on_window_created();

    //int m_dpi = get_window_dpi(hwnd);
    w->set_size(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT, WEBVIEW_HINT_NONE);

    ShowWindow(hwnd, true);
    UpdateWindow(hwnd);

    return hwnd;
}

int invokeUiMain(_In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {

    // To automatically redirect cout to file 'console.log' uncomment this:
    //setupCout();

    // To create a Console window for printf uncomment this:
    //setupPrintf();

    cout << "<<<<<<<< test cout   <<<<<<<<" << endl;
    printf( ">>>>>>>> test printf >>>>>>>>\n" );
    fflush(stdout);



    _gParams.name = getUserName();
    //_gParams.htmlPath = getHtmlPath();

    //string mainHtml = generateHtml_Main(_gParams);

    string contentToShow = "index.html";
    if (_gParams.htmlFilename.size()) {
        //taking file as inputstream
        ifstream f(_gParams.htmlFilename);
        if (f) {
            ostringstream ss;
            ss << f.rdbuf(); // reading data
            contentToShow = ss.str();
        }
    }


    //webui::window view{};
    // @TODO: Separate webserver API from window API
    //webui::window& webserver = view;
    webui::window webserver{};

    //webui::set_timeout(40);

#if 0
    try {
        webview::webview w(false, nullptr);
        w.set_title("Basic Example");
        w.set_size(480, 320, WEBVIEW_HINT_NONE);
        w.set_html("Thanks for using webview!");
        w.run();
    }
    catch (const webview::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
#endif // 0

    webserver.bind("", handleWebEvents);
    webserver.bind("RefreshButtonHandler", handleRefresh);
    webserver.bind("JsBtnDouble", handleIndex);

    webserver.set_root_folder(GetAppDir());
    // show_browser will initialize the webserver and
    //  facilitate javascript bindings with the server
    //  for the first 'view' id.
    // using browser=NoBrowser(0) to avoid spawning
    //  instances of a locally-installed browser via webui
    webserver.show_browser("index.html", 0);
    cout << "port: " << webserver.get_server_port() << endl;

#if 1
    //webserver.validate_required({ "index.html", "404.html" });
    //view.navigate("index.html");
    //window.init("index.html");
    ostringstream url;
    url << "http://localhost:" << webserver.get_server_port() << "/index.html";

    //window.navigate("C:\\Users\\Ace01\\AppData\\Roaming\\Simple-App\\index.html");
    //window.navigate(url.str());
    //webview_navigate(w, url.str().c_str());
    //window.set_html("https://localhost/index.html");
    //window.run();
#endif // USE_WEBVIEW

    int horizontal = 0;
    int vertical = 0;
    GetDesktopResolution(horizontal, vertical);


    //win32_screen screen(false, _gParams.hWnd);

#if 1
    win32_window wnd{};
    HWND hWnd = CreateScreen(&wnd);
    wnd.init(hWnd);
#else
    webview::webview wnd(false, nullptr);
    HWND hWnd = static_cast<HWND>(wnd.window().value());
#endif // 0
    //HWND hWnd = _gParams.hWnd = CreateScreen();

    wnd.set_title(_gParams.name + "'s App ~[" + url.str() + " ][ " + GetAppDir() + " ]~");

    // @FIX: window position
    //wnd.set_position(horizontal, vertical);
    wnd.set_size(WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN, WEBVIEW_HINT_MIN);
    wnd.set_size(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT, WEBVIEW_HINT_NONE);

    // @TODO: Separate webview from window
    wnd.navigate(url.str());

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    SetFocus(hWnd);


    HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_SIMPLEAPP));
    MSG msg;

    const char* const setPropertyStr = "document.getElementById('{}').{} = '{}';";
    auto tStart = steady_clock::now();
    auto tLast = tStart;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        if (TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);

        auto tNow = steady_clock::now();
        auto durMs = duration_cast<milliseconds>(tNow - tLast);

        auto strMs = format("{}", durMs);
        char* end = (char*)(strMs.c_str() + strMs.length());
        long long number = strtoll(strMs.c_str(), &end, 10);

        if (number > 10) {
            auto js = format(setPropertyStr, "ElementId", "value", number);
            tLast = tNow;

            wnd.eval(js);
        }
    }

#if 0
    auto tStart = steady_clock::now();
    //for (; view.is_shown(); ) {
    for (; 1; ) {
        auto tNow = steady_clock::now();
        auto durMs = duration_cast<milliseconds>(tNow - tStart);
        auto js = format(setPropertyStr, "ElementId", "value", durMs);
        view.run(js);
        Sleep(100);
    }
#endif // 0

    // All views should already be closed
    // view.close();

    webui::wait();
    webui::clean();

    // make sure to restore the original so we don't get a crash on close!
    cout.rdbuf(_gParams.restoreCout);
    _gParams.logFileH.close();

    return 0;
}


