#ifndef USERLOGINWINDOW_H
#define USERLOGINWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class UserLoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserLoginWindow(QWidget *welcomeWindow, QWidget *parent = nullptr);

private:
    QLineEdit *editUsername;
    QLineEdit *editPassword;
    QPushButton *btnLogin;
    QPushButton *btnRegister;
    QPushButton *btnBack;
    QWidget *m_welcomeWindow;
};

#endif // USERLOGINWINDOW_H
