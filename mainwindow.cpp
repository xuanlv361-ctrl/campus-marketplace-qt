#include <QAbstractButton>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include "mainwindow.h"
#include "accountstore.h"
#include "adminwindow.h"
#include "goodswindow.h"
#include "orderswindow.h"
#include "statswindow.h"
#include "userswindow.h"
#include "welcomewindow.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {
const char *kAdminQss = R"(
QMainWindow, QWidget#Root { background: #F4F5F8; color: #222; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#Sidebar { background: #FFFFFF; border-right: 1px solid #E8E8F0; }
QLabel#Logo { color: #7C3AED; font-size: 22px; font-weight: 900; }
QLabel#SideHint { color: #888; font-size: 12px; }
QPushButton[nav="true"] { text-align: left; padding-left: 18px; min-height: 42px; border-radius: 14px; border: none; background: #FFFFFF; color: #333; font-weight: 700; }
QPushButton[nav="true"]:hover { background: #F3EFFE; color: #7C3AED; }
QPushButton[active="true"] { background: #7C3AED; color: #FFFFFF; }
QFrame#Card { background: #FFFFFF; border: 1px solid #ECECF3; border-radius: 18px; }
QLabel#Title { font-size: 28px; font-weight: 900; color: #111; }
QLabel#Subtitle { color: #777; font-size: 14px; }
QLabel#Metric { color: #7C3AED; font-size: 28px; font-weight: 900; }
QPushButton#Primary { background: #7C3AED; color: #FFFFFF; border: none; border-radius: 12px; padding: 10px 18px; font-weight: 800; }
QPushButton#Primary:hover { background: #6D28D9; }
QPushButton#Danger { background: #FEE2E2; color: #DC2626; border: 1px solid #FECACA; border-radius: 12px; padding: 10px 18px; font-weight: 800; }
QPushButton#Danger:hover { background: #FECACA; }
QPushButton#Light { background: #FFFFFF; color: #444; border: 1px solid #DDDDE8; border-radius: 12px; padding: 10px 18px; font-weight: 700; }
QLineEdit, QComboBox, QSpinBox, QTextEdit { background: #FFFFFF; border: 1px solid #DDDDE8; border-radius: 10px; padding: 8px 10px; }
QTableWidget { background: #FFFFFF; border: none; gridline-color: #EEEEF5; selection-background-color: #EDE9FE; selection-color: #111; }
QHeaderView::section { background: #F7F7FA; border: none; border-bottom: 1px solid #E8E8F0; padding: 10px; font-weight: 800; }
)";

QTableWidgetItem *item(const QString &text)
{
    auto *it = new QTableWidgetItem(text);
    it->setTextAlignment(Qt::AlignCenter);
    return it;
}

QFrame *line()
{
    auto *f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("background:#EEEEF5; max-height:1px;");
    return f;
}

QFrame *metricCard(const QString &title, QLabel **valueLabel, const QString &desc = QString())
{
    auto *card = new QFrame;
    card->setObjectName("Card");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(6);

    *valueLabel = new QLabel("0");
    (*valueLabel)->setObjectName("Metric");
    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-weight: 900; color: #111;");
    layout->addWidget(*valueLabel);
    layout->addWidget(titleLabel);
    if (!desc.isEmpty()) {
        auto *descLabel = new QLabel(desc);
        descLabel->setObjectName("Subtitle");
        layout->addWidget(descLabel);
    }
    return card;
}

QWidget *embeddedAdminPage(QWidget *page, MainWindow *mainWindow)
{
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const auto sidebars = page->findChildren<QFrame *>("Sidebar");
    for (auto *sidebar : sidebars) {
        sidebar->hide();
        sidebar->setFixedWidth(0);
    }
    const auto buttons = page->findChildren<QPushButton *>();
    for (auto *button : buttons) {
        if (button->text().contains("返回管理员后台")) {
            QObject::disconnect(button, &QPushButton::clicked, page, nullptr);
            QObject::connect(button, &QPushButton::clicked, mainWindow, [mainWindow]() {
                mainWindow->showDashboard();
            });
        }
    }
    return page;
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(1180, 760);
    setMinimumSize(1040, 680);
    setWindowTitle("管理员后台");
    setStyleSheet(kAdminQss);

    auto *root = new QWidget;
    root->setObjectName("Root");
    auto *rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    rootLayout->addWidget(createSidebar());

    m_stack = new QStackedWidget;
    m_dashboardPage = createDashboardPage();
    m_goodsPage = createGoodsPage();
    m_ordersPage = createOrdersPage();
    m_usersPage = createUsersPage();
    m_statsPage = createStatsPage();

    m_stack->addWidget(m_dashboardPage);
    m_stack->addWidget(m_goodsPage);
    m_stack->addWidget(m_ordersPage);
    m_stack->addWidget(m_usersPage);
    m_stack->addWidget(m_statsPage);
    rootLayout->addWidget(m_stack, 1);

    setCentralWidget(root);
    refreshAll();
    switchPage(m_dashboardPage, m_btnDashboard);
}

MainWindow::~MainWindow() = default;

QPushButton *MainWindow::makeNavButton(const QString &text)
{
    auto *btn = new QPushButton(text);
    btn->setProperty("nav", true);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QPushButton *MainWindow::makePrimaryButton(const QString &text)
{
    auto *btn = new QPushButton(text);
    btn->setObjectName("Primary");
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QPushButton *MainWindow::makeDangerButton(const QString &text)
{
    auto *btn = new QPushButton(text);
    btn->setObjectName("Danger");
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QLabel *MainWindow::makeTitle(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName("Title");
    return label;
}

QLabel *MainWindow::makeSubtitle(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName("Subtitle");
    return label;
}

QWidget *MainWindow::makeCard()
{
    auto *card = new QFrame;
    card->setObjectName("Card");
    return card;
}

QWidget *MainWindow::createSidebar()
{
    auto *side = new QFrame;
    side->setObjectName("Sidebar");
    side->setFixedWidth(230);
    auto *layout = new QVBoxLayout(side);
    layout->setContentsMargins(20, 26, 20, 24);
    layout->setSpacing(12);

    auto *logo = new QLabel("校园二手后台");
    logo->setObjectName("Logo");
    auto *hint = new QLabel("Admin Management System");
    hint->setObjectName("SideHint");
    layout->addWidget(logo);
    layout->addWidget(hint);
    layout->addSpacing(18);

    m_btnDashboard = makeNavButton("管理员中心");
    m_btnGoods = makeNavButton("商品管理");
    m_btnOrders = makeNavButton("订单管理");
    m_btnUsers = makeNavButton("用户管理");
    m_btnStats = makeNavButton("数据统计");

    layout->addWidget(m_btnDashboard);
    layout->addWidget(m_btnGoods);
    layout->addWidget(m_btnOrders);
    layout->addWidget(m_btnUsers);
    layout->addWidget(m_btnStats);
    layout->addStretch();

    auto *home = new QPushButton("返回后台首页");
    home->setObjectName("Light");
    auto *logout = makeDangerButton("退出系统");
    layout->addWidget(home);
    layout->addWidget(logout);

    connect(m_btnDashboard, &QPushButton::clicked, this, &MainWindow::showDashboard);
    connect(m_btnGoods, &QPushButton::clicked, this, [=] { openGoodsWindow(false); });
    connect(m_btnOrders, &QPushButton::clicked, this, &MainWindow::openOrdersWindow);
    connect(m_btnUsers, &QPushButton::clicked, this, &MainWindow::openUsersWindow);
    connect(m_btnStats, &QPushButton::clicked, this, &MainWindow::openStatsWindow);
    connect(home, &QPushButton::clicked, this, &MainWindow::showDashboard);
    connect(logout, &QPushButton::clicked, this, &MainWindow::backToWelcome);

    return side;
}

QWidget *MainWindow::createDashboardPage()
{
    return embeddedAdminPage(new AdminWindow(this), this);
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(18);
    layout->addWidget(makeTitle("管理员中心"));
    layout->addWidget(makeSubtitle("统一后台入口。左侧菜单是全局导航，所有管理页面都在当前窗口内切换。"));

    auto *metricRow = new QHBoxLayout;
    auto makeMetric = [&](const QString &name, QLabel **valueLabel) {
        auto *card = makeCard();
        auto *v = new QVBoxLayout(card);
        v->setContentsMargins(22, 18, 22, 18);
        auto *nameLabel = new QLabel(name);
        nameLabel->setObjectName("Subtitle");
        *valueLabel = new QLabel("0");
        (*valueLabel)->setObjectName("Metric");
        v->addWidget(nameLabel);
        v->addWidget(*valueLabel);
        metricRow->addWidget(card);
    };
    makeMetric("商品总数", &m_totalGoodsLabel);
    makeMetric("待审核商品", &m_pendingGoodsLabel);
    makeMetric("订单总数", &m_orderCountLabel);
    makeMetric("用户总数", &m_userCountLabel);
    layout->addLayout(metricRow);

    auto *card = makeCard();
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 22, 24, 22);
    cardLayout->setSpacing(14);
    cardLayout->addWidget(new QLabel("快捷入口"));
    auto *btnRow = new QHBoxLayout;
    auto *goods = makePrimaryButton("进入商品管理");
    auto *orders = makePrimaryButton("进入订单管理");
    auto *users = makePrimaryButton("进入用户管理");
    auto *stats = makePrimaryButton("查看数据统计");
    btnRow->addWidget(goods);
    btnRow->addWidget(orders);
    btnRow->addWidget(users);
    btnRow->addWidget(stats);
    cardLayout->addLayout(btnRow);
    layout->addWidget(card);
    layout->addStretch();

    connect(goods, &QPushButton::clicked, this, [=] { openGoodsWindow(false); });
    connect(orders, &QPushButton::clicked, this, &MainWindow::openOrdersWindow);
    connect(users, &QPushButton::clicked, this, &MainWindow::openUsersWindow);
    connect(stats, &QPushButton::clicked, this, &MainWindow::openStatsWindow);
    return page;
}

QWidget *MainWindow::createGoodsPage()
{
    return embeddedAdminPage(new GoodsWindow(this, nullptr, false), this);
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(18);
    layout->addWidget(makeTitle("商品管理"));
    layout->addWidget(makeSubtitle("管理平台商品信息、库存状态、销售情况与审核状态。"));

    auto *metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    metrics->addWidget(metricCard("商品总数", &m_goodsTotalMetric, "平台商品数量"));
    metrics->addWidget(metricCard("已上架商品", &m_goodsOnlineMetric, "正常销售中"));
    metrics->addWidget(metricCard("待审核商品", &m_goodsPendingMetric, "等待管理员审核"));
    metrics->addWidget(metricCard("库存预警商品", &m_goodsLowStockMetric, "库存不足 10 件"));
    layout->addLayout(metrics);

    auto *panel = makeCard();
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(20, 20, 20, 20);
    panelLayout->setSpacing(14);

    auto *toolbar = new QHBoxLayout;
    m_goodsSearchEdit = new QLineEdit;
    m_goodsSearchEdit->setPlaceholderText("搜索商品名称 / ID / 分类 / 卖家");
    m_goodsCategoryCombo = new QComboBox;
    m_goodsCategoryCombo->addItems({"全部分类", "代步工具", "电子数码", "服饰鞋包", "美妆零食", "宿舍用品", "学习资料", "运动器材"});
    m_goodsStatusCombo = new QComboBox;
    m_goodsStatusCombo->addItems({"全部状态", "已上架", "待审核", "已驳回", "已下架"});
    auto *search = makePrimaryButton("查询");
    auto *reset = new QPushButton("重置"); reset->setObjectName("Light");
    auto *pending = makePrimaryButton("待审核商品");
    toolbar->addWidget(m_goodsSearchEdit, 1);
    toolbar->addWidget(m_goodsCategoryCombo);
    toolbar->addWidget(m_goodsStatusCombo);
    toolbar->addWidget(search);
    toolbar->addWidget(reset);
    toolbar->addWidget(pending);
    panelLayout->addLayout(toolbar);

    m_goodsTable = new QTableWidget;
    m_goodsTable->setColumnCount(9);
    m_goodsTable->setHorizontalHeaderLabels({"ID", "商品名称", "分类", "价格", "库存", "销量", "卖家", "成色", "状态"});
    m_goodsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_goodsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_goodsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    panelLayout->addWidget(m_goodsTable, 1);

    auto *actions = new QHBoxLayout;
    auto *add = makePrimaryButton("新增商品");
    auto *edit = makePrimaryButton("编辑商品");
    auto *approve = makePrimaryButton("审核通过");
    auto *reject = makeDangerButton("驳回商品");
    auto *del = makeDangerButton("删除商品");
    actions->addWidget(add);
    actions->addWidget(edit);
    actions->addWidget(approve);
    actions->addWidget(reject);
    actions->addWidget(del);
    actions->addStretch();
    panelLayout->addLayout(actions);
    layout->addWidget(panel, 1);

    connect(search, &QPushButton::clicked, this, [=] { refreshGoodsTable(false); });
    connect(reset, &QPushButton::clicked, this, [=] {
        m_goodsSearchEdit->clear();
        m_goodsCategoryCombo->setCurrentIndex(0);
        m_goodsStatusCombo->setCurrentIndex(0);
        refreshGoodsTable(false);
    });
    connect(pending, &QPushButton::clicked, this, [=] { refreshGoodsTable(true); });
    connect(add, &QPushButton::clicked, this, &MainWindow::addGoods);
    connect(edit, &QPushButton::clicked, this, &MainWindow::editGoods);
    connect(approve, &QPushButton::clicked, this, &MainWindow::approveSelectedGoods);
    connect(reject, &QPushButton::clicked, this, &MainWindow::rejectSelectedGoods);
    connect(del, &QPushButton::clicked, this, &MainWindow::deleteGoods);
    return page;
}

QWidget *MainWindow::createOrdersPage()
{
    return embeddedAdminPage(new OrdersWindow(this), this);
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(18);
    layout->addWidget(makeTitle("订单管理"));
    layout->addWidget(makeSubtitle("查看交易订单、成交价格与订单处理状态。"));

    auto *metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    metrics->addWidget(metricCard("订单总数", &m_ordersTotalMetric, "累计交易订单"));
    metrics->addWidget(metricCard("未完成订单", &m_ordersPendingMetric, "等待处理"));
    metrics->addWidget(metricCard("已完成订单", &m_ordersDoneMetric, "已完成交易"));
    metrics->addWidget(metricCard("已取消订单", &m_ordersCancelMetric, "已取消交易"));
    layout->addLayout(metrics);

    auto *panel = makeCard();
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(20, 20, 20, 20);
    panelLayout->setSpacing(14);

    auto *filterBar = new QHBoxLayout;
    m_ordersSearchEdit = new QLineEdit;
    m_ordersSearchEdit->setPlaceholderText("搜索订单ID、商品名称或买家");
    m_ordersStatusCombo = new QComboBox;
    m_ordersStatusCombo->addItems({"全部状态", "未完成", "已完成", "已取消"});
    auto *query = makePrimaryButton("查询");
    auto *reset = new QPushButton("重置"); reset->setObjectName("Light");
    filterBar->addWidget(m_ordersSearchEdit, 1);
    filterBar->addWidget(m_ordersStatusCombo);
    filterBar->addWidget(query);
    filterBar->addWidget(reset);
    panelLayout->addLayout(filterBar);

    auto *actions = new QHBoxLayout;
    auto *add = makePrimaryButton("新增订单");
    auto *edit = makePrimaryButton("修改订单");
    auto *exportBtn = new QPushButton("导出订单"); exportBtn->setObjectName("Light");
    auto *del = makeDangerButton("删除订单");
    actions->addWidget(add);
    actions->addWidget(edit);
    actions->addWidget(exportBtn);
    actions->addStretch();
    actions->addWidget(del);
    panelLayout->addLayout(actions);

    m_ordersTable = new QTableWidget;
    m_ordersTable->setColumnCount(7);
    m_ordersTable->setHorizontalHeaderLabels({"订单ID", "商品名称", "买家", "成交价格", "数量", "状态", "购买时间"});
    m_ordersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    panelLayout->addWidget(m_ordersTable, 1);
    layout->addWidget(panel, 1);

    connect(add, &QPushButton::clicked, this, &MainWindow::addOrder);
    connect(edit, &QPushButton::clicked, this, &MainWindow::editOrder);
    connect(del, &QPushButton::clicked, this, &MainWindow::deleteOrder);
    connect(exportBtn, &QPushButton::clicked, this, [=] { QMessageBox::information(this, "导出订单", "订单数据已准备导出。"); });
    connect(query, &QPushButton::clicked, this, &MainWindow::refreshOrdersTable);
    connect(reset, &QPushButton::clicked, this, [=] {
        m_ordersSearchEdit->clear();
        m_ordersStatusCombo->setCurrentIndex(0);
        refreshOrdersTable();
    });
    return page;
}

QWidget *MainWindow::createUsersPage()
{
    return embeddedAdminPage(new UsersWindow(this), this);
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(18);
    layout->addWidget(makeTitle("用户管理"));
    layout->addWidget(makeSubtitle("维护平台用户信息、账号状态与偏好分类。"));

    auto *metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    metrics->addWidget(metricCard("用户总数", &m_usersTotalMetric, "平台注册用户"));
    metrics->addWidget(metricCard("正常用户", &m_usersNormalMetric, "可正常交易"));
    metrics->addWidget(metricCard("禁用用户", &m_usersDisabledMetric, "已限制登录"));
    metrics->addWidget(metricCard("管理员数量", &m_usersAdminMetric, "后台管理账号"));
    layout->addLayout(metrics);

    auto *panel = makeCard();
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(20, 20, 20, 20);
    panelLayout->setSpacing(14);

    auto *filterBar = new QHBoxLayout;
    m_usersSearchEdit = new QLineEdit;
    m_usersSearchEdit->setPlaceholderText("搜索用户名或手机号");
    m_usersRoleCombo = new QComboBox;
    m_usersRoleCombo->addItems({"全部角色", "管理员", "普通用户"});
    m_usersStatusCombo = new QComboBox;
    m_usersStatusCombo->addItems({"全部状态", "正常", "禁用"});
    auto *query = makePrimaryButton("查询");
    auto *reset = new QPushButton("重置"); reset->setObjectName("Light");
    filterBar->addWidget(m_usersSearchEdit, 1);
    filterBar->addWidget(m_usersRoleCombo);
    filterBar->addWidget(m_usersStatusCombo);
    filterBar->addWidget(query);
    filterBar->addWidget(reset);
    panelLayout->addLayout(filterBar);

    auto *actions = new QHBoxLayout;
    auto *add = makePrimaryButton("新增用户");
    auto *edit = makePrimaryButton("修改用户");
    auto *recommend = new QPushButton("推荐商品"); recommend->setObjectName("Light");
    auto *del = makeDangerButton("删除用户");
    actions->addWidget(add);
    actions->addWidget(edit);
    actions->addWidget(recommend);
    actions->addStretch();
    actions->addWidget(del);
    panelLayout->addLayout(actions);

    m_usersTable = new QTableWidget;
    m_usersTable->setColumnCount(6);
    m_usersTable->setHorizontalHeaderLabels({"用户ID", "用户名", "手机号", "角色", "状态", "喜好品类"});
    m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    panelLayout->addWidget(m_usersTable, 1);
    layout->addWidget(panel, 1);

    connect(add, &QPushButton::clicked, this, &MainWindow::addUser);
    connect(edit, &QPushButton::clicked, this, &MainWindow::editUser);
    connect(del, &QPushButton::clicked, this, &MainWindow::deleteUser);
    connect(recommend, &QPushButton::clicked, this, &MainWindow::recommendGoodsForUser);
    connect(query, &QPushButton::clicked, this, &MainWindow::refreshUsersTable);
    connect(reset, &QPushButton::clicked, this, [=] {
        m_usersSearchEdit->clear();
        m_usersRoleCombo->setCurrentIndex(0);
        m_usersStatusCombo->setCurrentIndex(0);
        refreshUsersTable();
    });
    return page;
}

QWidget *MainWindow::createStatsPage()
{
    return embeddedAdminPage(new StatsWindow(this), this);
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(18);
    layout->addWidget(makeTitle("数据统计"));
    layout->addWidget(makeSubtitle("查看平台商品、用户、订单与分类销售数据。"));

    auto *metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    metrics->addWidget(metricCard("商品总数", &m_statsGoodsMetric));
    metrics->addWidget(metricCard("用户总数", &m_statsUsersMetric));
    metrics->addWidget(metricCard("订单总数", &m_statsOrdersMetric));
    metrics->addWidget(metricCard("已完成订单数", &m_statsDoneMetric));
    layout->addLayout(metrics);

    auto *summaryCard = makeCard();
    auto *summaryLayout = new QVBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(20, 18, 20, 18);
    summaryLayout->setSpacing(8);
    auto *summaryTitle = new QLabel("分类销售概览");
    summaryTitle->setStyleSheet("font-size: 18px; font-weight: 900; color: #111;");
    m_statsSummaryLabel = new QLabel;
    m_statsSummaryLabel->setObjectName("Subtitle");
    summaryLayout->addWidget(summaryTitle);
    summaryLayout->addWidget(m_statsSummaryLabel);
    layout->addWidget(summaryCard);

    auto *tableCard = makeCard();
    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(20, 20, 20, 20);
    tableLayout->setSpacing(14);
    m_statsTable = new QTableWidget;
    m_statsTable->setColumnCount(6);
    m_statsTable->setHorizontalHeaderLabels({"分类", "商品数", "库存", "销量", "销售额", "待审核"});
    m_statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableLayout->addWidget(m_statsTable, 1);
    auto *refresh = makePrimaryButton("刷新统计");
    tableLayout->addWidget(refresh, 0, Qt::AlignLeft);
    layout->addWidget(tableCard, 1);
    connect(refresh, &QPushButton::clicked, this, &MainWindow::refreshStatsPage);
    return page;
}

void MainWindow::switchPage(QWidget *page, QPushButton *activeButton)
{
    if (!m_stack || !page) return;
    refreshAll();
    m_stack->setCurrentWidget(page);
    updateSidebarHighlight(activeButton);
}

void MainWindow::updateSidebarHighlight(QPushButton *activeButton)
{
    const QList<QPushButton *> buttons = {m_btnDashboard, m_btnGoods, m_btnOrders, m_btnUsers, m_btnStats};
    for (auto *btn : buttons) {
        btn->setProperty("active", btn == activeButton);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

void MainWindow::showDashboard() { switchPage(m_dashboardPage, m_btnDashboard); }
void MainWindow::openGoodsWindow(bool showPending) { switchPage(m_goodsPage, m_btnGoods); refreshGoodsTable(showPending); }
void MainWindow::openOrdersWindow() { switchPage(m_ordersPage, m_btnOrders); }
void MainWindow::openUsersWindow() { switchPage(m_usersPage, m_btnUsers); }
void MainWindow::openStatsWindow() { switchPage(m_statsPage, m_btnStats); }

void MainWindow::backToWelcome()
{
    auto *welcome = new WelcomeWindow;
    welcome->setAttribute(Qt::WA_DeleteOnClose);
    welcome->show();
    close();
}

void MainWindow::refreshAll()
{
    refreshDashboard();
    refreshGoodsTable(false);
    refreshOrdersTable();
    refreshUsersTable();
    refreshStatsPage();
}

void MainWindow::refreshDashboard()
{
    const auto &goods = AccountStore::goods();
    const auto &orders = AccountStore::orders();
    const auto &users = AccountStore::users();
    int pending = 0;
    for (const auto &g : goods) if (g.status == "待审核") ++pending;
    if (m_totalGoodsLabel) m_totalGoodsLabel->setText(QString::number(goods.size()));
    if (m_pendingGoodsLabel) m_pendingGoodsLabel->setText(QString::number(pending));
    if (m_orderCountLabel) m_orderCountLabel->setText(QString::number(orders.size()));
    if (m_userCountLabel) m_userCountLabel->setText(QString::number(users.size()));
}

void MainWindow::refreshGoodsTable(bool pendingOnly)
{
    if (!m_goodsTable) return;
    const QString keyword = m_goodsSearchEdit ? m_goodsSearchEdit->text().trimmed() : QString();
    const QString category = m_goodsCategoryCombo ? m_goodsCategoryCombo->currentText() : QString("全部分类");
    const QString status = m_goodsStatusCombo ? m_goodsStatusCombo->currentText() : QString("全部状态");
    m_goodsTable->setRowCount(0);
    const auto &goods = AccountStore::goods();

    int online = 0, pending = 0, lowStock = 0;
    for (const auto &g : goods) {
        if (g.status == "已上架") ++online;
        if (g.status == "待审核") ++pending;
        if (g.stock < 10) ++lowStock;
    }
    if (m_goodsTotalMetric) m_goodsTotalMetric->setText(QString::number(goods.size()));
    if (m_goodsOnlineMetric) m_goodsOnlineMetric->setText(QString::number(online));
    if (m_goodsPendingMetric) m_goodsPendingMetric->setText(QString::number(pending));
    if (m_goodsLowStockMetric) m_goodsLowStockMetric->setText(QString::number(lowStock));

    for (const auto &g : goods) {
        if (pendingOnly && g.status != "待审核") continue;
        if (status != "全部状态" && g.status != status) continue;
        if (category != "全部分类" && g.category != category && !g.category.contains(category)) continue;
        if (!keyword.isEmpty() && !g.name.contains(keyword, Qt::CaseInsensitive)
            && !g.id.contains(keyword, Qt::CaseInsensitive)
            && !g.category.contains(keyword, Qt::CaseInsensitive)
            && !g.seller.contains(keyword, Qt::CaseInsensitive)) continue;
        const int row = m_goodsTable->rowCount();
        m_goodsTable->insertRow(row);
        m_goodsTable->setItem(row, 0, item(g.id));
        m_goodsTable->setItem(row, 1, item(g.name));
        m_goodsTable->setItem(row, 2, item(g.category));
        m_goodsTable->setItem(row, 3, item(QString::number(g.price, 'f', 2)));
        m_goodsTable->setItem(row, 4, item(QString::number(g.stock)));
        m_goodsTable->setItem(row, 5, item(QString::number(g.sales)));
        m_goodsTable->setItem(row, 6, item(g.seller));
        m_goodsTable->setItem(row, 7, item(g.condition));
        m_goodsTable->setItem(row, 8, item(g.status));
    }
}

void MainWindow::refreshOrdersTable()
{
    if (!m_ordersTable) return;
    m_ordersTable->setRowCount(0);
    const auto &orders = AccountStore::orders();
    const QString keyword = m_ordersSearchEdit ? m_ordersSearchEdit->text().trimmed() : QString();
    const QString statusFilter = m_ordersStatusCombo ? m_ordersStatusCombo->currentText() : QString("全部状态");

    int pending = 0, done = 0, cancel = 0;
    for (const auto &o : orders) {
        if (o.status.contains("取消")) ++cancel;
        else if (o.status.contains("完成") && !o.status.contains("未完成")) ++done;
        else ++pending;

        if (statusFilter != "全部状态" && !o.status.contains(statusFilter)) continue;
        if (!keyword.isEmpty() && !o.id.contains(keyword, Qt::CaseInsensitive)
            && !o.goodsName.contains(keyword, Qt::CaseInsensitive)
            && !o.buyer.contains(keyword, Qt::CaseInsensitive)) continue;

        const int row = m_ordersTable->rowCount();
        m_ordersTable->insertRow(row);
        m_ordersTable->setItem(row, 0, item(o.id));
        m_ordersTable->setItem(row, 1, item(o.goodsName));
        m_ordersTable->setItem(row, 2, item(o.buyer));
        m_ordersTable->setItem(row, 3, item(QString::number(o.price * o.quantity, 'f', 2)));
        m_ordersTable->setItem(row, 4, item(QString::number(o.quantity)));
        m_ordersTable->setItem(row, 5, item(o.status));
        m_ordersTable->setItem(row, 6, item(o.time));
    }
    if (m_ordersTotalMetric) m_ordersTotalMetric->setText(QString::number(orders.size()));
    if (m_ordersPendingMetric) m_ordersPendingMetric->setText(QString::number(pending));
    if (m_ordersDoneMetric) m_ordersDoneMetric->setText(QString::number(done));
    if (m_ordersCancelMetric) m_ordersCancelMetric->setText(QString::number(cancel));
}

void MainWindow::refreshUsersTable()
{
    if (!m_usersTable) return;
    m_usersTable->setRowCount(0);
    const auto &users = AccountStore::users();
    const QString keyword = m_usersSearchEdit ? m_usersSearchEdit->text().trimmed() : QString();
    const QString roleFilter = m_usersRoleCombo ? m_usersRoleCombo->currentText() : QString("全部角色");
    const QString statusFilter = m_usersStatusCombo ? m_usersStatusCombo->currentText() : QString("全部状态");

    int normal = 0, disabled = 0, admins = 0;
    for (int i = 0; i < users.size(); ++i) {
        const QString role = users[i].username.startsWith("admin", Qt::CaseInsensitive) ? "管理员" : "普通用户";
        const QString status = "正常";
        const QString preference = role == "管理员" ? "数据统计" : "校园好物";
        if (status == "正常") ++normal; else ++disabled;
        if (role == "管理员") ++admins;

        if (roleFilter != "全部角色" && role != roleFilter) continue;
        if (statusFilter != "全部状态" && status != statusFilter) continue;
        if (!keyword.isEmpty() && !users[i].username.contains(keyword, Qt::CaseInsensitive)
            && !users[i].phone.contains(keyword, Qt::CaseInsensitive)) continue;

        const int row = m_usersTable->rowCount();
        m_usersTable->insertRow(row);
        m_usersTable->setItem(row, 0, item(QString("U%1").arg(i + 1, 3, 10, QChar('0'))));
        m_usersTable->setItem(row, 1, item(users[i].username));
        m_usersTable->setItem(row, 2, item(users[i].phone));
        m_usersTable->setItem(row, 3, item(role));
        m_usersTable->setItem(row, 4, item(status));
        m_usersTable->setItem(row, 5, item(preference));
    }
    if (m_usersTotalMetric) m_usersTotalMetric->setText(QString::number(users.size()));
    if (m_usersNormalMetric) m_usersNormalMetric->setText(QString::number(normal));
    if (m_usersDisabledMetric) m_usersDisabledMetric->setText(QString::number(disabled));
    if (m_usersAdminMetric) m_usersAdminMetric->setText(QString::number(admins));
}

void MainWindow::refreshStatsPage()
{
    if (!m_statsTable) return;
    struct S { int count = 0; int stock = 0; int sales = 0; int pending = 0; double amount = 0; };
    QMap<QString, S> stats;
    const QStringList categories = {"代步工具", "电子数码", "服饰鞋包", "美妆零食", "宿舍用品", "学习资料", "运动器材"};
    auto normalizedCategory = [](const QString &category) {
        const QString text = category.trimmed();
        if (text == "代步工具" || text.contains("代步")) return QString("代步工具");
        if (text == "电子数码" || text.contains("数码") || text.contains("电子")) return QString("电子数码");
        if (text == "服饰鞋包" || text.contains("服饰") || text.contains("鞋") || text.contains("包")) return QString("服饰鞋包");
        if (text == "美妆零食" || text.contains("美妆") || text.contains("个护") || text.contains("食品") || text.contains("零食")) return QString("美妆零食");
        if (text == "宿舍用品" || text.contains("宿舍") || text.contains("生活")) return QString("宿舍用品");
        if (text == "学习资料" || text.contains("教材") || text.contains("书籍") || text.contains("学习") || text.contains("办公")) return QString("学习资料");
        if (text == "运动器材" || text.contains("运动")) return QString("运动器材");
        return QString("宿舍用品");
    };
    for (const QString &category : categories) {
        stats[category] = S{};
    }
    double total = 0;
    int pending = 0;
    for (const auto &g : AccountStore::goods()) {
        auto &s = stats[normalizedCategory(g.category)];
        s.count++;
        s.stock += g.stock;
        s.sales += g.sales;
        s.amount += g.sales * g.price;
        total += g.sales * g.price;
        if (g.status == "待审核") {
            ++pending;
            ++s.pending;
        }
    }
    int completedOrders = 0;
    for (const auto &o : AccountStore::orders()) {
        if (o.status.contains("完成") && !o.status.contains("未完成")) ++completedOrders;
    }
    if (m_statsGoodsMetric) m_statsGoodsMetric->setText(QString::number(AccountStore::goods().size()));
    if (m_statsUsersMetric) m_statsUsersMetric->setText(QString::number(AccountStore::users().size()));
    if (m_statsOrdersMetric) m_statsOrdersMetric->setText(QString::number(AccountStore::orders().size()));
    if (m_statsDoneMetric) m_statsDoneMetric->setText(QString::number(completedOrders));
    m_statsSummaryLabel->setText(QString("总销售额：￥%1    待审核商品：%2    分类数量：%3")
                                     .arg(total, 0, 'f', 2)
                                     .arg(pending)
                                     .arg(stats.size()));
    m_statsTable->setRowCount(0);
    for (auto it = stats.constBegin(); it != stats.constEnd(); ++it) {
        const int row = m_statsTable->rowCount();
        m_statsTable->insertRow(row);
        m_statsTable->setItem(row, 0, item(it.key()));
        m_statsTable->setItem(row, 1, item(QString::number(it.value().count)));
        m_statsTable->setItem(row, 2, item(QString::number(it.value().stock)));
        m_statsTable->setItem(row, 3, item(QString::number(it.value().sales)));
        m_statsTable->setItem(row, 4, item(QString::number(it.value().amount, 'f', 2)));
        m_statsTable->setItem(row, 5, item(QString::number(it.value().pending)));
    }
}

int MainWindow::selectedGoodsIndex() const
{
    if (!m_goodsTable || m_goodsTable->currentRow() < 0) return -1;
    const QString id = m_goodsTable->item(m_goodsTable->currentRow(), 0)->text();
    auto &goods = AccountStore::goods();
    for (int i = 0; i < goods.size(); ++i) if (goods[i].id == id) return i;
    return -1;
}

int MainWindow::selectedOrderIndex() const
{
    if (!m_ordersTable || m_ordersTable->currentRow() < 0 || !m_ordersTable->item(m_ordersTable->currentRow(), 0)) return -1;
    const QString id = m_ordersTable->item(m_ordersTable->currentRow(), 0)->text();
    auto &orders = AccountStore::orders();
    for (int i = 0; i < orders.size(); ++i) if (orders[i].id == id) return i;
    return -1;
}

int MainWindow::selectedUserIndex() const
{
    if (!m_usersTable || m_usersTable->currentRow() < 0 || !m_usersTable->item(m_usersTable->currentRow(), 1)) return -1;
    const QString username = m_usersTable->item(m_usersTable->currentRow(), 1)->text();
    auto &users = AccountStore::users();
    for (int i = 0; i < users.size(); ++i) if (users[i].username == username) return i;
    return -1;
}

void MainWindow::addOrder()
{
    bool ok = false;
    const QString id = QInputDialog::getText(this, "新增订单", "订单ID：", QLineEdit::Normal, "", &ok);
    if (!ok || id.trimmed().isEmpty()) return;
    const QString goodsName = QInputDialog::getText(this, "新增订单", "商品名称：", QLineEdit::Normal, "", &ok);
    if (!ok || goodsName.trimmed().isEmpty()) return;
    const QString buyer = QInputDialog::getText(this, "新增订单", "买家：", QLineEdit::Normal, "", &ok);
    if (!ok || buyer.trimmed().isEmpty()) return;
    const double price = QInputDialog::getDouble(this, "新增订单", "成交价格：", 1.00, 0.01, 999999.00, 2, &ok);
    if (!ok) return;
    const QString status = QInputDialog::getItem(this, "新增订单", "状态：", {"未完成", "已完成", "已取消"}, 0, false, &ok);
    if (!ok || status.trimmed().isEmpty()) return;
    AccountStore::orders().push_back({id, goodsName, buyer, price, 1, status, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")});
    refreshAll();
}

void MainWindow::editOrder()
{
    const int idx = selectedOrderIndex();
    if (idx < 0) { QMessageBox::warning(this, "提示", "请先选中要修改的订单。"); return; }
    auto &order = AccountStore::orders()[idx];
    bool ok = false;
    const QString goodsName = QInputDialog::getText(this, "修改订单", "商品名称：", QLineEdit::Normal, order.goodsName, &ok);
    if (!ok || goodsName.trimmed().isEmpty()) return;
    const QString buyer = QInputDialog::getText(this, "修改订单", "买家：", QLineEdit::Normal, order.buyer, &ok);
    if (!ok || buyer.trimmed().isEmpty()) return;
    const double price = QInputDialog::getDouble(this, "修改订单", "成交价格：", order.price * order.quantity, 0.01, 999999.00, 2, &ok);
    if (!ok) return;
    const QString status = QInputDialog::getItem(this, "修改订单", "状态：", {"未完成", "已完成", "已取消"}, 0, false, &ok);
    if (!ok || status.trimmed().isEmpty()) return;
    order.goodsName = goodsName;
    order.buyer = buyer;
    order.price = price / qMax(1, order.quantity);
    order.status = status;
    refreshAll();
}

void MainWindow::deleteOrder()
{
    const int idx = selectedOrderIndex();
    if (idx < 0) { QMessageBox::warning(this, "提示", "请先选中要删除的订单。"); return; }
    if (QMessageBox::question(this, "确认删除", "确定删除当前选中的订单吗？") == QMessageBox::Yes) {
        AccountStore::orders().remove(idx);
        refreshAll();
    }
}

void MainWindow::addUser()
{
    bool ok = false;
    const QString username = QInputDialog::getText(this, "新增用户", "用户名：", QLineEdit::Normal, "", &ok);
    if (!ok || username.trimmed().isEmpty()) return;
    const QString password = QInputDialog::getText(this, "新增用户", "密码：", QLineEdit::Normal, "123456", &ok);
    if (!ok || password.trimmed().isEmpty()) return;
    const QString phone = QInputDialog::getText(this, "新增用户", "手机号：", QLineEdit::Normal, "", &ok);
    if (!ok || phone.trimmed().isEmpty()) return;
    if (!AccountStore::addUser(username, password, phone)) {
        QMessageBox::warning(this, "新增失败", "新增用户失败，可能账号已存在或数据库连接异常。");
        return;
    }
    refreshAll();
}

void MainWindow::editUser()
{
    const int idx = selectedUserIndex();
    if (idx < 0) { QMessageBox::warning(this, "提示", "请先选中要修改的用户。"); return; }
    auto &user = AccountStore::users()[idx];
    bool ok = false;
    const QString username = QInputDialog::getText(this, "修改用户", "用户名：", QLineEdit::Normal, user.username, &ok);
    if (!ok || username.trimmed().isEmpty()) return;
    const QString phone = QInputDialog::getText(this, "修改用户", "手机号：", QLineEdit::Normal, user.phone, &ok);
    if (!ok || phone.trimmed().isEmpty()) return;
    user.username = username;
    user.phone = phone;
    refreshAll();
}

void MainWindow::deleteUser()
{
    const int idx = selectedUserIndex();
    if (idx < 0) { QMessageBox::warning(this, "提示", "请先选中要删除的用户。"); return; }
    if (QMessageBox::question(this, "确认删除", "确定删除当前选中的用户吗？") == QMessageBox::Yes) {
        AccountStore::users().remove(idx);
        refreshAll();
    }
}

void MainWindow::recommendGoodsForUser()
{
    const int idx = selectedUserIndex();
    if (idx < 0) { QMessageBox::warning(this, "提示", "请先选中一个用户。"); return; }
    QMessageBox::information(this, "推荐商品", "可优先为 " + AccountStore::users()[idx].username + " 推荐校园热门商品。");
}

void MainWindow::addGoods()
{
    QDialog dialog(this);
    dialog.setWindowTitle("新增商品");
    auto *form = new QFormLayout(&dialog);
    QLineEdit name, category, seller, condition;
    QDoubleSpinBox price; price.setMaximum(999999); price.setDecimals(2);
    QSpinBox stock; stock.setRange(1, 99999);
    QTextEdit desc;
    seller.setText(AccountStore::users().isEmpty() ? "user1" : AccountStore::users().first().username);
    condition.setText("九成新");
    form->addRow("商品名称", &name);
    form->addRow("分类", &category);
    form->addRow("价格", &price);
    form->addRow("库存", &stock);
    form->addRow("卖家账号", &seller);
    form->addRow("成色", &condition);
    form->addRow("描述", &desc);
    auto *ok = makePrimaryButton("提交为待审核");
    form->addRow(ok);
    connect(ok, &QPushButton::clicked, &dialog, &QDialog::accept);
    if (dialog.exec() == QDialog::Accepted) {
        if (!AccountStore::submitGoodsForReview(name.text(), category.text(), price.value(), stock.value(), seller.text(), desc.toPlainText(), condition.text())) {
            QMessageBox::warning(this, "失败", "新增失败，请检查卖家账号是否存在、数据库是否连接。 ");
            return;
        }
        refreshAll();
    }
}

void MainWindow::editGoods()
{
    const int idx = selectedGoodsIndex();
    if (idx < 0) { QMessageBox::information(this, "提示", "请先选择一行商品。 "); return; }
    auto &g = AccountStore::goods()[idx];
    const QString status = QInputDialog::getItem(this, "修改状态", "商品状态", {"已上架", "待审核", "已驳回"}, 0, false);
    if (!status.isEmpty()) {
        AccountStore::updateGoodsStatus(g.id, status);
        refreshAll();
    }
}

void MainWindow::deleteGoods()
{
    const int idx = selectedGoodsIndex();
    if (idx < 0) { QMessageBox::information(this, "提示", "请先选择一行商品。 "); return; }
    auto &g = AccountStore::goods()[idx];
    if (QMessageBox::question(this, "确认", "确定删除/驳回该商品吗？") == QMessageBox::Yes) {
        AccountStore::updateGoodsStatus(g.id, "已驳回");
        refreshAll();
    }
}

void MainWindow::approveSelectedGoods()
{
    const int idx = selectedGoodsIndex();
    if (idx < 0) { QMessageBox::information(this, "提示", "请先选择一行商品。 "); return; }
    AccountStore::updateGoodsStatus(AccountStore::goods()[idx].id, "已上架");
    refreshAll();
}

void MainWindow::rejectSelectedGoods()
{
    const int idx = selectedGoodsIndex();
    if (idx < 0) { QMessageBox::information(this, "提示", "请先选择一行商品。 "); return; }
    AccountStore::updateGoodsStatus(AccountStore::goods()[idx].id, "已驳回");
    refreshAll();
}
