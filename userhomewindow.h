#ifndef USERHOMEWINDOW_H
#define USERHOMEWINDOW_H

#include <QWidget>
#include <QSet>
#include <QString>
#include <QVector>

class QLabel;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QLineEdit;
class QComboBox;
class QGridLayout;
class QVBoxLayout;
class QSpinBox;
class QTextEdit;
class QWidget;
class QScrollArea;

struct UserGoodsItem
{
    QString id;
    QString name;
    QString category;
    double price;
    int stock;
};

class UserHomeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserHomeWindow(const QString &username, QWidget *parent = nullptr);

private:
    QWidget *createSidebar();
    QWidget *createHomePage();
    QWidget *createCategoryPage();
    QWidget *createCartPage();
    QWidget *createSellPage();
    QWidget *createHistoryPage();
    QWidget *createProfilePage();

    QPushButton *makeNavButton(const QString &text);
    QPushButton *makePrimaryButton(const QString &text);
    QPushButton *makeDangerButton(const QString &text);
    QLabel *makeTitle(const QString &text);
    QLabel *makeSubtitle(const QString &text);
    QWidget *makeCard();

    void switchPage(QWidget *page, QPushButton *activeButton);
    void updateSidebarHighlight(QPushButton *activeButton);
    void loadGoodsData();
    QVector<UserGoodsItem> filteredGoods() const;
    void updateGoodsTable(const QVector<UserGoodsItem> &items);
    void updateCategoryTable();
    void updateCartTable();
    void updateHistoryTable();
    void updateHistoryStats();

    void searchGoods();
    void recommendGoods();
    void addToCart();
    void buyGoods();
    void buySelectedCartItem();
    void removeSelectedCartItem();
    void submitGoodsForSale();
    void clearHistory();
    int selectedGoodsRow() const;
    int selectedCartRow() const;

private:
    QString m_username;
    QVector<UserGoodsItem> m_goods;
    QVector<UserGoodsItem> m_currentItems;
    QVector<UserGoodsItem> m_cart;
    QSet<int> m_selectedCartRows;

    QStackedWidget *m_stack = nullptr;
    QWidget *m_homePage = nullptr;
    QWidget *m_categoryPage = nullptr;
    QWidget *m_cartPage = nullptr;
    QWidget *m_sellPage = nullptr;
    QWidget *m_historyPage = nullptr;
    QWidget *m_profilePage = nullptr;

    QPushButton *m_btnHome = nullptr;
    QPushButton *m_btnCategory = nullptr;
    QPushButton *m_btnCart = nullptr;
    QPushButton *m_btnSell = nullptr;
    QPushButton *m_btnHistory = nullptr;
    QPushButton *m_btnProfile = nullptr;

    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_categoryCombo = nullptr;
    QLineEdit *m_minPriceEdit = nullptr;
    QLineEdit *m_maxPriceEdit = nullptr;
    QTableWidget *m_goodsTable = nullptr;
    QTableWidget *m_categoryTable = nullptr;
    QGridLayout *m_homeCategoryGrid = nullptr;
    QScrollArea *m_homeCategoryScroll = nullptr;
    QWidget *m_homeCategoryViewport = nullptr;
    QGridLayout *m_categoryGoodsGrid = nullptr;
    QLineEdit *m_categorySearchEdit = nullptr;
    QVector<QPushButton *> m_categoryPillButtons;
    QTableWidget *m_cartTable = nullptr;
    QTableWidget *m_historyTable = nullptr;

    QLabel *m_cartTotalLabel = nullptr;
    QLabel *m_cartItemCountLabel = nullptr;
    QLabel *m_cartSubtotalLabel = nullptr;
    QLabel *m_cartPayableLabel = nullptr;
    QVBoxLayout *m_cartCardsLayout = nullptr;
    QLabel *m_historyOrderCountLabel = nullptr;
    QLabel *m_historyAmountLabel = nullptr;
    QLabel *m_historyPendingLabel = nullptr;
    QLabel *m_historyDoneLabel = nullptr;
    QVBoxLayout *m_historyListLayout = nullptr;

    QLineEdit *m_sellNameEdit = nullptr;
    QComboBox *m_sellCategoryCombo = nullptr;
    QLineEdit *m_sellPriceEdit = nullptr;
    QSpinBox *m_sellStockSpin = nullptr;
    QLineEdit *m_sellConditionEdit = nullptr;
    QTextEdit *m_sellDescriptionEdit = nullptr;
};

#endif // USERHOMEWINDOW_H
