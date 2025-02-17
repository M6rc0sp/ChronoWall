#pragma once
#include <QCache>
#include <QPixmap>
#include <QString>
#include <QThreadPool>
#include <QRunnable>
#include <QMutex>

class ThumbnailCache {
public:
    static ThumbnailCache& instance();
    QPixmap getThumbnail(const QString &path, const QSize &size);

private:
    ThumbnailCache();
    ~ThumbnailCache();
    
    QCache<QString, QPixmap> m_cache;
    QThreadPool m_threadPool;
    QMutex m_mutex;
    
    QString getCacheKey(const QString &path, const QSize &size);
    QPixmap generateThumbnail(const QString &path, const QSize &size);
};
