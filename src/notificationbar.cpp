// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

#include <functional>

#include "notificationbar.h"

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
    inline QPropertyAnimation *createShowAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr, int duration = 1000)
    {
        auto showAnimation = new QPropertyAnimation{target, propertyName, parent};
        showAnimation->setStartValue(0.0);
        showAnimation->setEndValue(1.0);
        showAnimation->setDuration(duration);
        return showAnimation;
    }

    inline QPropertyAnimation *createHideAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr, int duration = 1000)
    {
        auto hideAnimation = new QPropertyAnimation{target, propertyName, parent};
        hideAnimation->setStartValue(1.0);
        hideAnimation->setEndValue(0.0);
        hideAnimation->setDuration(duration);
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
    connect(hideAnimation, &QAbstractAnimation::finished, this, [this]{
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

NotificationBar *NotificationBar::createInfoBar(QWidget *parent)
{
    const QColor borderColor{64, 158, 255};
    const QColor bgColor{236, 245, 255, 80};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_MessageBoxInformation));
    return bar;
}

NotificationBar *NotificationBar::createErrorBar(QWidget *parent)
{
    const QColor borderColor{0xf5, 0x6c, 0x6c};
    const QColor bgColor{254, 240, 240, 80};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_BrowserStop));
    return bar;
}

NotificationBar *NotificationBar::createSuccessBar(QWidget *parent)
{
    const QColor borderColor{0x27, 0xae, 0x60};
    const QColor bgColor{0xc7, 0xe2, 0xd4};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_DialogApplyButton));
    return bar;
}

NotificationBar *NotificationBar::createNotificationBar(const NotificationBar::NotificationType type, const QString &msg, QWidget *parent) {
    using creator_t = std::function<NotificationBar*(QWidget *)>;
    static std::unordered_map<NotificationType, creator_t> notificationFactory = {
            {
                NotificationType::INFO,
                &NotificationBar::createInfoBar
            },
            {
                NotificationType::ERROR,
                &NotificationBar::createErrorBar
            },
            {
                NotificationType::SUCCESS,
                &NotificationBar::createSuccessBar
            },
    };
    NotificationBar *bar = notificationFactory[type](parent);
    if (msg != "") {
        bar->setText(msg);
    }
    return bar;
}
