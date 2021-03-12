#include "notificationbar.h"

#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

NotificationBar::NotificationBar(const QColor &borderColor, const QColor &bgColor, QWidget *parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setColor(borderColor, bgColor);
    effect = new QGraphicsOpacityEffect{this};
    effect->setOpacity(0);
    setGraphicsEffect(effect);

    iconLabel = new QLabel{this};
    iconLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    auto policy = iconLabel->sizePolicy();
    policy.setHeightForWidth(true);
    iconLabel->setSizePolicy(policy);
    iconLabel->hide();
    textLabel = new QLabel{this};
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    closeBtn = new QPushButton{style()->standardPixmap(QStyle::SP_DialogCloseButton), "", this};
    connect(closeBtn, &QPushButton::clicked, this, &NotificationBar::animatedHide);
    closeBtn->setAttribute(Qt::WA_StyledBackground);
    closeBtn->setStyleSheet(".QPushButton{background:rgba(0,0,0,0);border:0;}");
    closeBtn->setToolTip(tr("close this notification"));
    closeBtn->hide();
    auto mainLayout = new QHBoxLayout;
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(textLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(closeBtn);
    setLayout(mainLayout);
    hide();
}

void NotificationBar::setCloseButtonVisible(bool visible)
{
    if (visible) {
        closeBtn->show();
        return;
    }
    closeBtn->hide();
}

void NotificationBar::setIcon(const QIcon &notifyIcon)
{
    if (notifyIcon.isNull()) {
        iconLabel->hide();
        return;
    }

    const int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    iconLabel->setPixmap(notifyIcon.pixmap(size, size));
    iconLabel->show();
}

void NotificationBar::setText(const QString &text)
{
    textLabel->setText(text);
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
    auto showAnimation = createShowAnimation(effect, "opacity", this);
    show();
    showAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationBar::animatedHide()
{
    // 防止动画期间多次触发关闭按钮
    closeBtn->hide();
    auto hideAnimation = createHideAnimation(effect, "opacity", this);
    connect(hideAnimation, &QAbstractAnimation::finished, [this]{
        closeBtn->show();
        hide();
    });
    hideAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationBar::showAndHide(int remainMsecs)
{
    auto showAnimation = createShowAnimation(effect, "opacity", this);
    auto hideAnimation = createHideAnimation(effect, "opacity", this);
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
