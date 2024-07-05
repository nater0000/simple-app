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
#if defined(USE_BEENUM)
#include "BeeNum/Math.h"
#endif // USE_BEENUM

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

#define WINDOW_WIDTH_DEFAULT (720)
#define WINDOW_HEIGHT_DEFAULT (640)
#define WINDOW_WIDTH_MIN (1024)
#define WINDOW_HEIGHT_MIN (360)

#if defined(USE_CEMBED)
#if !defined(EMBEDDED_ROOT_DIR)
#error "Error: Set Define EMBEDDED_ROOT_DIR"
#endif // EMBEDDED_ROOT_DIR
#endif // USE_CEMBED

struct MainParams {
    string name;
    ofstream logFileH;
    streambuf* restoreCout;
};
MainParams _gParams;


void handleClick(webui::window::event* e) {

    // JavaScript:
    // webui.call('MyID_One', 'Hello');

    std::string str_1 = e->get_string(); // Or e->get_string(0);
    //std::string str_2 = e->get_string(1);
    string str_2 = _gParams.name;

    cout << "handleClick: " << str_1 << str_2 << endl; // Hello
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

    err = getenv_s(&ret, buffer, bufferSize, "APPDATA");
    if (ret > 0 && 0 == err) {
        appdataPath = string(buffer) + "\\Simple-App";
        //appdataPath = string(buffer);
    }

    // Fallback to using the current directory
    DWORD result = GetCurrentDirectoryA(MAX_LOADSTRING, buffer);
    if (0 != result) {
        appdataPath = buffer;
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

void handleCalc(webui::window::event* e) {

    // From JavaScript:
    // webui.call('JsBtnCalc', num2, op, num2).then(...)

    size_t tailLen = 0;
    string a = e->get_string(0);
    //long long op = e->get_int(1);
    string op = e->get_string(1);
    string b = e->get_string(2);

    const char* begin = a.c_str();
    char* end = (char*)(a.c_str() + a.length());
    long long number = strtoll(begin, &end, 10);
#if defined(USE_BEENUM)
    BeeNum::Bint intA(a);
    BeeNum::Bint intB(b);
    BeeNum::Bint res(-1);
#else
    unsigned long long intA = 0;
    unsigned long long intB = 0;
    unsigned long long res = -1;
#endif // USE_BEENUM

    string opStr = "?";
    if (0 == op.compare("add")) {
        opStr = "+";
        res = intA + intB;
    }
    else if (0 == op.compare("subtract")) {
        opStr = "-";
        res = intA - intB;
    }
    else if (0 == op.compare("multiply")) {
        opStr = "x";
        res = intA * intB;
    }
    else if (0 == op.compare("divide")) {
        opStr = "÷";
        if (intB == 0) {
            // Avoid divide by zero
            res = 0;
        }
        else {
            res = intA / intB;
        }
    }

#if defined(USE_BEENUM)
    string resultStr = res.toString();
#else
    string resultStr = std::to_string(res);
#endif // USE_BEENUM
    cout << "handleCalc: " << a << " " << opStr << " " << b << " = " << resultStr << endl;

    // Send back the response to JavaScript
    e->return_string(resultStr);
}

void handleNextPage(webui::window::event* e)
{
    // From JavaScript:
    // webui.call('JsBtnNext', "next_page.html")
    
    webui_run(e->window, "window.location.href='page2.html';");
    //webui_navigate(e->window, "page2.html");
}

void handleExit(webui::window::event* e)
{
    // From JavaScript:
    // webui.call('JsBtnExit')

    webui::exit();
    PostQuitMessage(0);
    exit(1);
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
#if defined(CEMBED_TRANSLATE)
#undef FILE
#endif // CEMBED_TRANSLATE
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

#if defined(_DEBUG) || 1
    // Redirect cout to file 'console.log'
    setupCout();

    // Creates a Console window for printf
    setupPrintf();
#endif // _DEBUG

    cout << "<<<<<<<< test cout   <<<<<<<<" << endl;
    printf( ">>>>>>>> test printf >>>>>>>>\n" );
    fflush(stdout);

    _gParams.name = getUserName();

    // @TODO: Rename or refactor webui to not use 'window'
    webui::window webserver{};
    //webui::set_timeout(2);

    webserver.bind("JsBtnCalc", handleCalc);
    webserver.bind("JsBtnNext", handleNextPage);
    webserver.bind("JsBtnExit", handleExit);
    //webserver.bind("", handleWebEvents);

    // Note: When using c-embed, this needs to match the directory provided
    string root_dir;
#if defined(USE_CEMBED)
    root_dir = EMBEDDED_ROOT_DIR;
    webserver.set_root_folder(root_dir);
    root_dir = "*virtual*";
#else // USE_CEMBED
    root_dir = GetAppDir();
    webserver.set_root_folder(root_dir);
#endif // USE_CEMBED

    // show_browser will initialize the webserver and
    //  facilitate javascript bindings with the server
    //  for the first 'view' id.
    // using browser=NoBrowser(0) to avoid spawning
    //  instances of a locally-installed browser via webui
    webserver.show_browser("index.html", 0);
    cout << "port: " << webserver.get_port() << endl;

    ostringstream url;
    url << "http://localhost:" << webserver.get_port() << "/index.html";

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
    wnd.set_title(_gParams.name + "> ~[" + url.str() + " ]~[ " + root_dir + " ]~");

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

#if 0
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
#endif // 0
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


