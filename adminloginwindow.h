#ifndef ADMINLOGINWINDOW_H
#define ADMINLOGINWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class AdminLoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminLoginWindow(QWidget *welcomeWindow, QWidget *parent = nullptr);

private:
    QLineEdit *editUsername;
    QLineEdit *editPassword;
    QPushButton *btnLogin;
    QPushButton *btnBack;
    QWidget *m_welcomeWindow;
};

#endif // ADMINLOGINWINDOW_H
