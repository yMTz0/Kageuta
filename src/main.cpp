#include <QApplication>
#include <QIcon>
#include <exception>
#include "mainwindow.h"
#include "styles.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Kageuta");
    app.setApplicationVersion("1.0.0");
    app.setStyle("Fusion");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(Styles::BG_PRIMARY));
    palette.setColor(QPalette::WindowText, QColor(Styles::TEXT_PRIMARY));
    palette.setColor(QPalette::Base, QColor(Styles::BG_CARD));
    palette.setColor(QPalette::AlternateBase, QColor(Styles::BG_HOVER));
    palette.setColor(QPalette::ToolTipBase, QColor(Styles::BG_HOVER));
    palette.setColor(QPalette::ToolTipText, QColor(Styles::TEXT_PRIMARY));
    palette.setColor(QPalette::Text, QColor(Styles::TEXT_PRIMARY));
    palette.setColor(QPalette::Button, QColor(Styles::BG_CARD));
    palette.setColor(QPalette::ButtonText, QColor(Styles::TEXT_PRIMARY));
    palette.setColor(QPalette::BrightText, QColor(Styles::ACCENT));
    palette.setColor(QPalette::Highlight, QColor(Styles::ACCENT));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(palette);

    app.setStyleSheet(Styles::APP_STYLESHEET);

    MainWindow window;
    window.show();

    return app.exec();
}
