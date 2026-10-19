#ifndef PRODUCTIMAGEMANAGER_H
#define PRODUCTIMAGEMANAGER_H

#include <QPixmap>
#include <QSize>
#include <QString>

class ProductImageManager
{
public:
    static QString getImagePath(const QString &productName);
    static QString resolveConfiguredPath(const QString &configuredPath);
    static QString imagePathForProduct(const QString &productName,
                                       const QString &category = QString(),
                                       const QString &preferredPath = QString());
    static QPixmap pixmapForProduct(const QString &productName,
                                    const QString &category,
                                    const QSize &targetSize,
                                    const QString &preferredPath = QString());
};

#endif // PRODUCTIMAGEMANAGER_H
