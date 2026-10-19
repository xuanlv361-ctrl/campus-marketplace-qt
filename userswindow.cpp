#include "userswindow.h"
#include "accountstore.h"
#include "languagemanager.h"
#include "mainwindow.h"
#include "welcomewindow.h"

#include <QAbstractItemView>
#include <algorithm>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

const char *kUsersQss = R"(
QWidget { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#Sidebar, QFrame#Panel, QFrame#MetricCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 22px; }
QLabel#Brand { color: #111111; font-size: 22px; font-weight: 900; }
QLabel#SideHint, QLabel#Desc, QLabel#MetricDesc { color: #666666; font-size: 13px; }
QLabel#Title { color: #111111; font-size: 34px; font-weight: 900; }
QLabel#MetricValue { color: #5B4BFF; font-size: 30px; font-weight: 900; }
QLabel#MetricTitle { color: #111111; font-size: 15px; font-weight: 900; }
QLabel#Crumb { color: #666666; font-size: 13px; }
QPushButton { min-height: 44px; border-radius: 16px; font-size: 14px; font-weight: 800; padding: 0 16px; }
QPushButton#PrimaryButton { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; }
QPushButton#PrimaryButton:hover { background: #493CE6; }
QPushButton#DangerButton, QPushButton#ExitButton { background: #FFF1F1; color: #E5484D; border: 1px solid #FFD1D1; }
QPushButton#DangerButton:hover, QPushButton#ExitButton:hover { background: #FFE8E8; }
QPushButton#SecondaryButton, QPushButton#SidebarButton { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; }
QPushButton#SecondaryButton:hover, QPushButton#SidebarButton:hover { background: #F7F7F7; border-color: #5B4BFF; }
QPushButton#SidebarButton, QPushButton#SidebarPrimary, QPushButton#ExitButton { text-align: left; }
QPushButton#SidebarPrimary { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; }
QLineEdit, QComboBox { background:#FFFFFF; border:1px solid #EAEAEA; border-radius:16px; min-height:42px; padding:0 14px; }
QLineEdit:focus, QComboBox:focus { border-color:#5B4BFF; }
QTableWidget { background: #FFFFFF; border: none; gridline-color: #EAEAEA; alternate-background-color: #FAFAFA; selection-background-color: #F1F0FF; selection-color: #111111; color: #222222; }
QHeaderView::section { background: #F7F7F7; color: #111111; border: none; border-right: 1px solid #EAEAEA; padding: 12px 8px; font-weight: 900; }
QLabel#StatusOk { background: #EAF7EC; color: #2E7D32; border-radius: 11px; padding: 3px 10px; font-weight: 800; }
QLabel#StatusBad { background: #FFF1F1; color: #E5484D; border-radius: 11px; padding: 3px 10px; font-weight: 800; }
)";

void addShadow(QWidget *widget)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(20, 20, 20, 12));
    widget->setGraphicsEffect(shadow);
}

QPushButton *sidebarButton(const QString &text, const char *name = "SidebarButton")
{
    auto *button = new QPushButton(text);
    button->setObjectName(name);
    return button;
}

QFrame *createSidebar(const QString &active, MainWindow *mainWindow, QWidget *self)
{
    auto *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(260);
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
    const QStringList items = {"🏠  管理员中心", "📦  商品管理", "🧾  订单管理", "👥  用户管理", "📊  数据统计"};
    for (const QString &item : items) {
        const QString plain = item.mid(4).trimmed();
        auto *btn = sidebarButton(item, plain == active ? "SidebarPrimary" : "SidebarButton");
        if (plain == "管理员中心") {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->showDashboard(); self->hide(); });
        }
        else if (plain == "商品管理") {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openGoodsWindow(false); self->hide(); });
        }
        else if (plain == "订单管理") {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openOrdersWindow(); self->hide(); });
        }
        else if (plain == "数据统计") {
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
    label->setObjectName(status == "禁用" ? "StatusBad" : "StatusOk");
    return label;
}

bool isValidPhone(const QString &phone)
{
    if (phone.size() != 11) return false;
    for (const QChar &ch : phone) if (!ch.isDigit()) return false;
    return true;
}

bool editUserDialog(QWidget *parent, const QString &title, const UserItem &initial, UserItem *result)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.resize(520, 390);

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);

    auto *usernameEdit = new QLineEdit(initial.username);
    auto *passwordEdit = new QLineEdit(initial.password);
    passwordEdit->setEchoMode(QLineEdit::Password);
    auto *emailEdit = new QLineEdit(initial.email);
    auto *phoneEdit = new QLineEdit(initial.phone);

    auto *roleBox = new QComboBox;
    const QStringList roles = {"普通用户", "管理员"};
    roleBox->addItems(roles);
    const int roleIndex = roles.indexOf(initial.role);
    roleBox->setCurrentIndex(roleIndex >= 0 ? roleIndex : 0);

    auto *statusBox = new QComboBox;
    const QStringList statuses = {"正常", "禁用"};
    statusBox->addItems(statuses);
    const int statusIndex = statuses.indexOf(initial.status);
    statusBox->setCurrentIndex(statusIndex >= 0 ? statusIndex : 0);

    auto *preferenceBox = new QComboBox;
    const QStringList categories = {"书籍教材", "学习资料", "电子数码", "宿舍用品", "服饰鞋包", "运动器材", "代步工具", "美妆零食"};
    preferenceBox->addItems(categories);
    const int preferenceIndex = categories.indexOf(initial.preference);
    preferenceBox->setCurrentIndex(preferenceIndex >= 0 ? preferenceIndex : 0);

    form->addRow("用户名", usernameEdit);
    form->addRow("密码", passwordEdit);
    form->addRow("邮箱", emailEdit);
    form->addRow("手机号", phoneEdit);
    form->addRow("角色", roleBox);
    form->addRow("状态", statusBox);
    form->addRow("喜好品类", preferenceBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("确定");
    buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (usernameEdit->text().trimmed().isEmpty() || passwordEdit->text().trimmed().isEmpty() ||
            emailEdit->text().trimmed().isEmpty() || phoneEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "用户名、密码、邮箱和手机号不能为空。");
            return;
        }
        if (!isValidPhone(phoneEdit->text().trimmed())) {
            QMessageBox::warning(&dialog, "提示", "请输入 11 位手机号。");
            return;
        }
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addLayout(form);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    *result = initial;
    result->username = usernameEdit->text().trimmed();
    result->password = passwordEdit->text().trimmed();
    result->email = emailEdit->text().trimmed();
    result->phone = phoneEdit->text().trimmed();
    result->role = LanguageManager::instance().comboValue(roleBox);
    result->status = LanguageManager::instance().comboValue(statusBox);
    result->preference = LanguageManager::instance().comboValue(preferenceBox);
    return true;
}

QStringList recommendedGoodsNames(const QString &category)
{
    QVector<MarketplaceGoods> goods = AccountStore::goods();
    std::sort(goods.begin(), goods.end(), [](const MarketplaceGoods &a, const MarketplaceGoods &b) {
        if (a.sales != b.sales) return a.sales > b.sales;
        return a.stock > b.stock;
    });

    QStringList names;
    auto appendMatches = [&](bool matchCategory) {
        for (const MarketplaceGoods &item : goods) {
            if (item.status != "已上架" || item.stock <= 0) continue;
            if (matchCategory && item.category != category && !item.category.contains(category) && !category.contains(item.category)) continue;
            if (!names.contains(item.name)) names << item.name;
            if (names.size() >= 5) return;
        }
    };

    appendMatches(true);
    if (names.size() < 3) appendMatches(false);
    return names.mid(0, qMin(5, names.size()));
}

void showRecommendationDialog(QWidget *parent,
                              const QString &username,
                              const QString &category,
                              const QStringList &names)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("智能推荐商品");
    dialog.resize(560, 520);
    dialog.setStyleSheet(R"(
QDialog { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#RecommendCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 18px; }
QFrame#ProductCard { background: #F8F7FF; border: 1px solid #E4E0FF; border-radius: 14px; }
QLabel { background: transparent; }
QLabel#DialogTitle { color: #111111; font-size: 26px; font-weight: 900; }
QLabel#SectionLabel { color: #666666; font-size: 13px; font-weight: 800; }
QLabel#SectionValue { color: #111111; font-size: 16px; font-weight: 900; }
QLabel#ProductName { color: #111111; font-size: 15px; font-weight: 800; }
QLabel#ReasonText { color: #444444; font-size: 14px; line-height: 150%; }
QPushButton#PrimaryButton { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; border-radius: 16px; min-height: 42px; padding: 0 28px; font-weight: 900; }
QPushButton#PrimaryButton:hover { background: #493CE6; }
)");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *card = new QFrame;
    card->setObjectName("RecommendCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(16);

    auto *title = new QLabel("智能推荐商品");
    title->setObjectName("DialogTitle");
    layout->addWidget(title);

    auto *infoRow = new QHBoxLayout;
    infoRow->setSpacing(12);
    auto addInfoBlock = [&](const QString &label, const QString &value) {
        auto *block = new QFrame;
        block->setObjectName("ProductCard");
        auto *blockLayout = new QVBoxLayout(block);
        blockLayout->setContentsMargins(14, 10, 14, 10);
        auto *labelWidget = new QLabel(label);
        labelWidget->setObjectName("SectionLabel");
        auto *valueWidget = new QLabel(value);
        valueWidget->setObjectName("SectionValue");
        blockLayout->addWidget(labelWidget);
        blockLayout->addWidget(valueWidget);
        infoRow->addWidget(block);
    };
    addInfoBlock("用户", username);
    addInfoBlock("推荐类别", category);
    layout->addLayout(infoRow);

    auto *productLabel = new QLabel("推荐商品");
    productLabel->setObjectName("SectionLabel");
    layout->addWidget(productLabel);

    for (int i = 0; i < names.size(); ++i) {
        auto *product = new QFrame;
        product->setObjectName("ProductCard");
        auto *productLayout = new QHBoxLayout(product);
        productLayout->setContentsMargins(14, 10, 14, 10);
        auto *name = new QLabel(QString("%1. %2").arg(i + 1).arg(names[i]));
        name->setObjectName("ProductName");
        productLayout->addWidget(name);
        layout->addWidget(product);
    }

    auto *reasonLabel = new QLabel("推荐理由");
    reasonLabel->setObjectName("SectionLabel");
    layout->addWidget(reasonLabel);

    auto *reason = new QLabel("根据该用户的偏好类别和平台热门商品，推荐以上" + category + "类商品。");
    reason->setObjectName("ReasonText");
    reason->setWordWrap(true);
    layout->addWidget(reason);

    root->addWidget(card);

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

void showUserDetailDialog(QWidget *parent, const UserItem &user)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("用户详情");
    dialog.resize(520, 470);
    dialog.setStyleSheet(R"(
QDialog { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#DetailCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 18px; }
QLabel { background: transparent; }
QLabel#DialogTitle { color: #111111; font-size: 26px; font-weight: 900; }
QLabel#FieldName { color: #666666; font-size: 14px; font-weight: 800; }
QLabel#FieldValue { color: #111111; font-size: 15px; font-weight: 800; }
QLabel#StatusOk { background: #EAF7EC; color: #2E7D32; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#StatusBad { background: #FFF1F1; color: #E5484D; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#RoleTag { background: #F1F0FF; color: #5B4BFF; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#CategoryTag { background: #EAF4FF; color: #2563EB; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QPushButton#PrimaryButton { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; border-radius: 16px; min-height: 42px; padding: 0 30px; font-weight: 900; }
QPushButton#PrimaryButton:hover { background: #493CE6; }
)");

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *card = new QFrame;
    card->setObjectName("DetailCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(14);

    auto *title = new QLabel("用户详情");
    title->setObjectName("DialogTitle");
    layout->addWidget(title);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(12);

    auto addTextRow = [&](int row, const QString &name, const QString &value) {
        auto *nameLabel = new QLabel(name);
        nameLabel->setObjectName("FieldName");
        auto *valueLabel = new QLabel(value);
        valueLabel->setObjectName("FieldValue");
        valueLabel->setWordWrap(true);
        grid->addWidget(nameLabel, row, 0, Qt::AlignTop);
        grid->addWidget(valueLabel, row, 1);
    };
    auto addTagRow = [&](int row, const QString &name, const QString &value, const char *objectName) {
        auto *nameLabel = new QLabel(name);
        nameLabel->setObjectName("FieldName");
        auto *valueLabel = new QLabel(value);
        valueLabel->setAlignment(Qt::AlignCenter);
        valueLabel->setObjectName(objectName);
        grid->addWidget(nameLabel, row, 0, Qt::AlignTop);
        grid->addWidget(valueLabel, row, 1, Qt::AlignLeft);
    };

    addTextRow(0, "用户ID", user.id);
    addTextRow(1, "用户名", user.username);
    addTextRow(2, "手机号", user.phone);
    addTextRow(3, "邮箱", user.email);
    addTagRow(4, "角色", user.role, "RoleTag");
    addTagRow(5, "状态", user.status, user.status == "正常" ? "StatusOk" : "StatusBad");
    addTagRow(6, "喜好品类", user.preference, "CategoryTag");

    grid->setColumnStretch(1, 1);
    layout->addLayout(grid);
    root->addWidget(card);

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


QFrame *metricCard(const QString &title, const QString &value, const QString &desc, QLabel **valueOut = nullptr)
{
    auto *card = new QFrame;
    card->setObjectName("MetricCard");
    addShadow(card);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(6);
    auto *valueLabel = new QLabel(value);
    valueLabel->setObjectName("MetricValue");
    if (valueOut) *valueOut = valueLabel;
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("MetricTitle");
    auto *descLabel = new QLabel(desc);
    descLabel->setObjectName("MetricDesc");
    layout->addWidget(valueLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    return card;
}

} // namespace

UsersWindow::UsersWindow(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), m_mainWindow(mainWindow)
{
    resize(1360, 820);
    setMinimumSize(1120, 700);
    setWindowTitle("校园二手交易平台 - 用户管理");
    setStyleSheet(kUsersQss);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(18);
    root->addWidget(createSidebar("用户管理", m_mainWindow, this));

    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);
    auto *crumb = new QLabel("← 返回后台首页 / 用户管理");
    crumb->setObjectName("Crumb");
    auto *title = new QLabel("用户管理");
    title->setObjectName("Title");
    auto *desc = new QLabel("维护平台用户信息、账号状态与偏好分类。");
    desc->setObjectName("Desc");

    const UserStats stats = AccountStore::managedUserStats();
    auto *metrics = new QGridLayout;
    metrics->setHorizontalSpacing(14);
    metrics->addWidget(metricCard("用户总数", QString::number(stats.total), "平台注册用户", &labelTotalMetric), 0, 0);
    metrics->addWidget(metricCard("正常用户", QString::number(stats.normal), "可正常交易", &labelNormalMetric), 0, 1);
    metrics->addWidget(metricCard("禁用用户", QString::number(stats.disabled), "已限制登录", &labelDisabledMetric), 0, 2);
    metrics->addWidget(metricCard("管理员数量", QString::number(stats.admins), "后台管理账号", &labelAdminMetric), 0, 3);

    auto *panel = new QFrame;
    panel->setObjectName("Panel");
    addShadow(panel);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(24, 24, 24, 24);
    panelLayout->setSpacing(14);

    auto *filterBar = new QHBoxLayout;
    editSearch = new QLineEdit;
    editSearch->setPlaceholderText("搜索用户名或手机号...");
    comboRole = new QComboBox;
    comboRole->addItems(QStringList() << "全部角色" << "管理员" << "普通用户");
    comboStatus = new QComboBox;
    comboStatus->addItems(QStringList() << "全部状态" << "正常" << "禁用");
    auto *query = new QPushButton("查询");
    auto *reset = new QPushButton("重置");
    query->setObjectName("PrimaryButton");
    reset->setObjectName("SecondaryButton");
    filterBar->addWidget(editSearch, 1);
    filterBar->addWidget(comboRole);
    filterBar->addWidget(comboStatus);
    filterBar->addWidget(query);
    filterBar->addWidget(reset);

    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(12);
    btnAdd = new QPushButton("＋ 新增用户");
    btnEdit = new QPushButton("✎ 修改用户");
    btnDelete = new QPushButton("删除用户");
    btnRecommend = new QPushButton("推荐商品");
    btnBack = new QPushButton("返回管理员后台");
    btnPrevPage = new QPushButton("上一页");
    btnNextPage = new QPushButton("下一页");
    labelPageInfo = new QLabel;
    labelPageInfo->setObjectName("Desc");
    btnAdd->setObjectName("PrimaryButton");
    btnEdit->setObjectName("SecondaryButton");
    btnRecommend->setObjectName("SecondaryButton");
    btnDelete->setObjectName("DangerButton");
    btnBack->setObjectName("SecondaryButton");
    btnPrevPage->setObjectName("SecondaryButton");
    btnNextPage->setObjectName("SecondaryButton");
    topBar->addWidget(btnAdd);
    topBar->addWidget(btnEdit);
    topBar->addWidget(btnRecommend);
    topBar->addStretch();
    topBar->addWidget(btnDelete);

    auto *pageBar = new QHBoxLayout;
    pageBar->addStretch();
    pageBar->addWidget(btnPrevPage);
    pageBar->addWidget(labelPageInfo);
    pageBar->addWidget(btnNextPage);

    table = new QTableWidget(this);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels(QStringList() << "用户ID" << "用户名" << "手机号" << "角色" << "状态" << "喜好品类" << "操作");
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(56);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);

    panelLayout->addLayout(filterBar);
    panelLayout->addLayout(topBar);
    panelLayout->addLayout(pageBar);
    panelLayout->addWidget(table, 1);
    layout->addWidget(crumb);
    layout->addWidget(title);
    layout->addWidget(desc);
    layout->addLayout(metrics);
    layout->addWidget(panel, 1);
    root->addWidget(content, 1);

    updateTable();

    connect(btnAdd, &QPushButton::clicked, this, &UsersWindow::addUser);
    connect(btnEdit, &QPushButton::clicked, this, [this]() { editUser(); });
    connect(btnDelete, &QPushButton::clicked, this, &UsersWindow::deleteUser);
    connect(btnRecommend, &QPushButton::clicked, this, &UsersWindow::recommendGoods);
    connect(btnBack, &QPushButton::clicked, this, [=]() { if (m_mainWindow) m_mainWindow->show(); hide(); });
    connect(query, &QPushButton::clicked, this, [=]() { m_page = 0; updateTable(); });
    connect(reset, &QPushButton::clicked, this, [=]() { editSearch->clear(); comboRole->setCurrentIndex(0); comboStatus->setCurrentIndex(0); m_page = 0; updateTable(); });
    connect(btnPrevPage, &QPushButton::clicked, this, [=]() { if (m_page > 0) { --m_page; updateTable(); } });
    connect(btnNextPage, &QPushButton::clicked, this, [=]() { if ((m_page + 1) * m_pageSize < m_totalRows) { ++m_page; updateTable(); } });
}

UsersWindow::~UsersWindow() {}

void UsersWindow::loadSampleData()
{
    m_users.clear();
    updateTable();
}

void UsersWindow::updateTable()
{
    m_users.clear();
    const QString keyword = editSearch ? editSearch->text().trimmed() : QString();
    const QString role = comboRole
                             ? LanguageManager::instance().comboValue(comboRole)
                             : QString("全部角色");
    const QString status = comboStatus
                               ? LanguageManager::instance().comboValue(comboStatus)
                               : QString("全部状态");
    m_totalRows = AccountStore::managedUserCount(keyword, role, status);
    const int offset = m_page * m_pageSize;
    for (const ManagedUser &user : AccountStore::managedUsers(keyword, role, status, m_pageSize, offset)) {
        m_users.push_back({user.id, user.username, user.password, user.email, user.phone, user.role, user.status, user.preference});
    }

    table->setUpdatesEnabled(false);
    table->clearContents();
    table->setRowCount(0);
    table->setRowCount(m_users.size());
    for (int i = 0; i < m_users.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(m_users[i].id));
        table->setItem(i, 1, new QTableWidgetItem(m_users[i].username));
        table->setItem(i, 2, new QTableWidgetItem(m_users[i].phone));
        table->setItem(i, 3, new QTableWidgetItem(m_users[i].role));
        table->setCellWidget(i, 4, statusBadge(m_users[i].status));
        table->setItem(i, 5, new QTableWidgetItem(m_users[i].preference));
        const QString userId = m_users[i].id;
        auto *ops = new QWidget;
        auto *opsLayout = new QHBoxLayout(ops);
        opsLayout->setContentsMargins(0, 0, 0, 0);
        auto *view = new QPushButton("查看");
        auto *edit = new QPushButton("编辑");
        auto *toggle = new QPushButton(m_users[i].status == "正常" ? "禁用" : "启用");
        view->setObjectName("SecondaryButton");
        edit->setObjectName("SecondaryButton");
        toggle->setObjectName("SecondaryButton");
        view->setCursor(Qt::PointingHandCursor);
        edit->setCursor(Qt::PointingHandCursor);
        toggle->setCursor(Qt::PointingHandCursor);
        connect(view, &QPushButton::clicked, this, [this, userId]() { showUserDetail(userId); });
        connect(edit, &QPushButton::clicked, this, [this, userId]() { editUser(userId); });
        connect(toggle, &QPushButton::clicked, this, [this, userId]() { toggleUserStatus(userId); });
        opsLayout->addWidget(view);
        opsLayout->addWidget(edit);
        opsLayout->addWidget(toggle);
        table->setCellWidget(i, 6, ops);
        for (int c = 0; c < 6; ++c) {
            if (c == 4) continue;
            table->item(i, c)->setTextAlignment(Qt::AlignCenter);
        }
    }
    table->setUpdatesEnabled(true);
    updatePageLabel();
}

void UsersWindow::updatePageLabel()
{
    const int totalPages = qMax(1, (m_totalRows + m_pageSize - 1) / m_pageSize);
    if (m_page >= totalPages) {
        m_page = totalPages - 1;
    }
    if (labelPageInfo) {
        labelPageInfo->setText(QString("第 %1 / %2 页，共 %3 条").arg(m_page + 1).arg(totalPages).arg(m_totalRows));
    }
    if (btnPrevPage) btnPrevPage->setEnabled(m_page > 0);
    if (btnNextPage) btnNextPage->setEnabled((m_page + 1) * m_pageSize < m_totalRows);
}

int UsersWindow::currentRowIndex() const { return table->currentRow(); }

void UsersWindow::updateStats()
{
    const UserStats stats = AccountStore::managedUserStats();
    if (labelTotalMetric) labelTotalMetric->setText(QString::number(stats.total));
    if (labelNormalMetric) labelNormalMetric->setText(QString::number(stats.normal));
    if (labelDisabledMetric) labelDisabledMetric->setText(QString::number(stats.disabled));
    if (labelAdminMetric) labelAdminMetric->setText(QString::number(stats.admins));
}

void UsersWindow::addUser()
{
    UserItem initial{QString(), QString(), QString("123456"), QString(), QString(), QString("普通用户"), QString("正常"), QString("学习资料")};
    UserItem edited;
    if (!editUserDialog(this, "新增用户", initial, &edited)) return;

    QString errorMessage;
    const ManagedUser user{QString(), edited.username, edited.password, edited.email, edited.phone, edited.role, edited.status, edited.preference};
    if (!AccountStore::addManagedUser(user, &errorMessage)) {
        QMessageBox::warning(this, "新增失败",
                             errorMessage.trimmed().isEmpty()
                                 ? "用户新增失败，请确认数据库连接正常。"
                                 : "用户新增失败，原因：\n" + errorMessage);
        return;
    }
    updateTable();
    updateStats();
}

void UsersWindow::editUser()
{
    int row = currentRowIndex();
    if (row < 0 || row >= m_users.size()) { QMessageBox::warning(this, "提示", "请先选中要修改的用户。"); return; }
    editUser(m_users[row].id);
}

void UsersWindow::showUserDetail(const QString &userId)
{
    for (const UserItem &user : m_users) {
        if (user.id == userId) {
            showUserDetailDialog(this, user);
            return;
        }
    }
    QMessageBox::warning(this, "提示", "未找到该用户，请刷新当前页后重试。");
}

void UsersWindow::editUser(const QString &userId)
{
    int row = -1;
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].id == userId) {
            row = i;
            break;
        }
    }
    if (row < 0) { QMessageBox::warning(this, "提示", "未找到该用户，请刷新当前页后重试。"); return; }

    UserItem &user = m_users[row];
    UserItem edited;
    if (!editUserDialog(this, "修改用户", user, &edited)) return;

    edited.id = user.id;
    QString errorMessage;
    const ManagedUser managed{edited.id, edited.username, edited.password, edited.email, edited.phone, edited.role, edited.status, edited.preference};
    if (!AccountStore::updateManagedUser(managed, &errorMessage)) {
        QMessageBox::warning(this, "修改失败",
                             errorMessage.trimmed().isEmpty()
                                 ? "用户修改失败，请确认数据库连接正常。"
                                 : "用户修改失败，原因：\n" + errorMessage);
        return;
    }
    updateTable();
    updateStats();
}

void UsersWindow::toggleUserStatus(const QString &userId)
{
    int row = -1;
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].id == userId) {
            row = i;
            break;
        }
    }
    if (row < 0) { QMessageBox::warning(this, "提示", "未找到该用户，请刷新当前页后重试。"); return; }

    UserItem edited = m_users[row];
    edited.status = edited.status == "正常" ? "禁用" : "正常";

    QString errorMessage;
    const ManagedUser managed{edited.id, edited.username, edited.password, edited.email, edited.phone, edited.role, edited.status, edited.preference};
    if (!AccountStore::updateManagedUser(managed, &errorMessage)) {
        QMessageBox::warning(this, "状态更新失败",
                             errorMessage.trimmed().isEmpty()
                                 ? "用户状态更新失败，请确认数据库连接正常。"
                                 : "用户状态更新失败，原因：\n" + errorMessage);
        return;
    }
    updateTable();
    updateStats();
}

void UsersWindow::deleteUser()
{
    int row = currentRowIndex();
    if (row < 0 || row >= m_users.size()) { QMessageBox::warning(this, "提示", "请先选中要删除的用户。"); return; }
    if (QMessageBox::question(this, "确认删除", "确定删除当前选中的用户吗？") == QMessageBox::Yes) {
        QString errorMessage;
        if (!AccountStore::deleteManagedUser(m_users[row].id, &errorMessage)) {
            QMessageBox::warning(this, "删除失败",
                                 errorMessage.trimmed().isEmpty()
                                     ? "用户删除失败，请确认数据库连接正常。"
                                     : "用户删除失败，原因：\n" + errorMessage);
            return;
        }
        updateTable();
        updateStats();
    }
}

void UsersWindow::recommendGoods()
{
    int row = currentRowIndex();
    if (row < 0 || row >= m_users.size()) { QMessageBox::warning(this, "提示", "请先选中一个用户。"); return; }
    const QStringList names = recommendedGoodsNames(m_users[row].preference);
    if (names.isEmpty()) {
        showRecommendationDialog(this, m_users[row].username, m_users[row].preference, {"暂无可推荐商品"});
        return;
    }
    showRecommendationDialog(this, m_users[row].username, m_users[row].preference, names);
}

void UsersWindow::closeEvent(QCloseEvent *event)
{
    if (m_mainWindow) m_mainWindow->show();
    event->accept();
}
