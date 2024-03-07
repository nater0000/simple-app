// simple-app.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <fstream>
#include "framework.h"
#include "simple-app.h"
#include "webui.hpp"

using std::string;
using std::cout;
using std::endl;
using std::streambuf;
using std::ofstream;

#define MAX_LOADSTRING (100)

const string _HtmlHelloWorld = R"(<html><head><script src=\"webui.js\"></script></head> C++ Hello World ! </html>)";

const string _HtmlMain = R"(
      <html>
        <head>
          <meta charset="UTF-8">
          <script src="webui.js"></script>

          <title>BLE Info</title>
          <style>
            body {
              background: linear-gradient(to left, #36265a, #654da9);
              color: AliceBlue;
              font-size: 16px sans-serif;
              text-align: center;
              margin-top: 30px;
            }
            button {
              margin: 5px 0 10px;
            }
          </style>
        </head>
        <body>
          <h1>Displaying BLE Info</h1>
          <p>(<em>loading ...</em>)</p>
          <button onclick="RefreshButtonHandlerJS();">Refresh</button>
          <br>
          <p>Call a C++ function that returns a response</p>
          <div>Query: <input type="text" id="IndexTrackerDiv" value="0">
          <button onclick="IndexButtonHandlerJS();">Query Status</button>
            </div>
          <script>
            function IndexButtonHandlerJS() {
                webui.call('RefreshButtonHandler', 'Hello');
            }
            function IndexButtonHandlerJS() {
              const indexInput = document.getElementById('IndexTrackerDiv');
              const number = indexInput.value;
              webui.call('IndexButtonHandler', number, 2).then((response) => {
                indexInput.value = response;
              });
            }
          </script>
        </body>
      </html>
    )";

struct MainParams {
    string name;
    ofstream logFileH;
    streambuf* restoreCout;
    string htmlPath;
};
MainParams _gParams;

string generateHtml_Main(const MainParams& m)
{
    string html = _HtmlMain;
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

bool isValidPath(string path)
{
    // @TODO: 
    return false;
}

string sanitizePath(string path)
{
    // @TODO: 
    return path;
}

string getHtmlPath()
{
    // Use the HTML_DIR environment variable if it is set
    //  to determine the working directory
    const string indexPage = "index.html";

    string userPath = "\\html\\";
#ifdef WIN32
    size_t ret = 0;
    char buffer[MAX_LOADSTRING] = { 0 };
    size_t bufferSize = sizeof(buffer);
    errno_t err = getenv_s(&ret, buffer, bufferSize, "HTML_PATH");
    if (ret > 0 && 0 == err) {
        userPath = buffer;
    }
#else
    string envPath = getenv("HTML_PATH");
#endif // WIN32

    string cleanPath = sanitizePath(userPath);
    if (isValidPath(cleanPath)) {
        return cleanPath + indexPage;
    }

    // Fallback to using the current directory
#ifdef WIN32
	GetCurrentDirectoryA(MAX_LOADSTRING, buffer);
	return string(buffer) + string("\\") + indexPage;
#endif // WIN32

    // @TODO: Check non-win32
    return "";
}

void handleIndex(webui::window::event* e) {

    // JavaScript:
    // webui.call('MyID_Four', number, 2).then(...)

    long long number = e->get_int(0);
    long long times = e->get_int(1);

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
    //streambuf* sbuf = cout.rdbuf();
    _gParams.restoreCout = cout.rdbuf(_gParams.logFileH.rdbuf());
    cout << std::flush;
}

int invokeUiMain(_In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {
    // @TODO: This doesn't seem to work
    //nCmdShow = SW_HIDE;

    setupCout();

    webui::window my_window{};

    // This does not seem to work properly at the moment
    if (nCmdShow == SW_HIDE) {
        my_window.set_hide(true);
    }
    else {
        my_window.set_hide(false);
    }

    my_window.bind("", handleWebEvents);
    my_window.bind("RefreshButtonHandler", handleRefresh);
    my_window.bind("IndexButtonHandler", handleIndex);

    _gParams.name = getUserName();
    _gParams.htmlPath = getHtmlPath();

    //string mainHtml = generateHtml_Main(_gParams);

    webui::set_timeout(1);
    my_window.show(_gParams.htmlPath); // my_window.show_browser("index.html", Chrome);
    //my_window.show(mainHtml);

    webui::wait();
    webui::clean();

    // make sure to restore the original so we don't get a crash on close!
    cout.rdbuf(_gParams.restoreCout);
    _gParams.logFileH.close();

    return 0;
}



// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
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
    MyRegisterClass(hInstance);

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



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIMPLEAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

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

#endif // 0