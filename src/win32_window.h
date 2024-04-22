#pragma once


#include <wrl.h>
#include "webview.h"
#include "WebView2.h"
using Microsoft::WRL::Callback;


using std::string;
using std::string_view;
using std::cout;
using std::endl;
using std::streambuf;
using std::ofstream;
using std::ifstream;
using std::ostringstream;


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
            PostQuitMessage(0);
            exit(1);
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



