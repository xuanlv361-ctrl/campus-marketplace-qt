#ifndef ORDERSWINDOW_H
#define ORDERSWINDOW_H

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

struct OrderItem
{
    QString id;
    QString goodsName;
    QString buyer;
    QString price;
    int quantity;
    QString status;
    QString time;
};

class OrdersWindow : public QWidget
{
    Q_OBJECT

public:
    explicit OrdersWindow(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~OrdersWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void loadSampleData();
    void updateTable();
    void updatePageLabel();
    int currentRowIndex() const;

    void addOrder();
    void editOrder();
    void showOrderDetail(const QString &orderId);
    void editOrder(const QString &orderId);
    void deleteOrder();

private:
    QTableWidget *table;
    QLineEdit *editSearch;
    QComboBox *comboStatus;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QPushButton *btnBack;
    QPushButton *btnPrevPage;
    QPushButton *btnNextPage;
    QLabel *labelPageInfo;

    QVector<OrderItem> m_orders;
    int m_page = 0;
    int m_pageSize = 100;
    int m_totalRows = 0;
    MainWindow *m_mainWindow;
};

#endif // ORDERSWINDOW_H
