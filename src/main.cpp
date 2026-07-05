#include <QApplication>
#include <QIcon>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <exception>
#include "mainwindow.h"
#include "styles.h"

void logMessage(const QString& msg) {
    QFile f(QCoreApplication::applicationDirPath() + "/debug.log");
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&f);
        ts << msg << "\n";
    }
}

int main(int argc, char* argv[]) {
    logMessage("=== Kageuta starting ===");

    try {
        QApplication app(argc, argv);
        logMessage("QApplication created");

        app.setApplicationName("Kageuta");
        app.setApplicationVersion("1.0.0");
        app.setStyle("Fusion");
        logMessage("Style set");

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
        logMessage("Palette set");

        app.setStyleSheet(Styles::APP_STYLESHEET);
        logMessage("Stylesheet set");

        logMessage("Creating MainWindow...");
        MainWindow window;
        logMessage("MainWindow created");
        window.show();
        logMessage("Window shown, entering event loop");

        return app.exec();
    } catch (const std::exception& e) {
        logMessage(QString("FATAL: ") + e.what());
        return 1;
    } catch (...) {
        logMessage("FATAL: Unknown error");
        return 1;
    }
}
