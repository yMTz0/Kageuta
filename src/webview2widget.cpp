#include "webview2widget.h"
#include <QWindow>

class EnvCompletedHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
public:
    EnvCompletedHandler(WebView2Widget* w) : m_widget(w), m_ref(1) {}

    ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG r = --m_ref;
        if (r == 0) delete this;
        return r;
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppv) override {
        if (iid == IID_IUnknown || iid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler) {
            *ppv = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Environment* env) override {
        if (!env) return E_FAIL;
        m_widget->onEnvironmentCreated(env);
        return S_OK;
    }

private:
    WebView2Widget* m_widget;
    ULONG m_ref;
};

class ControllerCompletedHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
public:
    ControllerCompletedHandler(WebView2Widget* w) : m_widget(w), m_ref(1) {}

    ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG r = --m_ref;
        if (r == 0) delete this;
        return r;
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppv) override {
        if (iid == IID_IUnknown || iid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler) {
            *ppv = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT result, ICoreWebView2Controller* controller) override {
        if (!controller) return E_FAIL;
        m_widget->onControllerCreated(controller);
        return S_OK;
    }

private:
    WebView2Widget* m_widget;
    ULONG m_ref;
};

WebView2Widget::WebView2Widget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_NativeWindow);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    initWebView();
}

WebView2Widget::~WebView2Widget() {
    if (m_controller) {
        m_controller->Close();
        m_controller->Release();
        m_controller = nullptr;
    }
    if (m_webview) {
        m_webview->Release();
        m_webview = nullptr;
    }
}

void WebView2Widget::initWebView() {
    HWND hwnd = (HWND)winId();
    CreateCoreWebView2Environment(new EnvCompletedHandler(this));
}

void WebView2Widget::onEnvironmentCreated(ICoreWebView2Environment* env) {
    m_env = env;
    env->AddRef();
    HWND hwnd = (HWND)winId();
    env->CreateCoreWebView2Controller(hwnd, new ControllerCompletedHandler(this));
}

void WebView2Widget::onControllerCreated(ICoreWebView2Controller* controller) {
    m_controller = controller;
    controller->AddRef();
    controller->get_CoreWebView2(&m_webview);

    ICoreWebView2Settings* settings;
    m_webview->get_Settings(&settings);
    settings->put_IsScriptEnabled(TRUE);
    settings->put_AreDevToolsEnabled(FALSE);
    settings->put_AreDefaultContextMenusEnabled(FALSE);
    settings->Release();

    m_controller->put_IsVisible(TRUE);
    m_initialized = true;

    RECT bounds;
    GetClientRect((HWND)winId(), &bounds);
    m_controller->put_Bounds(bounds);
}

void WebView2Widget::navigate(const QString& url) {
    if (m_webview && m_initialized) {
        m_webview->Navigate(url.toStdWString().c_str());
    } else if (m_env && !m_controller) {
        HWND hwnd = (HWND)winId();
        m_env->CreateCoreWebView2Controller(hwnd, new ControllerCompletedHandler(this));
        m_pendingUrl = url;
    } else {
        m_pendingUrl = url;
    }
}

void WebView2Widget::setVisible(bool visible) {
    QWidget::setVisible(visible);
    if (m_controller) {
        m_controller->put_IsVisible(visible);
    }
}

void WebView2Widget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_controller) {
        RECT bounds;
        GetClientRect((HWND)winId(), &bounds);
        m_controller->put_Bounds(bounds);
    }
}
