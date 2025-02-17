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
    
    // Sincronizar com o início do próximo minuto (segundo zero)
    QTime currentTime = QTime::currentTime();
    int msToNextMinute = (60 - currentTime.second()) * 1000 - currentTime.msec();
    
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
    
    if (newWallpaper.isEmpty()) {
        qDebug() << QObject::tr("No wallpaper defined for current time");
    } else if (newWallpaper != m_currentWallpaper) {
        qDebug() << QObject::tr("Changing wallpaper to: %1").arg(newWallpaper);
        setWallpaper(newWallpaper);
        m_currentWallpaper = newWallpaper;
    } else {
        qDebug() << QObject::tr("Current wallpaper is up to date");
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

void ChronoWallService::setWallpaper(const QString& path) {
    if (path.isEmpty()) {
        qWarning() << QObject::tr("Empty wallpaper path.");
        return;
    }

    bool success = false;
    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");

    if (desktopEnv.contains("GNOME", Qt::CaseInsensitive)) {
        success = QProcess::startDetached("gsettings", {"set", "org.gnome.desktop.background", 
                                                      "picture-uri", "file://" + path});
        success &= QProcess::startDetached("gsettings", {"set", "org.gnome.desktop.background", 
                                                       "picture-uri-dark", "file://" + path});
        qDebug() << QObject::tr("GNOME wallpaper change: %1").arg(success ? "success" : "failed");

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
        emit wallpaperChanged(path);
    }
}

void ChronoWallService::loadSettings() {
    QSettings settings("ChronoWall", "Settings");
    if (settings.status() != QSettings::NoError) {
        qWarning() << QObject::tr("Failed to load settings. Using default values.");
        return;
    }

    m_periods.clear();
    int size = settings.beginReadArray("periods");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ChronoPeriod period;
        period.startTime = settings.value("startTime").toTime();
        period.endTime = settings.value("endTime").toTime();
        period.wallpaper = settings.value("wallpaper").toString();

        if (period.isValid()) {
            m_periods.append(period);
        }
    }
    settings.endArray();
}

void ChronoWallService::saveSettings() {
    QSettings settings("ChronoWall", "Settings");
    settings.beginWriteArray("periods");
    
    for (int i = 0; i < m_periods.size(); ++i) {
        if (!m_periods[i].isValid()) continue;
        
        settings.setArrayIndex(i);
        QVariantMap periodMap = m_periods[i].toVariantMap();
        settings.setValue("startTime", periodMap["startTime"]);
        settings.setValue("endTime", periodMap["endTime"]);
        settings.setValue("wallpaper", periodMap["wallpaper"]);
    }
    settings.endArray();
    settings.sync();
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