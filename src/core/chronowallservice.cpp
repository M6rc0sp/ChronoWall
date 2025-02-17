#include "chronowallservice.h"
#include <QTimer>
#include <QProcess>
#include <QSettings>
#include <QDebug>
#include <QTime>
#include <QDir>

ChronoWallService::~ChronoWallService() {
    if (m_checkTimer) {
        m_checkTimer->stop();
        delete m_checkTimer;
        m_checkTimer = nullptr;
    }
    m_periods.clear();
    m_currentWallpaper.clear();
    m_imageList.clear();
}

ChronoWallService& ChronoWallService::instance() {
    static ChronoWallService instance;
    return instance;
}

ChronoWallService::ChronoWallService() : QObject(nullptr) {
    qDebug() << QObject::tr("Starting ChronoWallService...");
    m_checkTimer = new QTimer(this);
    
    // Configurar watcher para monitorar alterações no arquivo de configuração
    m_configWatcher = new QFileSystemWatcher(this);
    QString configPath = SharedSettings::getConfigPath();
    m_configWatcher->addPath(configPath);
    
    connect(m_configWatcher, &QFileSystemWatcher::fileChanged, this, [this, configPath]() {
        qDebug() << QObject::tr("Config file changed, reloading settings...");
        // Recarregar configurações
        loadSettings();
        // Verificar wallpaper imediatamente
        checkTime();
        // Readicionar o arquivo ao watcher (alguns sistemas removem após a primeira mudança)
        if (!m_configWatcher->files().contains(configPath)) {
            m_configWatcher->addPath(configPath);
        }
    });
    
    // Sincronizar com o início do próximo minuto (milissegundo zero)
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDateTime nextMinute = currentDateTime.addSecs(60);
    nextMinute.setTime(QTime(nextMinute.time().hour(), nextMinute.time().minute(), 0, 0));
    int msToNextMinute = currentDateTime.msecsTo(nextMinute);
    
    qDebug() << QObject::tr("Will sync at: %1 (in %2 ms)")
               .arg(nextMinute.toString("HH:mm:ss.zzz"))
               .arg(msToNextMinute);
    
    // Timer inicial para sincronizar com o segundo zero
    QTimer::singleShot(msToNextMinute, this, [this]() {
        // Verificar o tempo imediatamente no segundo zero
        checkTime();
        
        // Configurar o timer para disparar a cada minuto (60.000 ms)
        m_checkTimer->setInterval(60000);
        m_checkTimer->start();
    });
    
    // Conectar o timer à função de verificação do tempo
    connect(m_checkTimer, &QTimer::timeout, this, &ChronoWallService::checkTime);
    
    // Carregar configurações
    loadSettings();
    
    // Verificar o tempo imediatamente ao iniciar o serviço
    QTimer::singleShot(0, this, &ChronoWallService::checkTime);
}

void ChronoWallService::checkTime() {
    QTime currentTime = QTime::currentTime();
    qDebug() << QObject::tr("Checking time at: %1").arg(currentTime.toString("HH:mm:ss"));
    
    QString newWallpaper = getCurrentPeriodWallpaper();
    QString currentSystemWallpaper = getCurrentSystemWallpaper();
    
    qDebug() << QObject::tr("Current system wallpaper: %1").arg(currentSystemWallpaper);
    qDebug() << QObject::tr("Current cached wallpaper: %1").arg(m_currentWallpaper);
    qDebug() << QObject::tr("New wallpaper to set: %1").arg(newWallpaper);
    
    // Forçar recarga das configurações antes de verificar
    loadSettings();
    
    if (newWallpaper.isEmpty()) {
        qDebug() << QObject::tr("No wallpaper defined for current time");
    } else if (newWallpaper != currentSystemWallpaper) {
        qDebug() << QObject::tr("Changing wallpaper from: %1 to: %2")
                   .arg(currentSystemWallpaper)
                   .arg(newWallpaper);
        setWallpaper(newWallpaper);
        m_currentWallpaper = newWallpaper;
    } else {
        qDebug() << QObject::tr("Current wallpaper is already set to: %1").arg(currentSystemWallpaper);
    }
}

QString ChronoWallService::getCurrentPeriodWallpaper() const {
    QTime currentTime = QTime::currentTime();
    
    for (const auto &period : m_periods) {
        if (period.containsTime(currentTime)) {
            qDebug() << QObject::tr("Found matching period: %1-%2 -> %3")
                       .arg(period.startTime.toString("HH:mm"))
                       .arg(period.endTime.toString("HH:mm"))
                       .arg(period.wallpaper);
            return period.wallpaper;
        }
    }
    
    qDebug() << QObject::tr("No matching period found for time: %1")
               .arg(currentTime.toString("HH:mm:ss"));
    return QString();
}

QString ChronoWallService::getCurrentSystemWallpaper() const {
    QString wallpaperPath;
    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");

    if (desktopEnv.contains("GNOME", Qt::CaseInsensitive)) {
        QProcess process;
        process.start("gsettings", {"get", "org.gnome.desktop.background", "picture-uri"});
        process.waitForFinished();
        wallpaperPath = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        
        // Remove 'file://' prefix se existir
        if (wallpaperPath.startsWith("'file://")) {
            wallpaperPath = wallpaperPath.mid(8, wallpaperPath.length() - 9);
        }
        
        qDebug() << QObject::tr("GNOME current wallpaper path: %1").arg(wallpaperPath);
    }
    // Adicionar suporte para outros ambientes desktop aqui se necessário
    
    return wallpaperPath;
}

void ChronoWallService::setWallpaper(const QString& path) {
    if (path.isEmpty()) {
        qWarning() << QObject::tr("Empty wallpaper path.");
        return;
    }

    QString currentWallpaper = getCurrentSystemWallpaper();
    if (currentWallpaper == path) {
        qDebug() << QObject::tr("Wallpaper is already set to: %1").arg(path);
        return;
    }

    bool success = false;
    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");

    if (desktopEnv.contains("GNOME", Qt::CaseInsensitive)) {
        success = QProcess::startDetached("gsettings", {"set", "org.gnome.desktop.background", 
                                                      "picture-uri", "file://" + path});
        success &= QProcess::startDetached("gsettings", {"set", "org.gnome.desktop.background", 
                                                       "picture-uri-dark", "file://" + path});
        qDebug() << QObject::tr("GNOME wallpaper change attempt: %1").arg(success ? "success" : "failed");

        // Verificar se a mudança foi efetiva
        QString newWallpaper = getCurrentSystemWallpaper();
        if (newWallpaper == path) {
            qDebug() << QObject::tr("Wallpaper change confirmed successful");
            success = true;
        } else {
            qWarning() << QObject::tr("Wallpaper change failed. Current: %1, Expected: %2")
                        .arg(newWallpaper)
                        .arg(path);
            success = false;
        }
    } else if (desktopEnv.contains("KDE", Qt::CaseInsensitive)) {
        QString script = QString(
            "var allDesktops = desktops();"
            "for (i=0;i<allDesktops.length;i++) {"
            "    d = allDesktops[i];"
            "    d.wallpaperPlugin = 'org.kde.image';"
            "    d.currentConfigGroup = Array('Wallpaper', 'org.kde.image', 'General');"
            "    d.writeConfig('Image', '%1')"
            "}").arg(path);
        
        success = QProcess::startDetached("qdbus", {"org.kde.plasmashell", "/PlasmaShell", 
                                          "org.kde.PlasmaShell.evaluateScript", script});
        qDebug() << QObject::tr("KDE wallpaper change: %1").arg(success ? "success" : "failed");

    } else if (desktopEnv.contains("XFCE", Qt::CaseInsensitive)) {
        success = QProcess::startDetached("xfconf-query", {"-c", "xfce4-desktop", "-p", 
                                                        "/backdrop/screen0/monitor0/image-path", "-s", path});
        qDebug() << QObject::tr("XFCE wallpaper change: %1").arg(success ? "success" : "failed");
    } else {
        qWarning() << QObject::tr("Unsupported desktop environment: %1. Wallpaper change may not work.").arg(desktopEnv);
    }

    if (success) {
        m_currentWallpaper = path;
        emit wallpaperChanged(path);
    }
}

void ChronoWallService::loadSettings() {
    qDebug() << QObject::tr("Reloading settings...");
    QSettings* settings = SharedSettings::getInstance();
    settings->sync(); // Forçar recarga do arquivo
    
    if (settings->status() != QSettings::NoError) {
        qWarning() << QObject::tr("Failed to load settings. Using default values.");
        return;
    }
    m_periods.clear();
    int size = settings->beginReadArray("periods");
    qDebug() << QObject::tr("Loading %1 periods from settings file: %2")
               .arg(size)
               .arg(settings->fileName());

    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        ChronoPeriod period;
        period.startTime = settings->value("startTime").toTime();
        period.endTime = settings->value("endTime").toTime();
        period.wallpaper = settings->value("wallpaper").toString();

        qDebug() << QObject::tr("Reading period %1:").arg(i)
                 << QObject::tr("\n - Start: %1").arg(period.startTime.toString("HH:mm:ss"))
                 << QObject::tr("\n - End: %1").arg(period.endTime.toString("HH:mm:ss"))
                 << QObject::tr("\n - Wallpaper: %1").arg(period.wallpaper);

        if (period.isValid()) {
            m_periods.append(period);
            qDebug() << QObject::tr("Period %1 is valid and added").arg(i);
        } else {
            qWarning() << QObject::tr("Period %1 is invalid and skipped").arg(i);
        }
    }
    settings->endArray();

    qDebug() << QObject::tr("Total periods loaded: %1").arg(m_periods.size());
}

void ChronoWallService::saveSettings() {
    QSettings* settings = SharedSettings::getInstance();
    settings->beginWriteArray("periods");
    
    for (int i = 0; i < m_periods.size(); ++i) {   
        if (!m_periods[i].isValid()) continue;
        settings->setArrayIndex(i);
        QVariantMap periodMap = m_periods[i].toVariantMap();
        settings->setValue("startTime", periodMap["startTime"]);
        settings->setValue("endTime", periodMap["endTime"]);
        settings->setValue("wallpaper", periodMap["wallpaper"]);

        qDebug() << QObject::tr("Saved period %1:").arg(i)
                 << QObject::tr("\n - Start: %1").arg(m_periods[i].startTime.toString("HH:mm:ss"))
                 << QObject::tr("\n - End: %1").arg(m_periods[i].endTime.toString("HH:mm:ss"))
                 << QObject::tr("\n - Wallpaper: %1").arg(m_periods[i].wallpaper);
    }
    settings->endArray();
    settings->sync();
    
    qDebug() << QObject::tr("Saved settings to: %1").arg(settings->fileName());
}

void ChronoWallService::setInterval(int milliseconds) {
    m_checkTimer->setInterval(milliseconds);
}

void ChronoWallService::updateWallpaper() {
    if (m_imageList.isEmpty()) {
        return;
    }

    // Modo por período: verifica qual wallpaper corresponde ao horário atual
    QTime currentTime = QTime::currentTime();
    for (const auto &period : m_periods) {
        if (isTimeInPeriod(currentTime, period.startTime, period.endTime)) {
            setWallpaper(period.wallpaper);
            break;
        }
    }
}

void ChronoWallService::startTimer() {
    m_checkTimer->start();
}

bool ChronoWallService::isTimeInPeriod(const QTime& current, const QTime& start, const QTime& end) const {
    if (start <= end) {
        return current >= start && current <= end;
    } else {
        // Período que cruza a meia-noite
        return current >= start || current <= end;
    }
}

void ChronoWallService::setImageList(const QStringList& images) {
    m_imageList = images;
    QSettings settings("ChronoWall", "Settings");
    settings.setValue("wallpapers", images);
    settings.sync();
}

void ChronoWallService::updatePeriods(const QVector<ChronoPeriod>& periods) {
    m_periods = periods;
    m_currentWallpaper.clear();
    saveSettings();
    emit periodsChanged();
    checkTime();
}

const QVector<ChronoPeriod>& ChronoWallService::periods() const {
    return m_periods;
}