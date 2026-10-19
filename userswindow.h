#ifndef USERSWINDOW_H
#define USERSWINDOW_H

#include <QWidget>
#include <QVector>
#include <QString>

class QTableWidget;
class QPushButton;
class QComboBox;
class QLabel;
class QLineEdit;
class MainWindow;
class QCloseEvent;

struct UserItem
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

class UsersWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UsersWindow(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~UsersWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void loadSampleData();
    void updateTable();
    void updateStats();
    void updatePageLabel();
    int currentRowIndex() const;

    void addUser();
    void editUser();
    void showUserDetail(const QString &userId);
    void editUser(const QString &userId);
    void toggleUserStatus(const QString &userId);
    void deleteUser();
    void recommendGoods();

private:
    QTableWidget *table;
    QLineEdit *editSearch;
    QComboBox *comboRole;
    QComboBox *comboStatus;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QPushButton *btnRecommend;
    QPushButton *btnBack;
    QPushButton *btnPrevPage;
    QPushButton *btnNextPage;
    QLabel *labelPageInfo;
    QLabel *labelTotalMetric;
    QLabel *labelNormalMetric;
    QLabel *labelDisabledMetric;
    QLabel *labelAdminMetric;

    QVector<UserItem> m_users;
    int m_page = 0;
    int m_pageSize = 100;
    int m_totalRows = 0;
    MainWindow *m_mainWindow;
};

#endif // USERSWINDOW_H
