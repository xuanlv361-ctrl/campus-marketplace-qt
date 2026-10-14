#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *loginWindow, QWidget *parent = nullptr);

private:
    QLineEdit *editUsername;
    QLineEdit *editPassword;
    QLineEdit *editPhone;
    QPushButton *btnSubmit;
    QPushButton *btnBack;

    QWidget *m_loginWindow;
};

#endif // REGISTERWINDOW_H
