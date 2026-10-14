#ifndef CARTWINDOW_H
#define CARTWINDOW_H

#include <QStringList>
#include <QWidget>

class QLabel;
class QPushButton;
class QScrollArea;
class QTableWidget;
class QVBoxLayout;
class QWidget;

class CartWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CartWindow(const QStringList &cartItems,
                        const QString &buyerName = "当前用户",
                        QWidget *parent = nullptr);

private:
    void buySelectedItem();
    void removeSelectedItem();
    void updateTotal();
    void updateEmptyState();
    void refreshCartCards();
    QStringList parseCartItem(const QString &itemText) const;

private:
    QTableWidget *table;
    QScrollArea *cardsScroll;
    QWidget *cardsContainer;
    QVBoxLayout *cardsLayout;
    QLabel *labelTotal;
    QLabel *labelItemCount;
    QLabel *labelSubtotal;
    QLabel *labelPayable;
    QLabel *emptyLabel;
    QPushButton *btnBuy;
    QPushButton *btnRemove;
    QPushButton *btnClose;
    QString m_buyerName;
};

#endif // CARTWINDOW_H
