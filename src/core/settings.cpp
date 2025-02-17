#include "settings.h"
#include <QDebug>
#include <QFile>
#include <QDir>

Settings::~Settings() {
    // Garantir que as últimas alterações sejam salvas
    if (!m_periods.isEmpty()) {
        settings.sync();
    }
    m_periods.clear();
}

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

bool Settings::load() {
    qDebug() << QObject::tr("Loading settings from file:") << settings.fileName();
    m_periods.clear();

    auto createDefaultPeriod = [this]() {
        ChronoPeriod defaultPeriod;
        defaultPeriod.startTime = QTime(0, 0);
        defaultPeriod.endTime = QTime(23, 59);
        
        QString savedWallpaper = settings.value("lastWallpaper").toString();
        if (!savedWallpaper.isEmpty() && QFile::exists(savedWallpaper)) {
            defaultPeriod.wallpaper = savedWallpaper;
        } else {
            defaultPeriod.wallpaper = QDir::homePath() + "/Imagens/wallpaper.jpg";
        }
        
        settings.remove("periods");
        m_periods.append(defaultPeriod);
        save();
        return true;
    };

    int size = settings.beginReadArray("periods");
    bool hasValidPeriods = false;
    
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QVariantMap periodMap;
        
        for(const QString &key : settings.childKeys()) {
            periodMap[key] = settings.value(key);
        }

        ChronoPeriod period = ChronoPeriod::fromVariantMap(periodMap);

        if (period.isValid()) {
            m_periods.append(period);
            hasValidPeriods = true;
        }
    }
    settings.endArray();
    
    if (!hasValidPeriods) {
        qDebug() << QObject::tr("No valid periods found, creating default...");
        return createDefaultPeriod();
    }

    return true;
}

bool Settings::save() {
    if (m_periods.isEmpty()) {
        qWarning() << QObject::tr("No periods to save!");
        return false;
    }

    settings.beginWriteArray("periods");
    for (int i = 0; i < m_periods.size(); ++i) {
        if (!m_periods[i].isValid()) {
            qWarning() << QObject::tr("Invalid period found at index %1 - skipping").arg(i);
            continue;
        }

        settings.setArrayIndex(i);
        QVariantMap periodMap = m_periods[i].toVariantMap();
        
        for(auto it = periodMap.begin(); it != periodMap.end(); ++it) {
            settings.setValue(it.key(), it.value());
        }
    }
    settings.endArray();
    
    settings.sync();
    return settings.status() == QSettings::NoError;
}

QVector<ChronoPeriod> Settings::getPeriods() const {
    return m_periods;
}

void Settings::setPeriods(const QVector<ChronoPeriod>& periods) {
    m_periods = periods;
    save();
}
