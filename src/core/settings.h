#pragma once
#include <QString>
#include <QSettings>
#include <QVector>
#include <QDir>
#include "chronoperiod.h"

class SharedSettings {
public:
    static QString getConfigPath() {
        return QDir::homePath() + "/.config/ChronoWall/settings.conf";
    }

    static QSettings* getInstance() {
        static QSettings settings(getConfigPath(), QSettings::IniFormat);
        return &settings;
    }
};

class Settings {
public:
    ~Settings();
    static Settings& instance();
    bool load();
    bool save();
    QVector<ChronoPeriod> getPeriods() const;
    void setPeriods(const QVector<ChronoPeriod>& periods);

private:
    Settings() : settings(SharedSettings::getConfigPath(), QSettings::IniFormat) {}
    QSettings settings;
    QVector<ChronoPeriod> m_periods;
};
