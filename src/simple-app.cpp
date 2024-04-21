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
#include <chrono>
#include <format>
#include "win32_window.h"

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
};
MainParams _gParams;


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
    return string(s, 0, strLen - eraseLen);
}

void handleIndex(webui::window::event* e) {

    // JavaScript:
    // webui.call('MyID_Four', number, 2).then(...)

    size_t tailLen = 0;
    string numberMs = e->get_string(0);
    long long times = e->get_int(1);

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

    // @TODO: Refactor this back to RegisterScreen+InitInstance
    int ret = invokeUiMain(lpCmdLine, nCmdShow);
    return ret;
}

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

    // @TODO: Rename or refactor webui to not use 'window'
    webui::window webserver{};
    //webui::set_timeout(2);

    webserver.bind("", handleWebEvents);
    webserver.bind("RefreshButtonHandler", handleRefresh);
    webserver.bind("JsBtnDouble", handleIndex);

    webserver.set_root_folder(GetAppDir());
    //@TODO: Write the embedded files to disk {"index.html", "404.html", ..}


    // show_browser will initialize the webserver and
    //  facilitate javascript bindings with the server
    //  for the first 'view' id.
    // using browser=NoBrowser(0) to avoid spawning
    //  instances of a locally-installed browser via webui
    webserver.show_browser("index.html", 0);
    cout << "port: " << webserver.get_server_port() << endl;

    ostringstream url;
    url << "http://localhost:" << webserver.get_server_port() << "/index.html";

    // In case the App size should adapt to the Desktop resolution
    int horizontal = 0;
    int vertical = 0;
    GetDesktopResolution(horizontal, vertical);

    // This is primarily a wrapper around a Microsoft WebView2 component
    //  but also abstracts the win32 Window management for simplicity
    // The inspiration comes from the webview project, and ultimately
    //  it can abstracted across platforms.
    win32_window wnd{};

    // This constructs a win32 Window similar to the default Windows project
    // It has been modified to take a context to avoid storing into global memory
    HWND hWnd = CreateScreen(&wnd);
    wnd.init(hWnd);

    // Sets the title of the win32 Window
    wnd.set_title(_gParams.name + "'s App ~[" + url.str() + " ][ " + GetAppDir() + " ]~");

    // @FIX: window position
    //wnd.set_position(horizontal, vertical);
    wnd.set_size(WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN, WEBVIEW_HINT_MIN);
    wnd.set_size(WINDOW_WIDTH_DEFAULT, WINDOW_HEIGHT_DEFAULT, WEBVIEW_HINT_NONE);

    // Load the first requested URL into the view
    wnd.navigate(url.str());

    // win32 Windows APIs to show the App and bring it into focus
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    SetFocus(hWnd);

    // Enable shortcut key 'accelerators' for app to handle
    HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_SIMPLEAPP));
    MSG msg;

    const char* const setPropertyStr = "document.getElementById('{}').{} = '{}';";
    auto tStart = steady_clock::now();
    auto tLast = tStart;

    // Main message loop
    // Currently, this performs the default behavior for managing win32 events
    // There is one additional behavior, where a javascript object will be updated
    //  if more than 10ms has elapsed since the last win32 event was handled
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

    // The webserver was initialized with NoBrowser so make sure to
    //  clean it up if necessary.
    webserver.close();

    webui::wait();
    webui::clean();

    // If necessary, make sure to restore the original so we don't get a crash on close!
    if (_gParams.restoreCout) {
        cout.rdbuf(_gParams.restoreCout);
        _gParams.logFileH.close();
    }

    return 0;
}


