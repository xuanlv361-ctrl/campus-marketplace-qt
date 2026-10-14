#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QList>
#include <QWidget>

class QFrame;
class QPropertyAnimation;
class QPushButton;

class WelcomeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);

private:
    QFrame *createFloatingCard(const QString &imagePath,
                               const QString &title,
                               const QString &description);
    QWidget *createFloatingCardSlot(QFrame *card);
    void startFloatingAnimation();
    void applyWarmMarketStyle();

private:
    QPushButton *btnUserLogin;
    QPushButton *btnAdminLogin;
    QList<QFrame*> floatingCards;
    QList<QPropertyAnimation*> floatingAnimations;
};

#endif // WELCOMEWINDOW_H