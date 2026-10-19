#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include <QString>
#include <QStringList>
#include <QWidget>

struct PurchaseRecord;

class QFrame;
class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

class HistoryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWindow(const QStringList &historyItems,
                           const QString &username = "user1",
                           QWidget *parent = nullptr);

private:
    void loadRecords();
    void renderStats(const QVector<PurchaseRecord> &records);
    void renderOrders(const QVector<PurchaseRecord> &records);

private:
    QString m_username;
    QLabel *orderCountLabel;
    QLabel *totalSpentLabel;
    QLabel *pendingCountLabel;
    QLabel *finishedCountLabel;
    QScrollArea *orderScroll;
    QWidget *orderViewport;
    QVBoxLayout *orderListLayout;
    QFrame *emptyCard;
    QPushButton *btnClear;
};

#endif // HISTORYWINDOW_H
