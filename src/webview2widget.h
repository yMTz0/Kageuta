#pragma once

#include <QWidget>
#include <windows.h>
#include "WebView2.h"

class WebView2Widget : public QWidget {
    Q_OBJECT

public:
    explicit WebView2Widget(QWidget* parent = nullptr);
    ~WebView2Widget();

    void navigate(const QString& url);
    void setVisible(bool visible) override;

    void onEnvironmentCreated(ICoreWebView2Environment* env);
    void onControllerCreated(ICoreWebView2Controller* controller);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void initWebView();

    ICoreWebView2Environment* m_env = nullptr;
    ICoreWebView2Controller* m_controller = nullptr;
    ICoreWebView2* m_webview = nullptr;
    bool m_initialized = false;
    QString m_pendingUrl;
};
