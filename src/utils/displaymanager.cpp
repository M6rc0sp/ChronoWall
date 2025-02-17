#include "displaymanager.h"
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QString DisplayManager::findDisplay() {
    // Tenta obter do ambiente
    QString display = qgetenv("DISPLAY");
    if (!display.isEmpty()) {
        if (testDisplay(display)) return display;
    }

    // Tenta displays comuns
    QStringList displays = {":0", ":1", ":2"};
    for (const auto &d : displays) {
        if (testDisplay(d)) return d;
    }

    // Tenta descobrir usando o comando w
    QProcess process;
    process.start("w", {qgetenv("USER")});
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QRegularExpression rx(":[0-9]+");
    QRegularExpressionMatch match = rx.match(output);
    if (match.hasMatch()) {
        QString found = match.captured(0);
        if (testDisplay(found)) return found;
    }

    return QString();
}

bool DisplayManager::testDisplay(const QString &display) {
    QProcess test;
    test.setEnvironment(QProcess::systemEnvironment() << "DISPLAY=" + display);
    test.start("xdpyinfo");
    return test.waitForFinished() && test.exitCode() == 0;
}
