#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

class QLabel;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QTextEdit;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showDashboard();
    void openGoodsWindow(bool showPending = false);
    void openOrdersWindow();
    void openUsersWindow();
    void openStatsWindow();
    void backToWelcome();

private:
    QWidget *createSidebar();
    QWidget *createDashboardPage();
    QWidget *createGoodsPage();
    QWidget *createOrdersPage();
    QWidget *createUsersPage();
    QWidget *createStatsPage();

    QPushButton *makeNavButton(const QString &text);
    QPushButton *makePrimaryButton(const QString &text);
    QPushButton *makeDangerButton(const QString &text);
    QLabel *makeTitle(const QString &text);
    QLabel *makeSubtitle(const QString &text);
    QWidget *makeCard();
    void switchPage(QWidget *page, QPushButton *activeButton);
    void updateSidebarHighlight(QPushButton *activeButton);
    void refreshAll();

    void refreshDashboard();
    void refreshGoodsTable(bool pendingOnly = false);
    void refreshOrdersTable();
    void refreshUsersTable();
    void refreshStatsPage();

    void addGoods();
    void editGoods();
    void deleteGoods();
    void approveSelectedGoods();
    void rejectSelectedGoods();
    void addOrder();
    void editOrder();
    void deleteOrder();
    void addUser();
    void editUser();
    void deleteUser();
    void recommendGoodsForUser();
    int selectedGoodsIndex() const;
    int selectedOrderIndex() const;
    int selectedUserIndex() const;

private:
    QStackedWidget *m_stack = nullptr;

    QWidget *m_dashboardPage = nullptr;
    QWidget *m_goodsPage = nullptr;
    QWidget *m_ordersPage = nullptr;
    QWidget *m_usersPage = nullptr;
    QWidget *m_statsPage = nullptr;

    QPushButton *m_btnDashboard = nullptr;
    QPushButton *m_btnGoods = nullptr;
    QPushButton *m_btnOrders = nullptr;
    QPushButton *m_btnUsers = nullptr;
    QPushButton *m_btnStats = nullptr;

    QLabel *m_totalGoodsLabel = nullptr;
    QLabel *m_pendingGoodsLabel = nullptr;
    QLabel *m_orderCountLabel = nullptr;
    QLabel *m_userCountLabel = nullptr;

    QTableWidget *m_goodsTable = nullptr;
    QLineEdit *m_goodsSearchEdit = nullptr;
    QComboBox *m_goodsCategoryCombo = nullptr;
    QComboBox *m_goodsStatusCombo = nullptr;
    QLabel *m_goodsTotalMetric = nullptr;
    QLabel *m_goodsOnlineMetric = nullptr;
    QLabel *m_goodsPendingMetric = nullptr;
    QLabel *m_goodsLowStockMetric = nullptr;

    QTableWidget *m_ordersTable = nullptr;
    QLineEdit *m_ordersSearchEdit = nullptr;
    QComboBox *m_ordersStatusCombo = nullptr;
    QLabel *m_ordersTotalMetric = nullptr;
    QLabel *m_ordersPendingMetric = nullptr;
    QLabel *m_ordersDoneMetric = nullptr;
    QLabel *m_ordersCancelMetric = nullptr;

    QTableWidget *m_usersTable = nullptr;
    QLineEdit *m_usersSearchEdit = nullptr;
    QComboBox *m_usersRoleCombo = nullptr;
    QComboBox *m_usersStatusCombo = nullptr;
    QLabel *m_usersTotalMetric = nullptr;
    QLabel *m_usersNormalMetric = nullptr;
    QLabel *m_usersDisabledMetric = nullptr;
    QLabel *m_usersAdminMetric = nullptr;

    QLabel *m_statsSummaryLabel = nullptr;
    QTableWidget *m_statsTable = nullptr;
    QLabel *m_statsGoodsMetric = nullptr;
    QLabel *m_statsUsersMetric = nullptr;
    QLabel *m_statsOrdersMetric = nullptr;
    QLabel *m_statsDoneMetric = nullptr;

};

#endif // MAINWINDOW_H
