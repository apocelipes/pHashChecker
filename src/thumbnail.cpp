// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#include <QLabel>
#include <QPixmap>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QParallelAnimationGroup>

#include <utility>

#include "thumbnail.h"
#include "utils/imageutils.h"
#include "utils/path.h"
#include "utils/sizeformat.h"
#include "utils/utils.h"

using namespace Qt::Literals::StringLiterals;

constexpr int ThumbnailWidth = 100, ThumbnailHeight = 100;
constexpr qreal DEFAULT_OPACITY = 0.7;
constexpr qreal DEFAULT_BLUR_RADIUS = 5.0;
constexpr int DEFAULT_ANIME_DURATION = 300; // ms

struct ThumbnailPrivate {
    Thumbnail *q = nullptr;
    QWidget *shadow = nullptr;
    QLabel *image = nullptr;
    QGraphicsBlurEffect *blurEffect = nullptr;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    QParallelAnimationGroup *showAnimation = nullptr;
    QParallelAnimationGroup *hideAnimation = nullptr;
    QString imgPath;

    explicit ThumbnailPrivate(QString &&path) noexcept: imgPath{std::forward<QString>(path)}
    {}

    void init(Thumbnail *q_ptr) noexcept;
    void initShowShadowAnimation() noexcept;
    void initHideShadowAnimation() noexcept;
};

void ThumbnailPrivate::init(Thumbnail *q_ptr) noexcept
{
    q = q_ptr;
    image = new QLabel{q};
    image->setGeometry(q->geometry());
    const auto &info = QFileInfo{imgPath};
    if (!info.exists()) {
        qFatal() << QObject::tr("Image file does not exist:") << imgPath;
        return;
    }

    if (Utils::isFormatNeedConvert(imgPath)) {
        image->setPixmap(Utils::convertToPixmap(imgPath, ThumbnailHeight, ThumbnailHeight));
    } else {
        image->setPixmap(QPixmap{imgPath}.scaled(ThumbnailWidth, ThumbnailHeight));
    }
    q->setToolTip(QObject::tr("%1<br>size: %2").arg(Utils::getAbsPath(imgPath)).arg(Utils::sizeFormat(info.size())));

    blurEffect = new QGraphicsBlurEffect{q};
    blurEffect->setBlurRadius(DEFAULT_BLUR_RADIUS);
    blurEffect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    image->setGraphicsEffect(blurEffect);

    shadow = new QWidget{q};
    shadow->setWindowFlag(Qt::FramelessWindowHint);
    shadow->setAttribute(Qt::WA_StyledBackground);
    shadow->setStyleSheet(QStringLiteral(u"background:rgb(255,255,255);"));
    shadow->setGeometry(q->geometry());
    opacityEffect = new QGraphicsOpacityEffect{shadow};
    opacityEffect->setOpacity(DEFAULT_OPACITY);
    shadow->setGraphicsEffect(opacityEffect);
}

inline void ThumbnailPrivate::initShowShadowAnimation() noexcept
{
    auto blurShowAnimation = new QPropertyAnimation{blurEffect, "blurRadius"_ba, q};
    blurShowAnimation->setDuration(DEFAULT_ANIME_DURATION);
    blurShowAnimation->setStartValue(0.0);
    blurShowAnimation->setEndValue(DEFAULT_BLUR_RADIUS);
    auto shadowShowAnimation = new QPropertyAnimation{opacityEffect, "opacity"_ba, q};
    shadowShowAnimation->setDuration(DEFAULT_ANIME_DURATION);
    shadowShowAnimation->setStartValue(0.0);
    shadowShowAnimation->setEndValue(DEFAULT_OPACITY);
    showAnimation = new QParallelAnimationGroup{q};
    showAnimation->addAnimation(blurShowAnimation);
    showAnimation->addAnimation(shadowShowAnimation);
}

inline void ThumbnailPrivate::initHideShadowAnimation() noexcept
{
    auto blurHideAnimation = new QPropertyAnimation{blurEffect, "blurRadius"_ba, q};
    blurHideAnimation->setDuration(DEFAULT_ANIME_DURATION);
    blurHideAnimation->setStartValue(DEFAULT_BLUR_RADIUS);
    blurHideAnimation->setEndValue(0.0);
    auto shadowHideAnimation = new QPropertyAnimation{opacityEffect, "opacity"_ba, q};
    shadowHideAnimation->setDuration(DEFAULT_ANIME_DURATION);
    shadowHideAnimation->setStartValue(DEFAULT_OPACITY);
    shadowHideAnimation->setEndValue(0.0);
    hideAnimation = new QParallelAnimationGroup{q};
    hideAnimation->addAnimation(blurHideAnimation);
    hideAnimation->addAnimation(shadowHideAnimation);
    QObject::connect(hideAnimation, &QAbstractAnimation::finished, shadow, &QWidget::hide);
}

Thumbnail::Thumbnail(QString path, QWidget *parent) noexcept
    : QWidget(parent), d{new ThumbnailPrivate{std::move(path)}}
{
    setFixedSize(ThumbnailWidth, ThumbnailHeight);
    d->init(this);
}

void Thumbnail::showShadow() noexcept
{
    if (d->showAnimation == nullptr) {
        d->initShowShadowAnimation();
    }
    d->shadow->show();
    d->showAnimation->start();
}

void Thumbnail::hideShadow() noexcept
{
    if (d->hideAnimation == nullptr) {
        d->initHideShadowAnimation();
    }
    d->hideAnimation->start();
}

QString Thumbnail::getImagePath() const noexcept
{
    return d->imgPath;
}

void Thumbnail::mouseReleaseEvent(QMouseEvent *event)
{
    Q_EMIT clicked();
    event->accept();
}

Thumbnail::~Thumbnail() noexcept = default;
