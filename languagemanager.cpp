#include "languagemanager.h"

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QSettings>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>

namespace {

constexpr int kSourceRole = Qt::UserRole + 87;

const QHash<QString, QString> &translations()
{
    static const QHash<QString, QString> values = {
        {"校园二手交易平台", "Campus Marketplace"},
        {"校园二手", "Campus Market"},
        {"校园二手后台", "Campus Market Admin"},
        {"用户登录", "User Sign In"},
        {"管理员登录", "Admin Sign In"},
        {"用户注册", "Create Account"},
        {"注册账号", "Create Account"},
        {"登录", "Sign In"},
        {"登录后台", "Open Admin Console"},
        {"注册", "Register"},
        {"首页", "Home"},
        {"分类", "Categories"},
        {"购物车", "Cart"},
        {"收藏", "Saved"},
        {"我要出售", "Sell an Item"},
        {"个人中心", "Profile"},
        {"购买记录", "Purchase History"},
        {"用户入口", "User Portal"},
        {"管理员入口", "Admin Portal"},
        {"发布商品", "List an Item"},
        {"菜单", "Menu"},
        {"搜索校园好物", "Search campus finds"},
        {"返回首页", "Back to Home"},
        {"返回封面", "Back to Welcome"},
        {"返回登录页", "Back to Sign In"},
        {"返回后台首页", "Back to Dashboard"},
        {"返回管理后台", "Back to Dashboard"},
        {"退出", "Exit"},
        {"退出系统", "Exit"},
        {"管理员中心", "Admin Overview"},
        {"管理员后台", "Admin Console"},
        {"商品管理", "Product Management"},
        {"订单管理", "Order Management"},
        {"用户管理", "User Management"},
        {"数据统计", "Analytics"},
        {"查询", "Search"},
        {"搜索", "Search"},
        {"重置", "Reset"},
        {"刷新", "Refresh"},
        {"加载数据", "Load Data"},
        {"推荐商品", "Recommendations"},
        {"查看购物车", "View Cart"},
        {"查看购买记录", "View History"},
        {"加入购物车", "Add to Cart"},
        {"立即购买", "Buy Now"},
        {"详情", "Details"},
        {"新增商品", "Add Product"},
        {"修改商品", "Edit Product"},
        {"删除商品", "Delete Product"},
        {"查看商品销量", "View Sales"},
        {"查看销量", "View Sales"},
        {"查看", "View"},
        {"查看详情", "View Details"},
        {"查看待审核商品", "View Pending Products"},
        {"编辑", "Edit"},
        {"编辑商品", "Edit Product"},
        {"删除", "Delete"},
        {"筛选", "Filter"},
        {"上一页", "Previous"},
        {"下一页", "Next"},
        {"导出数据", "Export Data"},
        {"导出订单", "Export Orders"},
        {"选择图片", "Choose Image"},
        {"待审核商品", "Pending Products"},
        {"审核通过", "Approve"},
        {"通过审核", "Approve"},
        {"驳回", "Reject"},
        {"驳回商品", "Reject Product"},
        {"新增订单", "Add Order"},
        {"修改订单", "Edit Order"},
        {"删除订单", "Delete Order"},
        {"新增用户", "Add User"},
        {"修改用户", "Edit User"},
        {"删除用户", "Delete User"},
        {"保存", "Save"},
        {"确定", "Confirm"},
        {"取消", "Cancel"},
        {"关闭", "Close"},
        {"返回", "Back"},
        {"清空记录", "Clear History"},
        {"移除选中商品", "Remove Selected"},
        {"购买选中商品", "Buy Selected"},
        {"提交审核", "Submit for Review"},
        {"提交为待审核", "Submit for Review"},
        {"立即审核", "Review Now"},
        {"立即结算", "Checkout Now"},
        {"去首页逛逛", "Browse Marketplace"},
        {"商品ID", "Product ID"},
        {"ID", "ID"},
        {"名称", "Name"},
        {"商品", "Product"},
        {"商品名称", "Product Name"},
        {"商品类别", "Category"},
        {"分类", "Category"},
        {"价格", "Price"},
        {"商品价格", "Price"},
        {"库存", "Stock"},
        {"库存数量", "Stock"},
        {"销量", "Sales"},
        {"状态", "Status"},
        {"操作", "Actions"},
        {"订单ID", "Order ID"},
        {"买家", "Buyer"},
        {"购买用户", "Buyer"},
        {"成交价格", "Total"},
        {"购买时间", "Purchased At"},
        {"数量", "Quantity"},
        {"选择", "Select"},
        {"用户ID", "User ID"},
        {"用户名", "Username"},
        {"密码", "Password"},
        {"邮箱", "Email"},
        {"手机号", "Phone"},
        {"角色", "Role"},
        {"喜好品类", "Preferred Category"},
        {"卖家", "Seller"},
        {"商品成色", "Condition"},
        {"成色", "Condition"},
        {"商品描述", "Description"},
        {"图片路径", "Image Path"},
        {"订单状态", "Order Status"},
        {"商品数量", "Quantity"},
        {"全部分类", "All Categories"},
        {"全部状态", "All Statuses"},
        {"全部角色", "All Roles"},
        {"书籍教材", "Books & Textbooks"},
        {"教材资料", "Study Materials"},
        {"学习资料", "Study Materials"},
        {"电子数码", "Electronics"},
        {"生活用品", "Everyday Essentials"},
        {"宿舍用品", "Dorm Essentials"},
        {"服饰鞋包", "Fashion"},
        {"服饰箱包", "Fashion"},
        {"运动器材", "Sports"},
        {"运动用品", "Sports"},
        {"代步工具", "Campus Transport"},
        {"美妆零食", "Beauty & Snacks"},
        {"已上架", "Live"},
        {"待审核", "Pending Review"},
        {"已驳回", "Rejected"},
        {"已下架", "Offline"},
        {"未完成", "Processing"},
        {"已完成", "Completed"},
        {"已取消", "Cancelled"},
        {"待付款", "Awaiting Payment"},
        {"待发货", "Awaiting Dispatch"},
        {"已发货", "Dispatched"},
        {"正常", "Active"},
        {"禁用", "Disabled"},
        {"普通用户", "User"},
        {"管理员", "Administrator"},
        {"九成新", "Like New"},
        {"八成新", "Very Good"},
        {"良好", "Good"},
        {"普通", "Fair"},
        {"商品总数", "Products"},
        {"用户总数", "Users"},
        {"订单总数", "Orders"},
        {"已完成订单数", "Completed Orders"},
        {"最受欢迎商品", "Top-selling Product"},
        {"待优化商品", "Needs Attention"},
        {"按分类统计销量", "Sales by Category"},
        {"快捷入口", "Quick Actions"},
        {"快捷操作", "Quick Actions"},
        {"订单列表", "Orders"},
        {"订单详情", "Order Details"},
        {"订单汇总", "Order Summary"},
        {"商品详情", "Product Details"},
        {"用户详情", "User Details"},
        {"商品销售排行榜", "Product Sales Ranking"},
        {"商品分类", "Product Category"},
        {"刷新统计", "Refresh Analytics"},
        {"分类销售概览", "Category Sales Overview"},
        {"分类商品", "Products by Category"},
        {"最近订单", "Recent Orders"},
        {"推荐理由", "Why this recommendation"},
        {"智能推荐商品", "Smart Recommendations"},
        {"发布闲置商品", "List a Pre-owned Item"},
        {"我的购物车", "My Cart"},
        {"我的购买记录", "My Purchase History"},
        {"校园二手集市", "Campus Market"},
        {"校园集市", "Campus Market"},
        {"校园好物", "Campus Finds"},
        {"返回管理员后台", "Back to Admin Console"},
        {"返回后台首页  /  商品管理", "Back to Dashboard  /  Products"},
        {"返回后台首页 / 订单管理", "Back to Dashboard / Orders"},
        {"返回后台首页 / 用户管理", "Back to Dashboard / Users"},
        {"进入 →", "Open →"},
        {"进入商品管理", "Manage Products"},
        {"进入订单管理", "Manage Orders"},
        {"进入用户管理", "Manage Users"},
        {"查看数据统计", "View Analytics"},
        {"暂无商品", "No products yet"},
        {"未找到相关商品", "No matching products"},
        {"当前分类暂无商品", "No products in this category"},
        {"购物车是空的", "Your cart is empty"},
        {"暂无购买记录", "No purchase history yet"},
        {"暂无数据", "No data available"},
        {"购物车暂无商品", "Your cart is empty"},
        {"无图", "No image"},
        {"请选择语言", "Choose language"},
        {"校园二手交易平台 · 用户入口", "Campus Marketplace · User Portal"},
        {"校园二手交易平台 · 管理员入口", "Campus Marketplace · Admin Portal"},
        {"校园二手集市上线 · 闲置好物重新流动 · 校园二手交易平台", "Campus Market · Give useful items a second life"},
        {"校园二手集市上线 · 闲置好物重新流动 · Campus Market", "Campus Market · Give useful items a second life"},
        {"登录後浏览校园二手好物", "Sign in to discover campus finds"},
        {"创建账号后即可发布与购买校园闲置", "Create an account to buy and list pre-owned items"},
        {"进入后台管理商品、订单與用户信息", "Manage products, orders and users"},
        {"教材、数码、宿舍用品和代步好物，一处完成发布、浏览和管理。", "Discover, list and manage useful campus items in one place."},
        {"教材、数码、宿舍用品和代步好物，一站式发布、浏览与交易。", "List, discover and trade useful campus items in one place."},
        {"平台核心业务数据概览", "Live marketplace overview"},
        {"统一后台入口。左侧菜单是全局导航，所有管理页面都在当前窗口内切换。", "A unified admin workspace with global navigation for every management area."},
        {"管理平台商品信息、库存状态、销售情况与审核状态。", "Manage product details, stock, sales and approval status."},
        {"管理平台商品信息、库存状态与销售情况", "Manage product details, stock and sales."},
        {"维护平台用户信息、账号状态与偏好分类。", "Manage user profiles, account status and category preferences."},
        {"查看交易订单、成交价格与订单处理状态。", "Review orders, totals and fulfilment status."},
        {"查看历史订单与交易状态。", "Review historical orders and their status."},
        {"查看平台商品、用户、订单与分类销售数据。", "Explore products, users, orders and category sales."},
        {"查看当前登录账号与平台使用信息。", "View the current account and marketplace information."},
        {"选择分类，浏览对应校园二手商品。", "Choose a category to browse matching campus listings."},
        {"填写商品信息后提交审核，管理员通过后即可上架。", "Complete the listing and submit it for admin approval."},
        {"核对商品信息后即可结算。", "Review the item information before checkout."},
        {"核对校园好物后即可结算，订单会按数据库商品编号完成购买。", "Review your items before checkout; orders are linked to their database product IDs."},
        {"购买记录会在结算后自动更新。", "Purchase history updates automatically after checkout."},
        {"你还没有购买任何商品，去首页看看校园好物吧。", "You have not purchased anything yet. Explore the marketplace to get started."},
        {"购物车为空\n去首页看看校园好物吧。", "Your cart is empty.\nExplore the marketplace to find something useful."},
        {"请输入账号，例如 user1", "Enter your username, for example user1"},
        {"请输入密码", "Enter your password"},
        {"请输入管理员账号，例如 admin1", "Enter an admin username, for example admin1"},
        {"请输入管理员密码", "Enter the admin password"},
        {"在当前分类中搜索商品...", "Search within this category..."},
        {"最高价", "Maximum price"},
        {"搜索商品名称 / 分类", "Search by product or category"},
        {"搜索商品名称 / ID / 分类 / 卖家", "Search name, ID, category or seller"},
        {"搜索商品名称、ID、分类或状态...", "Search name, ID, category or status..."},
        {"搜索订单ID、商品名称或买家", "Search order ID, product or buyer"},
        {"搜索订单ID、商品名称或买家...", "Search order ID, product or buyer..."},
        {"搜索用户名或手机号", "Search username or phone"},
        {"搜索用户名或手机号...", "Search username or phone..."},
        {"成色，例如 九成新 / 八成新", "Condition, for example like new / very good"},
        {"欢迎回来，Admin。这里可以管理校园二手平台的商品、订单、用户与交易数据。", "Welcome back, Admin. Manage products, orders, users and marketplace activity here."}
    };
    return values;
}

QString translatedCore(const QString &source)
{
    const auto it = translations().constFind(source);
    return it == translations().constEnd() ? source : it.value();
}

QString translateDynamic(const QString &source)
{
    const QString windowPrefix = "校园二手交易平台 - ";
    if (source.startsWith(windowPrefix)) {
        return "Campus Marketplace - " + translatedCore(source.mid(windowPrefix.size()));
    }

    static const QList<QPair<QString, QString>> prefixes = {
        {"当前用户：", "Signed in as: "},
        {"欢迎你，", "Welcome, "},
        {"欢迎回来，", "Welcome back, "},
        {"库存：", "Stock: "},
        {"销量：", "Sales: "},
        {"卖家：", "Seller: "},
        {"成色：", "Condition: "},
        {"分类：", "Category: "},
        {"价格：", "Price: "},
        {"合计：", "Total: "},
        {"订单编号：", "Order: "},
        {"购买时间：", "Purchased: "},
        {"买家：", "Buyer: "},
        {"数量：", "Quantity: "},
        {"用户名：", "Username: "},
        {"状态：", "Status: "},
        {"商品数量：", "Items: "},
        {"购物车商品数：", "Cart items: "},
        {"商品总额：", "Subtotal: "},
        {"应付合计：", "Amount due: "},
        {"运费：", "Delivery: "},
        {"订单汇总：", "Order total: "},
        {"销量 ", "Sales "},
        {"今日概览 · ", "Today's overview · "},
        {"根据该用户的偏好类别和平台热门商品，推荐以上", "Recommended from this user's preferences and popular listings: "},
        {"成色：良好    库存：", "Condition: Good    Stock: "}
    };
    for (const auto &entry : prefixes) {
        if (source.startsWith(entry.first)) {
            return entry.second + source.mid(entry.first.size());
        }
    }

    int firstLetter = 0;
    while (firstLetter < source.size() && !source.at(firstLetter).isLetterOrNumber()) {
        ++firstLetter;
    }
    if (firstLetter > 0 && firstLetter < source.size()) {
        const QString body = source.mid(firstLetter);
        const QString translated = translatedCore(body);
        if (translated != body) return source.left(firstLetter) + translated;
    }
    return source;
}

QString sourceProperty(QObject *object, const char *name, const QString &current,
                       LanguageManager::Language language)
{
    Q_UNUSED(language)
    QString source = object->property(name).toString();
    if (source.isEmpty()) {
        source = current;
        object->setProperty(name, source);
    }
    return source;
}

} // namespace

LanguageManager &LanguageManager::instance()
{
    static LanguageManager manager;
    return manager;
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
{
    QSettings settings("CampusMarketplace", "SecondHandSystem");
    m_language = settings.value("language", "zh").toString() == "en"
                     ? Language::English
                     : Language::Chinese;
    if (qApp) qApp->installEventFilter(this);
}

LanguageManager::Language LanguageManager::language() const
{
    return m_language;
}

QWidget *LanguageManager::createSelector(QWidget *parent)
{
    auto *selector = new QComboBox(parent);
    selector->setObjectName("LanguageSelector");
    selector->setAccessibleName("Language");
    selector->setToolTip("语言 / Language");
    selector->addItem("中文", int(Language::Chinese));
    selector->addItem("English", int(Language::English));
    selector->setCurrentIndex(m_language == Language::English ? 1 : 0);
    selector->setMinimumWidth(108);
    selector->setMinimumHeight(34);
    selector->setStyleSheet(
        "QComboBox#LanguageSelector { background: #FFFFFF; color: #3D2C2E; "
        "border: 1px solid #EADFD3; border-radius: 8px; padding: 5px 28px 5px 10px; }"
        "QComboBox#LanguageSelector:hover { border-color: #F4A261; }"
        "QComboBox#LanguageSelector::drop-down { border: none; width: 24px; }"
        "QComboBox#LanguageSelector QAbstractItemView { background: #FFFFFF; color: #3D2C2E; "
        "selection-background-color: #FDE8D2; selection-color: #3D2C2E; }");
    connect(selector, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, selector](int) {
        setLanguage(static_cast<Language>(selector->currentData().toInt()));
    });
    connect(this, &LanguageManager::languageChanged, selector, [selector](Language language) {
        const QSignalBlocker blocker(selector);
        selector->setCurrentIndex(language == Language::English ? 1 : 0);
    });
    return selector;
}

QString LanguageManager::translate(const QString &source) const
{
    if (m_language == Language::Chinese) return source;
    const QString exact = translatedCore(source);
    return exact == source ? translateDynamic(source) : exact;
}

QString LanguageManager::sourceText(const QString &displayText) const
{
    for (auto it = translations().constBegin(); it != translations().constEnd(); ++it) {
        if (it.value() == displayText) return it.key();
    }
    return displayText;
}

QString LanguageManager::comboValue(const QComboBox *combo) const
{
    if (!combo || combo->currentIndex() < 0) return QString();
    const QString source = combo->itemData(combo->currentIndex(), kSourceRole).toString();
    return source.isEmpty() ? sourceText(combo->currentText()) : source;
}

void LanguageManager::setComboValue(QComboBox *combo, const QString &sourceValue) const
{
    if (!combo) return;
    for (int i = 0; i < combo->count(); ++i) {
        const QString source = combo->itemData(i, kSourceRole).toString();
        if (source == sourceValue || sourceText(combo->itemText(i)) == sourceValue) {
            combo->setCurrentIndex(i);
            return;
        }
    }
}

void LanguageManager::setLanguage(Language language)
{
    if (m_language == language) return;
    m_language = language;
    QSettings("CampusMarketplace", "SecondHandSystem")
        .setValue("language", language == Language::English ? "en" : "zh");
    retranslateAll();
    emit languageChanged(language);
}

void LanguageManager::retranslateAll()
{
    if (!qApp) return;
    const auto windows = qApp->topLevelWidgets();
    for (QWidget *window : windows) retranslateWidget(window);
}

bool LanguageManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        if (auto *widget = qobject_cast<QWidget *>(watched)) {
            QPointer<QWidget> guard(widget);
            QTimer::singleShot(0, this, [this, guard]() {
                if (guard) retranslateWidget(guard);
            });
        }
    }
    return QObject::eventFilter(watched, event);
}

void LanguageManager::retranslateCombo(QComboBox *combo)
{
    if (!combo || combo->objectName() == "LanguageSelector" || combo->isEditable()) return;
    const QSignalBlocker blocker(combo);
    for (int i = 0; i < combo->count(); ++i) {
        QString source = combo->itemData(i, kSourceRole).toString();
        if (source.isEmpty()) {
            source = sourceText(combo->itemText(i));
            combo->setItemData(i, source, kSourceRole);
        }
        combo->setItemText(i, translate(source));
    }
}

void LanguageManager::retranslateWidget(QWidget *widget)
{
    if (!widget) return;

    if (widget->isWindow()) {
        const QString source = sourceProperty(widget, "i18nWindowTitle", widget->windowTitle(), m_language);
        if (!source.isEmpty()) widget->setWindowTitle(translate(source));
    }

    const auto objects = widget->findChildren<QWidget *>(QString(), Qt::FindChildrenRecursively);
    QList<QWidget *> all = objects;
    all.prepend(widget);
    for (QWidget *child : all) {
        if (auto *combo = qobject_cast<QComboBox *>(child)) {
            retranslateCombo(combo);
            continue;
        }
        if (auto *button = qobject_cast<QAbstractButton *>(child)) {
            const QString source = sourceProperty(button, "i18nText", button->text(), m_language);
            button->setText(translate(source));
        } else if (auto *label = qobject_cast<QLabel *>(child)) {
            const QString source = sourceProperty(label, "i18nText", label->text(), m_language);
            label->setText(translate(source));
        } else if (auto *group = qobject_cast<QGroupBox *>(child)) {
            const QString source = sourceProperty(group, "i18nTitle", group->title(), m_language);
            group->setTitle(translate(source));
        }

        if (auto *edit = qobject_cast<QLineEdit *>(child)) {
            const QString source = sourceProperty(edit, "i18nPlaceholder", edit->placeholderText(), m_language);
            edit->setPlaceholderText(translate(source));
        } else if (auto *edit = qobject_cast<QTextEdit *>(child)) {
            const QString source = sourceProperty(edit, "i18nPlaceholder", edit->placeholderText(), m_language);
            edit->setPlaceholderText(translate(source));
        }

        if (auto *table = qobject_cast<QTableWidget *>(child)) {
            for (int column = 0; column < table->columnCount(); ++column) {
                QTableWidgetItem *header = table->horizontalHeaderItem(column);
                if (!header) continue;
                QString source = header->data(kSourceRole).toString();
                if (source.isEmpty()) {
                    source = sourceText(header->text());
                    header->setData(kSourceRole, source);
                }
                header->setText(translate(source));
            }
        }

        if (auto *tabs = qobject_cast<QTabWidget *>(child)) {
            for (int i = 0; i < tabs->count(); ++i) {
                const QString propertyName = QString("i18nTab%1").arg(i);
                QString source = tabs->property(propertyName.toUtf8().constData()).toString();
                if (source.isEmpty()) {
                    source = sourceText(tabs->tabText(i));
                    tabs->setProperty(propertyName.toUtf8().constData(), source);
                }
                tabs->setTabText(i, translate(source));
            }
        }
    }
}
