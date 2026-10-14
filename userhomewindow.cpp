#include "userhomewindow.h"
#include "accountstore.h"
#include "productimagemanager.h"
#include "welcomewindow.h"

#include <QApplication>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHash>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {
const char *kUserQss = R"(
QWidget#Root, QWidget { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#Sidebar { background: #FFFFFF; border: none; border-right: 1px solid #EAEAEA; }
QLabel#TopStrip { background: #111111; color: #FFFFFF; border-radius: 0px; padding: 8px 14px; font-weight: 800; }
QLabel#Banner { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 20px; padding: 10px; }
QFrame#MarketCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 22px; }
QFrame#CategoryCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 20px; }
QFrame#ProductCard { min-height: 360px; }
QLabel#HomeTitle { color: #111111; font-size: 36px; font-weight: 900; background: transparent; }
QLabel#HomeSubtitle { color: #666666; font-size: 15px; background: transparent; }
QLabel#ProductImage, QLabel#CategoryImage { background: #F6F6F8; border: 1px solid #EAEAEA; border-radius: 14px; }
QLabel#TagLabel { background: #F2F2F4; color: #666666; border-radius: 10px; padding: 4px 8px; font-size: 12px; }
QPushButton#Pill { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; border-radius: 18px; min-height: 34px; padding: 0 16px; }
QPushButton#Pill:hover { background: #F1F0FF; border-color: #5B4BFF; }
QPushButton#Pill[active="true"] { background: #5B4BFF; color: #FFFFFF; border-color: #5B4BFF; }
QLabel#Logo { background: #5B4BFF; color: #FFFFFF; border-radius: 16px; font-size: 18px; font-weight: 900; padding: 8px 12px; }
QLabel#SideHint { color: #666666; font-size: 13px; font-weight: 800; }
QPushButton[nav="true"] { text-align: left; padding-left: 14px; min-height: 42px; border-radius: 12px; border: none; background: #FFFFFF; color: #333333; font-weight: 800; }
QPushButton[nav="true"]:hover { background: #F1F0FF; color: #111111; }
QPushButton[active="true"] { background: #5B4BFF; color: #FFFFFF; }
QFrame#Card, QFrame#Panel, QFrame#ProductCard, QFrame#CartCard, QFrame#SummaryCard, QFrame#OrderCard, QFrame#StatCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 20px; }
QLabel#Title { font-size: 32px; font-weight: 900; color: #111111; background: transparent; }
QLabel#Subtitle, QLabel#Desc, QLabel#ProductMeta, QLabel#SummaryText { color: #666666; font-size: 14px; background: transparent; }
QLabel#SectionTitle { color: #111111; font-size: 22px; font-weight: 900; background: transparent; }
QLabel#ProductName { color: #111111; font-size: 18px; font-weight: 900; background: transparent; }
QLabel#ProductPrice, QLabel#Metric, QLabel#Total { color: #5B4BFF; font-size: 25px; font-weight: 900; background: transparent; }
QLabel#MetricSmall { color: #111111; font-size: 15px; font-weight: 800; background: transparent; }
QPushButton { min-height: 42px; border-radius: 16px; font-size: 14px; font-weight: 900; padding: 0 18px; }
QPushButton#Primary { background: #5B4BFF; color: #FFFFFF; border: none; }
QPushButton#Primary:hover { background: #493CE6; }
QPushButton#Danger { background: #FFF1F1; color: #EF4444; border: 1px solid #FFD1D1; }
QPushButton#Danger:hover { background: #FFE8E8; }
QPushButton#Light { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; }
QPushButton#Light:hover { background: #F7F7FA; border-color: #5B4BFF; }
QLineEdit, QComboBox, QSpinBox, QTextEdit { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 16px; min-height: 42px; padding: 0 14px; }
QLineEdit:focus, QComboBox:focus, QTextEdit:focus { border-color: #5B4BFF; }
QTableWidget { background: #FFFFFF; border: none; gridline-color: #EAEAEA; alternate-background-color: #FAFAFA; selection-background-color: #F1F0FF; selection-color: #111111; }
QHeaderView::section { background: #F7F7F9; color: #111111; border: none; border-right: 1px solid #EAEAEA; padding: 12px 8px; font-weight: 900; }
QScrollArea { background: transparent; border: none; }
)";

QTableWidgetItem *item(const QString &text)
{
    auto *it = new QTableWidgetItem(text);
    it->setTextAlignment(Qt::AlignCenter);
    return it;
}

void clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *child = nullptr;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        if (child->layout()) clearLayout(child->layout());
        delete child;
    }
}

QFrame *statCard(const QString &title, QLabel **valueLabel)
{
    auto *card = new QFrame;
    card->setObjectName("StatCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(6);
    *valueLabel = new QLabel("0");
    (*valueLabel)->setObjectName("Metric");
    auto *label = new QLabel(title);
    label->setObjectName("Subtitle");
    layout->addWidget(*valueLabel);
    layout->addWidget(label);
    return card;
}

QLabel *imageBox(const QString &path, const QSize &size, const char *objectName)
{
    auto *label = new QLabel;
    label->setObjectName(objectName);
    label->setFixedSize(size);
    label->setAlignment(Qt::AlignCenter);
    QPixmap pix(path);
    if (!pix.isNull()) {
        label->setPixmap(pix.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    return label;
}

QLabel *fitImageBox(const QString &path, const QSize &size, const char *objectName)
{
    auto *label = new QLabel;
    label->setObjectName(objectName);
    label->setFixedHeight(size.height());
    label->setMinimumWidth(size.width());
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QPixmap pix(path);
    if (!pix.isNull()) {
        label->setPixmap(pix.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    return label;
}

QLabel *productImageBox(const QString &name, const QString &category, const QSize &size, const char *objectName)
{
    auto *label = new QLabel;
    label->setObjectName(objectName);
    label->setFixedSize(size);
    label->setAlignment(Qt::AlignCenter);
    const QPixmap pix = ProductImageManager::pixmapForProduct(name, category, size);
    if (!pix.isNull()) {
        label->setPixmap(pix);
    }
    return label;
}

struct CategoryDisplayItem
{
    QString name;
    QString category;
    QString imagePath;
    double price;
    int stock;
};

QVector<CategoryDisplayItem> scannedCategoryItems()
{
    const QString configuredRoot = qEnvironmentVariable("SECONDHAND_IMAGE_ROOT").trimmed();
    const QString root = configuredRoot.isEmpty()
                             ? QDir(QApplication::applicationDirPath()).filePath("ui_assets/catalog")
                             : configuredRoot;
    const QStringList categories = {"代步工具", "电子数码", "服饰鞋包", "美妆零食", "宿舍用品", "学习资料", "运动器材"};
    const QStringList filters = {"*.jpg", "*.jpeg", "*.png", "*.webp"};
    QVector<CategoryDisplayItem> items;

    for (const QString &category : categories) {
        QDir dir(root + "/" + category);
        if (!dir.exists()) {
            qDebug() << "Category directory not found:" << dir.absolutePath();
            continue;
        }

        const QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);
        int index = 0;
        for (const QFileInfo &file : files) {
            const QString productName = file.completeBaseName().trimmed();
            if (productName.isEmpty()) continue;
            const uint seed = qHash(category + productName);
            items.push_back({
                productName,
                category,
                QDir::toNativeSeparators(file.absoluteFilePath()).replace("\\", "/"),
                10.0 + double(seed % 50000) / 100.0,
                1 + int((seed + uint(index)) % 12)
            });
            ++index;
        }
    }

    return items;
}

QLabel *categoryImageBox(const QString &imagePath, const QSize &size, const char *objectName)
{
    auto *label = new QLabel;
    label->setObjectName(objectName);
    label->setFixedSize(size);
    label->setAlignment(Qt::AlignCenter);
    QPixmap pix(imagePath);
    if (!pix.isNull()) {
        label->setPixmap(pix.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        qDebug() << "Category page image load failed:" << imagePath;
    }
    return label;
}

QString normalizedProductNameForImage(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.contains("三轮车")) return "电动车";
    if (trimmed.contains("摩托车")) return "电动车";
    return trimmed;
}

CategoryDisplayItem scannedProductForDisplay(const QString &productName, const QString &category = QString())
{
    const QVector<CategoryDisplayItem> products = scannedCategoryItems();
    const QString normalizedName = normalizedProductNameForImage(productName);
    const QString normalizedCategory = category.trimmed();

    for (const auto &product : products) {
        if (!normalizedCategory.isEmpty() && product.category == normalizedCategory && product.name == normalizedName) {
            return product;
        }
    }
    for (const auto &product : products) {
        if (product.name == normalizedName) {
            return product;
        }
    }
    for (const auto &product : products) {
        if ((!normalizedCategory.isEmpty() && product.category == normalizedCategory)
            && (product.name.contains(normalizedName, Qt::CaseInsensitive)
                || normalizedName.contains(product.name, Qt::CaseInsensitive))) {
            return product;
        }
    }
    for (const auto &product : products) {
        if (product.name.contains(normalizedName, Qt::CaseInsensitive)
            || normalizedName.contains(product.name, Qt::CaseInsensitive)) {
            return product;
        }
    }

    return {};
}

QLabel *scannedProductImageBox(const QString &productName, const QString &category, const QSize &size, const char *objectName)
{
    const CategoryDisplayItem product = scannedProductForDisplay(productName, category);
    if (!product.imagePath.isEmpty()) {
        return categoryImageBox(product.imagePath, size, objectName);
    }

    qDebug() << "Product image not found:" << productName;
    auto *label = new QLabel("无图");
    label->setObjectName(objectName);
    label->setFixedSize(size);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("background: #F1F1F3; color: #999999; border: 1px dashed #CCCCCC; border-radius: 14px;");
    return label;
}

struct HomeCategoryPreviewItem
{
    QString name;
    QString imagePath;
};

struct HomeCategoryPreview
{
    QString category;
    QStringList itemNames;
};

QString imagePathFromScannedItems(const QString &category, const QString &name)
{
    for (const auto &item : scannedCategoryItems()) {
        if (item.category == category && item.name == name) return item.imagePath;
    }
    qDebug() << "Home preview image path not found:" << category << name;
    return QString();
}

QVector<HomeCategoryPreview> homeCategoryPreviews()
{
    return {
        {"教材资料", {"英語资料", "高数", "作业本", "C++程序設計"}},
        {"电子数码", {"苹果笔记本电脑", "平板", "蓝牙耳机", "苹果手机"}},
        {"宿舍用品", {"宿舍台灯", "电风扇", "水杯", "收纳箱"}},
        {"服饰鞋包", {"双肩包", "帆布包", "nike运动鞋", "外套"}},
        {"运动器材", {"篮球", "排球", "羽毛球拍", "瑜伽垫"}},
        {"学习资料", {"pencil_", "打印机", "计算器", "文件夹"}},
        {"代步工具", {"电动车", "滑板", "平衡车", "自行车"}},
        {"美妆零食", {"香水", "口红", "饼干礼盒", "咖啡礼盒"}}
    };
}

void populateHomeRecommendationGrid(QGridLayout *grid,
                                    QWidget *viewport,
                                    const QVector<CategoryDisplayItem> &items)
{
    if (!grid || !viewport) return;
    clearLayout(grid);

    if (items.isEmpty()) {
        auto *empty = new QLabel("未找到相关商品");
        empty->setObjectName("Subtitle");
        empty->setAlignment(Qt::AlignCenter);
        empty->setMinimumSize(320, 160);
        grid->addWidget(empty, 0, 0);
        viewport->setFixedSize(360, 220);
        return;
    }

    for (int i = 0; i < items.size(); ++i) {
        const auto item = items[i];
        auto *card = new QFrame;
        card->setObjectName("CategoryCard");
        card->setFixedSize(180, 210);
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 12, 12, 12);
        cardLayout->setSpacing(8);

        auto *img = categoryImageBox(item.imagePath, QSize(150, 110), "CategoryImage");
        auto *name = new QLabel(item.name);
        name->setObjectName("ProductName");
        name->setAlignment(Qt::AlignCenter);
        name->setWordWrap(true);
        name->setStyleSheet("font-size: 14px;");
        auto *category = new QLabel(item.category);
        category->setObjectName("ProductMeta");
        category->setAlignment(Qt::AlignCenter);
        auto *price = new QLabel("¥" + QString::number(item.price, 'f', 2));
        price->setObjectName("ProductPrice");
        price->setAlignment(Qt::AlignCenter);

        cardLayout->addWidget(img, 0, Qt::AlignCenter);
        cardLayout->addWidget(name);
        cardLayout->addWidget(category);
        cardLayout->addWidget(price);
        grid->addWidget(card, 0, i);
    }

    viewport->setFixedSize(items.size() * 196, 235);
}

void populateHomePreviewGrid(QGridLayout *grid, QWidget *viewport)
{
    if (!grid || !viewport) return;
    clearLayout(grid);

    int catIndex = 0;
    for (const auto &cat : homeCategoryPreviews()) {
        auto *card = new QFrame;
        card->setObjectName("CategoryCard");
        card->setFixedSize(250, 220);
        card->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 10, 12, 12);
        cardLayout->setSpacing(8);
        auto *titleRow = new QHBoxLayout;
        auto *title = new QLabel(cat.category);
        title->setObjectName("ProductName");
        auto *arrow = new QLabel(">");
        arrow->setObjectName("ProductPrice");
        titleRow->addWidget(title);
        titleRow->addStretch();
        titleRow->addWidget(arrow);
        cardLayout->addLayout(titleRow);

        auto *imgGrid = new QGridLayout;
        imgGrid->setHorizontalSpacing(8);
        imgGrid->setVerticalSpacing(6);
        for (int i = 0; i < cat.itemNames.size() && i < 4; ++i) {
            const QString itemName = cat.itemNames[i];
            const QString imagePath = imagePathFromScannedItems(cat.category == "教材资料" || cat.category == "学习资料" ? QString("学习资料") : cat.category, itemName);
            QString displayName = itemName;
            if (displayName == "英語资料") displayName = "英语资料";
            if (displayName == "C++程序設計") displayName = "C++程序设计";
            if (displayName == "pencil_") displayName = "Apple Pencil";
            if (displayName == "nike运动鞋") displayName = "Nike运动鞋";
            if (displayName == "平板") displayName = "平板电脑";
            if (displayName == "宿舍台灯") displayName = "台灯";
            if (displayName == "电风扇") displayName = "风扇";
            auto *cell = new QVBoxLayout;
            cell->setSpacing(3);
            cell->addWidget(categoryImageBox(imagePath, QSize(75, 60), "CategoryImage"), 0, Qt::AlignCenter);
            auto *tag = new QLabel(displayName);
            tag->setObjectName("TagLabel");
            tag->setAlignment(Qt::AlignCenter);
            tag->setStyleSheet("font-size: 11px;");
            cell->addWidget(tag);
            imgGrid->addLayout(cell, i / 2, i % 2);
        }

        cardLayout->addLayout(imgGrid);
        grid->addWidget(card, 0, catIndex);
        ++catIndex;
    }

    viewport->setFixedSize(catIndex * 266, 250);
}

bool categoryMatches(const QString &category, const QString &filter)
{
    if (filter.contains("全部") || filter.contains("å…¨")) return true;
    if (category == filter || category.contains(filter)) return true;
    if (filter.contains("教材") && (category.contains("书") || category.contains("教材"))) return true;
    if (filter.contains("数码") && (category.contains("电子") || category.contains("数码"))) return true;
    if (filter.contains("宿舍") && (category.contains("生活") || category.contains("宿舍"))) return true;
    if (filter.contains("服饰") && (category.contains("服饰") || category.contains("鞋") || category.contains("包"))) return true;
    if (filter.contains("运动") && category.contains("运动")) return true;
    if (filter.contains("办公") && (category.contains("学习") || category.contains("办公"))) return true;
    if (filter.contains("代步") && category.contains("代步")) return true;
    if (filter.contains("美妆") && (category.contains("美妆") || category.contains("零食"))) return true;
    return false;
}

int stableIndex(const QString &id, const QString &name, const QString &category, int size)
{
    if (size <= 0) return 0;
    QRegularExpression numberPattern("(\\d+)");
    const QRegularExpressionMatch numberMatch = numberPattern.match(name);
    if (numberMatch.hasMatch()) {
        const int number = numberMatch.captured(1).toInt();
        if (number > 0) return (number - 1) % size;
    }
    const int numericId = id.toInt();
    if (numericId > 0) return (numericId - 1) % size;
    return int(qHash(name + category) % uint(size));
}

QString displayProductName(const UserGoodsItem &item)
{
    const QString raw = item.name.trimmed();
    if (!raw.contains("商品")) return raw;

    const QString category = item.category;
    auto pick = [&](const QStringList &names) {
        return names.value(stableIndex(item.id, raw, category, names.size()));
    };

    if (categoryMatches(category, "教材资料") || category.contains("书") || category.contains("教材")) {
        return pick({"高等数学教材", "英语资料", "C++程序设计", "课堂笔记", "阅读资料"});
    }
    if (categoryMatches(category, "电子数码") || category.contains("电子") || category.contains("数码")) {
        return pick({"笔记本电脑", "苹果笔记本电脑", "平板电脑", "耳机", "头戴式耳机", "相机", "佳能相机", "键盘", "鼠标", "显示器", "苹果手机"});
    }
    if (categoryMatches(category, "宿舍用品") || category.contains("生活") || category.contains("宿舍")) {
        return pick({"台灯", "毛毯", "电风扇", "小风扇", "加湿器", "水杯", "收纳箱", "收纳盒", "吹风机", "小冰箱"});
    }
    if (categoryMatches(category, "学习资料") || category.contains("学习") || category.contains("办公")) {
        return pick({"触控笔", "计算器", "打印机", "文件夹", "书桌支架", "显示器支架"});
    }
    if (categoryMatches(category, "服饰鞋包") || category.contains("服饰") || category.contains("鞋") || category.contains("包")) {
        return pick({"双肩包", "帆布包", "女士包包", "运动鞋", "卫衣", "外套", "饰品", "通勤包"});
    }
    if (categoryMatches(category, "运动器材") || category.contains("运动")) {
        return pick({"自行车", "篮球", "羽毛球拍", "哑铃", "瑜伽垫", "滑板"});
    }
    if (categoryMatches(category, "代步工具") || category.contains("代步")) {
        return pick({"自行车", "电动车", "滑板车", "平衡车"});
    }
    if (categoryMatches(category, "美妆零食") || category.contains("美妆") || category.contains("零食")) {
        return pick({"保湿面霜", "巧克力礼盒", "口红", "面膜", "香水", "护手霜", "洗面奶", "坚果礼盒", "饼干礼盒", "咖啡礼盒"});
    }

    return raw;
}

QString categoryProductNameAt(const QString &category, int index)
{
    auto pick = [index](const QStringList &names) {
        return names.value(index % names.size());
    };

    if (categoryMatches(category, "教材资料") || category.contains("书") || category.contains("教材")) {
        return pick({"高等数学教材", "英语资料", "C++程序设计", "课堂笔记", "阅读资料"});
    }
    if (categoryMatches(category, "电子数码") || category.contains("电子") || category.contains("数码")) {
        return pick({"笔记本电脑", "苹果笔记本电脑", "平板电脑", "耳机", "头戴式耳机", "相机", "佳能相机", "键盘", "鼠标", "显示器", "苹果手机"});
    }
    if (categoryMatches(category, "宿舍用品") || category.contains("生活") || category.contains("宿舍")) {
        return pick({"台灯", "毛毯", "电风扇", "小风扇", "加湿器", "水杯", "收纳箱", "收纳盒", "吹风机", "小冰箱"});
    }
    if (categoryMatches(category, "学习资料") || category.contains("学习") || category.contains("办公")) {
        return pick({"触控笔", "计算器", "打印机", "文件夹", "书桌支架", "显示器支架"});
    }
    if (categoryMatches(category, "服饰鞋包") || category.contains("服饰") || category.contains("鞋") || category.contains("包")) {
        return pick({"双肩包", "帆布包", "女士包包", "运动鞋", "卫衣", "外套", "饰品", "通勤包"});
    }
    if (categoryMatches(category, "运动器材") || category.contains("运动")) {
        return pick({"自行车", "篮球", "羽毛球拍", "哑铃", "瑜伽垫", "滑板"});
    }
    if (categoryMatches(category, "代步工具") || category.contains("代步")) {
        return pick({"自行车", "电动车", "滑板车", "平衡车"});
    }
    if (categoryMatches(category, "美妆零食") || category.contains("美妆") || category.contains("零食")) {
        return pick({"保湿面霜", "巧克力礼盒", "口红", "面膜", "香水", "护手霜", "洗面奶", "坚果礼盒", "饼干礼盒", "咖啡礼盒"});
    }

    return QString();
}

void appendFallbackTravelGoods(QVector<UserGoodsItem> &goods)
{
    for (const auto &g : goods) {
        if (categoryMatches(g.category, "代步工具")) return;
    }

    goods.push_back({"900001", "自行车", "代步工具", 180.00, 3});
    goods.push_back({"900002", "电动车", "代步工具", 680.00, 2});
    goods.push_back({"900003", "滑板车", "代步工具", 120.00, 4});
    goods.push_back({"900004", "平衡车", "代步工具", 520.00, 1});
}
}

UserHomeWindow::UserHomeWindow(const QString &username, QWidget *parent)
    : QWidget(parent), m_username(username)
{
    resize(1180, 760);
    setMinimumSize(1040, 680);
    setWindowTitle("校园二手交易平台");
    setStyleSheet(kUserQss);

    auto *rootLayout = new QHBoxLayout(this);
    setObjectName("Root");
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(createSidebar());

    m_stack = new QStackedWidget;
    m_homePage = createHomePage();
    m_categoryPage = createCategoryPage();
    m_cartPage = createCartPage();
    m_sellPage = createSellPage();
    m_historyPage = createHistoryPage();
    m_profilePage = createProfilePage();
    m_stack->addWidget(m_homePage);
    m_stack->addWidget(m_categoryPage);
    m_stack->addWidget(m_cartPage);
    m_stack->addWidget(m_sellPage);
    m_stack->addWidget(m_historyPage);
    m_stack->addWidget(m_profilePage);
    rootLayout->addWidget(m_stack, 1);

    loadGoodsData();
    updateGoodsTable(m_goods);
    updateCategoryTable();
    updateCartTable();
    updateHistoryTable();
    switchPage(m_homePage, m_btnHome);
}

QPushButton *UserHomeWindow::makeNavButton(const QString &text)
{
    auto *btn = new QPushButton(text);
    btn->setProperty("nav", true);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QPushButton *UserHomeWindow::makePrimaryButton(const QString &text)
{
    auto *btn = new QPushButton(text);
    btn->setObjectName("Primary");
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QPushButton *UserHomeWindow::makeDangerButton(const QString &text)
{
    auto *btn = new QPushButton(text);
    btn->setObjectName("Danger");
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QLabel *UserHomeWindow::makeTitle(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName("Title");
    return label;
}

QLabel *UserHomeWindow::makeSubtitle(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName("Subtitle");
    return label;
}

QWidget *UserHomeWindow::makeCard()
{
    auto *card = new QFrame;
    card->setObjectName("Card");
    return card;
}

QWidget *UserHomeWindow::createSidebar()
{
    auto *side = new QFrame;
    side->setObjectName("Sidebar");
    side->setFixedWidth(230);
    auto *layout = new QVBoxLayout(side);
    layout->setContentsMargins(20, 26, 20, 24);
    layout->setSpacing(12);

    auto *logo = new QLabel("校园二手");
    logo->setObjectName("Logo");
    auto *hint = new QLabel("当前用户：" + m_username);
    hint->setObjectName("SideHint");
    layout->addWidget(logo);
    layout->addWidget(hint);
    layout->addSpacing(18);

    m_btnHome = makeNavButton("首页");
    m_btnCategory = makeNavButton("分类");
    m_btnCart = makeNavButton("购物车");
    m_btnSell = makeNavButton("我要出售");
    m_btnHistory = makeNavButton("购买记录");
    m_btnProfile = makeNavButton("个人中心");

    layout->addWidget(m_btnHome);
    layout->addWidget(m_btnCategory);
    layout->addWidget(m_btnCart);
    layout->addWidget(m_btnSell);
    layout->addWidget(m_btnHistory);
    layout->addWidget(m_btnProfile);
    layout->addStretch();

    auto *backHome = new QPushButton("返回首页");
    backHome->setObjectName("Light");
    auto *logout = makeDangerButton("退出系统");
    layout->addWidget(backHome);
    layout->addWidget(logout);

    connect(m_btnHome, &QPushButton::clicked, this, [=] { switchPage(m_homePage, m_btnHome); });
    connect(m_btnCategory, &QPushButton::clicked, this, [=] { updateCategoryTable(); switchPage(m_categoryPage, m_btnCategory); });
    connect(m_btnCart, &QPushButton::clicked, this, [=] { updateCartTable(); switchPage(m_cartPage, m_btnCart); });
    connect(m_btnSell, &QPushButton::clicked, this, [=] { switchPage(m_sellPage, m_btnSell); });
    connect(m_btnHistory, &QPushButton::clicked, this, [=] { updateHistoryTable(); switchPage(m_historyPage, m_btnHistory); });
    connect(m_btnProfile, &QPushButton::clicked, this, [=] { switchPage(m_profilePage, m_btnProfile); });
    connect(backHome, &QPushButton::clicked, this, [=] {
        switchPage(m_homePage, m_btnHome);
    });
    connect(logout, &QPushButton::clicked, this, [=]() {
        auto *welcome = new WelcomeWindow;
        welcome->setAttribute(Qt::WA_DeleteOnClose);
        welcome->show();
        close();
    });
    return side;
}

QWidget *UserHomeWindow::createHomePage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(16);
    auto *strip = new QLabel("校园二手集市上线 · 闲置好物重新流动 · Campus Market");
    strip->setObjectName("TopStrip");
    layout->addWidget(strip);

    auto *heroCard = new QFrame;
    heroCard->setObjectName("MarketCard");
    heroCard->setFixedHeight(260);
    auto *heroLayout = new QHBoxLayout(heroCard);
    heroLayout->setContentsMargins(22, 20, 28, 20);
    heroLayout->setSpacing(32);
    auto *imageContainer = new QWidget;
    auto *imageLayout = new QHBoxLayout(imageContainer);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    auto *banner = new QLabel;
    banner->setObjectName("Banner");
    banner->setAlignment(Qt::AlignCenter);
    banner->setScaledContents(true);
    banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const QString bannerPath = QDir(QApplication::applicationDirPath()).filePath("ui_assets/home_banner.jpg");
    QPixmap bannerPix(bannerPath);
    if (bannerPix.isNull()) {
        bannerPix.load(QDir::current().filePath("ui_assets/home_banner.jpg"));
    }
    if (!bannerPix.isNull()) {
        banner->setPixmap(bannerPix.scaled(QSize(520, 240), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        qDebug() << "Home hero image load failed:" << bannerPath;
    }
    imageLayout->addWidget(banner);
    heroLayout->addWidget(imageContainer, 1);

    auto *textContainer = new QWidget;
    auto *textContainerLayout = new QVBoxLayout(textContainer);
    textContainerLayout->setContentsMargins(0, 0, 0, 0);
    auto *heroText = new QVBoxLayout;
    heroText->setSpacing(12);
    auto *homeTitle = new QLabel("校园二手集市");
    homeTitle->setObjectName("HomeTitle");
    homeTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto *homeSubtitle = new QLabel("教材、数码、宿舍用品和代步好物，一站式发布、浏览与交易。");
    homeSubtitle->setObjectName("HomeSubtitle");
    homeSubtitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    homeSubtitle->setWordWrap(true);
    heroText->addStretch();
    heroText->addWidget(homeTitle);
    heroText->addWidget(homeSubtitle);
    heroText->addStretch();
    textContainerLayout->addLayout(heroText);
    heroLayout->addWidget(textContainer, 1);
    layout->addWidget(heroCard);

    auto *toolbarCard = new QFrame;
    toolbarCard->setObjectName("Panel");
    auto *toolbar = new QHBoxLayout(toolbarCard);
    toolbar->setContentsMargins(18, 18, 18, 18);
    toolbar->setSpacing(12);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("搜索商品名称 / 分类");
    auto *search = makePrimaryButton("搜索");
    auto *recommend = makePrimaryButton("推荐商品");
    toolbar->addWidget(m_searchEdit, 1);
    toolbar->addWidget(search);
    toolbar->addWidget(recommend);
    search->setText("➜");
    recommend->setText("推荐");
    layout->addWidget(toolbarCard);

    auto *pillRow = new QHBoxLayout;
    const QStringList pills = {"学习资料", "电子数码", "宿舍用品", "服饰鞋包", "运动器材", "代步工具", "美妆零食"};
    for (const QString &name : pills) {
        auto *pill = new QPushButton(name);
        pill->setObjectName("Pill");
        pill->setCursor(Qt::PointingHandCursor);
        connect(pill, &QPushButton::clicked, this, [=]() {
            if (m_categoryCombo) m_categoryCombo->setCurrentText(name);
            updateCategoryTable();
            switchPage(m_categoryPage, m_btnCategory);
        });
        pillRow->addWidget(pill);
    }
    layout->addLayout(pillRow);

    auto *goodsPanel = new QFrame;
    goodsPanel->setObjectName("Panel");
    auto *goodsPanelLayout = new QVBoxLayout(goodsPanel);
    goodsPanelLayout->setContentsMargins(20, 20, 20, 20);
    goodsPanelLayout->setSpacing(14);
    auto *goodsSectionTitle = new QLabel("校园好物");
    goodsSectionTitle->setObjectName("SectionTitle");
    goodsSectionTitle->setText("校园分类");
    goodsPanelLayout->addWidget(goodsSectionTitle);

    m_homeCategoryGrid = new QGridLayout;
    m_homeCategoryGrid->setHorizontalSpacing(16);
    m_homeCategoryGrid->setVerticalSpacing(0);
    m_homeCategoryGrid->setContentsMargins(0, 0, 0, 0);
    m_homeCategoryScroll = new QScrollArea;
    m_homeCategoryScroll->setWidgetResizable(false);
    m_homeCategoryScroll->setFixedHeight(330);
    m_homeCategoryScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_homeCategoryScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_homeCategoryScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_homeCategoryViewport = new QWidget;
    m_homeCategoryViewport->setLayout(m_homeCategoryGrid);
    m_homeCategoryScroll->setWidget(m_homeCategoryViewport);
    populateHomePreviewGrid(m_homeCategoryGrid, m_homeCategoryViewport);
    goodsPanelLayout->addWidget(m_homeCategoryScroll, 1);

    m_goodsTable = new QTableWidget;
    m_goodsTable->setColumnCount(5);
    m_goodsTable->setHorizontalHeaderLabels({"ID", "商品", "分类", "价格", "库存"});
    m_goodsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_goodsTable->verticalHeader()->setVisible(false);
    m_goodsTable->setAlternatingRowColors(true);
    m_goodsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_goodsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_goodsTable->hide();
    layout->addWidget(goodsPanel, 1);

    connect(search, &QPushButton::clicked, this, &UserHomeWindow::searchGoods);
    connect(recommend, &QPushButton::clicked, this, &UserHomeWindow::recommendGoods);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &UserHomeWindow::searchGoods);
    return page;
}

QWidget *UserHomeWindow::createCategoryPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(16);
    auto *strip = new QLabel("校园二手集市上线 · 闲置好物重新流动 · Campus Market");
    strip->setObjectName("TopStrip");
    layout->addWidget(strip);
    layout->addWidget(makeTitle("商品分类"));
    layout->addWidget(makeSubtitle("选择分类，浏览对应校园二手商品。"));

    auto *toolbarCard = new QFrame;
    toolbarCard->setObjectName("Panel");
    auto *toolbar = new QHBoxLayout(toolbarCard);
    toolbar->setContentsMargins(18, 18, 18, 18);
    toolbar->setSpacing(12);
    m_categoryCombo = new QComboBox;
    m_categoryCombo->addItems({"代步工具", "电子数码", "服饰鞋包", "美妆零食", "宿舍用品", "学习资料", "运动器材"});
    m_minPriceEdit = new QLineEdit; m_minPriceEdit->setPlaceholderText("在当前分类中搜索商品...");
    m_categorySearchEdit = m_minPriceEdit;
    m_maxPriceEdit = new QLineEdit; m_maxPriceEdit->setPlaceholderText("最高价");
    m_maxPriceEdit->hide();
    auto *filter = makePrimaryButton("筛选");
    toolbar->addWidget(m_categoryCombo);
    toolbar->addWidget(m_minPriceEdit);
    toolbar->addWidget(m_maxPriceEdit);
    filter->setText("➜");
    toolbar->addWidget(filter);
    toolbar->addStretch();
    layout->addWidget(toolbarCard);

    auto *categoryPills = new QHBoxLayout;
    const QStringList categoryNames = {"代步工具", "电子数码", "服饰鞋包", "美妆零食", "宿舍用品", "学习资料", "运动器材"};
    for (const QString &name : categoryNames) {
        auto *pill = new QPushButton(name);
        pill->setObjectName("Pill");
        pill->setProperty("active", m_categoryPillButtons.isEmpty());
        pill->setCursor(Qt::PointingHandCursor);
        m_categoryPillButtons.push_back(pill);
        connect(pill, &QPushButton::clicked, this, [=]() {
            m_categoryCombo->setCurrentText(name);
            for (auto *btn : m_categoryPillButtons) {
                btn->setProperty("active", btn == pill);
                btn->style()->unpolish(btn);
                btn->style()->polish(btn);
            }
            updateCategoryTable();
        });
        categoryPills->addWidget(pill);
    }
    layout->addLayout(categoryPills);

    auto *categoryPanel = new QFrame;
    categoryPanel->setObjectName("MarketCard");
    auto *categoryPanelLayout = new QVBoxLayout(categoryPanel);
    categoryPanelLayout->setContentsMargins(20, 20, 20, 20);
    categoryPanelLayout->setSpacing(14);
    auto *categorySectionTitle = new QLabel("分类商品");
    categorySectionTitle->setObjectName("SectionTitle");
    categorySectionTitle->setText("商品分类");
    categoryPanelLayout->addWidget(categorySectionTitle);

    m_categoryGoodsGrid = new QGridLayout;
    m_categoryGoodsGrid->setHorizontalSpacing(16);
    m_categoryGoodsGrid->setVerticalSpacing(16);
    m_categoryGoodsGrid->setContentsMargins(0, 0, 0, 0);
    auto *productScroll = new QScrollArea;
    productScroll->setWidgetResizable(true);
    productScroll->setMinimumHeight(420);
    productScroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *productViewport = new QWidget;
    productViewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    productViewport->setLayout(m_categoryGoodsGrid);
    productScroll->setWidget(productViewport);
    categoryPanelLayout->addWidget(productScroll, 1);

    m_categoryTable = new QTableWidget;
    m_categoryTable->setColumnCount(5);
    m_categoryTable->setHorizontalHeaderLabels({"ID", "商品", "分类", "价格", "库存"});
    m_categoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_categoryTable->verticalHeader()->setVisible(false);
    m_categoryTable->setAlternatingRowColors(true);
    m_categoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_categoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_categoryTable->hide();
    layout->addWidget(categoryPanel, 1);
    connect(filter, &QPushButton::clicked, this, &UserHomeWindow::updateCategoryTable);
    connect(m_categorySearchEdit, &QLineEdit::returnPressed, this, &UserHomeWindow::updateCategoryTable);
    m_categoryCombo->setCurrentText("代步工具");
    return page;
}

QWidget *UserHomeWindow::createCartPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(16);
    layout->addWidget(makeTitle("我的购物车"));
    layout->addWidget(makeSubtitle("核对商品信息后即可结算。"));

    m_cartTable = new QTableWidget;
    m_cartTable->setColumnCount(5);
    m_cartTable->setHorizontalHeaderLabels({"ID", "商品", "分类", "价格", "库存"});
    m_cartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_cartTable->hide();

    auto *cartScroll = new QScrollArea;
    cartScroll->setWidgetResizable(true);
    auto *cartViewport = new QWidget;
    m_cartCardsLayout = new QVBoxLayout(cartViewport);
    m_cartCardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cartCardsLayout->setSpacing(14);
    cartScroll->setWidget(cartViewport);
    layout->addWidget(cartScroll, 1);

    auto *summary = makeCard();
    auto *summaryLayout = new QHBoxLayout(summary);
    summaryLayout->setContentsMargins(22, 16, 22, 16);
    m_cartTotalLabel = new QLabel("订单汇总：￥0.00");
    m_cartTotalLabel->setObjectName("Total");
    m_cartItemCountLabel = new QLabel("商品数量：0");
    m_cartItemCountLabel->setObjectName("SummaryText");
    m_cartSubtotalLabel = new QLabel("商品总额：¥0.00");
    m_cartSubtotalLabel->setObjectName("SummaryText");
    m_cartPayableLabel = new QLabel("应付合计：¥0.00");
    m_cartPayableLabel->setObjectName("MetricSmall");
    auto *checkout = makePrimaryButton("立即结算");
    auto *remove = makeDangerButton("移除选中商品");
    auto *summaryText = new QVBoxLayout;
    summaryText->addWidget(m_cartItemCountLabel);
    summaryText->addWidget(m_cartSubtotalLabel);
    summaryText->addWidget(m_cartPayableLabel);
    summaryLayout->addLayout(summaryText);
    summaryLayout->addStretch();
    summaryLayout->addWidget(m_cartTotalLabel);
    summaryLayout->addWidget(checkout);
    summaryLayout->addWidget(remove);
    layout->addWidget(summary);

    connect(checkout, &QPushButton::clicked, this, &UserHomeWindow::buySelectedCartItem);
    connect(remove, &QPushButton::clicked, this, &UserHomeWindow::removeSelectedCartItem);
    return page;
}

QWidget *UserHomeWindow::createSellPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(16);
    layout->addWidget(makeTitle("发布闲置商品"));
    layout->addWidget(makeSubtitle("填写商品信息后提交审核，管理员通过后即可上架。"));

    auto *card = makeCard();
    auto *form = new QVBoxLayout(card);
    form->setContentsMargins(24, 22, 24, 22);
    form->setSpacing(12);
    m_sellNameEdit = new QLineEdit; m_sellNameEdit->setPlaceholderText("商品名称");
    m_sellCategoryCombo = new QComboBox; m_sellCategoryCombo->addItems({"代步工具", "电子数码", "服饰鞋包", "美妆零食", "宿舍用品", "学习资料", "运动器材"});
    m_sellPriceEdit = new QLineEdit; m_sellPriceEdit->setPlaceholderText("价格");
    m_sellStockSpin = new QSpinBox; m_sellStockSpin->setRange(1, 9999); m_sellStockSpin->setValue(1);
    m_sellConditionEdit = new QLineEdit; m_sellConditionEdit->setPlaceholderText("成色，例如 九成新 / 八成新");
    m_sellDescriptionEdit = new QTextEdit; m_sellDescriptionEdit->setPlaceholderText("商品描述");
    form->addWidget(new QLabel("商品名称")); form->addWidget(m_sellNameEdit);
    form->addWidget(new QLabel("商品分类")); form->addWidget(m_sellCategoryCombo);
    form->addWidget(new QLabel("价格")); form->addWidget(m_sellPriceEdit);
    form->addWidget(new QLabel("数量")); form->addWidget(m_sellStockSpin);
    form->addWidget(new QLabel("成色")); form->addWidget(m_sellConditionEdit);
    form->addWidget(new QLabel("商品描述")); form->addWidget(m_sellDescriptionEdit);
    auto *actions = new QHBoxLayout;
    auto *cancel = new QPushButton("取消"); cancel->setObjectName("Light");
    auto *submit = makePrimaryButton("提交审核");
    actions->addStretch(); actions->addWidget(cancel); actions->addWidget(submit);
    form->addLayout(actions);
    layout->addWidget(card);
    layout->addStretch();

    connect(cancel, &QPushButton::clicked, this, [=] { switchPage(m_homePage, m_btnHome); });
    connect(submit, &QPushButton::clicked, this, &UserHomeWindow::submitGoodsForSale);
    return page;
}

QWidget *UserHomeWindow::createHistoryPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(16);
    layout->addWidget(makeTitle("我的购买记录"));
    layout->addWidget(makeSubtitle("查看历史订单与交易状态。"));

    auto *metrics = new QHBoxLayout;
    auto makeMetric = [&](const QString &name, QLabel **label) {
        auto *card = makeCard();
        auto *v = new QVBoxLayout(card);
        v->setContentsMargins(20, 16, 20, 16);
        auto *n = new QLabel(name); n->setObjectName("Subtitle");
        *label = new QLabel("0"); (*label)->setObjectName("Metric");
        v->addWidget(n); v->addWidget(*label);
        metrics->addWidget(card);
    };
    makeMetric("累计订单", &m_historyOrderCountLabel);
    makeMetric("累计消费", &m_historyAmountLabel);
    makeMetric("待完成订单", &m_historyPendingLabel);
    makeMetric("已完成订单", &m_historyDoneLabel);
    layout->addLayout(metrics);

    m_historyTable = new QTableWidget;
    m_historyTable->setColumnCount(6);
    m_historyTable->setHorizontalHeaderLabels({"商品", "分类", "单价", "数量", "状态", "时间"});
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->hide();

    auto *section = new QLabel("订单列表");
    section->setObjectName("SectionTitle");
    layout->addWidget(section);

    auto *historyScroll = new QScrollArea;
    historyScroll->setWidgetResizable(true);
    auto *historyViewport = new QWidget;
    m_historyListLayout = new QVBoxLayout(historyViewport);
    m_historyListLayout->setContentsMargins(0, 0, 0, 0);
    m_historyListLayout->setSpacing(14);
    historyScroll->setWidget(historyViewport);
    layout->addWidget(historyScroll, 1);
    auto *clear = makeDangerButton("清空记录");
    layout->addWidget(clear, 0, Qt::AlignLeft);
    connect(clear, &QPushButton::clicked, this, &UserHomeWindow::clearHistory);
    return page;
}

QWidget *UserHomeWindow::createProfilePage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(16);
    layout->addWidget(makeTitle("个人中心"));
    layout->addWidget(makeSubtitle("查看当前登录账号与平台使用信息。"));
    auto *card = makeCard();
    auto *v = new QVBoxLayout(card);
    v->setContentsMargins(24, 22, 24, 22);
    v->addWidget(new QLabel("用户名：" + m_username));
    v->addWidget(new QLabel("购物车商品数：0"));
    v->addWidget(new QLabel("购买记录会在结算后自动更新。"));
    layout->addWidget(card);
    layout->addStretch();
    return page;
}

void UserHomeWindow::switchPage(QWidget *page, QPushButton *activeButton)
{
    if (!m_stack || !page) return;
    loadGoodsData();
    if (page == m_homePage) updateGoodsTable(filteredGoods());
    if (page == m_categoryPage) updateCategoryTable();
    if (page == m_cartPage) updateCartTable();
    if (page == m_historyPage) updateHistoryTable();
    m_stack->setCurrentWidget(page);
    updateSidebarHighlight(activeButton);
}

void UserHomeWindow::updateSidebarHighlight(QPushButton *activeButton)
{
    const QList<QPushButton *> buttons = {m_btnHome, m_btnCategory, m_btnCart, m_btnSell, m_btnHistory, m_btnProfile};
    for (auto *btn : buttons) {
        btn->setProperty("active", btn == activeButton);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

void UserHomeWindow::loadGoodsData()
{
    m_goods.clear();
    for (const auto &g : AccountStore::goods()) {
        if (g.status == "已上架" && g.stock > 0) {
            m_goods.push_back({g.id, g.name, g.category, g.price, g.stock});
        }
    }
    appendFallbackTravelGoods(m_goods);
    m_currentItems = m_goods;
}

QVector<UserGoodsItem> UserHomeWindow::filteredGoods() const
{
    QVector<UserGoodsItem> result;
    const QString keyword = m_searchEdit ? m_searchEdit->text().trimmed() : QString();
    for (const auto &g : m_goods) {
        if (keyword.isEmpty() || g.name.contains(keyword, Qt::CaseInsensitive) || g.category.contains(keyword, Qt::CaseInsensitive)) {
            result.push_back(g);
        }
    }
    return result;
}

void UserHomeWindow::updateGoodsTable(const QVector<UserGoodsItem> &items)
{
    if (!m_goodsTable) return;
    m_currentItems = items;
    m_goodsTable->setRowCount(0);
    for (const auto &g : items) {
        const int row = m_goodsTable->rowCount();
        m_goodsTable->insertRow(row);
        m_goodsTable->setItem(row, 0, item(g.id));
        m_goodsTable->setItem(row, 1, item(g.name));
        m_goodsTable->setItem(row, 2, item(g.category));
        m_goodsTable->setItem(row, 3, item(QString::number(g.price, 'f', 2)));
        m_goodsTable->setItem(row, 4, item(QString::number(g.stock)));
    }
}

void UserHomeWindow::updateCategoryTable()
{
    if (!m_categoryTable) return;
    QVector<CategoryDisplayItem> result;
    const QString cat = m_categoryCombo ? m_categoryCombo->currentText() : QString("代步工具");
    const QString keyword = m_categorySearchEdit ? m_categorySearchEdit->text().trimmed() : QString();
    for (const auto &g : scannedCategoryItems()) {
        if (keyword.isEmpty()) {
            if (g.category != cat) continue;
        } else if (!g.name.contains(keyword, Qt::CaseInsensitive)) {
            continue;
        }
        result.push_back(g);
    }
    m_categoryTable->setRowCount(0);
    for (int i = 0; i < result.size(); ++i) {
        const auto &g = result[i];
        const int row = m_categoryTable->rowCount();
        m_categoryTable->insertRow(row);
        m_categoryTable->setItem(row, 0, item(QString("UC%1").arg(i + 1, 3, 10, QChar('0'))));
        m_categoryTable->setItem(row, 1, item(g.name));
        m_categoryTable->setItem(row, 2, item(g.category));
        m_categoryTable->setItem(row, 3, item(QString::number(g.price, 'f', 2)));
        m_categoryTable->setItem(row, 4, item(QString::number(g.stock)));
    }
    if (m_categoryGoodsGrid) {
        clearLayout(m_categoryGoodsGrid);
        for (int i = 0; i < result.size(); ++i) {
            const auto g = result[i];
            const QString goodsId = QString("UC%1").arg(i + 1, 3, 10, QChar('0'));
            auto *card = new QFrame;
            card->setObjectName("ProductCard");
            card->setMinimumSize(320, 360);
            card->setFixedWidth(320);
            card->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            auto *cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(14, 14, 14, 14);
            cardLayout->setSpacing(8);

            auto *img = categoryImageBox(g.imagePath, QSize(220, 140), "ProductImage");
            auto *name = new QLabel(g.name);
            name->setObjectName("ProductName");
            name->setWordWrap(true);
            auto *category = new QLabel(g.category);
            category->setObjectName("ProductMeta");
            auto *price = new QLabel("¥" + QString::number(g.price, 'f', 2));
            price->setObjectName("ProductPrice");
            auto *meta = new QLabel("成色：良好    库存：" + QString::number(g.stock));
            meta->setObjectName("ProductMeta");
            auto *seller = new QLabel("卖家：" + (i % 2 == 0 ? QString("user1") : QString("user2")));
            seller->setObjectName("ProductMeta");

            auto *detail = new QPushButton("详情");
            detail->setObjectName("Light");
            auto *add = makePrimaryButton("加入购物车");
            auto *buy = makePrimaryButton("立即购买");
            detail->setFixedSize(72, 36);
            add->setFixedSize(96, 36);
            buy->setFixedSize(96, 36);
            detail->setStyleSheet("QPushButton { background: #FFFFFF; color: #111111; border: 1px solid #D1D5DB; border-radius: 8px; }"
                                  "QPushButton:hover { background: #F7F7FA; }");
            add->setStyleSheet("QPushButton { background: #4F46E5; color: #FFFFFF; border: none; border-radius: 8px; }"
                               "QPushButton:hover { background: #4338CA; }");
            buy->setStyleSheet("QPushButton { background: #10B981; color: #FFFFFF; border: none; border-radius: 8px; }"
                               "QPushButton:hover { background: #059669; }");
            connect(detail, &QPushButton::clicked, this, [=]() {
                QMessageBox::information(this,
                                         "商品详情",
                                         g.name +
                                         "\n分类：" + g.category +
                                         "\n价格：¥" + QString::number(g.price, 'f', 2) +
                                         "\n库存：" + QString::number(g.stock) +
                                         "\n图片：" + g.imagePath);
            });
            connect(add, &QPushButton::clicked, this, [=]() {
                m_cart.push_back({goodsId, g.name, g.category, g.price, g.stock});
                updateCartTable();
                QMessageBox::information(this, "成功", "已加入购物车。");
            });
            connect(buy, &QPushButton::clicked, this, [=]() {
                AccountStore::addPurchaseRecord(g.name, g.category, g.price, 1, "未完成");
                updateHistoryTable();
                QMessageBox::information(this, "购买成功", "订单已生成，可在购买记录中查看。");
            });

            cardLayout->addWidget(img, 0, Qt::AlignCenter);
            cardLayout->addWidget(name);
            cardLayout->addWidget(category);
            cardLayout->addWidget(price);
            cardLayout->addWidget(meta);
            cardLayout->addWidget(seller);
            cardLayout->addStretch();
            auto *buttonRow = new QHBoxLayout;
            buttonRow->setContentsMargins(0, 0, 0, 0);
            buttonRow->setSpacing(12);
            buttonRow->addWidget(detail);
            buttonRow->addWidget(add);
            buttonRow->addWidget(buy);
            cardLayout->addLayout(buttonRow);
            m_categoryGoodsGrid->addWidget(card, i / 4, i % 4);
        }
        if (result.isEmpty()) {
            auto *empty = new QLabel("当前分类暂无商品");
            empty->setObjectName("Subtitle");
            empty->setAlignment(Qt::AlignCenter);
            m_categoryGoodsGrid->addWidget(empty, 0, 0);
        }
    }
}

void UserHomeWindow::updateCartTable()
{
    if (!m_cartTable) return;
    double total = 0;
    QSet<int> validSelectedRows;
    for (int row : m_selectedCartRows) {
        if (row >= 0 && row < m_cart.size()) validSelectedRows.insert(row);
    }
    m_selectedCartRows = validSelectedRows;
    for (int row : m_selectedCartRows) {
        total += m_cart[row].price;
    }
    m_cartTable->setRowCount(0);
    for (const auto &g : m_cart) {
        const CategoryDisplayItem displayProduct = scannedProductForDisplay(g.name, g.category);
        const QString displayName = displayProduct.name.isEmpty() ? g.name : displayProduct.name;
        const QString displayCategory = displayProduct.category.isEmpty() ? g.category : displayProduct.category;
        const int row = m_cartTable->rowCount();
        m_cartTable->insertRow(row);
        m_cartTable->setItem(row, 0, item(g.id));
        m_cartTable->setItem(row, 1, item(displayName));
        m_cartTable->setItem(row, 2, item(displayCategory));
        m_cartTable->setItem(row, 3, item(QString::number(g.price, 'f', 2)));
        m_cartTable->setItem(row, 4, item(QString::number(g.stock)));
    }
    if (m_cartCardsLayout) {
        clearLayout(m_cartCardsLayout);
        for (int row = 0; row < m_cart.size(); ++row) {
            const auto g = m_cart[row];
            const bool selected = m_selectedCartRows.contains(row);
            const CategoryDisplayItem displayProduct = scannedProductForDisplay(g.name, g.category);
            const QString displayName = displayProduct.name.isEmpty() ? g.name : displayProduct.name;
            const QString displayCategory = displayProduct.category.isEmpty() ? g.category : displayProduct.category;
            auto *card = new QFrame;
            card->setObjectName("CartCard");
            if (selected) {
                card->setStyleSheet("QFrame#CartCard { border: 2px solid #5B4BFF; background: #F4F2FF; border-radius: 20px; }");
            }
            auto *rowLayout = new QHBoxLayout(card);
            rowLayout->setContentsMargins(18, 16, 18, 16);
            rowLayout->setSpacing(16);

            auto *thumb = scannedProductImageBox(displayName, displayCategory, QSize(118, 88), "ProductImage");
            auto *info = new QVBoxLayout;
            auto *name = new QLabel(displayName);
            name->setObjectName("ProductName");
            auto *meta = new QLabel("分类：" + g.category + "    商品ID：" + g.id + "    库存：" + QString::number(g.stock));
            meta->setObjectName("ProductMeta");
            info->addWidget(name);
            meta->setText(QString("分类：%1    商品ID：%2    库存：%3").arg(displayCategory, g.id).arg(g.stock));
            info->addWidget(meta);
            info->addStretch();

            auto *price = new QLabel("¥" + QString::number(g.price, 'f', 2));
            price->setObjectName("ProductPrice");
            price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            auto *select = makePrimaryButton("选择");
            select->setText(selected ? "已选择" : "选择");
            connect(select, &QPushButton::clicked, this, [=]() {
                if (m_selectedCartRows.contains(row)) {
                    m_selectedCartRows.remove(row);
                } else {
                    m_selectedCartRows.insert(row);
                }
                updateCartTable();
            });

            rowLayout->addWidget(thumb);
            rowLayout->addLayout(info, 1);
            rowLayout->addWidget(price);
            rowLayout->addWidget(select);
            m_cartCardsLayout->addWidget(card);
        }
        if (m_cart.isEmpty()) {
            auto *empty = new QLabel("购物车暂无商品");
            empty->setObjectName("Subtitle");
            empty->setAlignment(Qt::AlignCenter);
            m_cartCardsLayout->addWidget(empty);
        }
        m_cartCardsLayout->addStretch();
    }
    if (m_cartTotalLabel) m_cartTotalLabel->setText(QString("订单汇总：¥%1").arg(total, 0, 'f', 2));
    if (m_cartItemCountLabel) m_cartItemCountLabel->setText("商品数量：" + QString::number(m_selectedCartRows.size()));
    if (m_cartSubtotalLabel) m_cartSubtotalLabel->setText("商品总额：¥" + QString::number(total, 'f', 2));
    if (m_cartPayableLabel) m_cartPayableLabel->setText("应付合计：¥" + QString::number(total, 'f', 2));
}

void UserHomeWindow::updateHistoryTable()
{
    if (!m_historyTable) return;
    const auto &records = AccountStore::purchaseRecords();
    m_historyTable->setRowCount(0);
    for (const auto &r : records) {
        const CategoryDisplayItem displayProduct = scannedProductForDisplay(r.name, r.category);
        const QString displayName = displayProduct.name.isEmpty() ? r.name : displayProduct.name;
        const QString displayCategory = displayProduct.category.isEmpty() ? r.category : displayProduct.category;
        const int row = m_historyTable->rowCount();
        m_historyTable->insertRow(row);
        m_historyTable->setItem(row, 0, item(displayName));
        m_historyTable->setItem(row, 1, item(displayCategory));
        m_historyTable->setItem(row, 2, item(QString::number(r.price, 'f', 2)));
        m_historyTable->setItem(row, 3, item(QString::number(r.quantity)));
        m_historyTable->setItem(row, 4, item(r.status));
        m_historyTable->setItem(row, 5, item(r.time));
    }
    updateHistoryStats();
    if (m_historyListLayout) {
        clearLayout(m_historyListLayout);
        for (const auto &r : records) {
            const CategoryDisplayItem displayProduct = scannedProductForDisplay(r.name, r.category);
            const QString displayName = displayProduct.name.isEmpty() ? r.name : displayProduct.name;
            const QString displayCategory = displayProduct.category.isEmpty() ? r.category : displayProduct.category;
            auto *card = new QFrame;
            card->setObjectName("OrderCard");
            auto *rowLayout = new QHBoxLayout(card);
            rowLayout->setContentsMargins(18, 16, 18, 16);
            rowLayout->setSpacing(16);

            auto *thumb = scannedProductImageBox(displayName, displayCategory, QSize(118, 88), "ProductImage");
            auto *info = new QVBoxLayout;
            auto *name = new QLabel(displayName);
            name->setObjectName("ProductName");
            auto *meta = new QLabel("分类：" + r.category + "    数量：" + QString::number(r.quantity) + "    时间：" + r.time.left(16));
            meta->setObjectName("ProductMeta");
            auto *status = new QLabel("状态：" + r.status);
            status->setObjectName("ProductMeta");
            meta->setText(QString("分类：%1    数量：%2    时间：%3").arg(displayCategory).arg(r.quantity).arg(r.time.left(16)));
            info->addWidget(name);
            info->addWidget(meta);
            info->addWidget(status);

            auto *price = new QLabel("¥" + QString::number(r.price * r.quantity, 'f', 2));
            price->setObjectName("ProductPrice");
            price->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            rowLayout->addWidget(thumb);
            rowLayout->addLayout(info, 1);
            rowLayout->addWidget(price);
            m_historyListLayout->addWidget(card);
        }
        if (records.isEmpty()) {
            auto *empty = new QLabel("暂无购买记录");
            empty->setObjectName("Subtitle");
            empty->setAlignment(Qt::AlignCenter);
            m_historyListLayout->addWidget(empty);
        }
        m_historyListLayout->addStretch();
    }
}

void UserHomeWindow::updateHistoryStats()
{
    const auto &records = AccountStore::purchaseRecords();
    double amount = 0;
    int pending = 0, done = 0;
    for (const auto &r : records) {
        amount += r.price * r.quantity;
        if (r.status.contains("完成")) done++; else pending++;
    }
    if (m_historyOrderCountLabel) m_historyOrderCountLabel->setText(QString::number(records.size()));
    if (m_historyAmountLabel) m_historyAmountLabel->setText(QString("￥%1").arg(amount, 0, 'f', 2));
    if (m_historyPendingLabel) m_historyPendingLabel->setText(QString::number(pending));
    if (m_historyDoneLabel) m_historyDoneLabel->setText(QString::number(done));
}

void UserHomeWindow::searchGoods()
{
    recommendGoods();
}

void UserHomeWindow::recommendGoods()
{
    QVector<CategoryDisplayItem> result;
    const QString keyword = m_searchEdit ? m_searchEdit->text().trimmed() : QString();
    if (keyword.isEmpty()) {
        populateHomePreviewGrid(m_homeCategoryGrid, m_homeCategoryViewport);
        return;
    }

    for (const auto &item : scannedCategoryItems()) {
        if (item.name.contains(keyword, Qt::CaseInsensitive)) {
            result.push_back(item);
        }
    }
    populateHomeRecommendationGrid(m_homeCategoryGrid, m_homeCategoryViewport, result);
}

int UserHomeWindow::selectedGoodsRow() const { return m_goodsTable ? m_goodsTable->currentRow() : -1; }
int UserHomeWindow::selectedCartRow() const { return m_cartTable ? m_cartTable->currentRow() : -1; }

void UserHomeWindow::addToCart()
{
    const int row = selectedGoodsRow();
    if (row < 0 || row >= m_currentItems.size()) { QMessageBox::information(this, "提示", "请先选择商品。 "); return; }
    m_cart.push_back(m_currentItems[row]);
    updateCartTable();
    QMessageBox::information(this, "成功", "已加入购物车。 ");
}

void UserHomeWindow::buyGoods()
{
    const int row = selectedGoodsRow();
    if (row < 0 || row >= m_currentItems.size()) { QMessageBox::information(this, "提示", "请先选择商品。 "); return; }
    const auto g = m_currentItems[row];
    QString err;
    if (!AccountStore::purchaseGoodsById(g.id, m_username, 1, g.name, g.category, &err)) {
        QMessageBox::warning(this, "购买失败", err.isEmpty() ? "购买失败，请检查数据库连接或库存。" : err);
        return;
    }
    QMessageBox::information(this, "购买成功", "订单已生成，可在购买记录中查看。 ");
    loadGoodsData();
    updateGoodsTable(filteredGoods());
    updateHistoryTable();
}

void UserHomeWindow::buySelectedCartItem()
{
    if (m_selectedCartRows.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择商品");
        return;
    }

    for (int row = m_cart.size() - 1; row >= 0; --row) {
        if (!m_selectedCartRows.contains(row)) continue;

        const auto g = m_cart[row];
        const CategoryDisplayItem displayProduct = scannedProductForDisplay(g.name, g.category);
        const QString displayName = displayProduct.name.isEmpty() ? g.name : displayProduct.name;
        const QString displayCategory = displayProduct.category.isEmpty() ? g.category : displayProduct.category;

        if (g.id.startsWith("UC")) {
            AccountStore::addPurchaseRecord(displayName, displayCategory, g.price, 1, "未完成");
        } else {
            QString err;
            if (!AccountStore::purchaseGoodsById(g.id, m_username, 1, displayName, displayCategory, &err)) {
                QMessageBox::warning(this, "结算失败", err.isEmpty() ? "结算失败，请检查数据库连接或库存。" : err);
                return;
            }
        }
        m_cart.removeAt(row);
    }

    m_selectedCartRows.clear();
    QMessageBox::information(this, "结算成功", "订单已生成，可在购买记录中查看。");
    loadGoodsData();
    updateCartTable();
    updateHistoryTable();
    return;

    const int row = selectedCartRow();
    if (row < 0 || row >= m_cart.size()) { QMessageBox::information(this, "提示", "请先选择购物车商品。 "); return; }
    const auto g = m_cart[row];
    QString err;
    if (!AccountStore::purchaseGoodsById(g.id, m_username, 1, g.name, g.category, &err)) {
        QMessageBox::warning(this, "结算失败", err.isEmpty() ? "结算失败，请检查数据库连接或库存。" : err);
        return;
    }
    m_cart.removeAt(row);
    QMessageBox::information(this, "结算成功", "订单已生成，可在购买记录中查看。 ");
    loadGoodsData();
    updateCartTable();
    updateHistoryTable();
}

void UserHomeWindow::removeSelectedCartItem()
{
    if (!m_selectedCartRows.isEmpty()) {
        for (int row = m_cart.size() - 1; row >= 0; --row) {
            if (m_selectedCartRows.contains(row)) m_cart.removeAt(row);
        }
        m_selectedCartRows.clear();
        updateCartTable();
        return;
    }

    const int row = selectedCartRow();
    if (row < 0 || row >= m_cart.size()) { QMessageBox::information(this, "提示", "请先选择要移除的商品。 "); return; }
    m_cart.removeAt(row);
    updateCartTable();
}

void UserHomeWindow::submitGoodsForSale()
{
    const QString name = m_sellNameEdit->text().trimmed();
    const QString category = m_sellCategoryCombo->currentText();
    const double price = m_sellPriceEdit->text().toDouble();
    const int stock = m_sellStockSpin->value();
    const QString condition = m_sellConditionEdit->text().trimmed().isEmpty() ? "八成新" : m_sellConditionEdit->text().trimmed();
    const QString desc = m_sellDescriptionEdit->toPlainText();
    if (name.isEmpty() || price <= 0) {
        QMessageBox::warning(this, "提示", "请填写商品名称和正确价格。 ");
        return;
    }
    QString submitError;
    if (!AccountStore::submitGoodsForReview(name, category, price, stock, m_username, desc, condition, &submitError)) {
        QMessageBox::warning(this,
                             "提交失败",
                             submitError.trimmed().isEmpty()
                                 ? "提交失败，未返回具体错误。"
                                 : "提交失败，原因：\n" + submitError);
        return;
    }
    m_sellNameEdit->clear();
    m_sellPriceEdit->clear();
    m_sellStockSpin->setValue(1);
    m_sellConditionEdit->clear();
    m_sellDescriptionEdit->clear();
    QMessageBox::information(this, "提交成功", "商品已提交审核，管理员通过后即可上架。 ");
    switchPage(m_homePage, m_btnHome);
}

void UserHomeWindow::clearHistory()
{
    if (QMessageBox::question(this, "确认", "确定清空购买记录吗？") == QMessageBox::Yes) {
        AccountStore::clearPurchaseRecords();
        updateHistoryTable();
    }
}
