#ifndef GOODSWINDOW_H
#define GOODSWINDOW_H

#include <QWidget>
#include <QVector>
#include <QString>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QVBoxLayout;
class MainWindow;
class QCloseEvent;

struct GoodsItem
{
    QString id;
    QString name;
    QString category;
    QString price;
    QString stock;
    QString sales;
    QString status;
};

class GoodsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GoodsWindow(MainWindow *mainWindow, QWidget *parent = nullptr, bool showPendingOnOpen = false);
    ~GoodsWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void loadSampleData();
    void updateTable(const QVector<GoodsItem> &items);
    void updateDashboardStats();
    void updateSalesRanking();
    void searchGoods();
    void refreshGoods();
    void showPendingGoods();
    int currentRowIndex() const;
    void showGoodsDetail(const QString &goodsId);
    void addGoods();
    void editGoods();
    void editGoods(const QString &goodsId);
    void deleteGoods();
    void showGoodsSales();
    int selectedGoodsStoreIndex() const;
    int goodsStoreIndexById(const QString &id) const;

private:
    QLineEdit *editSearch;
    QComboBox *comboCategory;
    QComboBox *comboStatus;
    QPushButton *btnSearch;
    QPushButton *btnReset;
    QPushButton *btnRefresh;
    QPushButton *btnLoad;
    QPushButton *btnAdd;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QPushButton *btnSales;
    QPushButton *btnAudit;
    QPushButton *btnExport;
    QPushButton *btnBack;
    QTableWidget *table;
    QLabel *labelHeaderStats;
    QLabel *labelTotalMetric;
    QLabel *labelOnlineMetric;
    QLabel *labelPendingMetric;
    QLabel *labelLowStockMetric;
    QVBoxLayout *rankLayout;

    QVector<GoodsItem> m_goodsData;
    MainWindow *m_mainWindow;
};

#endif // GOODSWINDOW_H
