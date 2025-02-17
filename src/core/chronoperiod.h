#pragma once
#include <QString>
#include <QTime>
#include <QVariantMap>

class ChronoPeriod {
public:
    QString wallpaper;
    QTime startTime;
    QTime endTime;

    bool isValid() const;
    bool containsTime(const QTime &time) const;
    QVariantMap toVariantMap() const;
    static ChronoPeriod fromVariantMap(const QVariantMap &map);
};
