#include "statswindow.h"

#include "accountstore.h"
#include "mainwindow.h"
#include "welcomewindow.h"

#include <QCloseEvent>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

const char *kStatsQss = R"(
QWidget { background: #FAFAFA; color: #111111; font-family: "Microsoft YaHei", "Segoe UI", Arial; font-size: 14px; }
QLabel { background: transparent; }
QFrame#Sidebar, QFrame#StatCard, QFrame#InfoCard, QFrame#ChartCard { background: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 22px; }
QLabel#Brand { color: #111111; font-size: 22px; font-weight: 900; }
QLabel#SideHint, QLabel#Desc, QLabel#CardLabel, QLabel#InfoText { color: #666666; font-size: 13px; }
QLabel#Title { color: #111111; font-size: 32px; font-weight: 900; }
QLabel#Number { color: #5B4BFF; font-size: 34px; font-weight: 900; }
QLabel#InfoTitle { color: #111111; font-size: 17px; font-weight: 900; }
QLabel#ChartTitle { color: #111111; font-size: 18px; font-weight: 900; }
QPushButton { min-height: 44px; border-radius: 16px; font-size: 14px; font-weight: 800; padding: 0 16px; }
QPushButton#SecondaryButton, QPushButton#SidebarButton { background: #FFFFFF; color: #111111; border: 1px solid #EAEAEA; }
QPushButton#SecondaryButton:hover, QPushButton#SidebarButton:hover { background: #F7F7F7; border-color: #5B4BFF; }
QPushButton#SidebarButton, QPushButton#SidebarPrimary, QPushButton#ExitButton { text-align: left; }
QPushButton#SidebarPrimary { background: #5B4BFF; color: #FFFFFF; border: 1px solid #5B4BFF; }
QPushButton#ExitButton { background: #FFF1F1; color: #E5484D; border: 1px solid #FFD1D1; }
QPushButton#ExitButton:hover { background: #FFE8E8; }
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

QStringList preferredCategories()
{
    return {"代步工具", "电子数码", "服饰鞋包", "美妆零食", "宿舍用品", "学习资料", "运动器材"};
}

QStringList statsCategories()
{
    return preferredCategories();
}

QString normalizedStatsCategory(const QString &category)
{
    const QString text = category.trimmed();
    if (text == "代步工具" || text.contains("代步")) return "代步工具";
    if (text == "电子数码" || text.contains("数码") || text.contains("电子")) return "电子数码";
    if (text == "服饰鞋包" || text.contains("服饰") || text.contains("鞋") || text.contains("包")) return "服饰鞋包";
    if (text == "美妆零食" || text.contains("美妆") || text.contains("个护") || text.contains("零食") || text.contains("食品")) return "美妆零食";
    if (text == "宿舍用品" || text.contains("宿舍") || text.contains("生活")) return "宿舍用品";
    if (text == "学习资料" || text.contains("教材") || text.contains("书籍") || text.contains("学习") || text.contains("办公")) return "学习资料";
    if (text == "运动器材" || text.contains("运动")) return "运动器材";
    if (text.contains("闲置")) return "宿舍用品";
    return statsCategories().contains(text) ? text : "宿舍用品";
}

QMap<QString, int> categorySalesData()
{
    QMap<QString, int> sales;
    for (const QString &category : statsCategories()) {
        sales[category] = 0;
    }
    for (const MarketplaceGoods &goods : AccountStore::goods()) {
        sales[normalizedStatsCategory(goods.category)] += qMax(0, goods.sales);
    }
    return sales;
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
        else if (plain == "用户管理") {
            QObject::connect(btn, &QPushButton::clicked, self, [=]() { if (mainWindow) mainWindow->openUsersWindow(); self->hide(); });
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

class SalesBarChart : public QWidget
{
public:
    explicit SalesBarChart(QWidget *parent = nullptr) : QWidget(parent) { setMinimumHeight(320); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        const QMap<QString, int> salesByCategory = categorySalesData();
        int maxSales = 1;
        for (auto it = salesByCategory.cbegin(); it != salesByCategory.cend(); ++it) {
            maxSales = qMax(maxSales, it.value());
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#FFFFFF"));

        const int yAxisWidth = 52;
        const QRect chartRect = rect().adjusted(yAxisWidth, 28, -24, -88);
        if (salesByCategory.isEmpty()) {
            painter.setPen(QColor("#666666"));
            painter.drawText(chartRect, Qt::AlignCenter, "暂无销量数据");
            return;
        }

        const int axisMax = qMax(10, static_cast<int>(std::ceil(maxSales / 100.0) * 100));
        const int tickCount = 5;
        painter.setPen(QPen(QColor("#EAEAEA"), 1));
        for (int tick = 0; tick <= tickCount; ++tick) {
            const int tickValue = axisMax * tick / tickCount;
            const double ratio = static_cast<double>(tickValue) / axisMax;
            const int y = chartRect.bottom() - static_cast<int>(ratio * chartRect.height());
            painter.drawLine(chartRect.left(), y, chartRect.right(), y);
            painter.setPen(QColor("#999999"));
            painter.drawText(QRect(4, y - 8, yAxisWidth - 10, 16), Qt::AlignRight | Qt::AlignVCenter, QString::number(tickValue));
            painter.setPen(QPen(QColor("#EAEAEA"), 1));
        }

        const int count = salesByCategory.size();
        const int gap = 18;
        const int barWidth = qMax(34, (chartRect.width() - gap * (count - 1)) / qMax(1, count));
        int x = chartRect.left();
        int index = 0;
        for (auto it = salesByCategory.cbegin(); it != salesByCategory.cend(); ++it, ++index) {
            const int barHeight = static_cast<int>((static_cast<double>(it.value()) / axisMax) * chartRect.height());
            QRect bar(x, chartRect.bottom() - barHeight, barWidth, barHeight);
            painter.setPen(Qt::NoPen);
            painter.setBrush(index % 2 == 0 ? QColor("#5B4BFF") : QColor("#5B4BFF"));
            painter.drawRoundedRect(bar, 8, 8);

            painter.setPen(QColor("#111111"));
            painter.drawText(QRect(x, bar.top() - 22, barWidth, 18), Qt::AlignCenter, QString::number(it.value()));
            painter.setPen(QColor("#666666"));
            painter.save();
            painter.translate(x + barWidth / 2, chartRect.bottom() + 16);
            painter.rotate(-28);
            painter.drawText(QRect(-70, 0, 140, 42), Qt::AlignCenter | Qt::TextWordWrap, it.key());
            painter.restore();
            x += barWidth + gap;
        }
    }
};

QFrame *createStatCard(const QString &number, const QString &label)
{
    auto *card = new QFrame;
    card->setObjectName("StatCard");
    addShadow(card);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(8);
    auto *num = new QLabel(number);
    num->setObjectName("Number");
    num->setAlignment(Qt::AlignCenter);
    auto *lab = new QLabel(label);
    lab->setObjectName("CardLabel");
    lab->setAlignment(Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(num);
    layout->addWidget(lab);
    layout->addStretch();
    return card;
}

QFrame *createInfoCard(const QString &title, const QString &text)
{
    auto *card = new QFrame;
    card->setObjectName("InfoCard");
    addShadow(card);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(22, 20, 22, 20);
    layout->setSpacing(8);
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("InfoTitle");
    auto *textLabel = new QLabel(text);
    textLabel->setObjectName("InfoText");
    textLabel->setWordWrap(true);
    layout->addWidget(titleLabel);
    layout->addWidget(textLabel);
    return card;
}

QString mostPopularText()
{
    const QMap<QString, int> categorySales = categorySalesData();
    if (categorySales.isEmpty()) return "暂无商品数据。";
    QString bestCategory;
    int bestSales = -1;
    for (auto it = categorySales.cbegin(); it != categorySales.cend(); ++it) if (it.value() > bestSales) { bestCategory = it.key(); bestSales = it.value(); }
    if (bestSales <= 0) return "当前暂无销量记录。";
    return bestCategory + "类，累计销量 " + QString::number(bestSales) + " 件。";
}

QString leastPopularText()
{
    const QMap<QString, int> categorySales = categorySalesData();
    if (categorySales.isEmpty()) return "暂无商品数据。";
    QString weakestCategory;
    int weakestSales = std::numeric_limits<int>::max();
    for (auto it = categorySales.cbegin(); it != categorySales.cend(); ++it) if (it.value() < weakestSales) { weakestCategory = it.key(); weakestSales = it.value(); }
    if (weakestSales <= 0) return "暂无低销量商品，当前销售情况良好，可继续保持。";
    return weakestCategory + "类，当前销量 " + QString::number(weakestSales) + " 件，可考虑调价或提升曝光。";
}

} // namespace

StatsWindow::StatsWindow(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), m_mainWindow(mainWindow)
{
    resize(1280, 800);
    setMinimumSize(1080, 700);
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#FAFAFA"));
    setPalette(pal);
    setWindowTitle("校园二手交易平台 - 数据统计");
    setStyleSheet(kStatsQss);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);
    root->setSpacing(18);
    root->addWidget(createSidebar("数据统计", m_mainWindow, this));

    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *title = new QLabel("数据统计");
    title->setObjectName("Title");
    auto *desc = new QLabel("查看平台商品、用户、订单与分类销售数据。");
    desc->setObjectName("Desc");

    const int completedOrders = std::count_if(AccountStore::orders().cbegin(), AccountStore::orders().cend(), [](const OrderRecord &order) {
        return order.status == "已完成";
    });

    auto *grid = new QGridLayout;
    grid->setSpacing(16);
    grid->addWidget(createStatCard(QString::number(AccountStore::goods().size()), "商品总数"), 0, 0);
    grid->addWidget(createStatCard(QString::number(AccountStore::users().size()), "用户总数"), 0, 1);
    grid->addWidget(createStatCard(QString::number(AccountStore::orders().size()), "订单总数"), 0, 2);
    grid->addWidget(createStatCard(QString::number(completedOrders), "已完成订单数"), 0, 3);

    auto *chartCard = new QFrame;
    chartCard->setObjectName("ChartCard");
    addShadow(chartCard);
    auto *chartLayout = new QVBoxLayout(chartCard);
    chartLayout->setContentsMargins(22, 20, 22, 20);
    chartLayout->setSpacing(12);
    auto *chartTitle = new QLabel("按分类统计销量");
    chartTitle->setObjectName("ChartTitle");
    chartLayout->addWidget(chartTitle);
    chartLayout->addWidget(new SalesBarChart(chartCard));

    auto *infoRow = new QHBoxLayout;
    infoRow->setSpacing(16);
    infoRow->addWidget(createInfoCard("最受欢迎商品", mostPopularText()));
    infoRow->addWidget(createInfoCard("待优化商品", leastPopularText()));

    btnBack = new QPushButton("返回管理员后台");
    btnBack->setObjectName("SecondaryButton");
    btnBack->setFixedWidth(180);

    auto *titleRow = new QHBoxLayout;
    titleRow->addWidget(title);
    titleRow->addStretch();
    titleRow->addWidget(btnBack);

    layout->addLayout(titleRow);
    layout->addWidget(desc);
    layout->addLayout(grid);
    layout->addWidget(chartCard, 1);
    layout->addLayout(infoRow);
    root->addWidget(content, 1);

    connect(btnBack, &QPushButton::clicked, this, [=]() { if (m_mainWindow) m_mainWindow->show(); hide(); });
}

StatsWindow::~StatsWindow() {}

void StatsWindow::closeEvent(QCloseEvent *event)
{
    if (m_mainWindow) m_mainWindow->show();
    event->accept();
}
