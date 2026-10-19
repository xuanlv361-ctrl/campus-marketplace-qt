#include "accountstore.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QStringList>

#include <mysql.h>

namespace {

MYSQL *g_conn = nullptr;
bool g_dbChecked = false;
bool g_usersLoaded = false;
bool g_goodsLoaded = false;
bool g_ordersLoaded = false;
QString g_lastError;

bool ensureDatabase()
{
    if (g_conn) {
        return true;
    }
    if (g_dbChecked) {
        return false;
    }

    g_dbChecked = true;
    g_conn = mysql_init(nullptr);
    if (!g_conn) {
        g_lastError = "mysql_init failed";
        return false;
    }

    mysql_options(g_conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    const QByteArray host = qEnvironmentVariable("SECONDHAND_DB_HOST", "127.0.0.1").toUtf8();
    const QByteArray database = qEnvironmentVariable("SECONDHAND_DB_NAME", "shop").toUtf8();
    const QByteArray user = qEnvironmentVariable("SECONDHAND_DB_USER", "root").toUtf8();
    const QByteArray password = qEnvironmentVariable("SECONDHAND_DB_PASSWORD").toUtf8();
    bool portOk = false;
    const unsigned int port = qEnvironmentVariableIntValue("SECONDHAND_DB_PORT", &portOk);

    if (!mysql_real_connect(g_conn,
                            host.constData(),
                            user.constData(),
                            password.constData(),
                            database.constData(),
                            portOk ? port : 3306,
                            nullptr,
                            0)) {
        g_lastError = QString::fromLocal8Bit(mysql_error(g_conn));
        mysql_close(g_conn);
        g_conn = nullptr;
        return false;
    }

    mysql_set_character_set(g_conn, "utf8mb4");
    mysql_query(g_conn, "SET NAMES utf8mb4 COLLATE utf8mb4_unicode_ci");

    const char *statusColumnSql =
        "ALTER TABLE `Goods` "
        "ADD COLUMN `status` VARCHAR(20) NOT NULL DEFAULT '已上架' COMMENT '商品审核状态'";
    if (mysql_query(g_conn, statusColumnSql) != 0) {
        const QString err = QString::fromLocal8Bit(mysql_error(g_conn));
        if (!err.contains("Duplicate column", Qt::CaseInsensitive) &&
            !err.contains("Duplicate", Qt::CaseInsensitive)) {
            g_lastError = err;
        }
    }
    mysql_query(g_conn, "UPDATE `Goods` SET `status`='已上架' WHERE `status` IS NULL OR `status`=''");

    const char *preferenceColumnSql =
        "ALTER TABLE `User` "
        "ADD COLUMN `preference_category` VARCHAR(50) NOT NULL DEFAULT '电子数码' COMMENT '用户喜好品类'";
    if (mysql_query(g_conn, preferenceColumnSql) != 0) {
        const QString err = QString::fromLocal8Bit(mysql_error(g_conn));
        if (!err.contains("Duplicate column", Qt::CaseInsensitive) &&
            !err.contains("Duplicate", Qt::CaseInsensitive)) {
            g_lastError = err;
        }
    }
    mysql_query(g_conn, "UPDATE `User` SET `preference_category`='电子数码' WHERE `preference_category` IS NULL OR `preference_category`=''");

    return true;
}

QString escapeSql(const QString &value)
{
    if (!ensureDatabase()) {
        return QString();
    }

    const QByteArray in = value.toUtf8();
    QByteArray out;
    out.resize(in.size() * 2 + 1);
    const unsigned long len = mysql_real_escape_string(g_conn, out.data(), in.constData(), in.size());
    return QString::fromUtf8(out.constData(), int(len));
}

bool execSql(const QString &sql)
{
    if (!ensureDatabase()) {
        return false;
    }

    const QByteArray bytes = sql.toUtf8();
    if (mysql_query(g_conn, bytes.constData()) != 0) {
        g_lastError = QString::fromUtf8(mysql_error(g_conn));
        return false;
    }
    return true;
}

QVector<QStringList> queryRows(const QString &sql)
{
    QVector<QStringList> rows;
    if (!execSql(sql)) {
        return rows;
    }

    MYSQL_RES *res = mysql_store_result(g_conn);
    if (!res) {
        return rows;
    }

    const int fields = int(mysql_num_fields(res));
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        unsigned long *lengths = mysql_fetch_lengths(res);
        QStringList values;
        for (int i = 0; i < fields; ++i) {
            values << (row[i] ? QString::fromUtf8(row[i], int(lengths[i])) : QString());
        }
        rows.push_back(values);
    }

    mysql_free_result(res);
    return rows;
}

int scalarInt(const QString &sql)
{
    const QVector<QStringList> rows = queryRows(sql);
    if (rows.isEmpty() || rows.first().isEmpty()) {
        return 0;
    }
    return rows.first().first().toInt();
}

int userIdByName(const QString &username)
{
    return scalarInt("SELECT user_id FROM `User` WHERE username='" + escapeSql(username) + "' LIMIT 1");
}

int categoryIdByName(const QString &category)
{
    int id = scalarInt("SELECT category_id FROM `Category` WHERE name='" + escapeSql(category) + "' LIMIT 1");
    if (id > 0) {
        return id;
    }

    if (!execSql("INSERT INTO `Category` (name, description) VALUES ('" + escapeSql(category) + "', '" + escapeSql(category) + "')")) {
        return 0;
    }
    return int(mysql_insert_id(g_conn));
}

QString conditionText(int level);

void appendGoodsFromRow(QVector<MarketplaceGoods> &goodsList, const QStringList &row)
{
    if (row.size() < 12) {
        return;
    }

    goodsList.push_back({
        row[0],
        row[1],
        row[2],
        row[3].toDouble(),
        row[4].toInt(),
        row[5].toInt(),
        row[6],
        row[7],
        conditionText(row[8].toInt()),
        row[9].trimmed().isEmpty() ? QString("已上架") : row[9].trimmed(),
        row[10],
        row[11]
    });
}

QString conditionText(int level)
{
    if (level >= 9) return "九成新";
    if (level >= 7) return "八成新";
    if (level >= 5) return "良好";
    return "普通";
}

QString purchaseDisplayName(const QString &rawName, const QString &category, const QString &goodsIdText)
{
    if (!rawName.contains("商品")) return rawName;

    const int id = goodsIdText.toInt();
    if (category.contains("电子") || category.contains("数码")) {
        const QStringList names = {"笔记本电脑", "平板电脑", "蓝牙降噪耳机", "Sony降噪耳机", "佳能EOS相机", "RK机械键盘", "罗技无线鼠标", "宿舍显示器"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (category.contains("书") || category.contains("教材")) {
        const QStringList names = {"高等数学教材", "雅思真题资料", "课堂笔记", "C++ 程序设计"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (category.contains("生活") || category.contains("宿舍")) {
        const QStringList names = {"护眼台灯", "柔软毛毯", "宿舍电风扇", "桌面加湿器", "保温水杯", "宿舍收纳箱", "便携吹风机", "宿舍小冰箱"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
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
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (category.contains("服饰") || category.contains("鞋") || category.contains("包")) {
        const QStringList names = {"通勤双肩包", "帆布包", "九成新运动鞋", "连帽卫衣", "秋季外套", "简约饰品"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (category.contains("运动")) {
        const QStringList names = {"校园自行车", "篮球", "羽毛球拍", "可调哑铃", "瑜伽垫", "滑板"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    if (category.contains("代步")) {
        const QStringList names = {"校园自行车", "校园电动车", "电动滑板车", "智能平衡车"};
        return names.value(id > 0 ? ((id * 37 + id / 10 + 3) % names.size()) : 0);
    }
    return rawName;
}

void resetGoodsCache()
{
    g_goodsLoaded = false;
}

void resetOrdersCache()
{
    g_ordersLoaded = false;
}

} // namespace

QVector<UserAccount>& AccountStore::users()
{
    static QVector<UserAccount> userList;
    if (!g_usersLoaded) {
        userList.clear();
        const QVector<QStringList> rows = queryRows("SELECT username, password_hash, COALESCE(phone, '') FROM `User` ORDER BY user_id");
        for (const QStringList &row : rows) {
            if (row.size() >= 3) {
                userList.push_back({row[0], row[1], row[2]});
            }
        }
        g_usersLoaded = true;
    }
    return userList;
}

bool AccountStore::checkUserLogin(const QString &username, const QString &password)
{
    return scalarInt("SELECT COUNT(*) FROM `User` WHERE username='" + escapeSql(username) +
                     "' AND password_hash='" + escapeSql(password) + "'") > 0;
}

bool AccountStore::checkAdminLogin(const QString &username, const QString &password)
{
    return scalarInt("SELECT COUNT(*) FROM `Admin` WHERE admin_name='" + escapeSql(username) +
                     "' AND password_hash='" + escapeSql(password) + "'") > 0;
}

bool AccountStore::addUser(const QString &username, const QString &password, const QString &phone)
{
    if (username.trimmed().isEmpty() || password.trimmed().isEmpty()) {
        return false;
    }

    const QString email = username + "@example.com";
    const bool ok = execSql("INSERT INTO `User` (username, email, password_hash, phone) VALUES ('" +
                            escapeSql(username) + "', '" + escapeSql(email) + "', '" +
                            escapeSql(password) + "', '" + escapeSql(phone) + "')");
    if (ok) {
        g_usersLoaded = false;
    }
    return ok;
}

QVector<ManagedUser> AccountStore::managedUsers(const QString &keyword,
                                                const QString &role,
                                                const QString &status,
                                                int limit,
                                                int offset)
{
    QVector<ManagedUser> result;
    if (role == "管理员") {
        return result;
    }

    QStringList conditions;
    conditions << "1=1";

    const QString trimmedKeyword = keyword.trimmed();
    if (!trimmedKeyword.isEmpty()) {
        const QString escaped = escapeSql(trimmedKeyword);
        conditions << "(username LIKE '%" + escaped + "%' OR COALESCE(phone, '') LIKE '%" + escaped + "%')";
    }

    if (status != "全部状态") {
        conditions << "status='" + escapeSql(status) + "'";
    }

    QString sql =
        "SELECT user_id, username, password_hash, email, COALESCE(phone, ''), status, "
        "COALESCE(preference_category, '电子数码') "
        "FROM `User` WHERE " + conditions.join(" AND ") + " ORDER BY user_id";
    if (limit > 0) {
        sql += " LIMIT " + QString::number(limit) + " OFFSET " + QString::number(qMax(0, offset));
    }

    const QVector<QStringList> rows = queryRows(sql);

    for (const QStringList &row : rows) {
        if (row.size() >= 7) {
            result.push_back({row[0], row[1], row[2], row[3], row[4], "普通用户", row[5], row[6]});
        }
    }
    return result;
}

int AccountStore::managedUserCount(const QString &keyword,
                                   const QString &role,
                                   const QString &status)
{
    if (role == "管理员") {
        return 0;
    }

    QStringList conditions;
    conditions << "1=1";
    const QString trimmedKeyword = keyword.trimmed();
    if (!trimmedKeyword.isEmpty()) {
        const QString escaped = escapeSql(trimmedKeyword);
        conditions << "(username LIKE '%" + escaped + "%' OR COALESCE(phone, '') LIKE '%" + escaped + "%')";
    }
    if (status != "全部状态") {
        conditions << "status='" + escapeSql(status) + "'";
    }

    return scalarInt("SELECT COUNT(*) FROM `User` WHERE " + conditions.join(" AND "));
}

UserStats AccountStore::managedUserStats()
{
    UserStats stats;
    stats.total = scalarInt("SELECT COUNT(*) FROM `User`");
    stats.normal = scalarInt("SELECT COUNT(*) FROM `User` WHERE status='正常'");
    stats.disabled = scalarInt("SELECT COUNT(*) FROM `User` WHERE status='禁用'");
    stats.admins = 0;
    return stats;
}

bool AccountStore::addManagedUser(const ManagedUser &user, QString *errorMessage)
{
    if (user.username.trimmed().isEmpty() || user.password.trimmed().isEmpty() ||
        user.email.trimmed().isEmpty() || user.phone.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "用户名、密码、邮箱和手机号不能为空。";
        return false;
    }

    const QString preference = user.preference.trimmed().isEmpty() ? QString("电子数码") : user.preference.trimmed();
    const bool ok = execSql("INSERT INTO `User` (username, email, password_hash, phone, status, preference_category) VALUES ('" +
                            escapeSql(user.username) + "', '" + escapeSql(user.email) + "', '" +
                            escapeSql(user.password) + "', '" + escapeSql(user.phone) + "', '" +
                            escapeSql(user.status) + "', '" + escapeSql(preference) + "')");
    if (ok) {
        g_usersLoaded = false;
    } else if (errorMessage) {
        *errorMessage = "INSERT INTO User 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

bool AccountStore::updateManagedUser(const ManagedUser &user, QString *errorMessage)
{
    const int id = user.id.toInt();
    if (id <= 0 || user.username.trimmed().isEmpty() || user.password.trimmed().isEmpty() ||
        user.email.trimmed().isEmpty() || user.phone.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "用户ID、用户名、密码、邮箱和手机号不能为空。";
        return false;
    }

    const QString preference = user.preference.trimmed().isEmpty() ? QString("电子数码") : user.preference.trimmed();
    const bool ok = execSql("UPDATE `User` SET username='" + escapeSql(user.username) +
                            "', email='" + escapeSql(user.email) +
                            "', password_hash='" + escapeSql(user.password) +
                            "', phone='" + escapeSql(user.phone) +
                            "', status='" + escapeSql(user.status) +
                            "', preference_category='" + escapeSql(preference) +
                            "' WHERE user_id=" + QString::number(id));
    if (ok) {
        g_usersLoaded = false;
    } else if (errorMessage) {
        *errorMessage = "UPDATE User 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

bool AccountStore::deleteManagedUser(const QString &userId, QString *errorMessage)
{
    const int id = userId.toInt();
    if (id <= 0) {
        if (errorMessage) *errorMessage = "用户ID无效。";
        return false;
    }

    const bool ok = execSql("DELETE FROM `User` WHERE user_id=" + QString::number(id));
    if (ok) {
        g_usersLoaded = false;
    } else if (errorMessage) {
        *errorMessage = "DELETE FROM User 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

QVector<PurchaseRecord>& AccountStore::purchaseRecords()
{
    static QVector<PurchaseRecord> records;
    return records;
}

void AccountStore::addPurchaseRecord(const QString &name, const QString &category, double price)
{
    addPurchaseRecord(name, category, price, 1, "未完成");
}

void AccountStore::addPurchaseRecord(const QString &name, const QString &category, double price, int quantity, const QString &status)
{
    purchaseRecords().push_back({
        name,
        category,
        price,
        quantity,
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
        status
    });
}

void AccountStore::clearPurchaseRecords()
{
    purchaseRecords().clear();
}

QVector<MarketplaceGoods>& AccountStore::goods()
{
    static QVector<MarketplaceGoods> goodsList;
    if (!g_goodsLoaded) {
        goodsList.clear();
        const QVector<QStringList> rows = queryRows(
            "SELECT g.goods_id, g.title, c.name, g.price, g.stock, COALESCE(g.sales_count, 0), "
            "u.username, COALESCE(g.description, ''), COALESCE(g.condition_level, 8), "
            "COALESCE(g.status, '已上架'), COALESCE(g.image_path, ''), "
            "DATE_FORMAT(g.created_at, '%Y-%m-%d %H:%i:%s') "
            "FROM `Goods` g "
            "JOIN `Category` c ON g.category_id = c.category_id "
            "JOIN `User` u ON g.user_id = u.user_id "
            "WHERE g.is_deleted = 0 "
            "ORDER BY g.goods_id");
        for (const QStringList &row : rows) {
            appendGoodsFromRow(goodsList, row);
        }
        g_goodsLoaded = true;
    }
    return goodsList;
}

QVector<MarketplaceGoods> AccountStore::searchGoods(const QString &keyword,
                                                    const QString &category,
                                                    const QString &status)
{
    QVector<MarketplaceGoods> result;
    QStringList conditions;
    conditions << "g.is_deleted = 0";

    const QString trimmedKeyword = keyword.trimmed();
    if (!trimmedKeyword.isEmpty()) {
        conditions << "g.title LIKE '%" + escapeSql(trimmedKeyword) + "%'";
    }

    if (category != "全部分类") {
        conditions << "c.name = '" + escapeSql(category) + "'";
    }

    if (status != "全部状态") {
        conditions << "COALESCE(g.status, '已上架') = '" + escapeSql(status) + "'";
    }

    const QString sql =
        "SELECT g.goods_id, g.title, c.name, g.price, g.stock, COALESCE(g.sales_count, 0), "
        "u.username, COALESCE(g.description, ''), COALESCE(g.condition_level, 8), "
        "COALESCE(g.status, '已上架'), COALESCE(g.image_path, ''), "
        "DATE_FORMAT(g.created_at, '%Y-%m-%d %H:%i:%s') "
        "FROM `Goods` g "
        "JOIN `Category` c ON g.category_id = c.category_id "
        "JOIN `User` u ON g.user_id = u.user_id "
        "WHERE " + conditions.join(" AND ") + " "
        "ORDER BY g.goods_id DESC";

    qDebug().noquote() << "[GoodsWindow] keyword =" << trimmedKeyword;
    qDebug().noquote() << "[GoodsWindow] category =" << category;
    qDebug().noquote() << "[GoodsWindow] status =" << status;
    qDebug().noquote() << "[GoodsWindow] SQL =" << sql;

    const QVector<QStringList> rows = queryRows(sql);
    qDebug().noquote() << "[GoodsWindow] result count =" << rows.size();

    for (const QStringList &row : rows) {
        appendGoodsFromRow(result, row);
    }

    return result;
}

bool AccountStore::addGoods(const MarketplaceGoods &goods, QString *errorMessage)
{
    if (goods.name.trimmed().isEmpty() || goods.category.trimmed().isEmpty() || goods.price < 0 || goods.stock < 0) {
        if (errorMessage) *errorMessage = "商品名称、分类、价格或库存参数无效。";
        return false;
    }

    int sellerId = userIdByName(goods.seller);
    if (sellerId <= 0) {
        sellerId = scalarInt("SELECT user_id FROM `User` ORDER BY user_id LIMIT 1");
    }
    if (sellerId <= 0) {
        if (errorMessage) {
            *errorMessage = "未找到可关联的卖家用户。";
            if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
        }
        return false;
    }

    const int categoryId = categoryIdByName(goods.category);
    if (categoryId <= 0) {
        if (errorMessage) {
            *errorMessage = "分类不存在或创建失败：" + goods.category;
            if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
        }
        return false;
    }

    const bool ok = execSql(
        "INSERT INTO `Goods` "
        "(user_id, category_id, title, price, stock, description, condition_level, image_path, status, sales_count, is_deleted) VALUES (" +
        QString::number(sellerId) + ", " + QString::number(categoryId) + ", '" +
        escapeSql(goods.name) + "', " + QString::number(goods.price, 'f', 2) + ", " +
        QString::number(goods.stock) + ", '" + escapeSql(goods.description) +
        "', 8, '" + escapeSql(goods.imagePath) + "', '已上架', 0, 0)");

    if (ok) {
        resetGoodsCache();
    } else if (errorMessage) {
        *errorMessage = "INSERT INTO Goods 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

bool AccountStore::updateGoods(const MarketplaceGoods &goods, QString *errorMessage)
{
    const int goodsId = goods.id.toInt();
    if (goodsId <= 0 || goods.name.trimmed().isEmpty() || goods.category.trimmed().isEmpty() ||
        goods.price < 0 || goods.stock < 0) {
        if (errorMessage) *errorMessage = "商品ID、名称、分类、价格或库存参数无效。";
        return false;
    }

    int sellerId = userIdByName(goods.seller);
    if (sellerId <= 0) {
        sellerId = scalarInt("SELECT user_id FROM `Goods` WHERE goods_id=" + QString::number(goodsId) + " LIMIT 1");
    }
    if (sellerId <= 0) {
        if (errorMessage) {
            *errorMessage = "未找到可关联的卖家用户：" + goods.seller;
            if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
        }
        return false;
    }

    const int categoryId = categoryIdByName(goods.category);
    if (categoryId <= 0) {
        if (errorMessage) {
            *errorMessage = "分类不存在或创建失败：" + goods.category;
            if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
        }
        return false;
    }

    const int conditionLevel = goods.condition.contains("全新") ? 10
                              : goods.condition.contains("几乎") ? 9
                              : goods.condition.contains("良好") ? 8
                              : goods.condition.contains("一般") ? 6
                              : 5;

    const bool ok = execSql(
        "UPDATE `Goods` SET user_id=" + QString::number(sellerId) +
        ", category_id=" + QString::number(categoryId) +
        ", title='" + escapeSql(goods.name) +
        "', price=" + QString::number(goods.price, 'f', 2) +
        ", stock=" + QString::number(goods.stock) +
        ", condition_level=" + QString::number(conditionLevel) +
        ", image_path='" + escapeSql(goods.imagePath) +
        "', description='" + escapeSql(goods.description) +
        "' WHERE goods_id=" + QString::number(goodsId) + " AND is_deleted=0");

    if (ok) {
        resetGoodsCache();
    } else if (errorMessage) {
        *errorMessage = "UPDATE Goods 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

QVector<OrderRecord>& AccountStore::orders()
{
    static QVector<OrderRecord> orderList;
    if (!g_ordersLoaded) {
        orderList.clear();
        const QVector<QStringList> rows = queryRows(
            "SELECT o.order_id, g.title, u.username, o.total_price, o.quantity, "
            "o.status, DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i:%s') "
            "FROM `Orders` o "
            "JOIN `Goods` g ON o.goods_id = g.goods_id "
            "JOIN `User` u ON o.user_id = u.user_id "
            "ORDER BY o.order_id DESC");
        for (const QStringList &row : rows) {
            if (row.size() >= 7) {
                const int quantity = qMax(1, row[4].toInt());
                orderList.push_back({row[0], row[1], row[2], row[3].toDouble() / quantity, quantity, row[5], row[6]});
            }
        }
        g_ordersLoaded = true;
    }
    return orderList;
}

QVector<OrderRecord> AccountStore::searchOrders(const QString &keyword,
                                                const QString &status,
                                                int limit,
                                                int offset)
{
    QVector<OrderRecord> result;
    QStringList conditions;
    conditions << "1=1";

    const QString trimmedKeyword = keyword.trimmed();
    if (!trimmedKeyword.isEmpty()) {
        const QString escaped = escapeSql(trimmedKeyword);
        conditions << "(CAST(o.order_id AS CHAR) LIKE '%" + escaped +
                          "%' OR g.title LIKE '%" + escaped +
                          "%' OR u.username LIKE '%" + escaped + "%')";
    }

    if (status != "全部状态") {
        conditions << "o.status='" + escapeSql(status) + "'";
    }

    QString sql =
        "SELECT o.order_id, g.title, u.username, o.total_price, o.quantity, "
        "o.status, DATE_FORMAT(o.created_at, '%Y-%m-%d %H:%i:%s') "
        "FROM `Orders` o "
        "JOIN `Goods` g ON o.goods_id = g.goods_id "
        "JOIN `User` u ON o.user_id = u.user_id "
        "WHERE " + conditions.join(" AND ") + " "
        "ORDER BY o.order_id DESC";
    if (limit > 0) {
        sql += " LIMIT " + QString::number(limit) + " OFFSET " + QString::number(qMax(0, offset));
    }

    const QVector<QStringList> rows = queryRows(sql);
    for (const QStringList &row : rows) {
        if (row.size() >= 7) {
            const int quantity = qMax(1, row[4].toInt());
            result.push_back({row[0], row[1], row[2], row[3].toDouble() / quantity, quantity, row[5], row[6]});
        }
    }
    return result;
}

int AccountStore::orderCount(const QString &keyword, const QString &status)
{
    QStringList conditions;
    conditions << "1=1";
    const QString trimmedKeyword = keyword.trimmed();
    if (!trimmedKeyword.isEmpty()) {
        const QString escaped = escapeSql(trimmedKeyword);
        conditions << "(CAST(o.order_id AS CHAR) LIKE '%" + escaped +
                          "%' OR g.title LIKE '%" + escaped +
                          "%' OR u.username LIKE '%" + escaped + "%')";
    }
    if (status != "全部状态") {
        conditions << "o.status='" + escapeSql(status) + "'";
    }

    return scalarInt(
        "SELECT COUNT(*) "
        "FROM `Orders` o "
        "JOIN `Goods` g ON o.goods_id = g.goods_id "
        "JOIN `User` u ON o.user_id = u.user_id "
        "WHERE " + conditions.join(" AND "));
}

OrderStats AccountStore::orderStats()
{
    OrderStats stats;
    stats.total = scalarInt("SELECT COUNT(*) FROM `Orders`");
    stats.pending = scalarInt("SELECT COUNT(*) FROM `Orders` WHERE status='未完成'");
    stats.done = scalarInt("SELECT COUNT(*) FROM `Orders` WHERE status='已完成'");
    stats.canceled = scalarInt("SELECT COUNT(*) FROM `Orders` WHERE status='已取消'");
    return stats;
}

int AccountStore::maxOrderId()
{
    return scalarInt("SELECT COALESCE(MAX(order_id), 0) FROM `Orders`");
}

bool AccountStore::addOrderRecord(const OrderRecord &order, QString *errorMessage)
{
    const int goodsId = scalarInt("SELECT goods_id FROM `Goods` WHERE title='" + escapeSql(order.goodsName) +
                                  "' AND is_deleted=0 ORDER BY goods_id LIMIT 1");
    const int userId = userIdByName(order.buyer);
    const int quantity = qMax(1, order.quantity);
    if (goodsId <= 0 || userId <= 0) {
        if (errorMessage) *errorMessage = "未找到对应商品或购买用户。";
        return false;
    }

    const bool ok = execSql("INSERT INTO `Orders` (user_id, goods_id, quantity, total_price, status) VALUES (" +
                            QString::number(userId) + ", " + QString::number(goodsId) + ", " +
                            QString::number(quantity) + ", " + QString::number(order.price * quantity, 'f', 2) +
                            ", '" + escapeSql(order.status) + "')");
    if (ok) {
        resetOrdersCache();
    } else if (errorMessage) {
        *errorMessage = "INSERT INTO Orders 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

bool AccountStore::updateOrderRecord(const OrderRecord &order, QString *errorMessage)
{
    const int orderId = order.id.toInt();
    const int goodsId = scalarInt("SELECT goods_id FROM `Goods` WHERE title='" + escapeSql(order.goodsName) +
                                  "' AND is_deleted=0 ORDER BY goods_id LIMIT 1");
    const int userId = userIdByName(order.buyer);
    const int quantity = qMax(1, order.quantity);
    if (orderId <= 0 || goodsId <= 0 || userId <= 0) {
        if (errorMessage) *errorMessage = "订单ID、商品或购买用户无效。";
        return false;
    }

    const bool ok = execSql("UPDATE `Orders` SET user_id=" + QString::number(userId) +
                            ", goods_id=" + QString::number(goodsId) +
                            ", quantity=" + QString::number(quantity) +
                            ", total_price=" + QString::number(order.price * quantity, 'f', 2) +
                            ", status='" + escapeSql(order.status) +
                            "' WHERE order_id=" + QString::number(orderId));
    if (ok) {
        resetOrdersCache();
    } else if (errorMessage) {
        *errorMessage = "UPDATE Orders 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

bool AccountStore::deleteOrderRecord(const QString &orderId, QString *errorMessage)
{
    const int id = orderId.toInt();
    if (id <= 0) {
        if (errorMessage) *errorMessage = "订单ID无效。";
        return false;
    }

    const bool ok = execSql("DELETE FROM `Orders` WHERE order_id=" + QString::number(id));
    if (ok) {
        resetOrdersCache();
    } else if (errorMessage) {
        *errorMessage = "DELETE FROM Orders 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

bool AccountStore::purchaseGoodsById(const QString &goodsIdText, const QString &buyer, int quantity, QString *errorMessage)
{
    return purchaseGoodsById(goodsIdText, buyer, quantity, QString(), QString(), errorMessage);
}

bool AccountStore::purchaseGoodsById(const QString &goodsIdText, const QString &buyer, int quantity, const QString &displayNameSnapshot, const QString &categorySnapshot, QString *errorMessage)
{
    if (quantity <= 0) {
        if (errorMessage) *errorMessage = "购买数量必须大于 0。";
        return false;
    }

    const int requestedId = goodsIdText.toInt();
    if (requestedId <= 0) {
        if (errorMessage) *errorMessage = "商品编号无效。";
        return false;
    }

    const QVector<QStringList> rows = queryRows(
        "SELECT goods_id, title, category_name, price, stock FROM view_goods_detail WHERE goods_id=" +
        QString::number(requestedId) + " LIMIT 1");
    if (rows.isEmpty() || rows.first().size() < 5) {
        if (errorMessage) *errorMessage = "未找到该商品，可能已被删除或下架。";
        return false;
    }

    const int goodsId = rows.first()[0].toInt();
    const QString goodsName = rows.first()[1];
    const QString goodsCategory = rows.first()[2];
    const double price = rows.first()[3].toDouble();
    const int stock = rows.first()[4].toInt();
    if (stock < quantity) {
        if (errorMessage) *errorMessage = "库存不足，当前库存：" + QString::number(stock);
        return false;
    }

    const int buyerId = userIdByName(buyer);
    if (buyerId <= 0) {
        if (errorMessage) *errorMessage = "未找到买家账号，请使用数据库中的用户登录。";
        return false;
    }

    if (!execSql("START TRANSACTION")) return false;
    const bool orderOk = execSql("INSERT INTO `Orders` (user_id, goods_id, quantity, total_price, status) VALUES (" +
                                 QString::number(buyerId) + ", " + QString::number(goodsId) + ", " +
                                 QString::number(quantity) + ", " + QString::number(price * quantity, 'f', 2) + ", '未完成')");
    const bool goodsOk = orderOk && execSql("UPDATE `Goods` SET stock=stock-" + QString::number(quantity) +
                                           ", sales_count=sales_count+" + QString::number(quantity) +
                                           " WHERE goods_id=" + QString::number(goodsId));
    if (!goodsOk) {
        execSql("ROLLBACK");
        if (errorMessage) *errorMessage = g_lastError;
        return false;
    }

    execSql("COMMIT");
    resetGoodsCache();
    resetOrdersCache();
    const QString finalName = displayNameSnapshot.trimmed().isEmpty()
                              ? purchaseDisplayName(goodsName, goodsCategory, QString::number(goodsId))
                              : displayNameSnapshot.trimmed();
    const QString finalCategory = categorySnapshot.trimmed().isEmpty() ? goodsCategory : categorySnapshot.trimmed();
    addPurchaseRecord(finalName, finalCategory, price, quantity, "未完成");
    return true;
}

bool AccountStore::purchaseGoods(const QString &goodsName, const QString &buyer, int quantity, QString *errorMessage)
{
    const QVector<QStringList> rows = queryRows("SELECT goods_id FROM `Goods` WHERE title='" +
                                               escapeSql(goodsName) + "' AND is_deleted=0 ORDER BY goods_id LIMIT 1");
    if (rows.isEmpty() || rows.first().isEmpty()) {
        if (errorMessage) *errorMessage = "未找到该商品，可能已被删除或下架。";
        return false;
    }
    return purchaseGoodsById(rows.first()[0], buyer, quantity, errorMessage);
}

bool AccountStore::submitGoodsForReview(const QString &name,
                                        const QString &category,
                                        double price,
                                        int stock,
                                        const QString &seller,
                                        const QString &description,
                                        const QString &condition,
                                        QString *errorMessage)
{
    if (name.trimmed().isEmpty() || category.trimmed().isEmpty() || price < 0 || stock <= 0) {
        if (errorMessage) *errorMessage = "商品名称、分类、价格或库存参数无效。";
        return false;
    }

    const int sellerId = userIdByName(seller);
    if (sellerId <= 0) {
        if (errorMessage) {
            *errorMessage = "当前用户不存在：" + seller;
            if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
        }
        return false;
    }

    const int categoryId = categoryIdByName(category);
    if (categoryId <= 0) {
        if (errorMessage) {
            *errorMessage = "分类不存在或创建失败：" + category;
            if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
        }
        return false;
    }

    const bool ok = execSql("INSERT INTO `Goods` (user_id, category_id, title, price, stock, description, condition_level, status) VALUES (" +
                            QString::number(sellerId) + ", " + QString::number(categoryId) + ", '" +
                            escapeSql(name) + "', " + QString::number(price, 'f', 2) + ", " + QString::number(stock) +
                            ", '" + escapeSql(description + "\n成色：" + condition) + "', 8, '待审核')");
    if (ok) {
        resetGoodsCache();
    } else if (errorMessage) {
        *errorMessage = "INSERT INTO Goods 失败。";
        if (!g_lastError.trimmed().isEmpty()) *errorMessage += "\nMySQL error: " + g_lastError;
    }
    return ok;
}

bool AccountStore::updateGoodsStatus(const QString &goodsId, const QString &status)
{
    const int id = goodsId.toInt();
    const QString normalized = status.trimmed();
    if (id <= 0 || normalized.isEmpty()) {
        return false;
    }

    const bool ok = execSql("UPDATE `Goods` SET status='" + escapeSql(normalized) +
                            "' WHERE goods_id=" + QString::number(id) + " AND is_deleted=0");
    if (ok) {
        resetGoodsCache();
    }
    return ok;
}
