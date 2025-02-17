#pragma once
#include <QString>
#include <QSettings>
#include <QVector>
#include "chronoperiod.h"

class Settings {
public:
    ~Settings();
    static Settings& instance();
    bool load();
    bool save();

    // Declarar apenas os métodos, implementação vai no .cpp
    QVector<ChronoPeriod> getPeriods() const;
    void setPeriods(const QVector<ChronoPeriod>& periods);

private:
    Settings() : settings("ChronoWall", "Settings") {}
    QSettings settings;
    QVector<ChronoPeriod> m_periods;
};
