#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("MC Modloader Installer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MCTools");

    // Try to load JetBrains Mono if available as embedded resource
    // QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");

    // Fallback monospace chain
    QFont defaultFont("JetBrains Mono");
    if (!defaultFont.exactMatch()) {
        defaultFont.setFamily("Cascadia Code");
    }
    if (!defaultFont.exactMatch()) {
        defaultFont.setFamily("Consolas");
    }
    defaultFont.setPointSize(11);
    app.setFont(defaultFont);

    MainWindow w;
    w.show();

    return app.exec();
}
