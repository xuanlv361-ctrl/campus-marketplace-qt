#include "historywindow.h"

#include "accountstore.h"
#include "productimagemanager.h"
#include "welcomewindow.h"

#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

const char *kHistoryQss = R"(
QWidget { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#Sidebar { background: #FFFFFF; border-right: 1px solid #EAEAEA; }
QLabel#LogoMark { background: #5B4BFF; color: #FFFFFF; border-radius: 18px; font-size: 18px; font-weight: 900; }
QLabel#LogoText { color: #111111; font-size: 18px; font-weight: 900; }
QPushButton.SideNav { background: transparent; color: #333333; border: none; border-radius: 12px; min-height: 44px; padding: 0 14px; text-align: left; font-weight: 800; }
QPushButton.SideNav:hover { background: #F3F2FF; }
QPushButton#SideNavActive { background: #5B4BFF; color: #FFFFFF; border: none; border-radius: 12px; min-height: 44px; padding: 0 14px; text-align: left; font-weight: 900; }
QPushButton#SideExit { background: #FEF2F2; color: #EF4444; border: 1px solid #FECACA; border-radius: 12px; min-height: 44px; padding: 0 14px; text-align: left; font-weight: 900; }
QLabel#SidebarUser { background: #F5F5F7; color: #111111; border-radius: 10px; padding: 8px; font-weight: 800; }
QLabel#SidebarRole { color: #666666; font-size: 12px; }
QLabel#PageTitle { color: #111111; font-size: 32px; font-weight: 900; }
QLabel#PageSubtitle { color: #666666; font-size: 15px; }
QPushButton#BackButton { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; border-radius: 18px; min-height: 40px; padding: 0 22px; font-weight: 900; }
QPushButton#BackButton:hover { background: #F6F6F8; border-color: #D8D8E0; }
QPushButton#DangerButton { background: #FEF2F2; color: #EF4444; border: 1px solid #FECACA; border-radius: 18px; min-height: 40px; padding: 0 22px; font-weight: 900; }
QPushButton#DangerButton:hover { background: #FEE2E2; }
QFrame#StatCard, QFrame#OrderCard, QFrame#EmptyCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 18px; }
QLabel.StatNumber { color: #111111; font-size: 26px; font-weight: 900; }
QLabel.StatLabel { color: #666666; font-size: 13px; }
QLabel#SectionTitle { color: #111111; font-size: 22px; font-weight: 900; }
QLabel#ProductImage { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 16px; }
QLabel#OrderName { color: #111111; font-size: 18px; font-weight: 900; }
QLabel.OrderMeta { color: #666666; font-size: 13px; }
QLabel#OrderPrice { color: #5B4BFF; font-size: 22px; font-weight: 900; }
QLabel.StatusPill { border-radius: 12px; padding: 5px 12px; font-weight: 900; }
QLabel#StatusPending { background: #FEF3C7; color: #A16207; }
QLabel#StatusDone { background: #DCFCE7; color: #15803D; }
QLabel#StatusCancel { background: #FEE2E2; color: #DC2626; }
QLabel#StatusShip { background: #DBEAFE; color: #2563EB; }
QPushButton#DetailButton { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; border-radius: 16px; min-height: 36px; padding: 0 18px; font-weight: 900; }
QPushButton#DetailButton:hover { background: #F6F6F8; }
QPushButton#PrimaryButton { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; border-radius: 18px; min-height: 40px; padding: 0 24px; font-weight: 900; }
QScrollArea { background: transparent; border: none; }
)";

QPushButton *sideNavButton(const QString &icon, const QString &text, const char *objectName = nullptr)
{
    auto *button = new QPushButton(icon + "  " + text);
    button->setProperty("class", "SideNav");
    if (objectName) button->setObjectName(objectName);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

QFrame *statCard(const QString &title, QLabel **valueLabel)
{
    auto *card = new QFrame;
    card->setObjectName("StatCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(8);

    *valueLabel = new QLabel("0");
    (*valueLabel)->setProperty("class", "StatNumber");
    auto *label = new QLabel(title);
    label->setProperty("class", "StatLabel");
    layout->addWidget(*valueLabel);
    layout->addWidget(label);
    return card;
}


int legacyVariedIndex(const QString &name, const QString &category, int size)
{
    if (size <= 0) return 0;
    QRegularExpression re("(\\d+)");
    const QRegularExpressionMatch match = re.match(name);
    const int id = match.hasMatch() ? match.captured(1).toInt() : 0;
    const uint seed = id > 0 ? uint(id) : qHash(name + category);
    return int(((seed * 37u) + (seed / 10u) + 3u) % uint(size));
}

QString displayRecordName(const PurchaseRecord &record)
{
    QString name = record.name.trimmed();
    const QString category = record.category;
    if (name == "宿舍小风扇") return "宿舍电风扇";
    if (!name.contains("商品")) return name;

    auto pick = [&](const QStringList &names) {
        return names.value(legacyVariedIndex(name, category, names.size()));
    };
    if (category.contains("学习") || category.contains("办公")) {
        return pick({"触控笔", "科学计算器", "小型打印机", "文件夹套装", "书桌支架", "显示器支架"});
    }
    if (category.contains("生活") || category.contains("宿舍")) {
        return pick({"护眼台灯", "柔软毛毯", "宿舍电风扇", "桌面加湿器", "保温水杯", "宿舍收纳箱", "便携吹风机", "宿舍小冰箱"});
    }
    if (category.contains("电子") || category.contains("数码")) {
        return pick({"笔记本电脑", "平板电脑", "蓝牙降噪耳机", "Sony降噪耳机", "佳能EOS相机", "RK机械键盘", "罗技无线鼠标", "宿舍显示器"});
    }
    if (category.contains("书") || category.contains("教材")) {
        return pick({"高等数学教材", "线性代数教材", "概率论教材", "操作系统教材", "C++ 程序设计", "雅思英语资料", "英语四级资料", "考研数学资料"});
    }
    if (category.contains("服饰") || category.contains("鞋") || category.contains("包")) {
        return pick({"通勤双肩包", "帆布包", "九成新运动鞋", "连帽卫衣", "秋季外套", "简约饰品"});
    }
    return name;
}

QString compactTime(const QString &time)
{
    const QDateTime dt = QDateTime::fromString(time, "yyyy-MM-dd HH:mm:ss");
    if (dt.isValid()) return dt.toString("yyyy-MM-dd HH:mm");
    return time.left(16).replace("\n", " ");
}

QString statusObjectName(const QString &status)
{
    if (status.contains("完成") && !status.contains("未完成")) return "StatusDone";
    if (status.contains("取消")) return "StatusCancel";
    if (status.contains("发货") || status.contains("待发")) return "StatusShip";
    return "StatusPending";
}

} // namespace

HistoryWindow::HistoryWindow(const QStringList &historyItems, const QString &username, QWidget *parent)
    : QWidget(parent),
      m_username(username.isEmpty() ? "user1" : username),
      orderCountLabel(nullptr),
      totalSpentLabel(nullptr),
      pendingCountLabel(nullptr),
      finishedCountLabel(nullptr),
      orderScroll(nullptr),
      orderViewport(nullptr),
      orderListLayout(nullptr),
      emptyCard(nullptr),
      btnClear(nullptr)
{
    Q_UNUSED(historyItems);

    resize(1180, 760);
    setMinimumSize(980, 620);
    setWindowTitle("校园二手交易平台 - 购买记录");
    setStyleSheet(kHistoryQss);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(170);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 22, 16, 22);
    sideLayout->setSpacing(10);

    auto *logoRow = new QHBoxLayout;
    auto *logoMark = new QLabel("=");
    logoMark->setObjectName("LogoMark");
    logoMark->setAlignment(Qt::AlignCenter);
    logoMark->setFixedSize(36, 36);
    auto *logoText = new QLabel("校园集市");
    logoText->setObjectName("LogoText");
    logoRow->addWidget(logoMark);
    logoRow->addWidget(logoText);
    logoRow->addStretch();

    auto *homeNav = sideNavButton("⌂", "首页");
    auto *categoryNav = sideNavButton("▦", "分类");
    auto *cartNav = sideNavButton("▣", "购物车");
    auto *sellNav = sideNavButton("+", "我要出售");
    auto *historyNav = sideNavButton("↺", "购买记录", "SideNavActive");
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
    auto *userHint = new QLabel("Hi, " + m_username);
    userHint->setObjectName("SidebarUser");
    userHint->setAlignment(Qt::AlignCenter);
    auto *roleHint = new QLabel("普通用户");
    roleHint->setObjectName("SidebarRole");
    roleHint->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(userHint);
    sideLayout->addWidget(roleHint);
    sideLayout->addWidget(exitNav);

    auto *main = new QWidget;
    auto *mainLayout = new QVBoxLayout(main);
    mainLayout->setContentsMargins(28, 26, 28, 26);
    mainLayout->setSpacing(20);

    auto *headerRow = new QHBoxLayout;
    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(6);
    auto *title = new QLabel("我的购买记录");
    title->setObjectName("PageTitle");
    auto *subtitle = new QLabel("查看历史订单与交易状态。");
    subtitle->setObjectName("PageSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);

    auto *backButton = new QPushButton("返回首页");
    backButton->setObjectName("BackButton");
    btnClear = new QPushButton("清空记录");
    btnClear->setObjectName("DangerButton");
    headerRow->addLayout(titleBox);
    headerRow->addStretch();
    headerRow->addWidget(btnClear);
    headerRow->addWidget(backButton);

    auto *statsGrid = new QGridLayout;
    statsGrid->setHorizontalSpacing(16);
    statsGrid->setVerticalSpacing(16);
    statsGrid->addWidget(statCard("累计订单", &orderCountLabel), 0, 0);
    statsGrid->addWidget(statCard("累计消费", &totalSpentLabel), 0, 1);
    statsGrid->addWidget(statCard("待完成订单", &pendingCountLabel), 0, 2);
    statsGrid->addWidget(statCard("已完成订单", &finishedCountLabel), 0, 3);

    auto *sectionRow = new QHBoxLayout;
    auto *sectionTitle = new QLabel("订单列表");
    sectionTitle->setObjectName("SectionTitle");
    sectionRow->addWidget(sectionTitle);
    sectionRow->addStretch();

    orderScroll = new QScrollArea;
    orderScroll->setWidgetResizable(true);
    orderViewport = new QWidget;
    orderListLayout = new QVBoxLayout(orderViewport);
    orderListLayout->setContentsMargins(0, 0, 0, 0);
    orderListLayout->setSpacing(14);
    orderScroll->setWidget(orderViewport);

    emptyCard = new QFrame;
    emptyCard->setObjectName("EmptyCard");
    auto *emptyLayout = new QVBoxLayout(emptyCard);
    emptyLayout->setContentsMargins(30, 42, 30, 42);
    emptyLayout->setSpacing(12);
    auto *emptyIcon = new QLabel("🛒");
    emptyIcon->setAlignment(Qt::AlignCenter);
    emptyIcon->setStyleSheet("font-size: 34px; background: transparent;");
    auto *emptyTitle = new QLabel("暂无购买记录");
    emptyTitle->setObjectName("OrderName");
    emptyTitle->setAlignment(Qt::AlignCenter);
    auto *emptyText = new QLabel("你还没有购买任何商品，去首页看看校园好物吧。");
    emptyText->setProperty("class", "OrderMeta");
    emptyText->setAlignment(Qt::AlignCenter);
    auto *goHomeButton = new QPushButton("去首页逛逛");
    goHomeButton->setObjectName("PrimaryButton");
    emptyLayout->addWidget(emptyIcon);
    emptyLayout->addWidget(emptyTitle);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addWidget(goHomeButton, 0, Qt::AlignCenter);

    mainLayout->addLayout(headerRow);
    mainLayout->addLayout(statsGrid);
    mainLayout->addLayout(sectionRow);
    mainLayout->addWidget(emptyCard);
    mainLayout->addWidget(orderScroll, 1);

    root->addWidget(sidebar);
    root->addWidget(main, 1);

    auto goHome = [this]() {
        auto *w = new WelcomeWindow();
        w->show();
        close();
    };
    connect(backButton, &QPushButton::clicked, this, goHome);
    connect(homeNav, &QPushButton::clicked, this, goHome);
    connect(exitNav, &QPushButton::clicked, this, goHome);
    connect(goHomeButton, &QPushButton::clicked, this, goHome);

    connect(btnClear, &QPushButton::clicked, this, [=]() {
        if (AccountStore::purchaseRecords().isEmpty()) return;
        if (QMessageBox::question(this, "确认清空", "确定清空全部购买记录吗？") == QMessageBox::Yes) {
            AccountStore::clearPurchaseRecords();
            loadRecords();
        }
    });

    auto unavailable = [this]() {
        QMessageBox::information(this, "提示", "请返回用户首页使用该功能。");
    };
    connect(categoryNav, &QPushButton::clicked, this, unavailable);
    connect(cartNav, &QPushButton::clicked, this, unavailable);
    connect(sellNav, &QPushButton::clicked, this, unavailable);
    connect(profileNav, &QPushButton::clicked, this, unavailable);

    loadRecords();
}

void HistoryWindow::loadRecords()
{
    const QVector<PurchaseRecord> records = AccountStore::purchaseRecords();
    renderStats(records);
    renderOrders(records);
}

void HistoryWindow::renderStats(const QVector<PurchaseRecord> &records)
{
    double total = 0.0;
    int pending = 0;
    int finished = 0;
    for (const PurchaseRecord &record : records) {
        total += record.price * record.quantity;
        if (record.status.contains("完成") && !record.status.contains("未完成")) {
            ++finished;
        } else {
            ++pending;
        }
    }
    orderCountLabel->setText(QString::number(records.size()));
    totalSpentLabel->setText("¥" + QString::number(total, 'f', 2));
    pendingCountLabel->setText(QString::number(pending));
    finishedCountLabel->setText(QString::number(finished));
    btnClear->setEnabled(!records.isEmpty());
}

void HistoryWindow::renderOrders(const QVector<PurchaseRecord> &records)
{
    while (QLayoutItem *item = orderListLayout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    const bool isEmpty = records.isEmpty();
    emptyCard->setVisible(isEmpty);
    orderScroll->setVisible(!isEmpty);
    if (isEmpty) return;

    for (int i = 0; i < records.size(); ++i) {
        const PurchaseRecord &record = records[i];
        auto *card = new QFrame;
        card->setObjectName("OrderCard");
        auto *row = new QHBoxLayout(card);
        row->setContentsMargins(18, 16, 18, 16);
        row->setSpacing(18);

        const QString displayName = displayRecordName(record);
        auto *thumb = new QLabel;
        thumb->setObjectName("ProductImage");
        thumb->setFixedSize(118, 88);
        thumb->setAlignment(Qt::AlignCenter);
        const QPixmap pix = ProductImageManager::pixmapForProduct(displayName, record.category, QSize(108, 78));
        if (!pix.isNull()) thumb->setPixmap(pix);

        auto *info = new QVBoxLayout;
        info->setSpacing(7);
        auto *name = new QLabel(displayName);
        name->setObjectName("OrderName");
        auto *category = new QLabel("分类：" + (record.category.isEmpty() ? "未分类" : record.category));
        category->setProperty("class", "OrderMeta");
        auto *quantity = new QLabel("数量：" + QString::number(record.quantity));
        quantity->setProperty("class", "OrderMeta");
        auto *time = new QLabel("购买时间：" + compactTime(record.time));
        time->setProperty("class", "OrderMeta");
        auto *orderId = new QLabel("订单编号：CM" + QString::number(10000 + i + 1));
        orderId->setProperty("class", "OrderMeta");
        info->addWidget(name);
        info->addWidget(category);
        info->addWidget(quantity);
        info->addWidget(time);
        info->addWidget(orderId);

        auto *right = new QVBoxLayout;
        right->setSpacing(10);
        right->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        auto *price = new QLabel("¥" + QString::number(record.price * record.quantity, 'f', 2));
        price->setObjectName("OrderPrice");
        price->setAlignment(Qt::AlignRight);
        auto *status = new QLabel(record.status.isEmpty() ? "未完成" : record.status);
        status->setProperty("class", "StatusPill");
        status->setObjectName(statusObjectName(status->text()));
        status->setAlignment(Qt::AlignCenter);
        auto *detail = new QPushButton("查看详情");
        detail->setObjectName("DetailButton");
        connect(detail, &QPushButton::clicked, this, [=]() {
            QMessageBox::information(this,
                                     "订单详情",
                                     "商品：" + displayName +
                                     "\n分类：" + (record.category.isEmpty() ? "校园好物" : record.category) +
                                     "\n数量：" + QString::number(record.quantity) +
                                     "\n金额：¥" + QString::number(record.price * record.quantity, 'f', 2) +
                                     "\n状态：" + (record.status.isEmpty() ? "未完成" : record.status) +
                                     "\n购买时间：" + compactTime(record.time));
        });
        right->addWidget(price);
        right->addWidget(status, 0, Qt::AlignRight);
        right->addWidget(detail);

        row->addWidget(thumb);
        row->addLayout(info, 1);
        row->addLayout(right);
        orderListLayout->addWidget(card);
    }
    orderListLayout->addStretch();
}
