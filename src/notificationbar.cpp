// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringBuilder>
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
    bool isClosing = false;  // hide动画运行期间一直为true

    QPropertyAnimation *hideAnimation = nullptr;
    QPropertyAnimation *showAnimation = nullptr;
    QSequentialAnimationGroup *animeGroup = nullptr;

    void init(NotificationBar *q_ptr) noexcept;

    void stopAllAnimations() noexcept {
        isClosing = false;
        if (showAnimation) {
            showAnimation->stop();
        }
        if (hideAnimation) {
            hideAnimation->stop();
        }
        if (animeGroup) {
            animeGroup->stop();
        }
    }
};

namespace {
    [[nodiscard]] inline QPropertyAnimation *createShowAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr, int duration = 1000) noexcept
    {
        auto showAnimation = new QPropertyAnimation{target, propertyName, parent};
        showAnimation->setStartValue(0.0);
        showAnimation->setEndValue(1.0);
        showAnimation->setDuration(duration);
        return showAnimation;
    }

    [[nodiscard]] inline QPropertyAnimation *createHideAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr, int duration = 1000) noexcept
    {
        auto hideAnimation = new QPropertyAnimation{target, propertyName, parent};
        hideAnimation->setStartValue(1.0);
        hideAnimation->setEndValue(0.0);
        hideAnimation->setDuration(duration);
        return hideAnimation;
    }
}

void NotificationBarPrivate::init(NotificationBar *q_ptr) noexcept
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
    closeBtn->setStyleSheet(QStringLiteral(u".QPushButton{background:rgba(0,0,0,0);border:0;}"));
    closeBtn->setToolTip(QObject::tr("close this notification"));
    closeBtn->hide();

    auto mainLayout = new QHBoxLayout;
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(textLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(closeBtn);
    q->setLayout(mainLayout);
}

NotificationBar::NotificationBar(const QColor &borderColor, const QColor &bgColor, QWidget *parent) noexcept
    : QFrame{parent}, d{new NotificationBarPrivate}
{
    setAttribute(Qt::WA_StyledBackground);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setColor(borderColor, bgColor);
    d->init(this);
    hide();
}

NotificationBar::~NotificationBar() noexcept = default;

void NotificationBar::setColor(const QColor &borColor, const QColor &bgColor) noexcept
{
    if (!borColor.isValid() || !bgColor.isValid()) {
        return;
    }

    const auto &borderColorStyle = QStringLiteral(u"rgba(%1,%2,%3,%4)").arg(borColor.red())
                                              .arg(borColor.green())
                                              .arg(borColor.blue())
                                              .arg(borColor.alpha());
    const auto &bgColorStyle = QStringLiteral(u"rgba(%1,%2,%3,%4)").arg(bgColor.red())
                                              .arg(bgColor.green())
                                              .arg(bgColor.blue())
                                              .arg(bgColor.alpha());
    setStyleSheet(QStringLiteral(u".NotificationBar{border: 1px solid ") %
                  borderColorStyle %
                  QStringLiteral(u"; background-color: ") %
                  bgColorStyle % QStringLiteral(u";}"));
}

void NotificationBar::setCloseButtonVisible(bool visible) noexcept
{
    if (visible) {
        d->closeBtn->show();
        return;
    }
    d->closeBtn->hide();
}

void NotificationBar::setIcon(const QIcon &notifyIcon) noexcept
{
    if (notifyIcon.isNull()) {
        d->iconLabel->hide();
        return;
    }

    const int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    d->iconLabel->setPixmap(notifyIcon.pixmap(size, size));
    d->iconLabel->show();
}

void NotificationBar::setText(const QString &text) noexcept
{
    d->textLabel->setText(text);
    updateGeometry();
}

void NotificationBar::animatedShow() noexcept
{
    d->stopAllAnimations();
    if (!d->showAnimation) {
        d->showAnimation = createShowAnimation(d->effect, "opacity", this);
    }
    show();
    d->showAnimation->start();
}

void NotificationBar::animatedHide() noexcept
{
    d->stopAllAnimations();
    d->isClosing = true;
    if (!d->hideAnimation) {
        d->hideAnimation = createHideAnimation(d->effect, "opacity", this);
        connect(d->hideAnimation, &QAbstractAnimation::finished, this, &NotificationBar::hide);
    }
    d->hideAnimation->start();
}

void NotificationBar::showAndHide(int remainMsecs) noexcept
{
    d->stopAllAnimations();
    if (!d->animeGroup) {
        auto showAnimation = createShowAnimation(d->effect, "opacity", this);
        auto hideAnimation = createHideAnimation(d->effect, "opacity", this);
        d->animeGroup = new QSequentialAnimationGroup{this};
        d->animeGroup->addAnimation(showAnimation);
        d->animeGroup->addPause(remainMsecs);
        d->animeGroup->addAnimation(hideAnimation);
        connect(hideAnimation, &QAbstractAnimation::stateChanged, this, [this](QAbstractAnimation::State newState, QAbstractAnimation::State) {
            if (newState == QAbstractAnimation::Running) {
                d->isClosing = true;
            }
        });
        connect(d->animeGroup, &QAbstractAnimation::finished, this, &NotificationBar::hide);
    }
    show();
    d->animeGroup->start();
}

// 因为要处理标志位所以重写覆盖了hide
void NotificationBar::hide() {
    d->isClosing = false;
    QWidget::hide();
}

NotificationBar *NotificationBar::createInfoBar(QWidget *parent) noexcept
{
    const QColor borderColor{64, 158, 255};
    const QColor bgColor{236, 245, 255, 80};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_MessageBoxInformation));
    return bar;
}

NotificationBar *NotificationBar::createErrorBar(QWidget *parent) noexcept
{
    const QColor borderColor{0xf5, 0x6c, 0x6c};
    const QColor bgColor{254, 240, 240, 80};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_BrowserStop));
    return bar;
}

NotificationBar *NotificationBar::createSuccessBar(QWidget *parent) noexcept
{
    const QColor borderColor{0x27, 0xae, 0x60};
    const QColor bgColor{0xc7, 0xe2, 0xd4};
    auto bar = new NotificationBar{borderColor, bgColor, parent};
    bar->setIcon(bar->style()->standardIcon(QStyle::SP_DialogApplyButton));
    return bar;
}

NotificationBar *NotificationBar::createNotificationBar(const NotificationBar::NotificationType type, const QString &msg, QWidget *parent) noexcept {
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
