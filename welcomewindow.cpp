#include "welcomewindow.h"

#include "adminloginwindow.h"
#include "languagemanager.h"
#include "productimagemanager.h"
#include "userloginwindow.h"

#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace {

const char *kCoverQss = R"(
QWidget#WelcomeWindow {
    background: #F4F4F7;
    color: #111111;
    font-family: "Microsoft YaHei", "Segoe UI", Arial;
    font-size: 14px;
}
QFrame#BrowserShell {
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
QFrame#NavBar {
    background: #FFFFFF;
    border: none;
    border-bottom: 1px solid #EFEFF3;
}
QLabel#BrandDot {
    background: #111111;
    color: #FFFFFF;
    border-radius: 15px;
    font-size: 18px;
    font-weight: 900;
}
QLabel#NavItem {
    color: #393941;
    font-size: 13px;
    font-weight: 700;
}
QLabel#SearchBox {
    background: #F6F6F8;
    color: #8B8B95;
    border: 1px solid #E9E9EF;
    border-radius: 16px;
    padding: 0 18px;
    font-size: 13px;
}
QLabel#HeroImage {
    background: transparent;
}
QLabel#HeroTitle {
    color: #050505;
    font-size: 58px;
    font-weight: 900;
    letter-spacing: 0px;
}
QLabel#HeroSubtitle {
    color: #55545D;
    font-size: 15px;
}
QPushButton#UserEntry {
    background: #C7FF00;
    color: #050505;
    border: none;
    border-radius: 18px;
    font-size: 14px;
    font-weight: 900;
    padding: 8px 24px;
}
QPushButton#UserEntry:hover {
    background: #B5F000;
}
QPushButton#AdminEntry {
    background: #FFFFFF;
    color: #050505;
    border: 1px solid #DCDCE5;
    border-radius: 18px;
    font-size: 14px;
    font-weight: 900;
    padding: 8px 24px;
}
QPushButton#AdminEntry:hover {
    background: #F6F6F8;
}
QFrame#ResourceCard {
    background: #FFFFFF;
    border: 1px solid #E7E7EF;
    border-radius: 12px;
}
QLabel#ResourceImage {
    background: #F5F5F7;
    border: none;
    border-top-left-radius: 12px;
    border-top-right-radius: 12px;
}
QLabel#ResourceTitle {
    background: transparent;
    color: #111111;
    font-size: 12px;
    font-weight: 900;
}
QPushButton#NavLogin {
    background: transparent;
    color: #111111;
    border: none;
    font-size: 13px;
    font-weight: 700;
    padding: 0 8px;
}
QPushButton#ProButton {
    background: #C7FF00;
    color: #111111;
    border: none;
    border-radius: 8px;
    font-size: 13px;
    font-weight: 900;
    padding: 6px 12px;
}
)";

void addSoftShadow(QWidget *widget, const QColor &color = QColor(20, 20, 40, 20), int blur = 32)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blur);
    shadow->setOffset(0, 12);
    shadow->setColor(color);
    widget->setGraphicsEffect(shadow);
}

QString firstExistingPath(const QStringList &paths)
{
    for (const QString &path : paths) {
        if (QFileInfo::exists(path)) return path;
    }
    return ProductImageManager::getImagePath(QString());
}

QString welcomeCardImagePath(const QString &fileName)
{
    return ProductImageManager::imagePathForProduct(QFileInfo(fileName).completeBaseName());
}

void welcomeLog(const QString &message)
{
    qDebug().noquote() << message;
}

QString yesNo(bool value)
{
    return value ? "true" : "false";
}

QLabel *navItem(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName("NavItem");
    return label;
}

} // namespace

WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QWidget(parent),
      btnUserLogin(nullptr),
      btnAdminLogin(nullptr)
{
    resize(1180, 740);
    setMinimumSize(1040, 680);
    setWindowTitle("校园二手交易平台");
    setObjectName("WelcomeWindow");
    welcomeLog("WelcomeWindow constructor started");
    applyWarmMarketStyle();

    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    auto *shell = new QFrame;
    shell->setObjectName("BrowserShell");
    addSoftShadow(shell);

    auto *shellLayout = new QVBoxLayout(shell);
    shellLayout->setContentsMargins(0, 0, 0, 0);
    shellLayout->setSpacing(0);

    auto *offerBar = new QFrame;
    offerBar->setObjectName("OfferBar");
    offerBar->setFixedHeight(28);
    auto *offerLayout = new QHBoxLayout(offerBar);
    offerLayout->setContentsMargins(0, 0, 0, 0);
    auto *offer = new QLabel("▷ 校园二手集市上线 · 闲置好物重新流动 · 校园二手交易平台");
    offer->setObjectName("OfferText");
    offer->setAlignment(Qt::AlignCenter);
    offerLayout->addWidget(offer);

    auto *nav = new QFrame;
    nav->setObjectName("NavBar");
    nav->setFixedHeight(58);
    auto *navLayout = new QHBoxLayout(nav);
    navLayout->setContentsMargins(22, 0, 22, 0);
    navLayout->setSpacing(22);

    auto *brand = new QLabel("◔");
    brand->setObjectName("BrandDot");
    brand->setFixedSize(30, 30);
    brand->setAlignment(Qt::AlignCenter);

    auto *search = new QLabel("搜索校园好物");
    search->setObjectName("SearchBox");
    search->setFixedSize(286, 32);
    search->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    auto *pro = new QPushButton("发布商品");
    pro->setObjectName("ProButton");
    pro->setEnabled(false);

    auto *loginHint = new QPushButton("登录");
    loginHint->setObjectName("NavLogin");
    loginHint->setEnabled(false);

    navLayout->addWidget(brand);
    navLayout->addWidget(navItem("首页"));
    navLayout->addWidget(navItem("分类"));
    navLayout->addWidget(navItem("购物车"));
    navLayout->addWidget(navItem("收藏"));
    navLayout->addStretch();
    navLayout->addWidget(search);
    navLayout->addStretch();
    navLayout->addWidget(navItem("我要出售"));
    navLayout->addWidget(pro);
    navLayout->addWidget(LanguageManager::instance().createSelector(nav));
    navLayout->addWidget(loginHint);
    navLayout->addWidget(navItem("菜单"));

    auto *hero = new QWidget;
    auto *heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(90, 38, 90, 28);
    heroLayout->setSpacing(14);

    auto *heroImage = new QLabel;
    heroImage->setObjectName("HeroImage");
    heroImage->setAlignment(Qt::AlignCenter);
    heroImage->setFixedHeight(150);
    QPixmap heroPix(firstExistingPath({
        QDir(QCoreApplication::applicationDirPath()).filePath("ui_assets/home_banner.jpg"),
        QDir::current().filePath("ui_assets/home_banner.jpg")
    }));
    if (!heroPix.isNull()) {
        heroImage->setPixmap(heroPix.scaled(520, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    auto *title = new QLabel("校园二手交易平台");
    title->setObjectName("HeroTitle");
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel("教材、数码、宿舍用品和代步好物，一处完成发布、浏览和管理。");
    subtitle->setObjectName("HeroSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    btnUserLogin = new QPushButton("用户入口");
    btnUserLogin->setObjectName("UserEntry");
    btnUserLogin->setMinimumHeight(38);

    btnAdminLogin = new QPushButton("管理员入口");
    btnAdminLogin->setObjectName("AdminEntry");
    btnAdminLogin->setMinimumHeight(38);

    auto *entryRow = new QHBoxLayout;
    entryRow->setSpacing(12);
    entryRow->addStretch();
    entryRow->addWidget(btnUserLogin);
    entryRow->addWidget(btnAdminLogin);
    entryRow->addStretch();

    auto *resourceGrid = new QGridLayout;
    resourceGrid->setHorizontalSpacing(24);
    resourceGrid->setVerticalSpacing(24);

    QFrame *cameraCard = createFloatingCard(ProductImageManager::imagePathForProduct("高等数学教材", "书籍教材"), "书籍教材", "高等数学教材");
    QFrame *blanketCard = createFloatingCard(ProductImageManager::imagePathForProduct("保温水杯", "宿舍用品"), "宿舍用品", "保温水杯");
    QFrame *laptopCard = createFloatingCard(ProductImageManager::imagePathForProduct("轻薄笔记本电脑", "电子数码"), "电子数码", "轻薄笔记本电脑");
    QFrame *headphoneCard = createFloatingCard(ProductImageManager::imagePathForProduct("蓝牙耳机", "电子数码"), "电子数码", "蓝牙耳机");
    QFrame *coatCard = createFloatingCard(ProductImageManager::imagePathForProduct("九成新运动鞋", "服饰鞋包"), "服饰鞋包", "九成新运动鞋");
    QFrame *perfumeCard = createFloatingCard(ProductImageManager::imagePathForProduct("校园自行车", "代步工具"), "代步工具", "校园自行车");
    QFrame *fanCard = createFloatingCard(ProductImageManager::imagePathForProduct("护眼台灯", "宿舍用品"), "宿舍用品", "护眼台灯");
    QFrame *badmintonCard = createFloatingCard(ProductImageManager::imagePathForProduct("羽毛球拍", "运动器材"), "运动器材", "羽毛球拍");

    resourceGrid->addWidget(createFloatingCardSlot(cameraCard), 0, 0);
    resourceGrid->addWidget(createFloatingCardSlot(blanketCard), 0, 1);
    resourceGrid->addWidget(createFloatingCardSlot(laptopCard), 0, 2);
    resourceGrid->addWidget(createFloatingCardSlot(headphoneCard), 0, 3);
    resourceGrid->addWidget(createFloatingCardSlot(coatCard), 1, 0);
    resourceGrid->addWidget(createFloatingCardSlot(perfumeCard), 1, 1);
    resourceGrid->addWidget(createFloatingCardSlot(fanCard), 1, 2);
    resourceGrid->addWidget(createFloatingCardSlot(badmintonCard), 1, 3);

    auto *gridRow = new QHBoxLayout;
    gridRow->addStretch();
    gridRow->addLayout(resourceGrid);
    gridRow->addStretch();

    heroLayout->addWidget(heroImage);
    heroLayout->addWidget(title);
    heroLayout->addWidget(subtitle);
    heroLayout->addLayout(entryRow);
    heroLayout->addSpacing(22);
    heroLayout->addLayout(gridRow);
    heroLayout->addStretch();

    shellLayout->addWidget(offerBar);
    shellLayout->addWidget(nav);
    shellLayout->addWidget(hero, 1);

    auto *center = new QHBoxLayout;
    center->setContentsMargins(0, 0, 0, 0);
    center->addStretch();
    center->addWidget(shell);
    center->addStretch();

    auto *pageCenter = new QVBoxLayout;
    pageCenter->setContentsMargins(0, 4, 0, 0);
    pageCenter->addLayout(center);
    pageLayout->addLayout(pageCenter);

    startFloatingAnimation();

    connect(btnUserLogin, &QPushButton::clicked, this, [=]() {
        UserLoginWindow *w = new UserLoginWindow(this);
        w->show();
        hide();
    });

    connect(btnAdminLogin, &QPushButton::clicked, this, [=]() {
        AdminLoginWindow *w = new AdminLoginWindow(this);
        w->show();
        hide();
    });
}

QFrame *WelcomeWindow::createFloatingCard(const QString &imagePath,
                                          const QString &title,
                                          const QString &description)
{
    auto *card = new QFrame;
    card->setObjectName("ResourceCard");
    card->setFixedSize(240, 220);
    addSoftShadow(card, QColor(20, 20, 40, 12), 16);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    auto *imageLabel = new QLabel;
    imageLabel->setObjectName("ResourceImage");
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setScaledContents(false);
    imageLabel->setFixedSize(120, 85);
    const QString productName = title + " · " + description;
    welcomeLog("--------------------------------");
    welcomeLog("商品名称: " + productName);
    welcomeLog("最终 imagePath: " + imagePath);
    welcomeLog("QFile::exists(imagePath): " + yesNo(QFile::exists(imagePath)));
    QPixmap pix(imagePath);
    welcomeLog("QPixmap pix(imagePath)");
    welcomeLog("pix.isNull(): " + yesNo(pix.isNull()));
    welcomeLog("pix.width(): " + QString::number(pix.width()));
    welcomeLog("pix.height(): " + QString::number(pix.height()));
    welcomeLog("imageLabel->size(): " + QString::number(imageLabel->width()) + "x" + QString::number(imageLabel->height()));
    if (pix.isNull() && QFile::exists(imagePath)) {
        QImageReader reader(imagePath);
        const QImage image = reader.read();
        if (!image.isNull()) {
            pix = QPixmap::fromImage(image);
        } else {
            welcomeLog("QImageReader load failed: " + imagePath + " " + reader.errorString());
        }
    }
    if (!pix.isNull()) {
        const QPixmap scaledPixmap = pix.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        welcomeLog("scaledPixmap.width(): " + QString::number(scaledPixmap.width()));
        welcomeLog("scaledPixmap.height(): " + QString::number(scaledPixmap.height()));
        imageLabel->setPixmap(scaledPixmap);
    } else {
        welcomeLog("Image load failed: " + imagePath);
    }
    welcomeLog("imageLabel->pixmap() == nullptr ?: " + yesNo(imageLabel->pixmap().isNull()));
    welcomeLog("imageLabel->isVisible(): " + yesNo(imageLabel->isVisible()));
    welcomeLog("imageLabel->isHidden(): " + yesNo(imageLabel->isHidden()));

    auto *titleLabel = new QLabel(title + "  ·  " + description);
    titleLabel->setObjectName("ResourceTitle");
    titleLabel->setFixedHeight(40);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #111;");

    layout->addWidget(imageLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel);

    floatingCards << card;
    return card;
}

QWidget *WelcomeWindow::createFloatingCardSlot(QFrame *card)
{
    auto *slot = new QWidget;
    slot->setFixedSize(252, 232);
    slot->setStyleSheet("background: transparent;");

    auto *layout = new QVBoxLayout(slot);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(card, 0, Qt::AlignCenter);

    return slot;
}

void WelcomeWindow::startFloatingAnimation()
{
    const QList<int> amplitudes = {2, 3, 3, 2, 3};
    const QList<int> durations = {3000, 3600, 4200, 3200, 3800};

    for (int i = 0; i < floatingCards.size(); ++i) {
        auto *animation = new QPropertyAnimation(floatingCards[i], "pos", this);
        animation->setDuration(durations[i % durations.size()]);
        animation->setStartValue(QPoint(4, 4));
        animation->setKeyValueAt(0.5, QPoint(4, 4 - amplitudes[i % amplitudes.size()]));
        animation->setEndValue(QPoint(4, 4));
        animation->setEasingCurve(QEasingCurve::InOutSine);
        animation->setLoopCount(-1);
        floatingAnimations << animation;
        animation->start();
    }
}

void WelcomeWindow::applyWarmMarketStyle()
{
    setStyleSheet(kCoverQss);
}

