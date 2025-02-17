#include "chronoperiod.h"
#include <QDebug>

bool ChronoPeriod::isValid() const {
    bool valid = !wallpaper.isEmpty() && 
           startTime.isValid() && 
           endTime.isValid();
    
    if (!valid) {
        qDebug() << QObject::tr("Invalid period:") 
                 << QObject::tr("wallpaper: %1, start: %2, end: %3")
                    .arg(!wallpaper.isEmpty())
                    .arg(startTime.isValid())
                    .arg(endTime.isValid());
    }
    
    return valid;
}

bool ChronoPeriod::containsTime(const QTime &time) const {
    if (startTime < endTime) {
        return time >= startTime && time < endTime;
    } else {
        // Para períodos que atravessam meia-noite
        return time >= startTime || time < endTime;
    }
}

QVariantMap ChronoPeriod::toVariantMap() const {
    QVariantMap map;
    map["wallpaper"] = wallpaper;
    map["startTime"] = startTime;
    map["endTime"] = endTime;
    return map;
}

ChronoPeriod ChronoPeriod::fromVariantMap(const QVariantMap &map) {
    ChronoPeriod period;
    period.wallpaper = map["wallpaper"].toString();
    period.startTime = map["startTime"].toTime();
    period.endTime = map["endTime"].toTime();
    return period;
}
