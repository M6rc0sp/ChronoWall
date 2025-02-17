#include "thumbnailcache.h"
#include <QImageReader>
#include <QCryptographicHash>

ThumbnailCache& ThumbnailCache::instance() {
    static ThumbnailCache instance;
    return instance;
}

ThumbnailCache::ThumbnailCache() {
    m_cache.setMaxCost(100 * 1024 * 1024); // 100MB cache
    m_threadPool.setMaxThreadCount(4);
}

ThumbnailCache::~ThumbnailCache() {
    m_threadPool.waitForDone();
}

QString ThumbnailCache::getCacheKey(const QString &path, const QSize &size) {
    return QString("%1_%2x%3").arg(path).arg(size.width()).arg(size.height());
}

QPixmap ThumbnailCache::getThumbnail(const QString &path, const QSize &size) {
    QString key = getCacheKey(path, size);
    
    QMutexLocker locker(&m_mutex);
    QPixmap *cached = m_cache.object(key);
    if (cached) return *cached;
    
    // Se não está em cache, gera
    QPixmap thumbnail = generateThumbnail(path, size);
    m_cache.insert(key, new QPixmap(thumbnail));
    
    return thumbnail;
}

QPixmap ThumbnailCache::generateThumbnail(const QString &path, const QSize &size) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    
    if (reader.canRead()) {
        QImage img = reader.read().scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        return QPixmap::fromImage(img);
    }
    
    return QPixmap(size); // Retorna pixmap vazio em caso de erro
}
