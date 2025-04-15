#ifndef ICON_CACHE_H
#define ICON_CACHE_H

#include <QString>
#include <QIcon>
#include <QMap>
#include <QMutex>

class AppIconCache {
public:
    static AppIconCache& instance();

    QIcon getIcon(const QString&);

    void preload_Icon(const QStringList&);

private:
    AppIconCache() = default;
    ~AppIconCache() = default;
    AppIconCache(const AppIconCache&) = delete;
    AppIconCache& operator=(const AppIconCache&) = delete;

    QMap<QString, QIcon> iconMap;
    QMutex mutex;  
};



#endif // ICON_CACHE_H
