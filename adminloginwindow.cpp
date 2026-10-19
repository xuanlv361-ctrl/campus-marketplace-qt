#include "adminloginwindow.h"

#include "accountstore.h"
#include "languagemanager.h"
#include "mainwindow.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

const char *kAdminAuthQss = R"(
QWidget {
    background: #F4F4F7;
    color: #111111;
    font-family: "Microsoft YaHei", "Segoe UI", Arial;
    font-size: 14px;
}
QFrame#Shell {
    background: #FFFFFF;
    border: 1px solid #DDDDE6;
    border-radius: 22px;
}
QFrame#OfferBar {
    background: #EA22D8;
    border: none;
    border-top-left-radius: 22px;
    border-top-right-radius: 22px;
}
QLabel#OfferText {
    background: transparent;
    color: #FFFFFF;
    font-size: 12px;
    font-weight: 800;
}
QFrame#AuthCard {
    background: #FFFFFF;
    border: 1px solid #E7E7EF;
    border-radius: 22px;
}
QLabel#Title {
    color: #050505;
    font-size: 34px;
    font-weight: 900;
}
QLabel#Subtitle {
    color: #55545D;
    font-size: 14px;
}
QLabel#Badge {
    background: #C7FF00;
    color: #050505;
    border-radius: 13px;
    padding: 5px 12px;
    font-size: 12px;
    font-weight: 900;
}
QLineEdit {
    background: #F6F6F8;
    border: 1px solid #E2E2EA;
    border-radius: 18px;
    color: #111111;
    padding: 0 18px;
    font-size: 15px;
}
QLineEdit:focus {
    border: 1px solid #EA22D8;
    background: #FFFFFF;
}
QPushButton {
    min-height: 42px;
    border-radius: 21px;
    font-size: 15px;
    font-weight: 900;
}
QPushButton#PrimaryButton {
    background: #C7FF00;
    color: #050505;
    border: none;
}
QPushButton#PrimaryButton:hover {
    background: #B5F000;
}
QPushButton#GhostButton {
    background: transparent;
    color: #55545D;
    border: none;
    font-weight: 700;
}
QPushButton#GhostButton:hover {
    color: #EA22D8;
}
)";

void addSoftShadow(QWidget *widget, const QColor &color = QColor(20, 20, 40, 22))
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(32);
    shadow->setOffset(0, 12);
    shadow->setColor(color);
    widget->setGraphicsEffect(shadow);
}

} // namespace

AdminLoginWindow::AdminLoginWindow(QWidget *welcomeWindow, QWidget *parent)
    : QWidget(parent), m_welcomeWindow(welcomeWindow)
{
    resize(820, 580);
    setMinimumSize(720, 540);
    setWindowTitle("管理员登录");
    setStyleSheet(kAdminAuthQss);

    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(34, 28, 34, 28);

    auto *shell = new QFrame;
    shell->setObjectName("Shell");
    addSoftShadow(shell);

    auto *shellLayout = new QVBoxLayout(shell);
    shellLayout->setContentsMargins(0, 0, 0, 34);
    shellLayout->setSpacing(0);

    auto *offerBar = new QFrame;
    offerBar->setObjectName("OfferBar");
    offerBar->setFixedHeight(28);
    auto *offerLayout = new QHBoxLayout(offerBar);
    offerLayout->setContentsMargins(0, 0, 0, 0);
    auto *offerText = new QLabel("▷ 校园二手交易平台 · 管理员入口");
    offerText->setObjectName("OfferText");
    offerText->setAlignment(Qt::AlignCenter);
    offerLayout->addWidget(offerText);

    auto *cardRow = new QHBoxLayout;
    cardRow->addStretch();

    auto *card = new QFrame;
    card->setObjectName("AuthCard");
    card->setFixedWidth(460);
    addSoftShadow(card, QColor(20, 20, 40, 16));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(38, 36, 38, 32);
    layout->setSpacing(14);

    auto *badgeRow = new QHBoxLayout;
    badgeRow->addStretch();
    auto *badge = new QLabel("MANAGEMENT");
    badge->setObjectName("Badge");
    badgeRow->addWidget(badge);
    badgeRow->addStretch();

    auto *title = new QLabel("管理员登录");
    title->setObjectName("Title");
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel("进入后台管理商品、订单與用户信息");
    subtitle->setObjectName("Subtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    editUsername = new QLineEdit;
    editUsername->setPlaceholderText("请输入管理员账号，例如 admin1");
    editUsername->setMinimumHeight(46);

    editPassword = new QLineEdit;
    editPassword->setPlaceholderText("请输入管理员密码");
    editPassword->setEchoMode(QLineEdit::Password);
    editPassword->setMinimumHeight(46);

    btnLogin = new QPushButton("登录后台");
    btnLogin->setObjectName("PrimaryButton");

    btnBack = new QPushButton("返回封面");
    btnBack->setObjectName("GhostButton");

    layout->addLayout(badgeRow);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(18);
    layout->addWidget(editUsername);
    layout->addWidget(editPassword);
    layout->addSpacing(8);
    layout->addWidget(btnLogin);
    layout->addWidget(btnBack);

    cardRow->addWidget(card);
    cardRow->addStretch();

    shellLayout->addWidget(offerBar);
    auto *languageRow = new QHBoxLayout;
    languageRow->setContentsMargins(0, 12, 20, 0);
    languageRow->addStretch();
    languageRow->addWidget(LanguageManager::instance().createSelector(shell));
    shellLayout->addLayout(languageRow);
    shellLayout->addStretch();
    shellLayout->addLayout(cardRow);
    shellLayout->addStretch();

    pageLayout->addWidget(shell);

    connect(btnLogin, &QPushButton::clicked, this, [=]() {
        QString username = editUsername->text().trimmed();
        QString password = editPassword->text().trimmed();

        if (AccountStore::checkAdminLogin(username, password)) {
            MainWindow *w = new MainWindow;
            w->show();
            this->close();
        } else {
            QMessageBox::warning(this, "登录失败", "管理员账号或密码错误。请使用数据库管理员，例如 admin1 / 123456。");
        }
    });

    connect(btnBack, &QPushButton::clicked, this, [=]() {
        if (m_welcomeWindow) {
            m_welcomeWindow->show();
        }
        this->close();
    });
}
