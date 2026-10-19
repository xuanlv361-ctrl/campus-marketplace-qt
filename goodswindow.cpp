#include "goodswindow.h"
#include "accountstore.h"
#include "languagemanager.h"
#include "mainwindow.h"
#include "productimagemanager.h"
#include "welcomewindow.h"

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

const char *kMarketTableQss = R"(
QWidget { background: #F8F9FB; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#Sidebar, QFrame#Panel, QFrame#MetricCard, QFrame#RankCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 22px; }
QLabel { background: transparent; }
QLabel#Brand { color: #111111; font-size: 22px; font-weight: 900; }
QLabel#SideHint, QLabel#Desc, QLabel#MetricDesc, QLabel#HeaderStats, QLabel#RankMeta { color: #666666; font-size: 13px; }
QLabel#Title { color: #111111; font-size: 34px; font-weight: 900; }
QLabel#SectionTitle { color: #111111; font-size: 20px; font-weight: 900; }
QLabel#MetricIcon { color: #5B4BFF; font-size: 22px; }
QLabel#MetricValue { color: #5B4BFF; font-size: 30px; font-weight: 900; }
QLabel#MetricTitle, QLabel#RankName { color: #111111; font-size: 15px; font-weight: 900; }
QPushButton { min-height: 42px; border-radius: 16px; font-size: 14px; font-weight: 900; padding: 0 16px; }
QPushButton#PrimaryButton { background: #5B4BFF; color: #FFFFFF; border: none; }
QPushButton#PrimaryButton:hover { background: #493CE6; }
QPushButton#DangerButton { background: #FFF1F1; color: #EF4444; border: 1px solid #FFD1D1; }
QPushButton#DangerButton:hover { background: #FFE8E8; }
QPushButton#SecondaryButton, QPushButton#SidebarButton { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; }
QPushButton#SecondaryButton:hover, QPushButton#SidebarButton:hover { background: #F1F0FF; border-color: #5B4BFF; }
QPushButton#SidebarButton, QPushButton#SidebarPrimary, QPushButton#ExitButton { text-align: left; }
QPushButton#SidebarPrimary { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; }
QPushButton#ExitButton { background: #FFF1F1; color: #EF4444; border: 1px solid #FFD1D1; text-align: left; }
QPushButton#TextButton { background: transparent; color: #5B4BFF; border: none; min-height: 28px; padding: 0 6px; }
QLineEdit, QComboBox { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 16px; color: #111111; min-height: 42px; padding: 0 14px; }
QLineEdit:focus, QComboBox:focus { border-color: #5B4BFF; }
QTableWidget { background: #FFFFFF; border: none; gridline-color: #EAEAEA; alternate-background-color: #FAFAFA; selection-background-color: #F1F0FF; selection-color: #111111; color: #222222; }
QHeaderView::section { background: #F7F7F9; color: #111111; border: none; border-right: 1px solid #EAEAEA; padding: 12px 8px; font-weight: 900; }
QLabel#StatusOk { background: #DCFCE7; color: #15803D; border-radius: 11px; padding: 4px 10px; font-weight: 900; }
QLabel#StatusWarn { background: #FEF3C7; color: #B45309; border-radius: 11px; padding: 4px 10px; font-weight: 900; }
QLabel#StatusBad { background: #FEE2E2; color: #DC2626; border-radius: 11px; padding: 4px 10px; font-weight: 900; }
QLabel#StockOk { color: #16A34A; font-weight: 900; }
QLabel#StockWarn { color: #F59E0B; font-weight: 900; }
QLabel#StockBad { color: #EF4444; font-weight: 900; }
QLabel#Thumb { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px; }
QScrollArea { background: transparent; border: none; }
)";

void addShadow(QWidget *widget, const QColor &color = QColor(20, 20, 20, 10))
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 8);
    shadow->setColor(color);
    widget->setGraphicsEffect(shadow);
}

void styleButton(QPushButton *button, const char *name)
{
    button->setObjectName(name);
    button->setMinimumWidth(96);
    button->setCursor(Qt::PointingHandCursor);
}

QStringList productCategories()
{
    return {"书籍教材", "学习资料", "电子数码", "宿舍用品", "服饰鞋包", "运动器材", "代步工具", "美妆零食"};
}

QStringList productConditions()
{
    return {"全新", "几乎全新", "良好", "一般", "较旧"};
}

QString imagePathFromDescription(const QString &description)
{
    const QString marker = "图片路径：";
    for (const QString &line : description.split('\n')) {
        if (line.startsWith(marker)) return line.mid(marker.size()).trimmed();
    }
    return QString();
}

QString descriptionWithoutImagePath(const QString &description)
{
    QStringList lines;
    const QString marker = "图片路径：";
    for (const QString &line : description.split('\n')) {
        if (!line.startsWith(marker)) lines << line;
    }
    return lines.join('\n').trimmed();
}

bool editProductDialog(QWidget *parent, const QString &title, const MarketplaceGoods &initial, MarketplaceGoods *result)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.resize(520, 560);

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);

    auto *nameEdit = new QLineEdit(initial.name);
    auto *categoryBox = new QComboBox;
    categoryBox->addItems(productCategories());
    categoryBox->setEditable(false);
    const int categoryIndex = productCategories().indexOf(initial.category);
    if (categoryIndex >= 0) categoryBox->setCurrentIndex(categoryIndex);

    auto *priceSpin = new QDoubleSpinBox;
    priceSpin->setRange(0, 999999);
    priceSpin->setDecimals(2);
    priceSpin->setValue(initial.price);
    priceSpin->setPrefix("¥");

    auto *stockSpin = new QSpinBox;
    stockSpin->setRange(0, 99999);
    stockSpin->setValue(initial.stock);

    auto *conditionBox = new QComboBox;
    conditionBox->addItems(productConditions());
    conditionBox->setEditable(false);
    const int conditionIndex = productConditions().indexOf(initial.condition);
    conditionBox->setCurrentIndex(conditionIndex >= 0 ? conditionIndex : productConditions().indexOf("良好"));

    auto *sellerEdit = new QLineEdit(initial.seller);
    auto *imagePathEdit = new QLineEdit(initial.imagePath.trimmed().isEmpty() ? imagePathFromDescription(initial.description) : initial.imagePath);
    auto *imageButton = new QPushButton("选择图片");
    styleButton(imageButton, "SecondaryButton");
    auto *imageRow = new QHBoxLayout;
    imageRow->setContentsMargins(0, 0, 0, 0);
    imageRow->setSpacing(8);
    imageRow->addWidget(imagePathEdit, 1);
    imageRow->addWidget(imageButton);

    auto *descEdit = new QTextEdit(descriptionWithoutImagePath(initial.description));
    descEdit->setMinimumHeight(110);

    form->addRow("商品名称", nameEdit);
    form->addRow("商品类别", categoryBox);
    form->addRow("商品价格", priceSpin);
    form->addRow("库存数量", stockSpin);
    form->addRow("商品成色", conditionBox);
    form->addRow("卖家", sellerEdit);
    form->addRow("商品图片路径", imageRow);
    form->addRow("商品描述", descEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("确认");
    buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "商品名称不能为空。");
            return;
        }
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(imageButton, &QPushButton::clicked, &dialog, [&]() {
        const QString path = QFileDialog::getOpenFileName(&dialog,
                                                          "选择图片",
                                                          imagePathEdit->text().trimmed(),
                                                          "Images (*.jpg *.jpeg *.png *.webp);;All Files (*.*)");
        if (!path.isEmpty()) imagePathEdit->setText(QDir::toNativeSeparators(path));
    });

    layout->addLayout(form);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) return false;

    MarketplaceGoods edited = initial;
    edited.name = nameEdit->text().trimmed();
    edited.category = LanguageManager::instance().comboValue(categoryBox);
    edited.price = priceSpin->value();
    edited.stock = stockSpin->value();
    edited.seller = sellerEdit->text().trimmed();
    edited.condition = LanguageManager::instance().comboValue(conditionBox);
    edited.description = descEdit->toPlainText().trimmed();
    const QString imagePath = imagePathEdit->text().trimmed();
    edited.imagePath = imagePath;
    if (!imagePath.isEmpty()) {
        if (!edited.description.isEmpty()) edited.description += "\n";
        edited.description += "图片路径：" + imagePath;
    }
    *result = edited;
    return true;
}

void showGoodsDetailDialog(QWidget *parent, const MarketplaceGoods &goods)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("商品详情");
    dialog.resize(620, 620);
    dialog.setStyleSheet(R"(
QDialog { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#DetailCard, QFrame#DescriptionCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 18px; }
QLabel { background: transparent; }
QLabel#DialogTitle { color: #111111; font-size: 28px; font-weight: 900; }
QLabel#FieldName { color: #666666; font-size: 14px; font-weight: 800; }
QLabel#FieldValue { color: #111111; font-size: 15px; font-weight: 800; }
QLabel#StatusOk { background: #DCFCE7; color: #15803D; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#StatusWarn { background: #FEF3C7; color: #B45309; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#StatusOff { background: #F3F4F6; color: #6B7280; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#StatusBad { background: #FEE2E2; color: #DC2626; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#StockWarnTag { background: #FFF7ED; color: #EA580C; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#SectionTitle { color: #111111; font-size: 16px; font-weight: 900; }
QLabel#DescriptionText { color: #333333; font-size: 14px; line-height: 150%; }
QPushButton#PrimaryButton { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; border-radius: 16px; min-height: 42px; padding: 0 30px; font-weight: 900; }
QPushButton#PrimaryButton:hover { background: #493CE6; }
)");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(14);

    auto *card = new QFrame;
    card->setObjectName("DetailCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(14);

    auto *title = new QLabel("商品详情");
    title->setObjectName("DialogTitle");
    layout->addWidget(title);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(12);

    auto addTextRow = [&](int row, const QString &name, const QString &value) {
        auto *nameLabel = new QLabel(name);
        nameLabel->setObjectName("FieldName");
        auto *valueLabel = new QLabel(value.trimmed().isEmpty() ? "未填写" : value);
        valueLabel->setObjectName("FieldValue");
        valueLabel->setWordWrap(true);
        grid->addWidget(nameLabel, row, 0, Qt::AlignTop);
        grid->addWidget(valueLabel, row, 1);
    };
    auto addTagRow = [&](int row, const QString &name, const QString &value, const char *objectName) {
        auto *nameLabel = new QLabel(name);
        nameLabel->setObjectName("FieldName");
        auto *valueLabel = new QLabel(value.trimmed().isEmpty() ? "未填写" : value);
        valueLabel->setAlignment(Qt::AlignCenter);
        valueLabel->setObjectName(objectName);
        grid->addWidget(nameLabel, row, 0, Qt::AlignTop);
        grid->addWidget(valueLabel, row, 1, Qt::AlignLeft);
    };

    const char *statusObject = goods.status == "已上架" ? "StatusOk"
                               : goods.status == "待审核" ? "StatusWarn"
                               : goods.status == "已下架" ? "StatusOff"
                               : "StatusBad";
    const QString stockText = goods.stock < 10
                              ? QString::number(goods.stock) + "（库存预警）"
                              : QString::number(goods.stock);

    addTextRow(0, "商品ID", goods.id);
    addTextRow(1, "商品名称", goods.name);
    addTextRow(2, "商品分类", goods.category);
    addTextRow(3, "商品价格", "¥" + QString::number(goods.price, 'f', 2));
    addTagRow(4, "库存数量", stockText, goods.stock < 10 ? "StockWarnTag" : "FieldValue");
    addTextRow(5, "销量", QString::number(goods.sales));
    addTextRow(6, "卖家", goods.seller);
    addTextRow(7, "商品成色", goods.condition);
    addTagRow(8, "商品状态", goods.status, statusObject);
    addTextRow(9, "图片路径", goods.imagePath);
    addTextRow(10, "创建时间", goods.createdAt);
    grid->setColumnStretch(1, 1);
    layout->addLayout(grid);
    root->addWidget(card);

    auto *descCard = new QFrame;
    descCard->setObjectName("DescriptionCard");
    auto *descLayout = new QVBoxLayout(descCard);
    descLayout->setContentsMargins(18, 14, 18, 14);
    auto *descTitle = new QLabel("商品描述");
    descTitle->setObjectName("SectionTitle");
    auto *desc = new QLabel(descriptionWithoutImagePath(goods.description).isEmpty()
                            ? "暂无描述"
                            : descriptionWithoutImagePath(goods.description));
    desc->setObjectName("DescriptionText");
    desc->setWordWrap(true);
    descLayout->addWidget(descTitle);
    descLayout->addWidget(desc);
    root->addWidget(descCard);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->addStretch();
    auto *okButton = new QPushButton("确定");
    okButton->setObjectName("PrimaryButton");
    okButton->setCursor(Qt::PointingHandCursor);
    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttonRow->addWidget(okButton);
    root->addLayout(buttonRow);

    dialog.exec();
}

QPushButton *sidebarButton(const QString &text, const char *name = "SidebarButton")
{
    auto *button = new QPushButton(text);
    button->setObjectName(name);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QFrame *createSidebar(const QString &active, MainWindow *mainWindow, QWidget *self)
{
    auto *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(250);
    addShadow(sidebar);
    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(22, 24, 22, 22);
    layout->setSpacing(10);
    auto *brand = new QLabel("管理员后台");
    brand->setObjectName("Brand");
    auto *hint = new QLabel("校园二手交易平台");
    hint->setObjectName("SideHint");
    layout->addWidget(brand);
    layout->addWidget(hint);
    layout->addSpacing(12);
    const QVector<QPair<QString, QString>> items = {
        {"🏠", "管理员中心"}, {"📦", "商品管理"}, {"🧾", "订单管理"}, {"👥", "用户管理"}, {"📊", "数据统计"}
    };
    int navIndex = 0;
    for (const auto &item : items) {
        const int index = navIndex++;
        auto *btn = sidebarButton(item.first + "  " + item.second, item.second == active ? "SidebarPrimary" : "SidebarButton");
        if (index == 0) {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->showDashboard(); self->hide(); });
        } else if (index == 2) {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openOrdersWindow(); self->hide(); });
        } else if (index == 3) {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openUsersWindow(); self->hide(); });
        } else if (index == 4) {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openStatsWindow(); self->hide(); });
        }
        layout->addWidget(btn);
    }
    layout->addStretch();
    layout->addWidget(LanguageManager::instance().createSelector(sidebar));
    auto *home = sidebarButton("↩  返回首页");
    QObject::connect(home, &QPushButton::clicked, self, [=]() { auto *w = new WelcomeWindow(); w->show(); self->hide(); });
    auto *exit = sidebarButton("🚪  退出系统", "ExitButton");
    QObject::connect(exit, &QPushButton::clicked, self, [=]() {
        if (mainWindow) {
            mainWindow->backToWelcome();
        } else {
            auto *welcome = new WelcomeWindow;
            welcome->setAttribute(Qt::WA_DeleteOnClose);
            welcome->show();
        }
        self->hide();
    });
    layout->addWidget(home);
    layout->addWidget(exit);
    return sidebar;
}

QLabel *statusBadge(const QString &status)
{
    auto *label = new QLabel(status);
    label->setAlignment(Qt::AlignCenter);
    label->setObjectName(status == "已上架" ? "StatusOk" : (status == "待审核" ? "StatusWarn" : "StatusBad"));
    return label;
}

QLabel *stockBadge(const QString &stockText)
{
    const int stock = stockText.toInt();
    auto *label = new QLabel((stock < 10 ? (stock < 5 ? "● " : "● ") : "● ") + stockText);
    label->setAlignment(Qt::AlignCenter);
    label->setObjectName(stock < 5 ? "StockBad" : (stock < 10 ? "StockWarn" : "StockOk"));
    return label;
}

QString niceName(const GoodsItem &item)
{
    if (!item.name.contains("商品")) return item.name;
    const int id = item.id.toInt();
    if (item.category.contains("电子")) {
        const QStringList names = {"MacBook Air M1", "iPad Air", "Sony 降噪耳机", "佳能EOS相机", "RK机械键盘", "罗技无线鼠标"};
        return names.value(id % names.size());
    }
    if (item.category.contains("书") || item.category.contains("教材")) {
        const QStringList names = {"高等数学教材", "雅思真题资料", "C++ 程序设计", "考研英语资料"};
        return names.value(id % names.size());
    }
    if (item.category.contains("生活") || item.category.contains("宿舍")) {
        const QStringList names = {"宿舍小风扇", "护眼台灯", "保温水杯", "桌面加湿器"};
        return names.value(id % names.size());
    }
    if (item.category.contains("服饰") || item.category.contains("包")) {
        const QStringList names = {"通勤双肩包", "帆布包", "九成新运动鞋", "连帽卫衣", "秋季外套", "简约饰品"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (item.category.contains("运动")) {
        const QStringList names = {"校园自行车", "篮球", "羽毛球拍", "可调哑铃", "瑜伽垫", "滑板"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (item.category.contains("代步")) {
        const QStringList names = {"校园自行车", "校园电动车", "电动滑板车", "智能平衡车"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (item.category.contains("美妆") || item.category.contains("个护")) {
        const QStringList names = {"保湿面霜", "巧克力礼盒", "口红", "面膜", "香水", "护手霜", "洗面奶", "坚果礼盒", "饼干礼盒", "咖啡礼盒"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (item.category.contains("食品") || item.category.contains("零食")) {
        const QStringList names = {"巧克力礼盒", "坚果礼盒", "饼干礼盒", "咖啡礼盒"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    return item.name;
}

QString imagePathFor(const GoodsItem &item)
{
    QString preferredPath;
    for (const MarketplaceGoods &goods : AccountStore::goods()) {
        if (goods.id == item.id) {
            preferredPath = goods.imagePath;
            break;
        }
    }
    return ProductImageManager::imagePathForProduct(niceName(item), item.category, preferredPath);
}

QLabel *thumbLabel(const GoodsItem &item)
{
    auto *label = new QLabel;
    label->setObjectName("Thumb");
    label->setFixedSize(62, 62);
    label->setAlignment(Qt::AlignCenter);
    QPixmap pix(imagePathFor(item));
    if (!pix.isNull()) label->setPixmap(pix.scaled(54, 54, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    return label;
}

QFrame *metricCard(const QString &icon, QLabel *valueLabel, const QString &title, const QString &desc)
{
    auto *card = new QFrame;
    card->setObjectName("MetricCard");
    card->setMinimumHeight(118);
    addShadow(card);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(2);
    auto *top = new QHBoxLayout;
    auto *iconLabel = new QLabel(icon);
    iconLabel->setObjectName("MetricIcon");
    valueLabel->setObjectName("MetricValue");
    valueLabel->setAlignment(Qt::AlignRight);
    top->addWidget(iconLabel);
    top->addStretch();
    top->addWidget(valueLabel);
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("MetricTitle");
    auto *descLabel = new QLabel(desc);
    descLabel->setObjectName("MetricDesc");
    layout->addLayout(top);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    return card;
}

void clearLayout(QLayout *layout)
{
    QLayoutItem *item = nullptr;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        if (item->layout()) clearLayout(item->layout());
        delete item;
    }
}

} // namespace

GoodsWindow::GoodsWindow(MainWindow *mainWindow, QWidget *parent, bool showPendingOnOpen)
    : QWidget(parent), m_mainWindow(mainWindow)
{
    resize(1320, 820);
    setMinimumSize(1120, 720);
    setWindowTitle("校园二手交易平台 - 商品管理");
    setStyleSheet(kMarketTableQss);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(18);
    root->addWidget(createSidebar("商品管理", m_mainWindow, this));

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto *content = new QWidget;
    auto *mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(18);

    auto *titleRow = new QHBoxLayout;
    auto *titleBlock = new QVBoxLayout;
    auto *crumb = new QLabel("← 返回后台首页  /  商品管理");
    crumb->setObjectName("Desc");
    auto *title = new QLabel("商品管理");
    title->setObjectName("Title");
    auto *desc = new QLabel("管理平台商品信息、库存状态与销售情况");
    desc->setObjectName("Desc");
    titleBlock->addWidget(crumb);
    titleBlock->addWidget(title);
    titleBlock->addWidget(desc);
    labelHeaderStats = new QLabel;
    labelHeaderStats->setObjectName("HeaderStats");
    labelHeaderStats->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    titleRow->addLayout(titleBlock, 1);
    titleRow->addWidget(labelHeaderStats);

    labelTotalMetric = new QLabel("0");
    labelOnlineMetric = new QLabel("0");
    labelPendingMetric = new QLabel("0");
    labelLowStockMetric = new QLabel("0");
    auto *metrics = new QGridLayout;
    metrics->setSpacing(14);
    metrics->addWidget(metricCard("📦", labelTotalMetric, "商品总数", "平台商品数量"), 0, 0);
    metrics->addWidget(metricCard("✅", labelOnlineMetric, "已上架商品", "正常销售中"), 0, 1);
    metrics->addWidget(metricCard("⏳", labelPendingMetric, "待审核商品", "等待管理员审核"), 0, 2);
    metrics->addWidget(metricCard("⚠", labelLowStockMetric, "库存预警商品", "库存不足10件"), 0, 3);

    auto *panel = new QFrame;
    panel->setObjectName("Panel");
    addShadow(panel);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(20, 20, 20, 20);
    panelLayout->setSpacing(14);

    auto *searchRow = new QHBoxLayout;
    searchRow->setSpacing(12);
    editSearch = new QLineEdit;
    editSearch->setPlaceholderText("搜索商品名称、ID、分类或状态...");
    comboCategory = new QComboBox;
    comboCategory->addItem("全部分类");
    comboCategory->addItems(productCategories());
    comboStatus = new QComboBox;
    comboStatus->addItems(QStringList() << "全部状态" << "已上架" << "待审核" << "已下架");
    btnSearch = new QPushButton("查询");
    btnReset = new QPushButton("重置");
    styleButton(btnSearch, "PrimaryButton");
    styleButton(btnReset, "SecondaryButton");
    searchRow->addWidget(editSearch, 1);
    searchRow->addWidget(comboCategory);
    searchRow->addWidget(comboStatus);
    searchRow->addWidget(btnSearch);
    searchRow->addWidget(btnReset);

    auto *actionBar = new QHBoxLayout;
    actionBar->setSpacing(12);
    btnAdd = new QPushButton("➕ 新增商品");
    btnAudit = new QPushButton("⏳ 待审核商品");
    btnEdit = new QPushButton("✏ 修改商品");
    btnSales = new QPushButton("📊 查看销量");
    btnExport = new QPushButton("📥 导出数据");
    btnDelete = new QPushButton("🗑 删除商品");
    btnRefresh = new QPushButton("刷新");
    btnLoad = new QPushButton("加载数据");
    btnBack = new QPushButton("返回管理员后台");
    styleButton(btnAdd, "PrimaryButton");
    styleButton(btnAudit, "SecondaryButton");
    styleButton(btnEdit, "SecondaryButton");
    styleButton(btnSales, "SecondaryButton");
    styleButton(btnExport, "SecondaryButton");
    styleButton(btnDelete, "DangerButton");
    styleButton(btnRefresh, "SecondaryButton");
    styleButton(btnLoad, "SecondaryButton");
    styleButton(btnBack, "SecondaryButton");
    actionBar->addWidget(btnAdd);
    actionBar->addWidget(btnAudit);
    actionBar->addWidget(btnEdit);
    actionBar->addWidget(btnSales);
    actionBar->addWidget(btnExport);
    actionBar->addWidget(btnRefresh);
    actionBar->addWidget(btnLoad);
    actionBar->addStretch();
    actionBar->addWidget(btnDelete);

    table = new QTableWidget(this);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels(QStringList() << "ID" << "商品名称" << "分类" << "价格" << "库存" << "销量" << "状态" << "操作");
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(78);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);

    panelLayout->addLayout(searchRow);
    panelLayout->addLayout(actionBar);
    panelLayout->addWidget(table, 1);

    auto *rankCard = new QFrame;
    rankCard->setObjectName("RankCard");
    addShadow(rankCard);
    auto *rankOuter = new QVBoxLayout(rankCard);
    rankOuter->setContentsMargins(20, 18, 20, 18);
    rankOuter->setSpacing(12);
    auto *rankTitle = new QLabel("商品销售排行榜");
    rankTitle->setObjectName("SectionTitle");
    rankLayout = new QVBoxLayout;
    rankLayout->setSpacing(8);
    rankOuter->addWidget(rankTitle);
    rankOuter->addLayout(rankLayout);

    mainLayout->addLayout(titleRow);
    mainLayout->addLayout(metrics);
    mainLayout->addWidget(panel, 1);
    mainLayout->addWidget(rankCard);
    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    loadSampleData();
    if (showPendingOnOpen) {
        LanguageManager::instance().setComboValue(comboStatus, "待审核");
        searchGoods();
    }

    connect(btnLoad, &QPushButton::clicked, this, [=]() { loadSampleData(); QMessageBox::information(this, "提示", "商品数据加载成功。"); });
    connect(btnSearch, &QPushButton::clicked, this, &GoodsWindow::searchGoods);
    connect(btnReset, &QPushButton::clicked, this, &GoodsWindow::refreshGoods);
    connect(btnRefresh, &QPushButton::clicked, this, &GoodsWindow::refreshGoods);
    connect(btnAudit, &QPushButton::clicked, this, &GoodsWindow::showPendingGoods);
    connect(btnAdd, &QPushButton::clicked, this, &GoodsWindow::addGoods);
    connect(btnEdit, &QPushButton::clicked, this, [this]() { editGoods(); });
    connect(btnDelete, &QPushButton::clicked, this, &GoodsWindow::deleteGoods);
    connect(btnSales, &QPushButton::clicked, this, &GoodsWindow::showGoodsSales);
    connect(btnExport, &QPushButton::clicked, this, [=]() { QMessageBox::information(this, "导出数据", "商品数据已准备导出。"); });
    connect(editSearch, &QLineEdit::returnPressed, this, &GoodsWindow::searchGoods);
    connect(crumb, &QLabel::linkActivated, this, [=]() { if (m_mainWindow) m_mainWindow->show(); hide(); });
    connect(btnBack, &QPushButton::clicked, this, [=]() { if (m_mainWindow) m_mainWindow->show(); hide(); });
}

GoodsWindow::~GoodsWindow() {}

void GoodsWindow::loadSampleData()
{
    m_goodsData.clear();
    for (const MarketplaceGoods &item : AccountStore::searchGoods(QString(), "全部分类", "全部状态")) {
        m_goodsData.push_back({item.id, item.name, item.category, QString::number(item.price, 'f', 2), QString::number(item.stock), QString::number(item.sales), item.status});
    }
    updateTable(m_goodsData);
    updateDashboardStats();
    updateSalesRanking();
}

void GoodsWindow::updateDashboardStats()
{
    int online = 0, pending = 0, offline = 0, low = 0;
    for (const GoodsItem &item : m_goodsData) {
        if (item.status == "已上架") ++online;
        else if (item.status == "待审核") ++pending;
        else ++offline;
        if (item.stock.toInt() < 10) ++low;
    }
    labelTotalMetric->setText(QString::number(m_goodsData.size()));
    labelOnlineMetric->setText(QString::number(online));
    labelPendingMetric->setText(QString::number(pending));
    labelLowStockMetric->setText(QString::number(low));
    labelHeaderStats->setText(QString("%1 件商品\n已上架 %2\n待审核 %3\n已下架 %4").arg(m_goodsData.size()).arg(online).arg(pending).arg(offline));
    btnAudit->setText(QString("⏳ 待审核商品（%1）").arg(pending));
}

void GoodsWindow::updateSalesRanking()
{
    clearLayout(rankLayout);
    QVector<GoodsItem> sorted = m_goodsData;
    std::sort(sorted.begin(), sorted.end(), [](const GoodsItem &a, const GoodsItem &b) {
        return a.sales.toInt() > b.sales.toInt();
    });
    auto *row = new QHBoxLayout;
    row->setSpacing(12);
    const int count = qMin(5, sorted.size());
    for (int i = 0; i < count; ++i) {
        auto *card = new QFrame;
        card->setObjectName("MetricCard");
        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(14, 12, 14, 12);
        auto *top = new QLabel(QString("TOP%1").arg(i + 1));
        top->setObjectName("MetricIcon");
        auto *name = new QLabel(niceName(sorted[i]));
        name->setObjectName("RankName");
        name->setWordWrap(true);
        auto *sales = new QLabel("销量 " + sorted[i].sales);
        sales->setObjectName("RankMeta");
        layout->addWidget(top);
        layout->addWidget(name);
        layout->addWidget(sales);
        row->addWidget(card);
    }
    rankLayout->addLayout(row);
}

void GoodsWindow::updateTable(const QVector<GoodsItem> &items)
{
    table->setUpdatesEnabled(false);
    table->clearContents();
    table->setRowCount(0);
    table->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        const GoodsItem item = items[i];
        table->setItem(i, 0, new QTableWidgetItem(item.id));
        table->setItem(i, 1, new QTableWidgetItem(niceName(item)));
        table->setItem(i, 2, new QTableWidgetItem(item.category));
        table->setItem(i, 3, new QTableWidgetItem("¥" + item.price));
        table->setCellWidget(i, 4, stockBadge(item.stock));
        table->setItem(i, 5, new QTableWidgetItem(item.sales));
        table->setCellWidget(i, 6, statusBadge(item.status));
        const QString goodsId = item.id;
        auto *ops = new QWidget;
        auto *opsLayout = new QHBoxLayout(ops);
        opsLayout->setContentsMargins(0, 0, 0, 0);
        opsLayout->setSpacing(4);
        auto *view = new QPushButton("查看");
        auto *edit = new QPushButton("编辑");
        auto *del = new QPushButton("删除");
        styleButton(view, "TextButton");
        styleButton(edit, "TextButton");
        styleButton(del, "TextButton");
        connect(view, &QPushButton::clicked, this, [this, goodsId]() { showGoodsDetail(goodsId); });
        connect(edit, &QPushButton::clicked, this, [this, goodsId]() { editGoods(goodsId); });
        connect(del, &QPushButton::clicked, this, [=]() { table->selectRow(i); deleteGoods(); });
        opsLayout->addWidget(view);
        if (item.status == "待审核") {
            auto *approve = new QPushButton("通过审核");
            styleButton(approve, "TextButton");
            connect(approve, &QPushButton::clicked, this, [=]() {
                if (AccountStore::updateGoodsStatus(item.id, "已上架")) {
                    QMessageBox::information(this, "审核通过", niceName(item) + " 已通过审核并上架。");
                    loadSampleData();
                    LanguageManager::instance().setComboValue(comboStatus, "已上架");
                    searchGoods();
                } else {
                    QMessageBox::warning(this, "审核失败", "商品状态更新失败，请确认数据库连接正常。");
                }
            });
            opsLayout->addWidget(approve);
        }
        opsLayout->addWidget(edit);
        opsLayout->addWidget(del);
        table->setCellWidget(i, 7, ops);
        for (int c = 0; c < table->columnCount(); ++c) {
            if (table->item(i, c)) table->item(i, c)->setTextAlignment(Qt::AlignCenter);
        }
    }
    table->setUpdatesEnabled(true);
}

void GoodsWindow::searchGoods()
{
    const QString keyword = editSearch->text().trimmed();
    const QString category = LanguageManager::instance().comboValue(comboCategory);
    const QString status = LanguageManager::instance().comboValue(comboStatus);
    QVector<GoodsItem> result;
    for (const MarketplaceGoods &item : AccountStore::searchGoods(keyword, category, status)) {
        result.push_back({item.id, item.name, item.category, QString::number(item.price, 'f', 2), QString::number(item.stock), QString::number(item.sales), item.status});
    }
    updateTable(result);
    if (result.isEmpty()) QMessageBox::information(this, "查询结果", "未找到符合条件的商品。");
}

void GoodsWindow::refreshGoods()
{
    editSearch->clear();
    comboCategory->setCurrentIndex(0);
    comboStatus->setCurrentIndex(0);
    loadSampleData();
}

void GoodsWindow::showPendingGoods()
{
    loadSampleData();
    LanguageManager::instance().setComboValue(comboStatus, "待审核");
    searchGoods();
}

int GoodsWindow::currentRowIndex() const { return table->currentRow(); }

void GoodsWindow::showGoodsDetail(const QString &goodsId)
{
    for (const MarketplaceGoods &item : AccountStore::searchGoods(QString(), "全部分类", "全部状态")) {
        if (item.id == goodsId) {
            showGoodsDetailDialog(this, item);
            return;
        }
    }
    QMessageBox::warning(this, "提示", "未找到该商品，请刷新后重试。");
}

void GoodsWindow::addGoods()
{
    MarketplaceGoods initial;
    initial.id = QString::number(AccountStore::goods().size() + 1);
    initial.category = productCategories().first();
    initial.price = 0;
    initial.stock = 1;
    initial.sales = 0;
    initial.seller = "管理员";
    initial.condition = "良好";
    initial.status = "已上架";

    MarketplaceGoods edited;
    if (!editProductDialog(this, "新增商品", initial, &edited)) return;

    edited.id = initial.id;
    edited.sales = 0;
    edited.status = "已上架";
    QString errorMessage;
    if (!AccountStore::addGoods(edited, &errorMessage)) {
        QMessageBox::warning(this, "新增失败",
                             errorMessage.trimmed().isEmpty()
                                 ? "商品新增失败，请确认数据库连接正常。"
                                 : "商品新增失败，原因：\n" + errorMessage);
        return;
    }
    LanguageManager::instance().setComboValue(comboStatus, "已上架");
    comboCategory->setCurrentIndex(0);
    editSearch->setText(edited.name);
    loadSampleData();
    searchGoods();
    QMessageBox::information(this, "提示", "商品新增成功。");
}

void GoodsWindow::editGoods()
{
    const int row = currentRowIndex();
    if (row < 0 || !table->item(row, 0)) { QMessageBox::warning(this, "提示", "请先选择要修改的商品。"); return; }
    editGoods(table->item(row, 0)->text());
}

void GoodsWindow::editGoods(const QString &goodsId)
{
    MarketplaceGoods storeItem;
    bool found = false;
    for (const MarketplaceGoods &item : AccountStore::searchGoods(QString(), "全部分类", "全部状态")) {
        if (item.id == goodsId) {
            storeItem = item;
            found = true;
            break;
        }
    }
    if (!found) { QMessageBox::warning(this, "提示", "未找到要修改的商品，请刷新后重试。"); return; }

    MarketplaceGoods edited;
    if (!editProductDialog(this, "修改商品", storeItem, &edited)) return;

    edited.id = storeItem.id;
    edited.sales = storeItem.sales;
    edited.status = storeItem.status;
    QString errorMessage;
    if (!AccountStore::updateGoods(edited, &errorMessage)) {
        QMessageBox::warning(this, "修改失败",
                             errorMessage.trimmed().isEmpty()
                                 ? "商品修改失败，请确认数据库连接正常。"
                                 : "商品修改失败，原因：\n" + errorMessage);
        return;
    }
    loadSampleData();
    searchGoods();
    QMessageBox::information(this, "提示", "商品修改成功。");
}

void GoodsWindow::deleteGoods()
{
    const int row = currentRowIndex();
    if (row < 0 || !table->item(row, 0)) { QMessageBox::warning(this, "提示", "请先选中要删除的商品。"); return; }
    if (QMessageBox::question(this, "确认删除", "确定删除当前选中的商品吗？") == QMessageBox::Yes) {
        const QString id = table->item(row, 0)->text();
        if (!AccountStore::updateGoodsStatus(id, "已下架")) {
            QMessageBox::warning(this, "删除失败", "商品状态更新失败，请确认数据库连接正常。");
            return;
        }
        loadSampleData();
        searchGoods();
    }
}

void GoodsWindow::showGoodsSales()
{
    const int row = currentRowIndex();
    if (row < 0 || !table->item(row, 1)) { QMessageBox::warning(this, "提示", "请先选中一个商品。"); return; }
    QMessageBox::information(this, "商品销量", table->item(row, 1)->text() + " 的销量为：" + table->item(row, 5)->text());
}

int GoodsWindow::goodsStoreIndexById(const QString &id) const
{
    QVector<MarketplaceGoods> &goods = AccountStore::goods();
    for (int i = 0; i < goods.size(); ++i) if (goods[i].id == id) return i;
    return -1;
}

int GoodsWindow::selectedGoodsStoreIndex() const
{
    const int row = currentRowIndex();
    if (row < 0 || !table->item(row, 0)) return -1;
    return goodsStoreIndexById(table->item(row, 0)->text());
}

void GoodsWindow::closeEvent(QCloseEvent *event)
{
    if (m_mainWindow) m_mainWindow->show();
    event->accept();
}
