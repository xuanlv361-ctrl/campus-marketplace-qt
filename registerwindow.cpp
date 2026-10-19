#include "registerwindow.h"

#include "accountstore.h"
#include "languagemanager.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

const char *kRegisterQss = R"(
QWidget {
    background: #FFF8EF;
    color: #3D2C2E;
    font-family: "Microsoft YaHei", "Segoe UI", Arial;
    font-size: 14px;
}
QFrame#AuthCard {
    background: #ffffff;
    border: 1px solid #EADFD3;
    border-radius: 28px;
}
QLabel#Title {
    color: #3D2C2E;
    font-size: 28px;
    font-weight: 800;
}
QLabel#Subtitle {
    color: #8A7A72;
    font-size: 13px;
}
QLineEdit {
    background: #ffffff;
    border: 1px solid #EADFD3;
    border-radius: 22px;
    color: #3D2C2E;
    padding: 0 18px;
    font-size: 14px;
}
QLineEdit:focus {
    border: 1px solid #F4A261;
}
QPushButton {
    min-height: 44px;
    border-radius: 22px;
    font-size: 15px;
    font-weight: 700;
}
QPushButton#PrimaryButton {
    background: #F4A261;
    color: #ffffff;
    border: none;
}
QPushButton#PrimaryButton:hover {
    background: #E98F47;
}
QPushButton#SecondaryButton {
    background: #ffffff;
    color: #3D2C2E;
    border: 1px solid #EADFD3;
}
QPushButton#SecondaryButton:hover {
    background: #FFF8EF;
    border-color: #F4A261;
    color: #B76E35;
}
)";

void addSoftShadow(QWidget *widget)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 24));
    widget->setGraphicsEffect(shadow);
}

} // namespace

RegisterWindow::RegisterWindow(QWidget *loginWindow, QWidget *parent)
    : QWidget(parent), m_loginWindow(loginWindow)
{
    resize(760, 560);
    setMinimumSize(680, 520);
    setWindowTitle("用户注册");
    setStyleSheet(kRegisterQss);

    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(40, 36, 40, 36);

    auto *cardRow = new QHBoxLayout;
    cardRow->addStretch();

    auto *card = new QFrame;
    card->setObjectName("AuthCard");
    card->setFixedWidth(430);
    addSoftShadow(card);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 34, 36, 30);
    layout->setSpacing(14);

    auto *title = new QLabel("注册账号");
    title->setObjectName("Title");
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel("创建账号后即可发布与购买校园闲置");
    subtitle->setObjectName("Subtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    editUsername = new QLineEdit;
    editUsername->setPlaceholderText("注册账号");
    editUsername->setMinimumHeight(46);

    editPassword = new QLineEdit;
    editPassword->setPlaceholderText("密码");
    editPassword->setEchoMode(QLineEdit::Password);
    editPassword->setMinimumHeight(46);

    editPhone = new QLineEdit;
    editPhone->setPlaceholderText("手机号");
    editPhone->setMinimumHeight(46);

    btnSubmit = new QPushButton("注册");
    btnSubmit->setObjectName("PrimaryButton");

    btnBack = new QPushButton("返回登录页");
    btnBack->setObjectName("SecondaryButton");

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(16);
    layout->addWidget(editUsername);
    layout->addWidget(editPassword);
    layout->addWidget(editPhone);
    layout->addSpacing(8);
    layout->addWidget(btnSubmit);
    layout->addWidget(btnBack);

    cardRow->addWidget(card);
    cardRow->addStretch();

    auto *languageRow = new QHBoxLayout;
    languageRow->addStretch();
    languageRow->addWidget(LanguageManager::instance().createSelector(this));
    pageLayout->addLayout(languageRow);
    pageLayout->addStretch();
    pageLayout->addLayout(cardRow);
    pageLayout->addStretch();

    connect(btnSubmit, &QPushButton::clicked, this, [=]() {
        QString username = editUsername->text().trimmed();
        QString password = editPassword->text().trimmed();
        QString phone = editPhone->text().trimmed();

        if (username.isEmpty() || password.isEmpty() || phone.isEmpty()) {
            QMessageBox::warning(this, "提示", "账号、密码、手机号不能为空。");
            return;
        }

        bool ok = AccountStore::addUser(username, password, phone);

        if (!ok) {
            QMessageBox::warning(this, "注册失败", "该账号已存在。");
            return;
        }

        QMessageBox::information(this, "注册成功", "注册成功，请返回登录。");

        if (m_loginWindow) {
            m_loginWindow->show();
        }

        close();
    });

    connect(btnBack, &QPushButton::clicked, this, [=]() {
        if (m_loginWindow) {
            m_loginWindow->show();
        }
        close();
    });
}
