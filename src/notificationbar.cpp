#include "notificationbar.h"

#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

struct NotificationBarPrivate {
    NotificationBar *q = nullptr;
    QLabel *iconLabel = nullptr;
    QLabel *textLabel = nullptr;
    QPushButton *closeBtn = nullptr;
    QGraphicsOpacityEffect *effect = nullptr;
    bool isClosing = false;

    void init(NotificationBar *q_ptr);
};

void NotificationBarPrivate::init(NotificationBar *q_ptr)
{
    q = q_ptr;

    effect = new QGraphicsOpacityEffect{q};
    effect->setOpacity(0);
    q->setGraphicsEffect(effect);

    iconLabel = new QLabel{q};
    iconLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    auto policy = iconLabel->sizePolicy();
    policy.setHeightForWidth(true);
    iconLabel->setSizePolicy(policy);
    iconLabel->hide();
    textLabel = new QLabel{q};
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    closeBtn = new QPushButton{q->style()->standardPixmap(QStyle::SP_DialogCloseButton), "", q};
    QObject::connect(closeBtn, &QPushButton::clicked, [this](){
        if (isClosing) {
            return;
        }

        q->animatedHide();
    });
    closeBtn->setAttribute(Qt::WA_StyledBackground);
    closeBtn->setStyleSheet(".QPushButton{background:rgba(0,0,0,0);border:0;}");
    closeBtn->setToolTip(QObject::tr("close this notification"));
    closeBtn->hide();

    auto mainLayout = new QHBoxLayout;
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(textLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(closeBtn);
    q->setLayout(mainLayout);
}

NotificationBar::NotificationBar(const QColor &borderColor, const QColor &bgColor, QWidget *parent)
    : QFrame{parent}, d{new NotificationBarPrivate}
{
    setAttribute(Qt::WA_StyledBackground);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setColor(borderColor, bgColor);
    d->init(this);
    hide();
}

NotificationBar::~NotificationBar() noexcept = default;

inline void NotificationBar::setColor(const QColor &borColor, const QColor &bgColor)
{
    if (!borColor.isValid() || !bgColor.isValid()) {
        return;
    }

    auto borderColorStyle = QString::asprintf("rgba(%d,%d,%d,%d)",
                                              borColor.red(),
                                              borColor.green(),
                                              borColor.blue(),
                                              borColor.alpha());
    auto bgColorStyle = QString::asprintf("rgba(%d,%d,%d,%d)",
                                          bgColor.red(),
                                          bgColor.green(),
                                          bgColor.blue(),
                                          bgColor.alpha());
    setStyleSheet(".NotificationBar{border: 1px solid " +
                  borderColorStyle +
                  "; background-color: " +
                  bgColorStyle + ";}");
}

void NotificationBar::setCloseButtonVisible(bool visible)
{
    if (visible) {
        d->closeBtn->show();
        return;
    }
    d->closeBtn->hide();
}

void NotificationBar::setIcon(const QIcon &notifyIcon)
{
    if (notifyIcon.isNull()) {
        d->iconLabel->hide();
        return;
    }

    const int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    d->iconLabel->setPixmap(notifyIcon.pixmap(size, size));
    d->iconLabel->show();
}

void NotificationBar::setText(const QString &text)
{
    d->textLabel->setText(text);
    updateGeometry();
}

namespace {
    inline QPropertyAnimation *createShowAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr)
    {
        auto showAnimation = new QPropertyAnimation{target, propertyName, parent};
        showAnimation->setStartValue(0.0);
        showAnimation->setEndValue(1.0);
        showAnimation->setDuration(1000);
        return showAnimation;
    }

    inline QPropertyAnimation *createHideAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr)
    {
        auto hideAnimation = new QPropertyAnimation{target, propertyName, parent};
        hideAnimation->setStartValue(1.0);
        hideAnimation->setEndValue(0.0);
        hideAnimation->setDuration(1000);
        return hideAnimation;
    }
}

void NotificationBar::animatedShow()
{
    auto showAnimation = createShowAnimation(d->effect, "opacity", this);
    show();
    showAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationBar::animatedHide()
{
    d->isClosing = true;
    auto hideAnimation = createHideAnimation(d->effect, "opacity", this);
    connect(hideAnimation, &QAbstractAnimation::finished, [this]{
        d->isClosing = false;
        hide();
    });
    hideAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationBar::showAndHide(int remainMsecs)
{
    auto showAnimation = createShowAnimation(d->effect, "opacity", this);
    auto hideAnimation = createHideAnimation(d->effect, "opacity", this);
    auto group = new QSequentialAnimationGroup{this};
    group->addAnimation(showAnimation);
    group->addPause(remainMsecs);
    group->addAnimation(hideAnimation);
    connect(group, &QAbstractAnimation::finished, this, &QWidget::hide);
    show();
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

NotificationBar *NotificationBar::createInformationBar(QWidget *parent)
{
    QColor borderColor{64, 158, 255};
    QColor bgColor{236, 245, 255, 80};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_MessageBoxInformation));
    return bar;
}

NotificationBar *NotificationBar::createErrorBar(QWidget *parent)
{
    QColor borderColor{0xf5, 0x6c, 0x6c};
    QColor bgColor{254, 240, 240, 80};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_BrowserStop));
    return bar;
}
