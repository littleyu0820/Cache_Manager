#include "Icon_Cache.h"
#include <Windows.h>
#include <QPixmap>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QMutexLocker>

AppIconCache& AppIconCache::instance() {
    static AppIconCache cache;
    return cache;
}



//////////////////////////////////轉換HICON成QPixmap//////////////////////////////////
QPixmap hIconToPixmap(HICON hIcon) {
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);

    BITMAP bmp = {};
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);

    QImage image(bmp.bmWidth, bmp.bmHeight, QImage::Format_ARGB32);
    HDC hDC = GetDC(nullptr);
    HDC hMemDC = CreateCompatibleDC(hDC);
    SelectObject(hMemDC, iconInfo.hbmColor);

    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = -bmp.bmHeight;  // top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    GetDIBits(hMemDC, iconInfo.hbmColor, 0, bmp.bmHeight, image.bits(), reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hDC);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return QPixmap::fromImage(image);
}
//////////////////////////////////轉換HICON成QPixmap//////////////////////////////////



QIcon AppIconCache::getIcon(const QString& exePath) {
    QMutexLocker locker(&mutex);

    if (iconMap.contains(exePath)) {
        return iconMap[exePath];
    }

    HICON hIcon = nullptr;
    if (ExtractIconExW((LPCWSTR)exePath.utf16(), 0, &hIcon, nullptr, 1) > 0 && hIcon) {
        QPixmap pixmap = hIconToPixmap(hIcon);
        DestroyIcon(hIcon);

        QIcon icon(pixmap);
        iconMap[exePath] = icon;
        return icon;
    }

    return QIcon(":/default_app_icon.png");
}

void AppIconCache::preload_Icon(const QStringList& path)
{
    for (const QString& path : path)
    {
        getIcon(path);
    }
}
