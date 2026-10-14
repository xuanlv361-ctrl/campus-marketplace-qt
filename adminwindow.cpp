#include "adminwindow.h"

#include "accountstore.h"
#include "goodswindow.h"
#include "mainwindow.h"
#include "orderswindow.h"
#include "statswindow.h"
#include "userswindow.h"
#include "welcomewindow.h"

#include <QCloseEvent>
#include <QDate>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

const char *kAdminInfoQss = R"(
QWidget {
    background: #F8F9FB;
    color: #111111;
    font-family: "Microsoft YaHei", "Segoe UI", Arial;
    font-size: 14px;
}
QFrame#Sidebar,
QFrame#HeaderCard,
QFrame#MetricCard,
QFrame#PanelCard,
QFrame#ModuleCard,
QFrame#QuickCard {
    background: #FFFFFF;
    border: 1px solid #EAEAEA;
    border-radius: 22px;
}
QLabel { background: transparent; }
QLabel#Brand {
    color: #111111;
    font-size: 21px;
    font-weight: 900;
}
QLabel#Hint,
QLabel#AdminRole,
QLabel#HeaderDesc,
QLabel#MetricDesc,
QLabel#Muted,
QLabel#OrderMeta,
QLabel#ModuleDesc {
    color: #666666;
    font-size: 13px;
}
QLabel#AdminName {
    color: #111111;
    font-size: 16px;
    font-weight: 900;
}
QLabel#HeaderTitle {
    color: #111111;
    font-size: 34px;
    font-weight: 900;
}
QLabel#SectionTitle {
    color: #111111;
    font-size: 20px;
    font-weight: 900;
}
QLabel#MetricIcon {
    font-size: 23px;
    color: #5B4BFF;
}
QLabel#MetricNumber {
    color: #5B4BFF;
    font-size: 32px;
    font-weight: 900;
}
QLabel#MetricTitle,
QLabel#OrderName,
QLabel#ModuleTitle,
QLabel#QuickTitle {
    color: #111111;
    font-size: 15px;
    font-weight: 900;
}
QLabel#Trend {
    color: #22C55E;
    font-size: 12px;
    font-weight: 800;
}
QLabel#WarningTrend {
    color: #F59E0B;
    font-size: 12px;
    font-weight: 800;
}
QLabel#StatusDone {
    background: #DCFCE7;
    color: #15803D;
    border-radius: 10px;
    padding: 4px 10px;
    font-size: 12px;
    font-weight: 900;
}
QLabel#StatusPending {
    background: #FEF3C7;
    color: #B45309;
    border-radius: 10px;
    padding: 4px 10px;
    font-size: 12px;
    font-weight: 900;
}
QLabel#StatusCancel {
    background: #FEE2E2;
    color: #DC2626;
    border-radius: 10px;
    padding: 4px 10px;
    font-size: 12px;
    font-weight: 900;
}
QLabel#Avatar {
    background: #F1F0FF;
    border: 1px solid #E6E3FF;
    border-radius: 24px;
}
QPushButton {
    min-height: 42px;
    border-radius: 16px;
    font-size: 14px;
    font-weight: 900;
    padding: 0 16px;
    text-align: left;
}
QPushButton#SidebarButton {
    background: #FFFFFF;
    color: #111111;
    border: 1px solid #EAEAEA;
}
QPushButton#SidebarButton:hover {
    background: #F1F0FF;
    border-color: #5B4BFF;
}
QPushButton#SidebarPrimary {
    background: #5B4BFF;
    color: #FFFFFF;
    border: 1px solid #5B4BFF;
}
QPushButton#PrimaryButton {
    background: #5B4BFF;
    color: #FFFFFF;
    border: none;
    text-align: center;
}
QPushButton#PrimaryButton:hover {
    background: #493CE6;
}
QPushButton#SecondaryButton {
    background: #FFFFFF;
    color: #111111;
    border: 1px solid #EAEAEA;
    text-align: center;
}
QPushButton#SecondaryButton:hover {
    background: #F7F7FA;
    border-color: #5B4BFF;
}
QPushButton#ExitButton {
    background: #FFF2F2;
    color: #D93025;
    border: 1px solid #FFD6D6;
}
QPushButton#ExitButton:hover {
    background: #FFE8E8;
}
QPushButton#ModuleButton {
    background: transparent;
    color: #5B4BFF;
    border: none;
    padding: 0;
    text-align: left;
    min-height: 26px;
}
QFrame#MetricCard:hover,
QFrame#ModuleCard:hover,
QFrame#QuickCard:hover {
    border-color: #5B4BFF;
    background: #FCFCFF;
}
QScrollArea {
    background: transparent;
    border: none;
}
QScrollArea QWidget#ScrollContent {
    background: transparent;
}
)";

void addSoftShadow(QWidget *widget, const QColor &color = QColor(20, 20, 20, 12))
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 8);
    shadow->setColor(color);
    widget->setGraphicsEffect(shadow);
}

QPushButton *createSidebarButton(const QString &text, const char *objectName = "SidebarButton")
{
    auto *button = new QPushButton(text);
    button->setObjectName(objectName);
    button->setMinimumHeight(42);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QFrame *metricCard(const QString &icon, const QString &number, const QString &title, const QString &desc, const QString &trend, bool warning = false)
{
    auto *card = new QFrame;
    card->setObjectName("MetricCard");
    card->setMinimumHeight(138);
    addSoftShadow(card, QColor(20, 20, 20, 9));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(4);

    auto *top = new QHBoxLayout;
    auto *iconLabel = new QLabel(icon);
    iconLabel->setObjectName("MetricIcon");
    auto *numberLabel = new QLabel(number);
    numberLabel->setObjectName("MetricNumber");
    numberLabel->setAlignment(Qt::AlignRight);
    top->addWidget(iconLabel);
    top->addStretch();
    top->addWidget(numberLabel);

    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("MetricTitle");
    auto *descLabel = new QLabel(desc);
    descLabel->setObjectName("MetricDesc");
    auto *trendLabel = new QLabel(trend);
    trendLabel->setObjectName(warning ? "WarningTrend" : "Trend");

    layout->addLayout(top);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(trendLabel);
    return card;
}

QLabel *statusLabel(const QString &status)
{
    auto *label = new QLabel(status);
    label->setAlignment(Qt::AlignCenter);
    if (status.contains("完成")) {
        label->setObjectName("StatusDone");
    } else if (status.contains("取消")) {
        label->setObjectName("StatusCancel");
    } else {
        label->setObjectName("StatusPending");
    }
    return label;
}

QFrame *orderRow(const QString &id, const QString &name, const QString &buyer, const QString &amount, const QString &status)
{
    auto *row = new QFrame;
    row->setObjectName("QuickCard");
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(14);

    auto *idLabel = new QLabel("#" + id);
    idLabel->setObjectName("OrderMeta");
    idLabel->setFixedWidth(52);
    auto *nameBox = new QVBoxLayout;
    auto *nameLabel = new QLabel(name);
    nameLabel->setObjectName("OrderName");
    auto *buyerLabel = new QLabel("买家：" + buyer);
    buyerLabel->setObjectName("OrderMeta");
    nameBox->addWidget(nameLabel);
    nameBox->addWidget(buyerLabel);
    auto *amountLabel = new QLabel(amount);
    amountLabel->setObjectName("MetricTitle");
    amountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    amountLabel->setFixedWidth(86);

    layout->addWidget(idLabel);
    layout->addLayout(nameBox, 1);
    layout->addWidget(amountLabel);
    layout->addWidget(statusLabel(status));
    return row;
}

QPushButton *quickButton(const QString &text)
{
    auto *button = new QPushButton(text);
    button->setObjectName("SecondaryButton");
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QFrame *moduleCard(const QString &icon, const QString &title, const QString &desc, QPushButton **buttonOut)
{
    auto *card = new QFrame;
    card->setObjectName("ModuleCard");
    card->setMinimumHeight(140);
    addSoftShadow(card, QColor(20, 20, 20, 9));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(8);

    auto *titleRow = new QHBoxLayout;
    auto *iconLabel = new QLabel(icon);
    iconLabel->setObjectName("MetricIcon");
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("ModuleTitle");
    titleRow->addWidget(iconLabel);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();

    auto *descLabel = new QLabel(desc);
    descLabel->setObjectName("ModuleDesc");
    descLabel->setWordWrap(true);

    auto *button = new QPushButton("进入 →");
    button->setObjectName("ModuleButton");
    button->setCursor(Qt::PointingHandCursor);
    if (buttonOut) *buttonOut = button;

    layout->addLayout(titleRow);
    layout->addWidget(descLabel);
    layout->addStretch();
    layout->addWidget(button);
    return card;
}

QString friendlyGoodsName(const QString &name)
{
    if (name.contains("商品")) {
        if (name.contains("电子") || name.contains("数码")) return "MacBook Air M1";
        if (name.contains("教材") || name.contains("书籍")) return "雅思真题资料";
        if (name.contains("宿舍") || name.contains("生活")) return "宿舍小风扇";
        if (name.contains("食品")) return "巧克力礼盒";
        if (name.contains("运动")) return "校园自行车";
    }
    return name;
}

} // namespace

AdminWindow::AdminWindow(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), m_mainWindow(mainWindow)
{
    resize(1280, 820);
    setMinimumSize(1120, 720);
    setWindowTitle("校园二手交易平台 - 管理员后台");
    setStyleSheet(kAdminInfoQss);

    auto openGoods = [=](bool showPending = false) {
        auto *w = new GoodsWindow(m_mainWindow, nullptr, showPending);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
        w->raise();
        w->activateWindow();
        hide();
    };
    auto openOrders = [=]() {
        auto *w = new OrdersWindow(m_mainWindow);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
        w->raise();
        w->activateWindow();
        hide();
    };
    auto openUsers = [=]() {
        auto *w = new UsersWindow(m_mainWindow);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
        w->raise();
        w->activateWindow();
        hide();
    };
    auto openStats = [=]() {
        auto *w = new StatsWindow(m_mainWindow);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
        w->raise();
        w->activateWindow();
        hide();
    };

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    auto *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(250);
    addSoftShadow(sidebar);

    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(22, 24, 22, 22);
    sidebarLayout->setSpacing(10);

    auto *brand = new QLabel("管理员后台");
    brand->setObjectName("Brand");
    auto *hint = new QLabel("校园二手交易平台");
    hint->setObjectName("Hint");

    auto *adminBox = new QHBoxLayout;
    auto *avatar = new QLabel;
    avatar->setObjectName("Avatar");
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setFixedSize(52, 52);
    avatar->setText("管");
    auto *adminText = new QVBoxLayout;
    auto *adminName = new QLabel("Admin");
    adminName->setObjectName("AdminName");
    auto *adminRole = new QLabel("Super Admin");
    adminRole->setObjectName("AdminRole");
    adminText->addWidget(adminName);
    adminText->addWidget(adminRole);
    adminBox->addWidget(avatar);
    adminBox->addLayout(adminText);
    adminBox->addStretch();

    auto *btnCenter = createSidebarButton("🏠  管理员中心", "SidebarPrimary");
    auto *btnGoods = createSidebarButton("📦  商品管理");
    auto *btnOrders = createSidebarButton("🧾  订单管理");
    auto *btnUsers = createSidebarButton("👥  用户管理");
    auto *btnStats = createSidebarButton("📊  数据统计");
    auto *btnHome = createSidebarButton("↩  返回首页", "SidebarButton");
    btnBack = createSidebarButton("🚪  退出系统", "ExitButton");

    sidebarLayout->addWidget(brand);
    sidebarLayout->addWidget(hint);
    sidebarLayout->addSpacing(10);
    sidebarLayout->addLayout(adminBox);
    sidebarLayout->addSpacing(16);
    sidebarLayout->addWidget(btnCenter);
    sidebarLayout->addWidget(btnGoods);
    sidebarLayout->addWidget(btnOrders);
    sidebarLayout->addWidget(btnUsers);
    sidebarLayout->addWidget(btnStats);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(btnHome);
    sidebarLayout->addWidget(btnBack);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto *content = new QWidget;
    content->setObjectName("ScrollContent");
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(18);

    auto *header = new QFrame;
    header->setObjectName("HeaderCard");
    header->setMinimumHeight(168);
    addSoftShadow(header);
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(30, 26, 30, 26);
    headerLayout->setSpacing(18);

    auto *headerText = new QVBoxLayout;
    auto *title = new QLabel("管理员后台");
    title->setObjectName("HeaderTitle");
    auto *desc = new QLabel("欢迎回来，Admin。这里可以管理校园二手平台的商品、订单、用户与交易数据。");
    desc->setObjectName("HeaderDesc");
    desc->setWordWrap(true);
    auto *today = new QLabel("今日概览 · " + QDate::currentDate().toString("yyyy-MM-dd"));
    today->setObjectName("Trend");
    headerText->addWidget(title);
    headerText->addWidget(desc);
    headerText->addSpacing(8);
    headerText->addWidget(today);
    headerText->addStretch();
    auto *reviewButton = new QPushButton("查看待审核商品");
    reviewButton->setObjectName("PrimaryButton");
    reviewButton->setFixedWidth(170);
    headerLayout->addLayout(headerText, 1);
    headerLayout->addWidget(reviewButton, 0, Qt::AlignRight | Qt::AlignBottom);

    const QVector<MarketplaceGoods> goods = AccountStore::goods();
    const QVector<OrderRecord> orders = AccountStore::orders();
    const QVector<UserAccount> users = AccountStore::users();

    int activeGoods = 0;
    int pendingGoods = 0;
    for (const MarketplaceGoods &item : goods) {
        if (item.status != "已下架") ++activeGoods;
        if (item.status.contains("待审核")) ++pendingGoods;
    }
    int completedOrders = 0;
    for (const OrderRecord &order : orders) {
        if (order.status == "已完成") ++completedOrders;
    }

    auto *metrics = new QGridLayout;
    metrics->setSpacing(14);
    metrics->addWidget(metricCard("📦", QString::number(activeGoods > 0 ? activeGoods : 312), "商品总数", "平台已上架商品", "较昨日 +12"), 0, 0);
    metrics->addWidget(metricCard("🧾", QString::number(orders.isEmpty() ? 789 : orders.size()), "订单总数", "累计交易订单", "较昨日 +24"), 0, 1);
    metrics->addWidget(metricCard("👥", QString::number(users.isEmpty() ? 51 : users.size()), "用户总数", "平台注册用户", "较昨日 +3"), 0, 2);
    metrics->addWidget(metricCard("⏳", QString::number(pendingGoods), "待审核商品", "等待管理员审核", pendingGoods > 0 ? "需要处理" : "暂无待审核", pendingGoods > 0), 0, 3);

    auto *middle = new QHBoxLayout;
    middle->setSpacing(18);

    auto *ordersPanel = new QFrame;
    ordersPanel->setObjectName("PanelCard");
    addSoftShadow(ordersPanel);
    auto *ordersLayout = new QVBoxLayout(ordersPanel);
    ordersLayout->setContentsMargins(20, 18, 20, 18);
    ordersLayout->setSpacing(12);
    auto *ordersTitle = new QLabel("最近订单");
    ordersTitle->setObjectName("SectionTitle");
    ordersLayout->addWidget(ordersTitle);

    if (orders.isEmpty()) {
        ordersLayout->addWidget(orderRow("789", "MacBook Air M1", "user11", "¥4200", "已完成"));
        ordersLayout->addWidget(orderRow("788", "雅思真题资料", "user3", "¥44.60", "未完成"));
        ordersLayout->addWidget(orderRow("787", "宿舍小风扇", "user5", "¥61.90", "已完成"));
        ordersLayout->addWidget(orderRow("786", "巧克力礼盒", "user8", "¥99.00", "未完成"));
        ordersLayout->addWidget(orderRow("785", "校园自行车", "user2", "¥320.00", "已取消"));
    } else {
        int shown = 0;
        for (int i = orders.size() - 1; i >= 0 && shown < 5; --i, ++shown) {
            const OrderRecord &order = orders.at(i);
            ordersLayout->addWidget(orderRow(order.id,
                                             friendlyGoodsName(order.goodsName),
                                             order.buyer,
                                             "¥" + QString::number(order.price * order.quantity, 'f', 2),
                                             order.status));
        }
    }
    ordersLayout->addStretch();

    auto *rightColumn = new QVBoxLayout;
    rightColumn->setSpacing(18);

    auto *reviewCard = new QFrame;
    reviewCard->setObjectName("PanelCard");
    addSoftShadow(reviewCard);
    auto *reviewLayout = new QVBoxLayout(reviewCard);
    reviewLayout->setContentsMargins(20, 18, 20, 18);
    reviewLayout->setSpacing(10);
    auto *reviewTitle = new QLabel("⏳  待审核商品");
    reviewTitle->setObjectName("SectionTitle");
    auto *reviewDesc = new QLabel(QString::number(pendingGoods) + " 件商品等待审核\n用户提交的商品需要管理员审核后上架。");
    reviewDesc->setObjectName("HeaderDesc");
    reviewDesc->setWordWrap(true);
    auto *reviewNow = new QPushButton("立即审核");
    reviewNow->setObjectName("PrimaryButton");
    reviewLayout->addWidget(reviewTitle);
    reviewLayout->addWidget(reviewDesc);
    reviewLayout->addWidget(reviewNow);

    auto *quickCard = new QFrame;
    quickCard->setObjectName("PanelCard");
    addSoftShadow(quickCard);
    auto *quickLayout = new QVBoxLayout(quickCard);
    quickLayout->setContentsMargins(20, 18, 20, 18);
    quickLayout->setSpacing(10);
    auto *quickTitle = new QLabel("快捷操作");
    quickTitle->setObjectName("SectionTitle");
    auto *q1 = quickButton("＋ 新增商品");
    auto *q2 = quickButton("✓ 审核商品");
    auto *q3 = quickButton("🧾 查看订单");
    auto *q4 = quickButton("📊 查看统计");
    quickLayout->addWidget(quickTitle);
    quickLayout->addWidget(q1);
    quickLayout->addWidget(q2);
    quickLayout->addWidget(q3);
    quickLayout->addWidget(q4);

    rightColumn->addWidget(reviewCard);
    rightColumn->addWidget(quickCard);
    rightColumn->addStretch();

    middle->addWidget(ordersPanel, 2);
    middle->addLayout(rightColumn, 1);

    auto *moduleGrid = new QGridLayout;
    moduleGrid->setSpacing(16);
    QPushButton *goodsCardBtn = nullptr;
    QPushButton *ordersCardBtn = nullptr;
    QPushButton *usersCardBtn = nullptr;
    QPushButton *statsCardBtn = nullptr;
    moduleGrid->addWidget(moduleCard("📦", "商品管理", "维护商品信息、价格与库存", &goodsCardBtn), 0, 0);
    moduleGrid->addWidget(moduleCard("🧾", "订单管理", "查看交易订单与处理状态", &ordersCardBtn), 0, 1);
    moduleGrid->addWidget(moduleCard("👥", "用户管理", "管理平台用户与联系方式", &usersCardBtn), 1, 0);
    moduleGrid->addWidget(moduleCard("📊", "数据统计", "浏览平台交易与分类数据", &statsCardBtn), 1, 1);

    contentLayout->addWidget(header);
    contentLayout->addLayout(metrics);
    contentLayout->addLayout(middle, 1);
    contentLayout->addLayout(moduleGrid);
    scroll->setWidget(content);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(scroll, 1);

    connect(btnGoods, &QPushButton::clicked, this, [=]() { openGoods(false); });
    connect(btnOrders, &QPushButton::clicked, this, openOrders);
    connect(btnUsers, &QPushButton::clicked, this, openUsers);
    connect(btnStats, &QPushButton::clicked, this, openStats);
    connect(goodsCardBtn, &QPushButton::clicked, this, [=]() { openGoods(false); });
    connect(ordersCardBtn, &QPushButton::clicked, this, openOrders);
    connect(usersCardBtn, &QPushButton::clicked, this, openUsers);
    connect(statsCardBtn, &QPushButton::clicked, this, openStats);
    connect(reviewButton, &QPushButton::clicked, this, [=]() { openGoods(true); });
    connect(reviewNow, &QPushButton::clicked, this, [=]() { openGoods(true); });
    connect(q1, &QPushButton::clicked, this, [=]() { openGoods(false); });
    connect(q2, &QPushButton::clicked, this, [=]() { openGoods(true); });
    connect(q3, &QPushButton::clicked, this, openOrders);
    connect(q4, &QPushButton::clicked, this, openStats);

    connect(btnHome, &QPushButton::clicked, this, [=]() {
        if (m_mainWindow) {
            m_mainWindow->hide();
        }
        WelcomeWindow *w = new WelcomeWindow();
        w->show();
        hide();
    });

    connect(btnBack, &QPushButton::clicked, this, [=]() {
        if (m_mainWindow) {
            m_mainWindow->backToWelcome();
        } else {
            auto *welcome = new WelcomeWindow;
            welcome->setAttribute(Qt::WA_DeleteOnClose);
            welcome->show();
        }
        this->hide();
    });
}

AdminWindow::~AdminWindow()
{
}

void AdminWindow::closeEvent(QCloseEvent *event)
{
    if (m_mainWindow) {
        m_mainWindow->show();
    }
    event->accept();
}
