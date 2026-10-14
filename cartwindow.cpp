#include "cartwindow.h"

#include "accountstore.h"
#include "productimagemanager.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

const char *kCartQss = R"(
QWidget { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#Sidebar { background: #FFFFFF; border: none; border-right: 1px solid #EAEAEA; }
QLabel#SidebarLogo { background: #5B4BFF; color: #FFFFFF; border-radius: 15px; font-size: 18px; font-weight: 900; }
QLabel#SidebarTitle { color: #111111; font-size: 16px; font-weight: 900; }
QLabel#UserHint { color: #666666; font-size: 13px; font-weight: 800; }
QPushButton#SideNav, QPushButton#SideNavActive {
    min-height: 40px; border-radius: 12px; padding: 0 12px; text-align: left; font-size: 13px; font-weight: 800;
}
QPushButton#SideNav { background: #FFFFFF; color: #333333; border: none; }
QPushButton#SideNav:hover { background: #F1F0FF; color: #111111; }
QPushButton#SideNavActive { background: #5B4BFF; color: #FFFFFF; border: none; }
QPushButton#SideExit { min-height: 40px; border-radius: 12px; padding: 0 12px; text-align: left; font-size: 13px; font-weight: 900; background: #FFF1F1; color: #EF4444; border: 1px solid #FFD1D1; }
QLabel#Title { color: #111111; font-size: 32px; font-weight: 900; background: transparent; }
QLabel#Desc, QLabel#Empty, QLabel#SummaryText, QLabel#ProductMeta { color: #666666; font-size: 14px; background: transparent; }
QFrame#CartCard, QFrame#SummaryCard, QFrame#EmptyCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 20px; }
QLabel#ProductImage { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 16px; }
QLabel#ProductName { color: #111111; font-size: 18px; font-weight: 900; background: transparent; }
QLabel#ProductPrice { color: #5B4BFF; font-size: 22px; font-weight: 900; background: transparent; }
QLabel#SummaryTitle { color: #111111; font-size: 22px; font-weight: 900; background: transparent; }
QLabel#SummaryValue { color: #111111; font-size: 15px; font-weight: 800; background: transparent; }
QLabel#Total { color: #5B4BFF; font-size: 26px; font-weight: 900; background: transparent; }
QPushButton { min-height: 42px; border-radius: 16px; font-size: 14px; font-weight: 900; padding: 0 18px; }
QPushButton#PrimaryButton { background: #5B4BFF; color: #FFFFFF; border: none; }
QPushButton#PrimaryButton:hover { background: #493CE6; }
QPushButton#DangerButton { background: #FFF1F1; color: #EF4444; border: 1px solid #FFD1D1; }
QPushButton#DangerButton:hover { background: #FFE8E8; }
QPushButton#SecondaryButton { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; }
QPushButton#SecondaryButton:hover { background: #F7F7FA; border-color: #5B4BFF; }
QCheckBox { background: transparent; spacing: 10px; font-weight: 800; color: #111111; }
QCheckBox::indicator { width: 22px; height: 22px; border-radius: 7px; border: 2px solid #D8D8E2; background: #FFFFFF; }
QCheckBox::indicator:checked { background: #5B4BFF; border: 2px solid #5B4BFF; }
QScrollArea { background: transparent; border: none; }
QScrollArea QWidget#CardsViewport { background: transparent; }
QTableWidget { background: #FFFFFF; border: none; }
)";

void addShadow(QWidget *widget)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(20, 20, 20, 12));
    widget->setGraphicsEffect(shadow);
}

QPushButton *sideNavButton(const QString &icon, const QString &text, const char *objectName = "SideNav")
{
    auto *button = new QPushButton(icon + "  " + text);
    button->setObjectName(objectName);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QString valueAfterColon(const QString &part)
{
    const int index = part.indexOf(QChar(0xFF1A));
    if (index >= 0) return part.mid(index + 1).trimmed();
    const int asciiIndex = part.indexOf(':');
    if (asciiIndex >= 0) return part.mid(asciiIndex + 1).trimmed();
    return part.trimmed();
}

double parsePrice(QString text)
{
    text.remove("¥");
    text.remove("￥");
    return text.trimmed().toDouble();
}

const MarketplaceGoods *goodsById(const QString &id)
{
    for (const MarketplaceGoods &goods : AccountStore::goods()) {
        if (goods.id == id) return &goods;
    }
    return nullptr;
}

int variedIndex(const QString &id, const QString &name, const QString &category, int size)
{
    if (size <= 0) return 0;
    const int numericId = id.toInt();
    const uint seed = numericId > 0 ? uint(numericId) : qHash(name + category);
    return int(((seed * 37u) + (seed / 10u) + 3u) % uint(size));
}

QString cartDisplayName(const QString &name, const QString &category, const QString &id)
{
    if (!name.contains("商品")) return name;

    if (category.contains("电子") || category.contains("数码")) {
        const QStringList names = {"笔记本电脑", "平板电脑", "蓝牙降噪耳机", "Sony降噪耳机", "佳能EOS相机", "RK机械键盘", "罗技无线鼠标", "宿舍显示器"};
        return names.value(variedIndex(id, name, category, names.size()));
    }
    if (category.contains("书") || category.contains("教材")) {
        const QStringList names = {"高等数学教材", "雅思真题资料", "课堂笔记", "C++ 程序设计"};
        return names.value(variedIndex(id, name, category, names.size()));
    }
    if (category.contains("生活") || category.contains("宿舍")) {
        const QStringList names = {"护眼台灯", "柔软毛毯", "宿舍电风扇", "桌面加湿器", "保温水杯", "宿舍收纳箱", "便携吹风机", "宿舍小冰箱"};
        return names.value(variedIndex(id, name, category, names.size()));
    }
    if (category.contains("学习") || category.contains("办公")) {
        const QStringList names = {
            "触控笔",
            "科学计算器",
            "小型打印机",
            "文件夹套装",
            "书桌支架",
            "显示器支架"
        };
        return names.value(variedIndex(id, name, category, names.size()));
    }
    if (category.contains("服饰") || category.contains("鞋") || category.contains("包")) {
        const QStringList names = {"通勤双肩包", "帆布包", "九成新运动鞋", "连帽卫衣", "秋季外套", "简约饰品"};
        return names.value(variedIndex(id, name, category, names.size()));
    }
    return name;
}

QString productImagePath(const QString &displayName, const QString &category)
{
    return ProductImageManager::imagePathForProduct(displayName, category);
}

QLabel *imageLabel(const QString &path)
{
    auto *label = new QLabel;
    label->setObjectName("ProductImage");
    label->setFixedSize(148, 112);
    label->setAlignment(Qt::AlignCenter);
    QPixmap pix(path);
    if (!pix.isNull()) {
        label->setPixmap(pix.scaled(132, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    return label;
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

CartWindow::CartWindow(const QStringList &cartItems, const QString &buyerName, QWidget *parent)
    : QWidget(parent),
      table(nullptr),
      cardsScroll(nullptr),
      cardsContainer(nullptr),
      cardsLayout(nullptr),
      labelTotal(nullptr),
      labelItemCount(nullptr),
      labelSubtotal(nullptr),
      labelPayable(nullptr),
      emptyLabel(nullptr),
      btnBuy(nullptr),
      btnRemove(nullptr),
      btnClose(nullptr),
      m_buyerName(buyerName)
{
    resize(1180, 720);
    setMinimumSize(1040, 640);
    setWindowTitle("校园二手交易平台 - 购物车");
    setStyleSheet(kCartQss);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(148);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(14, 22, 14, 18);
    sideLayout->setSpacing(10);

    auto *logoRow = new QHBoxLayout;
    auto *logo = new QLabel("二");
    logo->setObjectName("SidebarLogo");
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(30, 30);
    auto *logoText = new QLabel("校园集市");
    logoText->setObjectName("SidebarTitle");
    logoRow->addWidget(logo);
    logoRow->addWidget(logoText);
    logoRow->addStretch();

    auto *homeNav = sideNavButton("⌂", "首页");
    auto *categoryNav = sideNavButton("▦", "分类");
    auto *cartNav = sideNavButton("▣", "购物车", "SideNavActive");
    auto *sellNav = sideNavButton("+", "我要出售");
    auto *historyNav = sideNavButton("↺", "购买记录");
    auto *profileNav = sideNavButton("◎", "个人中心");
    auto *exitNav = sideNavButton("×", "退出登录", "SideExit");

    sideLayout->addLayout(logoRow);
    sideLayout->addSpacing(18);
    sideLayout->addWidget(homeNav);
    sideLayout->addWidget(categoryNav);
    sideLayout->addWidget(cartNav);
    sideLayout->addWidget(sellNav);
    sideLayout->addWidget(historyNav);
    sideLayout->addWidget(profileNav);
    sideLayout->addStretch();
    auto *userHint = new QLabel("Hi, " + m_buyerName);
    userHint->setObjectName("UserHint");
    userHint->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(userHint);
    sideLayout->addWidget(exitNav);

    auto *main = new QWidget;
    auto *layout = new QVBoxLayout(main);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(18);

    auto *titleRow = new QHBoxLayout;
    auto *titleBlock = new QVBoxLayout;
    auto *title = new QLabel("我的购物车");
    title->setObjectName("Title");
    auto *desc = new QLabel("核对校园好物后即可结算，订单会按数据库商品编号完成购买。");
    desc->setObjectName("Desc");
    titleBlock->addWidget(title);
    titleBlock->addWidget(desc);
    btnClose = new QPushButton("返回首页");
    btnClose->setObjectName("SecondaryButton");
    titleRow->addLayout(titleBlock);
    titleRow->addStretch();
    titleRow->addWidget(btnClose);

    auto *contentRow = new QHBoxLayout;
    contentRow->setSpacing(20);

    auto *listPanel = new QFrame;
    listPanel->setObjectName("CartCard");
    addShadow(listPanel);
    auto *listLayout = new QVBoxLayout(listPanel);
    listLayout->setContentsMargins(18, 18, 18, 18);
    listLayout->setSpacing(14);

    emptyLabel = new QLabel("购物车为空\n去首页看看校园好物吧。");
    emptyLabel->setObjectName("Empty");
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setMinimumHeight(220);

    cardsScroll = new QScrollArea;
    cardsScroll->setWidgetResizable(true);
    cardsContainer = new QWidget;
    cardsContainer->setObjectName("CardsViewport");
    cardsLayout = new QVBoxLayout(cardsContainer);
    cardsLayout->setContentsMargins(0, 0, 8, 0);
    cardsLayout->setSpacing(14);
    cardsScroll->setWidget(cardsContainer);

    listLayout->addWidget(emptyLabel);
    listLayout->addWidget(cardsScroll, 1);

    auto *summary = new QFrame;
    summary->setObjectName("SummaryCard");
    summary->setFixedWidth(300);
    addShadow(summary);
    auto *summaryLayout = new QVBoxLayout(summary);
    summaryLayout->setContentsMargins(22, 22, 22, 22);
    summaryLayout->setSpacing(14);

    auto *summaryTitle = new QLabel("订单汇总");
    summaryTitle->setObjectName("SummaryTitle");
    labelItemCount = new QLabel;
    labelItemCount->setObjectName("SummaryValue");
    labelSubtotal = new QLabel;
    labelSubtotal->setObjectName("SummaryValue");
    auto *freight = new QLabel("运费：¥0.00");
    freight->setObjectName("SummaryValue");
    labelPayable = new QLabel;
    labelPayable->setObjectName("SummaryValue");
    labelTotal = new QLabel;
    labelTotal->setObjectName("Total");

    btnBuy = new QPushButton("立即结算");
    btnBuy->setObjectName("PrimaryButton");
    btnRemove = new QPushButton("移除选中商品");
    btnRemove->setObjectName("DangerButton");

    summaryLayout->addWidget(summaryTitle);
    summaryLayout->addSpacing(8);
    summaryLayout->addWidget(labelItemCount);
    summaryLayout->addWidget(labelSubtotal);
    summaryLayout->addWidget(freight);
    summaryLayout->addWidget(labelPayable);
    summaryLayout->addSpacing(6);
    summaryLayout->addWidget(labelTotal);
    summaryLayout->addStretch();
    summaryLayout->addWidget(btnBuy);
    summaryLayout->addWidget(btnRemove);

    contentRow->addWidget(listPanel, 1);
    contentRow->addWidget(summary);

    table = new QTableWidget(this);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels(QStringList() << "选择" << "商品ID" << "商品名称" << "分类" << "价格" << "数量");
    table->hide();
    table->setRowCount(cartItems.size());
    for (int i = 0; i < cartItems.size(); ++i) {
        const QStringList fields = parseCartItem(cartItems[i]);
        auto *checkItem = new QTableWidgetItem;
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        table->setItem(i, 0, checkItem);
        for (int c = 1; c < 6; ++c) {
            auto *cell = new QTableWidgetItem(fields.value(c - 1));
            if (c != 5) cell->setFlags(cell->flags() & ~Qt::ItemIsEditable);
            cell->setTextAlignment(Qt::AlignCenter);
            table->setItem(i, c, cell);
        }
    }

    layout->addLayout(titleRow);
    layout->addLayout(contentRow, 1);
    layout->addWidget(table);

    root->addWidget(sidebar);
    root->addWidget(main, 1);

    refreshCartCards();
    updateTotal();
    updateEmptyState();

    connect(table, &QTableWidget::itemChanged, this, &CartWindow::updateTotal);
    connect(btnBuy, &QPushButton::clicked, this, &CartWindow::buySelectedItem);
    connect(btnRemove, &QPushButton::clicked, this, &CartWindow::removeSelectedItem);
    connect(btnClose, &QPushButton::clicked, this, &QWidget::close);
    connect(homeNav, &QPushButton::clicked, this, &QWidget::close);
    connect(categoryNav, &QPushButton::clicked, this, &QWidget::close);
    connect(sellNav, &QPushButton::clicked, this, [=]() { QMessageBox::information(this, "我要出售", "请返回首页后点击“我要出售”发布商品。"); });
    connect(historyNav, &QPushButton::clicked, this, &QWidget::close);
    connect(profileNav, &QPushButton::clicked, this, [=]() { QMessageBox::information(this, "个人中心", "当前登录用户：" + m_buyerName); });
    connect(exitNav, &QPushButton::clicked, this, &QWidget::close);
}

QStringList CartWindow::parseCartItem(const QString &itemText) const
{
    const QStringList parts = itemText.split("|");
    if (parts.size() >= 4) {
        return {
            valueAfterColon(parts[0]),
            valueAfterColon(parts[1]),
            valueAfterColon(parts[2]),
            valueAfterColon(parts[3]),
            parts.size() >= 5 ? valueAfterColon(parts[4]) : "1"
        };
    }
    return {"", itemText, "未分类", "0.00", "1"};
}

void CartWindow::refreshCartCards()
{
    clearLayout(cardsLayout);
    for (int row = 0; row < table->rowCount(); ++row) {
        const QString goodsId = table->item(row, 1)->text();
        const MarketplaceGoods *goods = goodsById(goodsId);
        const QString rawName = goods ? goods->name : table->item(row, 2)->text();
        const QString originalCartName = table->item(row, 2)->text();
        const QString category = goods ? goods->category : table->item(row, 3)->text();
        const QString price = goods ? QString::number(goods->price, 'f', 2) : table->item(row, 4)->text();
        const QString quantity = table->item(row, 5)->text();
        const QString displayName = originalCartName.contains("商品")
                                        ? cartDisplayName(rawName, category, goodsId)
                                        : originalCartName;
        const QString seller = goods ? goods->seller : (goodsId.toInt() % 2 == 0 ? "user2" : "user1");

        auto *card = new QFrame;
        card->setObjectName("CartCard");
        auto *rowLayout = new QHBoxLayout(card);
        rowLayout->setContentsMargins(16, 16, 16, 16);
        rowLayout->setSpacing(16);

        auto *check = new QCheckBox("选择");
        check->setChecked(table->item(row, 0)->checkState() == Qt::Checked);
        connect(check, &QCheckBox::toggled, this, [=](bool checked) {
            table->item(row, 0)->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
            table->setCurrentCell(row, 0);
            updateTotal();
        });

        auto *infoLayout = new QVBoxLayout;
        auto *name = new QLabel(displayName);
        name->setObjectName("ProductName");
        auto *meta = new QLabel("分类：" + category + "    卖家：" + seller);
        meta->setObjectName("ProductMeta");
        auto *count = new QLabel("数量：" + quantity + "    商品ID：" + goodsId);
        count->setObjectName("ProductMeta");
        infoLayout->addWidget(name);
        infoLayout->addWidget(meta);
        infoLayout->addWidget(count);
        infoLayout->addStretch();

        auto *priceLabel = new QLabel("¥" + QString::number(parsePrice(price), 'f', 2));
        priceLabel->setObjectName("ProductPrice");
        priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        rowLayout->addWidget(check);
        rowLayout->addLayout(infoLayout, 1);
        rowLayout->addWidget(priceLabel);
        cardsLayout->addWidget(card);
    }
    cardsLayout->addStretch();
}

void CartWindow::updateTotal()
{
    double total = 0.0;
    int count = 0;
    for (int row = 0; row < table->rowCount(); ++row) {
        const double price = parsePrice(table->item(row, 4)->text());
        const int quantity = qMax(1, table->item(row, 5)->text().toInt());
        total += price * quantity;
        count += quantity;
    }
    labelItemCount->setText("商品数量：" + QString::number(count));
    labelSubtotal->setText("商品总额：¥" + QString::number(total, 'f', 2));
    labelPayable->setText("应付合计：¥" + QString::number(total, 'f', 2));
    labelTotal->setText("¥" + QString::number(total, 'f', 2));
    updateEmptyState();
}

void CartWindow::updateEmptyState()
{
    const bool isEmpty = table->rowCount() == 0;
    emptyLabel->setVisible(isEmpty);
    cardsScroll->setVisible(!isEmpty);
    btnBuy->setEnabled(!isEmpty);
    btnRemove->setEnabled(!isEmpty);
}

void CartWindow::buySelectedItem()
{
    QList<int> rows;
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->checkState() == Qt::Checked) rows.prepend(row);
    }
    if (rows.isEmpty() && table->currentRow() >= 0) rows << table->currentRow();
    if (rows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先勾选或选中要购买的商品。");
        return;
    }

    QStringList boughtNames;
    for (int row : rows) {
        const QString goodsId = table->item(row, 1)->text();
        const QString rawName = table->item(row, 2)->text();
        const QString category = table->item(row, 3)->text();
        const QString name = cartDisplayName(rawName, category, goodsId);
        const int quantity = qMax(1, table->item(row, 5)->text().toInt());
        QString errorMessage;
        if (!AccountStore::purchaseGoodsById(goodsId, m_buyerName, quantity, name, category, &errorMessage)) {
            QMessageBox::warning(this, "购买失败", name + "：" + errorMessage);
            continue;
        }
        boughtNames << name;
        table->removeRow(row);
    }

    refreshCartCards();
    if (!boughtNames.isEmpty()) QMessageBox::information(this, "购买成功", "你已购买：" + boughtNames.join("、"));
    updateTotal();
}

void CartWindow::removeSelectedItem()
{
    QList<int> rows;
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->checkState() == Qt::Checked) rows.prepend(row);
    }
    if (rows.isEmpty() && table->currentRow() >= 0) rows << table->currentRow();
    if (rows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先勾选或选中要移除的商品。");
        return;
    }
    if (QMessageBox::question(this, "确认移除", "确定从购物车移除选中的商品吗？") != QMessageBox::Yes) return;
    for (int row : rows) {
        table->removeRow(row);
    }
    refreshCartCards();
    updateTotal();
}
