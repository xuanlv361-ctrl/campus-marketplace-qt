#ifndef ACCOUNTSTORE_H
#define ACCOUNTSTORE_H

#include <QString>
#include <QVector>

struct UserAccount
{
    QString username;
    QString password;
    QString phone;
};

struct ManagedUser
{
    QString id;
    QString username;
    QString password;
    QString email;
    QString phone;
    QString role;
    QString status;
    QString preference;
};

struct UserStats
{
    int total = 0;
    int normal = 0;
    int disabled = 0;
    int admins = 0;
};

struct PurchaseRecord
{
    QString name;
    QString category;
    double price;
    int quantity;
    QString time;
    QString status;
};

struct MarketplaceGoods
{
    QString id;
    QString name;
    QString category;
    double price;
    int stock;
    int sales;
    QString seller;
    QString description;
    QString condition;
    QString status;
    QString imagePath;
    QString createdAt;
};

struct OrderRecord
{
    QString id;
    QString goodsName;
    QString buyer;
    double price;
    int quantity;
    QString status;
    QString time;
};

struct OrderStats
{
    int total = 0;
    int pending = 0;
    int done = 0;
    int canceled = 0;
};

class AccountStore
{
public:
    static QVector<UserAccount>& users();

    static bool checkUserLogin(const QString &username, const QString &password);
    static bool checkAdminLogin(const QString &username, const QString &password);

    static bool addUser(const QString &username,
                        const QString &password,
                        const QString &phone);

    static QVector<ManagedUser> managedUsers(const QString &keyword = QString(),
                                             const QString &role = QStringLiteral("全部角色"),
                                             const QString &status = QStringLiteral("全部状态"),
                                             int limit = 0,
                                             int offset = 0);

    static int managedUserCount(const QString &keyword = QString(),
                                const QString &role = QStringLiteral("全部角色"),
                                const QString &status = QStringLiteral("全部状态"));

    static UserStats managedUserStats();

    static bool addManagedUser(const ManagedUser &user,
                               QString *errorMessage = nullptr);

    static bool updateManagedUser(const ManagedUser &user,
                                  QString *errorMessage = nullptr);

    static bool deleteManagedUser(const QString &userId,
                                  QString *errorMessage = nullptr);

    static QVector<PurchaseRecord>& purchaseRecords();

    static void addPurchaseRecord(const QString &name,
                                  const QString &category,
                                  double price);

    static void addPurchaseRecord(const QString &name,
                                  const QString &category,
                                  double price,
                                  int quantity,
                                  const QString &status);

    static void clearPurchaseRecords();

    static QVector<MarketplaceGoods>& goods();
    static QVector<MarketplaceGoods> searchGoods(const QString &keyword,
                                                 const QString &category,
                                                 const QString &status);
    static QVector<OrderRecord>& orders();

    static QVector<OrderRecord> searchOrders(const QString &keyword = QString(),
                                             const QString &status = QStringLiteral("全部状态"),
                                             int limit = 0,
                                             int offset = 0);

    static int orderCount(const QString &keyword = QString(),
                          const QString &status = QStringLiteral("全部状态"));

    static OrderStats orderStats();

    static int maxOrderId();

    static bool addOrderRecord(const OrderRecord &order,
                               QString *errorMessage = nullptr);

    static bool updateOrderRecord(const OrderRecord &order,
                                  QString *errorMessage = nullptr);

    static bool deleteOrderRecord(const QString &orderId,
                                  QString *errorMessage = nullptr);

    static bool addGoods(const MarketplaceGoods &goods,
                         QString *errorMessage = nullptr);

    static bool updateGoods(const MarketplaceGoods &goods,
                            QString *errorMessage = nullptr);

    static bool purchaseGoods(const QString &goodsName,
                              const QString &buyer,
                              int quantity,
                              QString *errorMessage = nullptr);

    static bool purchaseGoodsById(const QString &goodsId,
                                  const QString &buyer,
                                  int quantity,
                                  QString *errorMessage = nullptr);

    static bool purchaseGoodsById(const QString &goodsId,
                                  const QString &buyer,
                                  int quantity,
                                  const QString &displayNameSnapshot,
                                  const QString &categorySnapshot,
                                  QString *errorMessage = nullptr);

    static bool submitGoodsForReview(const QString &name,
                                     const QString &category,
                                     double price,
                                     int stock,
                                     const QString &seller,
                                     const QString &description,
                                     const QString &condition,
                                     QString *errorMessage = nullptr);

    static bool updateGoodsStatus(const QString &goodsId,
                                  const QString &status);
};

#endif // ACCOUNTSTORE_H

