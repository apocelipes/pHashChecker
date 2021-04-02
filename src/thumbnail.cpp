#include "thumbnail.h"

#include <QLabel>
#include <QPixmap>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include <filesystem>
#include <utility>

constexpr int ThumbnailWidth = 100, ThumbnailHeight = 100;
constexpr qreal DEFAULT_OPACITY = 0.8;
constexpr qreal DEFAULT_BLUR_RADIUS = 5.0;
constexpr int DEFAULT_ANIME_DURATION = 300; // ms

Thumbnail::Thumbnail(QString path, QWidget *parent)
    : QWidget(parent), imgPath{std::move(path)}
{
    setFixedSize(ThumbnailWidth, ThumbnailHeight);
    image = new QLabel{this};
    image->setGeometry(geometry());
    if (std::filesystem::exists(imgPath.toStdString())) {
        QPixmap data{imgPath};
        image->setPixmap(data.scaled(ThumbnailWidth, ThumbnailHeight));
    }
    blurEffect = new QGraphicsBlurEffect{this};
    blurEffect->setBlurRadius(DEFAULT_BLUR_RADIUS);
    blurEffect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    image->setGraphicsEffect(blurEffect);

    shadow = new QWidget{this};
    shadow->setWindowFlag(Qt::FramelessWindowHint);
    shadow->setAttribute(Qt::WA_StyledBackground);
    shadow->setStyleSheet("background:rgb(255,255,255);");
    shadow->setGeometry(geometry());
    opacityEffect = new QGraphicsOpacityEffect{this};
    opacityEffect->setOpacity(DEFAULT_OPACITY);
    shadow->setGraphicsEffect(opacityEffect);
}

void Thumbnail::initShowShadowAnimation()
{
    auto blurShowAnimation = new QPropertyAnimation{blurEffect, "blurRadius", this};
    blurShowAnimation->setDuration(DEFAULT_ANIME_DURATION);
    blurShowAnimation->setStartValue(0.0);
    blurShowAnimation->setEndValue(DEFAULT_BLUR_RADIUS);
    auto shadowShowAnimation = new QPropertyAnimation{opacityEffect, "opacity", this};
    shadowShowAnimation->setDuration(DEFAULT_ANIME_DURATION);
    shadowShowAnimation->setStartValue(0.0);
    shadowShowAnimation->setEndValue(DEFAULT_OPACITY);
    showAnimation = new QParallelAnimationGroup{this};
    showAnimation->addAnimation(blurShowAnimation);
    showAnimation->addAnimation(shadowShowAnimation);
}

void Thumbnail::initHideShadowAnimation()
{
    auto blurHideAnimation = new QPropertyAnimation{blurEffect, "blurRadius", this};
    blurHideAnimation->setDuration(DEFAULT_ANIME_DURATION);
    blurHideAnimation->setStartValue(DEFAULT_BLUR_RADIUS);
    blurHideAnimation->setEndValue(0.0);
    auto shadowHideAnimation = new QPropertyAnimation{opacityEffect, "opacity", this};
    shadowHideAnimation->setDuration(DEFAULT_ANIME_DURATION);
    shadowHideAnimation->setStartValue(DEFAULT_OPACITY);
    shadowHideAnimation->setEndValue(0.0);
    hideAnimation = new QParallelAnimationGroup{this};
    hideAnimation->addAnimation(blurHideAnimation);
    hideAnimation->addAnimation(shadowHideAnimation);
    connect(hideAnimation, &QAbstractAnimation::finished, shadow, &QWidget::hide);
}
