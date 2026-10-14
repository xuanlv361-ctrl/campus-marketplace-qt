#include "orderswindow.h"
#include "accountstore.h"
#include "mainwindow.h"
#include "welcomewindow.h"

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
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
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

const char *kOrdersQss = R"(
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
QLabel#StatusWarn { background: #FFF8E6; color: #B77900; border-radius: 11px; padding: 3px 10px; font-weight: 800; }
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
        else if (plain == "用户管理") {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openUsersWindow(); self->hide(); });
        }
        else if (plain == "数据统计") {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openStatsWindow(); self->hide(); });
        }
        layout->addWidget(btn);
    }
    layout->addStretch();
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
    if (status == "已完成") label->setObjectName("StatusOk");
    else if (status == "已取消") label->setObjectName("StatusBad");
    else label->setObjectName("StatusWarn");
    return label;
}


QFrame *metricCard(const QString &title, const QString &value, const QString &desc)
{
    auto *card = new QFrame;
    card->setObjectName("MetricCard");
    addShadow(card);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(6);
    auto *valueLabel = new QLabel(value);
    valueLabel->setObjectName("MetricValue");
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("MetricTitle");
    auto *descLabel = new QLabel(desc);
    descLabel->setObjectName("MetricDesc");
    layout->addWidget(valueLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    return card;
}


QString adminGoodsName(const QString &raw, const QString &id)
{
    if (!raw.contains("商品")) return raw;
    const int n = id.toInt();
    auto pick = [n](const QStringList &names) {
        return names.value(n > 0 ? ((n * 37 + n / 10 + 3) % names.size()) : 0);
    };
    if (raw.contains("服饰") || raw.contains("鞋包")) {
        return pick({"通勤双肩包", "帆布包", "九成新运动鞋", "连帽卫衣", "秋季外套", "简约饰品"});
    }
    if (raw.contains("生活") || raw.contains("宿舍")) {
        return pick({"护眼台灯", "柔软毛毯", "宿舍小风扇", "桌面加湿器", "保温水杯", "宿舍收纳箱"});
    }
    if (raw.contains("运动")) {
        return pick({"校园自行车", "篮球", "羽毛球拍", "可调哑铃", "瑜伽垫", "滑板"});
    }
    if (raw.contains("电子") || raw.contains("手机") || raw.contains("数码")) {
        return pick({"MacBook Air M1", "iPad Air", "AirPods Pro", "Sony降噪耳机", "佳能EOS相机", "RK机械键盘"});
    }
    if (raw.contains("教材") || raw.contains("书籍")) {
        return pick({"高等数学教材", "雅思真题资料", "C++ 程序设计", "课堂笔记"});
    }
    if (raw.contains("美妆") || raw.contains("食品") || raw.contains("零食")) {
        return pick({"保湿面霜", "巧克力礼盒", "口红", "面膜", "咖啡礼盒", "坚果礼盒"});
    }
    return raw;
}

bool editOrderDialog(QWidget *parent, const QString &title, const OrderItem &initial, OrderItem *result)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.resize(480, 320);

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignRight);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);

    auto *goodsEdit = new QLineEdit(initial.goodsName);
    auto *buyerEdit = new QLineEdit(initial.buyer);
    auto *quantitySpin = new QSpinBox;
    quantitySpin->setRange(1, 9999);
    quantitySpin->setValue(qMax(1, initial.quantity));

    auto *priceSpin = new QDoubleSpinBox;
    priceSpin->setRange(0.01, 999999.00);
    priceSpin->setDecimals(2);
    priceSpin->setPrefix("¥");
    priceSpin->setValue(initial.price.toDouble() > 0 ? initial.price.toDouble() : 1.00);

    auto *statusBox = new QComboBox;
    const QStringList statuses = {"未完成", "已完成", "已取消"};
    statusBox->addItems(statuses);
    const int statusIndex = statuses.indexOf(initial.status);
    statusBox->setCurrentIndex(statusIndex >= 0 ? statusIndex : 0);

    form->addRow("商品名称", goodsEdit);
    form->addRow("购买用户", buyerEdit);
    form->addRow("商品数量", quantitySpin);
    form->addRow("成交价格", priceSpin);
    form->addRow("订单状态", statusBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("确定");
    buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (goodsEdit->text().trimmed().isEmpty() || buyerEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "提示", "商品名称和购买用户不能为空。");
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
    result->goodsName = goodsEdit->text().trimmed();
    result->buyer = buyerEdit->text().trimmed();
    result->quantity = quantitySpin->value();
    result->price = QString::number(priceSpin->value(), 'f', 2);
    result->status = statusBox->currentText();
    return true;
}

void showOrderDetailDialog(QWidget *parent, const OrderItem &order)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("订单详情");
    dialog.resize(500, 430);
    dialog.setStyleSheet(R"(
QDialog { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QFrame#DetailCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 18px; }
QLabel { background: transparent; }
QLabel#DialogTitle { color: #111111; font-size: 26px; font-weight: 900; }
QLabel#FieldName { color: #666666; font-size: 14px; font-weight: 800; }
QLabel#FieldValue { color: #111111; font-size: 15px; font-weight: 800; }
QLabel#StatusOk { background: #EAF7EC; color: #2E7D32; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#StatusWarn { background: #FFF8E6; color: #B77900; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
QLabel#StatusBad { background: #FFF1F1; color: #E5484D; border-radius: 11px; padding: 5px 12px; font-weight: 900; }
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

    auto *title = new QLabel("订单详情");
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
    addTextRow(0, "订单ID", order.id);
    addTextRow(1, "商品名称", order.goodsName);
    addTextRow(2, "买家", order.buyer);
    addTextRow(3, "成交价格", "¥" + order.price);
    addTextRow(4, "购买时间", order.time);

    auto *statusName = new QLabel("订单状态");
    statusName->setObjectName("FieldName");
    auto *status = new QLabel(order.status);
    status->setAlignment(Qt::AlignCenter);
    status->setObjectName(order.status == "已完成" ? "StatusOk" : (order.status == "已取消" ? "StatusBad" : "StatusWarn"));
    grid->addWidget(statusName, 5, 0, Qt::AlignTop);
    grid->addWidget(status, 5, 1, Qt::AlignLeft);

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

} // namespace

OrdersWindow::OrdersWindow(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), m_mainWindow(mainWindow)
{
    resize(1360, 820);
    setMinimumSize(1120, 700);
    setWindowTitle("校园二手交易平台 - 订单管理");
    setStyleSheet(kOrdersQss);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(18);
    root->addWidget(createSidebar("订单管理", m_mainWindow, this));

    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *crumb = new QLabel("← 返回后台首页 / 订单管理");
    crumb->setObjectName("Crumb");
    auto *title = new QLabel("订单管理");
    title->setObjectName("Title");
    auto *desc = new QLabel("查看交易订单、成交价格与订单处理状态。");
    desc->setObjectName("Desc");

    const OrderStats stats = AccountStore::orderStats();
    auto *metrics = new QGridLayout;
    metrics->setHorizontalSpacing(14);
    metrics->addWidget(metricCard("订单总数", QString::number(stats.total), "累计交易订单"), 0, 0);
    metrics->addWidget(metricCard("未完成订单", QString::number(stats.pending), "等待处理"), 0, 1);
    metrics->addWidget(metricCard("已完成订单", QString::number(stats.done), "已完成交易"), 0, 2);
    metrics->addWidget(metricCard("已取消订单", QString::number(stats.canceled), "已取消交易"), 0, 3);

    auto *panel = new QFrame;
    panel->setObjectName("Panel");
    addShadow(panel);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(24, 24, 24, 24);
    panelLayout->setSpacing(14);

    auto *filterBar = new QHBoxLayout;
    editSearch = new QLineEdit;
    editSearch->setPlaceholderText("搜索订单ID、商品名称或买家...");
    comboStatus = new QComboBox;
    comboStatus->addItems(QStringList() << "全部状态" << "未完成" << "已完成" << "已取消");
    auto *query = new QPushButton("查询");
    auto *reset = new QPushButton("重置");
    query->setObjectName("PrimaryButton");
    reset->setObjectName("SecondaryButton");
    filterBar->addWidget(editSearch, 1);
    filterBar->addWidget(comboStatus);
    filterBar->addWidget(query);
    filterBar->addWidget(reset);

    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(12);
    btnAdd = new QPushButton("＋ 新增订单");
    btnEdit = new QPushButton("✎ 修改订单");
    auto *btnExport = new QPushButton("导出订单");
    btnDelete = new QPushButton("删除订单");
    btnBack = new QPushButton("返回管理员后台");
    btnPrevPage = new QPushButton("上一页");
    btnNextPage = new QPushButton("下一页");
    labelPageInfo = new QLabel;
    labelPageInfo->setObjectName("Desc");
    btnAdd->setObjectName("PrimaryButton");
    btnEdit->setObjectName("SecondaryButton");
    btnExport->setObjectName("SecondaryButton");
    btnDelete->setObjectName("DangerButton");
    btnBack->setObjectName("SecondaryButton");
    btnPrevPage->setObjectName("SecondaryButton");
    btnNextPage->setObjectName("SecondaryButton");
    topBar->addWidget(btnAdd);
    topBar->addWidget(btnEdit);
    topBar->addWidget(btnExport);
    topBar->addStretch();
    topBar->addWidget(btnDelete);

    auto *pageBar = new QHBoxLayout;
    pageBar->addStretch();
    pageBar->addWidget(btnPrevPage);
    pageBar->addWidget(labelPageInfo);
    pageBar->addWidget(btnNextPage);

    table = new QTableWidget(this);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels(QStringList() << "订单ID" << "商品名称" << "买家" << "成交价格" << "购买时间" << "状态" << "操作");
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

    connect(btnAdd, &QPushButton::clicked, this, &OrdersWindow::addOrder);
    connect(btnEdit, &QPushButton::clicked, this, [this]() { editOrder(); });
    connect(btnDelete, &QPushButton::clicked, this, &OrdersWindow::deleteOrder);
    connect(btnBack, &QPushButton::clicked, this, [=]() { if (m_mainWindow) m_mainWindow->show(); hide(); });
    connect(btnExport, &QPushButton::clicked, this, [=]() { QMessageBox::information(this, "导出订单", "订单数据已准备导出。"); });
    connect(query, &QPushButton::clicked, this, [=]() { m_page = 0; updateTable(); });
    connect(reset, &QPushButton::clicked, this, [=]() { editSearch->clear(); comboStatus->setCurrentIndex(0); m_page = 0; updateTable(); });
    connect(btnPrevPage, &QPushButton::clicked, this, [=]() { if (m_page > 0) { --m_page; updateTable(); } });
    connect(btnNextPage, &QPushButton::clicked, this, [=]() { if ((m_page + 1) * m_pageSize < m_totalRows) { ++m_page; updateTable(); } });
}

OrdersWindow::~OrdersWindow() {}

void OrdersWindow::loadSampleData()
{
    m_orders.clear();
    const QString keyword = editSearch ? editSearch->text().trimmed() : QString();
    const QString status = comboStatus ? comboStatus->currentText() : QString("全部状态");
    m_totalRows = AccountStore::orderCount(keyword, status);
    const int offset = m_page * m_pageSize;
    for (const OrderRecord &order : AccountStore::searchOrders(keyword, status, m_pageSize, offset)) {
    m_orders.push_back({order.id, adminGoodsName(order.goodsName, order.id), order.buyer, QString::number(order.price * order.quantity, 'f', 2), order.quantity, order.status, order.time});
    }
}

void OrdersWindow::updateTable()
{
    loadSampleData();
    table->setUpdatesEnabled(false);
    table->clearContents();
    table->setRowCount(0);
    table->setRowCount(m_orders.size());
    for (int i = 0; i < m_orders.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(m_orders[i].id));
        table->setItem(i, 1, new QTableWidgetItem(m_orders[i].goodsName));
        table->setItem(i, 2, new QTableWidgetItem(m_orders[i].buyer));
        table->setItem(i, 3, new QTableWidgetItem("¥" + m_orders[i].price));
        table->setItem(i, 4, new QTableWidgetItem(m_orders[i].time));
        table->setCellWidget(i, 5, statusBadge(m_orders[i].status));
        const QString orderId = m_orders[i].id;
        auto *ops = new QWidget;
        auto *opsLayout = new QHBoxLayout(ops);
        opsLayout->setContentsMargins(0, 0, 0, 0);
        auto *view = new QPushButton("查看");
        auto *edit = new QPushButton("编辑");
        view->setObjectName("SecondaryButton");
        edit->setObjectName("SecondaryButton");
        view->setCursor(Qt::PointingHandCursor);
        edit->setCursor(Qt::PointingHandCursor);
        connect(view, &QPushButton::clicked, this, [this, orderId]() { showOrderDetail(orderId); });
        connect(edit, &QPushButton::clicked, this, [this, orderId]() { editOrder(orderId); });
        opsLayout->addWidget(view);
        opsLayout->addWidget(edit);
        table->setCellWidget(i, 6, ops);
        for (int c = 0; c < 5; ++c) table->item(i, c)->setTextAlignment(Qt::AlignCenter);
    }
    table->setUpdatesEnabled(true);
    updatePageLabel();
}

void OrdersWindow::updatePageLabel()
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

int OrdersWindow::currentRowIndex() const { return table->currentRow(); }

void OrdersWindow::addOrder()
{
    OrderItem initial{QString::number(AccountStore::maxOrderId() + 1), QString(), QString(), "1.00", 1, "未完成", QString()};
    OrderItem edited;
    if (!editOrderDialog(this, "新增订单", initial, &edited)) return;

    const double unitPrice = edited.price.toDouble() / qMax(1, edited.quantity);
    QString errorMessage;
    if (!AccountStore::addOrderRecord({edited.id, edited.goodsName, edited.buyer, unitPrice, edited.quantity, edited.status, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")}, &errorMessage)) {
        QMessageBox::warning(this, "新增失败",
                             errorMessage.trimmed().isEmpty()
                                 ? "订单新增失败，请确认数据库连接正常。"
                                 : "订单新增失败，原因：\n" + errorMessage);
        return;
    }
    updateTable();
}

void OrdersWindow::editOrder()
{
    int row = currentRowIndex();
    if (row < 0 || row >= m_orders.size()) { QMessageBox::warning(this, "提示", "请先选中要修改的订单。"); return; }
    editOrder(m_orders[row].id);
}

void OrdersWindow::showOrderDetail(const QString &orderId)
{
    for (const OrderItem &order : m_orders) {
        if (order.id == orderId) {
            showOrderDetailDialog(this, order);
            return;
        }
    }
    QMessageBox::warning(this, "提示", "未找到该订单，请刷新当前页后重试。");
}

void OrdersWindow::editOrder(const QString &orderId)
{
    int row = -1;
    for (int i = 0; i < m_orders.size(); ++i) {
        if (m_orders[i].id == orderId) {
            row = i;
            break;
        }
    }
    if (row < 0) { QMessageBox::warning(this, "提示", "未找到该订单，请刷新当前页后重试。"); return; }

    OrderItem edited;
    if (!editOrderDialog(this, "修改订单", m_orders[row], &edited)) return;

    QString errorMessage;
    if (!AccountStore::updateOrderRecord({m_orders[row].id,
                                          edited.goodsName,
                                          edited.buyer,
                                          edited.price.toDouble() / qMax(1, edited.quantity),
                                          edited.quantity,
                                          edited.status,
                                          QString()},
                                         &errorMessage)) {
        QMessageBox::warning(this, "修改失败",
                             errorMessage.trimmed().isEmpty()
                                 ? "订单修改失败，请确认数据库连接正常。"
                                 : "订单修改失败，原因：\n" + errorMessage);
        return;
    }
    updateTable();
}

void OrdersWindow::deleteOrder()
{
    int row = currentRowIndex();
    if (row < 0 || row >= m_orders.size()) { QMessageBox::warning(this, "提示", "请先选中要删除的订单。"); return; }
    if (QMessageBox::question(this, "确认删除", "确定删除当前选中的订单吗？") == QMessageBox::Yes) {
        QString errorMessage;
        if (!AccountStore::deleteOrderRecord(m_orders[row].id, &errorMessage)) {
            QMessageBox::warning(this, "删除失败",
                                 errorMessage.trimmed().isEmpty()
                                     ? "订单删除失败，请确认数据库连接正常。"
                                     : "订单删除失败，原因：\n" + errorMessage);
            return;
        }
        updateTable();
    }
}

void OrdersWindow::closeEvent(QCloseEvent *event)
{
    if (m_mainWindow) m_mainWindow->show();
    event->accept();
}
