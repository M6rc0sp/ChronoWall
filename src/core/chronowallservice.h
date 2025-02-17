#pragma once
#include <QObject>
#include <QVector>
#include <QTimer>
#include <QDateTime>
#include "chronoperiod.h"
#include "settings.h"

class ChronoWallService : public QObject {
    Q_OBJECT
public:
    ~ChronoWallService();
    static ChronoWallService& instance();
    void updatePeriods(const QVector<ChronoPeriod>& periods);
    const QVector<ChronoPeriod>& periods() const;
    
    void setInterval(int milliseconds);
    int interval() const { return m_checkTimer->interval(); }
    
    // Nova API para lista de imagens
    void setImageList(const QStringList& images);
    const QStringList& imageList() const { return m_imageList; }

signals:
    void periodsChanged();
    void wallpaperChanged(const QString& path);

private slots:
    void checkTime();
    void updateWallpaper();
    void startTimer();

private:
    ChronoWallService();
    QVector<ChronoPeriod> m_periods;
    QVector<ChronoPeriod> m_savedPeriods;
    QTimer* m_checkTimer;
    QString m_currentWallpaper;
    QStringList m_imageList;
    int m_currentImageIndex;
    int currentWallpaperIndex = 0;
    
    void setWallpaper(const QString& path);
    void loadSettings();
    void saveSettings();
    QString getCurrentPeriodWallpaper() const;
    void switchToNextImage();
    bool isTimeInPeriod(const QTime& current, const QTime& start, const QTime& end) const;
    QTimer* timer;
};
