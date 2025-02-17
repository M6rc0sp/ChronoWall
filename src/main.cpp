#include <QApplication>
#include <QCommandLineParser>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QIcon>
#include <QTranslator>
#include "ui/mainwindow.h"
#include "core/chronowallservice.h"
#include "utils/displaymanager.h"
#include "core/settings.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Impedir que o app feche quando a última janela for fechada
    // app.setQuitOnLastWindowClosed(false);
    
    // Carregar traduções
    QTranslator translator;
    QString locale = QLocale::system().name();
    if (translator.load(QString(":/translations/chronowall_%1").arg(locale))) {
        app.installTranslator(&translator);
    }
    
    // Handler de mensagens com tr() corrigido
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QString txt;
        switch (type) {
            case QtDebugMsg:
                txt = QString(QObject::tr("Debug: %1")).arg(msg);
                break;
            case QtWarningMsg:
                txt = QString(QObject::tr("Warning: %1")).arg(msg);
                break;
            case QtCriticalMsg:
                txt = QString(QObject::tr("Critical: %1")).arg(msg);
                break;
            case QtFatalMsg:
                txt = QString(QObject::tr("Fatal: %1")).arg(msg);
                break;
        }
        
        QFile outFile("/tmp/chronowall-debug.log");
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << QDateTime::currentDateTime().toString() << " - " << txt << " (" 
           << context.file << ":" << context.line << ")\n";
    });

    // Habilitar debug de exceções não capturadas
    std::set_terminate([]() {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception& e) {
            qFatal("Uncaught exception: %s", e.what());
        } catch (...) {
            qFatal("Uncaught unknown exception");
        }
    });
    
    // Configurar aplicativo
    app.setApplicationName("ChronoWall");
    app.setApplicationVersion("1.0");
    app.setDesktopFileName("chronowall");
    
    // Verificar display
    QString display = DisplayManager::findDisplay();
    if (display.isEmpty()) {
        qDebug() << QObject::tr("Warning: Display not found, using system default");
    } else {
        qputenv("DISPLAY", display.toLocal8Bit());
    }

    // Parser de linha de comando
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("ChronoWall - Wallpaper Time Scheduler"));
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption daemonOption({"d", "daemon"}, QObject::tr("Run in daemon mode"));
    parser.addOption(daemonOption);
    
    QCommandLineOption installOption({"i", "install"}, QObject::tr("Install service and autostart"));
    parser.addOption(installOption);
    
    parser.process(app);

    // Instalar serviço e autostart
    if (parser.isSet(installOption)) {
        QString serviceFile = QDir::homePath() + "/.config/systemd/user/chronowall.service";
        QDir().mkpath(QFileInfo(serviceFile).path());
        QFile::copy(":/chronowall.service", serviceFile);
        
        QProcess::execute("systemctl", {"--user", "enable", "chronowall"});
        QProcess::execute("systemctl", {"--user", "start", "chronowall"});
        
        QString autostartDir = QDir::homePath() + "/.config/autostart";
        QDir().mkpath(autostartDir);
        QFile::copy(":/chronowall.desktop", autostartDir + "/chronowall.desktop");
        
        qDebug() << QObject::tr("ChronoWall service installed successfully!");
        return 0;
    }

    // Modo daemon (executar o serviço em segundo plano)
    if (parser.isSet(daemonOption)) {
        qDebug() << QObject::tr("Starting in daemon mode...");

        // Iniciar o serviço em um processo separado
        ChronoWallService::instance();
        qDebug() << QObject::tr("ChronoWallService instance created");
        return app.exec();
    }

    // Modo GUI (interface gráfica)
    qDebug() << QObject::tr("Starting in GUI mode...");

    // Carregar configurações
    qDebug() << QObject::tr("Loading settings...");
    bool settingsLoaded = Settings::instance().load();
    if (settingsLoaded) {
        qDebug() << QObject::tr("Settings loaded successfully.");
    } else {
        qDebug() << QObject::tr("Failed to load settings.");
    }

    // Configurar ícone do aplicativo
    QIcon appIcon;
    const QStringList iconPaths = {
        QString("/usr/share/icons/hicolor/scalable/apps/chronowall.svg"),
        QString(":/resources/icons/chronowall.svg"),
        QString::fromLatin1("preferences-desktop-wallpaper")
    };

    for (const QString &path : iconPaths) {
        if (path.startsWith("/") || path.startsWith(":")) {
            if (QFile::exists(path)) {
                appIcon = QIcon(path);
                break;
            }
        } else {
            appIcon = QIcon::fromTheme(path);
            if (!appIcon.isNull()) break;
        }
    }

    if (appIcon.isNull()) {
        qDebug() << QObject::tr("Warning: Could not load icon");
        appIcon = QIcon::fromTheme("application-x-executable");
    }

    // Configurar ícone da bandeja
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&app);
    trayIcon->setIcon(appIcon);
    
    QMenu *trayMenu = new QMenu();
    QObject::connect(trayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            auto *window = new ChronoWallWindow();
            window->show();
        }
    });
    
    trayMenu->addAction(QObject::tr("Open ChronoWall"), [&]() {
        auto *window = new ChronoWallWindow();
        window->show();
    });
    trayMenu->addSeparator();
    trayMenu->addAction(QObject::tr("Exit"), &app, &QApplication::quit);
    
    trayIcon->setToolTip("ChronoWall");
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    // Definir ícone para a janela também
    app.setWindowIcon(appIcon);

    // Criar e exibir a janela principal
    ChronoWallWindow window;
    qDebug() << QObject::tr("Main window created");
    window.show();
    qDebug() << QObject::tr("Main window displayed");
    
    return app.exec();
}