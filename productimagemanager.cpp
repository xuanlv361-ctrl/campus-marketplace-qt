#include "productimagemanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QStringList>
#include <QVector>

#include <algorithm>
#include <initializer_list>

namespace {

struct ProductImageRule
{
    QString keyword;
    QString path;
};

QString text(const char *value)
{
    return QString::fromUtf8(value);
}

QString bundledAsset(const QString &fileName)
{
    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath("ui_assets/" + fileName),
        QDir(QCoreApplication::applicationDirPath()).filePath("../ui_assets/" + fileName),
        QDir::current().filePath("ui_assets/" + fileName)
    };
    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) return QDir::cleanPath(candidate);
    }
    return QString();
}

QString externalImageRoot()
{
    return qEnvironmentVariable("SECONDHAND_IMAGE_ROOT").trimmed();
}

QString imageFile(const char *fileName)
{
    const QString root = externalImageRoot();
    if (root.isEmpty()) return QString();
    return QDir(root).filePath(text(fileName));
}

QString existingPath(const QString &path)
{
    return QFileInfo::exists(path) ? path : QString();
}

QHash<QString, QString> bundledProductImages()
{
    return {
        {text("高等数学教材"), "calculus_textbook.jpg"},
        {text("C++ 程序设计"), "cpp_textbook.jpg"},
        {text("雅思备考资料"), "ielts_guide.jpg"},
        {text("科学计算器"), "calculator.jpg"},
        {text("二手 iPhone 12"), "iphone12.jpg"},
        {text("蓝牙耳机"), "bluetooth_headphones.jpg"},
        {text("轻薄笔记本电脑"), "laptop.jpg"},
        {text("机械键盘"), "mechanical_keyboard.jpg"},
        {text("护眼台灯"), "desk_lamp.jpg"},
        {text("宿舍收纳箱"), "storage_box.jpg"},
        {text("保温水杯"), "water_bottle.jpg"},
        {text("桌面小风扇"), "desk_fan.jpg"},
        {text("九成新运动鞋"), "sneakers.jpg"},
        {text("校园双肩包"), "backpack.jpg"},
        {text("秋季外套"), "coat.jpg"},
        {text("羽毛球拍"), "badminton_racket.jpg"},
        {text("篮球"), "basketball.jpg"},
        {text("瑜伽垫"), "yoga_mat.jpg"},
        {text("校园自行车"), "bicycle.jpg"},
        {text("折叠滑板车"), "scooter.jpg"}
    };
}

QString firstExisting(std::initializer_list<QString> paths)
{
    for (const QString &path : paths) {
        const QString found = existingPath(path);
        if (!found.isEmpty()) return found;
    }
    return QString();
}

QVector<ProductImageRule> imageRules()
{
    QVector<ProductImageRule> rules = {
        // 教材资料
        {text("高等数学教材"), firstExisting({imageFile("高数.jpg"), imageFile("高數教材.jpg")})},
        {text("高数教材"), firstExisting({imageFile("高数.jpg"), imageFile("高數教材.jpg")})},
        {text("高数"), firstExisting({imageFile("高数.jpg"), imageFile("高數教材.jpg")})},
        {text("英语资料"), firstExisting({imageFile("英语资料.jpg"), imageFile("英語资料.jpg"), imageFile("英語資料.jpg")})},
        {text("雅思"), firstExisting({imageFile("英语资料.jpg"), imageFile("英語资料.jpg"), imageFile("英語資料.jpg")})},
        {text("C++程序设计"), firstExisting({imageFile("C++程序设计.jpg"), imageFile("C++程序設計.jpg")})},
        {text("C++资料"), firstExisting({imageFile("C++程序设计.jpg"), imageFile("C++程序設計.jpg")})},
        {text("C++"), firstExisting({imageFile("C++程序设计.jpg"), imageFile("C++程序設計.jpg")})},
        {text("课堂笔记"), imageFile("作业本.jpg")},
        {text("阅读资料"), imageFile("阅读资料.webp")},

        // 电子数码
        {text("苹果笔记本电脑"), firstExisting({imageFile("苹果笔记本电脑.jpg"), imageFile("蘋果電腦.jpg"), imageFile("macbook_.jpg")})},
        {text("笔记本电脑"), firstExisting({imageFile("笔记本电脑.jpg"), imageFile("筆記本電腦.jpg"), imageFile("macbook_.jpg")})},
        {text("笔记本"), firstExisting({imageFile("笔记本电脑.jpg"), imageFile("筆記本電腦.jpg"), imageFile("macbook_.jpg")})},
        {text("MacBook"), firstExisting({imageFile("苹果笔记本电脑.jpg"), imageFile("蘋果電腦.jpg"), imageFile("macbook_.jpg")})},
        {text("平板电脑"), firstExisting({imageFile("平板电脑.webp"), imageFile("平板電腦.webp")})},
        {text("耳机"), firstExisting({imageFile("耳机.webp"), imageFile("耳機.webp"), imageFile("headphone.jpg")})},
        {text("头戴式耳机"), firstExisting({imageFile("头戴式耳机.jpg"), imageFile("头戴式耳机 (2).jpg")})},
        {text("相机"), firstExisting({imageFile("相机.jpg"), imageFile("相機.jpg"), imageFile("cmaera_.jpg")})},
        {text("佳能相机"), imageFile("佳能相机.jpg")},
        {text("EOS"), imageFile("佳能相机.jpg")},
        {text("键盘"), imageFile("键盘.webp")},
        {text("鼠标"), firstExisting({imageFile("鼠标.webp"), imageFile("鼠標.webp")})},
        {text("显示器"), imageFile("显示器.jpg")},
        {text("苹果手机"), imageFile("苹果手机.jpg")},
        {text("手机"), imageFile("苹果手机.jpg")},

        // 宿舍用品
        {text("台灯"), imageFile("臺燈.webp")},
        {text("毛毯"), firstExisting({imageFile("毛毯.jpg"), imageFile("blanket.jpg")})},
        {text("电风扇"), imageFile("電風扇.jpg")},
        {text("小风扇"), imageFile("小风扇.webp")},
        {text("风扇"), firstExisting({imageFile("電風扇.jpg"), imageFile("小风扇.webp")})},
        {text("加湿器"), imageFile("加湿器.jpg")},
        {text("水杯"), firstExisting({imageFile("水杯.jpg"), imageFile("水杯.webp")})},
        {text("收纳箱"), imageFile("收納箱.jpg")},
        {text("收纳盒"), imageFile("收納盒.jpg")},
        {text("吹风机"), imageFile("吹风机.jpg")},
        {text("小冰箱"), imageFile("小冰箱.jpg")},

        // 学习资料
        {text("触控笔"), imageFile("pencil_.jpg")},
        {text("计算器"), imageFile("计算器.webp")},
        {text("打印机"), imageFile("打印机.jpg")},
        {text("文件夹"), imageFile("文件夹.jpg")},
        {text("书桌支架"), imageFile("书桌支架.jpg")},
        {text("显示器支架"), imageFile("显示器支架.webp")},

        // 服饰鞋包
        {text("双肩包"), imageFile("双肩包.jpg")},
        {text("通勤包"), imageFile("通勤包.jpg")},
        {text("通勤双肩包"), firstExisting({imageFile("双肩包.jpg"), imageFile("通勤包.jpg")})},
        {text("帆布包"), imageFile("帆布包.webp")},
        {text("女士包包"), imageFile("女士包包 (2).jpg")},
        {text("运动鞋"), imageFile("nike运动鞋.jpg")},
        {text("卫衣"), imageFile("卫衣.jpg")},
        {text("外套"), imageFile("外套.jpg")},
        {text("饰品"), imageFile("饰品.webp")},

        // 运动器材
        {text("篮球"), firstExisting({imageFile("篮球.jpg"), imageFile("籃球.jpg")})},
        {text("羽毛球拍"), imageFile("羽毛球拍.webp")},
        {text("哑铃"), imageFile("哑铃.jpg")},
        {text("瑜伽垫"), imageFile("瑜伽垫.jpg")},
        {text("滑板"), imageFile("滑板.jpg")},

        // 代步工具
        {text("自行车"), imageFile("自行车.jpg")},
        {text("电动车"), imageFile("电动车.jpg")},
        {text("平衡车"), imageFile("平衡车.jpg")},
        {text("滑板车"), imageFile("滑板.jpg")},

        // 美妆零食
        {text("保湿面霜"), firstExisting({imageFile("保湿面霜.webp"), imageFile("保湿面霜 (2).webp")})},
        {text("口红"), imageFile("口红.jpg")},
        {text("面膜"), imageFile("面膜.jpg")},
        {text("香水"), imageFile("香水.jpg")},
        {text("护手霜"), imageFile("护手霜.jpg")},
        {text("洗面奶"), imageFile("洗面奶.jpg")},
        {text("巧克力礼盒"), imageFile("巧克力禮盒.jpg")},
        {text("饼干礼盒"), imageFile("饼干礼盒.jpg")},
        {text("坚果礼盒"), firstExisting({imageFile("堅果禮盒.jpg"), imageFile("坚果礼盒.jpg")})},
        {text("咖啡礼盒"), firstExisting({imageFile("咖啡禮盒.jpg"), imageFile("咖啡礼盒.jpg")})}
    };

    std::sort(rules.begin(), rules.end(), [](const ProductImageRule &a, const ProductImageRule &b) {
        return a.keyword.size() > b.keyword.size();
    });
    return rules;
}

QString defaultImagePath()
{
    return firstExisting({
        imageFile("default_product.png"),
        bundledAsset("default_product.png")
    });
}

} // namespace

QString ProductImageManager::getImagePath(const QString &productName)
{
    return imagePathForProduct(productName, QString());
}

QString ProductImageManager::resolveConfiguredPath(const QString &configuredPath)
{
    const QString path = QDir::fromNativeSeparators(configuredPath.trimmed());
    if (path.isEmpty()) return QString();

    const QFileInfo info(path);
    if (info.isAbsolute() && info.exists()) return QDir::cleanPath(info.absoluteFilePath());

    QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(path),
        QDir(QCoreApplication::applicationDirPath()).filePath("../" + path),
        QDir::current().filePath(path)
    };
    if (!path.startsWith("ui_assets/")) {
        candidates << QDir(QCoreApplication::applicationDirPath()).filePath("ui_assets/" + path)
                   << QDir::current().filePath("ui_assets/" + path);
    }
    const QString root = externalImageRoot();
    if (!root.isEmpty()) candidates << QDir(root).filePath(path);

    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) return QDir::cleanPath(candidate);
    }
    return QString();
}

QString ProductImageManager::imagePathForProduct(const QString &productName,
                                                 const QString &category,
                                                 const QString &preferredPath)
{
    const QString name = productName.trimmed();
    const QString combined = name + " " + category.trimmed();

    const QString configured = resolveConfiguredPath(preferredPath);
    if (!configured.isEmpty()) return configured;

    const auto bundled = bundledProductImages();
    const auto exact = bundled.constFind(name);
    if (exact != bundled.constEnd()) {
        const QString path = bundledAsset("products/" + exact.value());
        if (!path.isEmpty()) return path;
    }

    for (const ProductImageRule &rule : imageRules()) {
        if (rule.path.isEmpty()) continue;
        if (name == rule.keyword || combined.contains(rule.keyword, Qt::CaseInsensitive)) {
            return QFileInfo::exists(rule.path) ? rule.path : defaultImagePath();
        }
    }

    return defaultImagePath();
}

QPixmap ProductImageManager::pixmapForProduct(const QString &productName,
                                              const QString &category,
                                              const QSize &targetSize,
                                              const QString &preferredPath)
{
    QPixmap pix(imagePathForProduct(productName, category, preferredPath));
    if (!pix.isNull() && targetSize.isValid()) {
        return pix.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return pix;
}
