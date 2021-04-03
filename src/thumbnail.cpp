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

struct ThumbnailPrivate {
    Thumbnail *q = nullptr;
    QWidget *shadow = nullptr;
    QLabel *image = nullptr;
    QGraphicsBlurEffect *blurEffect = nullptr;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    QParallelAnimationGroup *showAnimation = nullptr;
    QParallelAnimationGroup *hideAnimation = nullptr;
    QString imgPath;

    explicit ThumbnailPrivate(QString &&path): imgPath{std::forward<QString>(path)}
    {}

    void init(Thumbnail *q_ptr);
    void initShowShadowAnimation();
    void initHideShadowAnimation();
};

void ThumbnailPrivate::init(Thumbnail *q_ptr)
{
    q = q_ptr;
    image = new QLabel{q};
    image->setGeometry(q->geometry());
    if (std::filesystem::exists(imgPath.toStdString())) {
        QPixmap data{imgPath};
        image->setPixmap(data.scaled(ThumbnailWidth, ThumbnailHeight));
    }
    blurEffect = new QGraphicsBlurEffect{q};
    blurEffect->setBlurRadius(DEFAULT_BLUR_RADIUS);
    blurEffect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    image->setGraphicsEffect(blurEffect);

    shadow = new QWidget{q};
    shadow->setWindowFlag(Qt::FramelessWindowHint);
    shadow->setAttribute(Qt::WA_StyledBackground);
    shadow->setStyleSheet("background:rgb(255,255,255);");
    shadow->setGeometry(q->geometry());
    opacityEffect = new QGraphicsOpacityEffect{q};
    opacityEffect->setOpacity(DEFAULT_OPACITY);
    shadow->setGraphicsEffect(opacityEffect);
}

inline void ThumbnailPrivate::initShowShadowAnimation()
{
    auto blurShowAnimation = new QPropertyAnimation{blurEffect, "blurRadius", q};
    blurShowAnimation->setDuration(DEFAULT_ANIME_DURATION);
    blurShowAnimation->setStartValue(0.0);
    blurShowAnimation->setEndValue(DEFAULT_BLUR_RADIUS);
    auto shadowShowAnimation = new QPropertyAnimation{opacityEffect, "opacity", q};
    shadowShowAnimation->setDuration(DEFAULT_ANIME_DURATION);
    shadowShowAnimation->setStartValue(0.0);
    shadowShowAnimation->setEndValue(DEFAULT_OPACITY);
    showAnimation = new QParallelAnimationGroup{q};
    showAnimation->addAnimation(blurShowAnimation);
    showAnimation->addAnimation(shadowShowAnimation);
}

inline void ThumbnailPrivate::initHideShadowAnimation()
{
    auto blurHideAnimation = new QPropertyAnimation{blurEffect, "blurRadius", q};
    blurHideAnimation->setDuration(DEFAULT_ANIME_DURATION);
    blurHideAnimation->setStartValue(DEFAULT_BLUR_RADIUS);
    blurHideAnimation->setEndValue(0.0);
    auto shadowHideAnimation = new QPropertyAnimation{opacityEffect, "opacity", q};
    shadowHideAnimation->setDuration(DEFAULT_ANIME_DURATION);
    shadowHideAnimation->setStartValue(DEFAULT_OPACITY);
    shadowHideAnimation->setEndValue(0.0);
    hideAnimation = new QParallelAnimationGroup{q};
    hideAnimation->addAnimation(blurHideAnimation);
    hideAnimation->addAnimation(shadowHideAnimation);
    QObject::connect(hideAnimation, &QAbstractAnimation::finished, shadow, &QWidget::hide);
}

Thumbnail::Thumbnail(QString path, QWidget *parent)
    : QWidget(parent), d{new ThumbnailPrivate{std::move(path)}}
{
    setFixedSize(ThumbnailWidth, ThumbnailHeight);
    d->init(this);
}

void Thumbnail::showShadow()
{
    if (d->showAnimation == nullptr) {
        d->initShowShadowAnimation();
    }
    d->shadow->show();
    d->showAnimation->start();
}

void Thumbnail::hideShadow()
{
    if (d->hideAnimation == nullptr) {
        d->initHideShadowAnimation();
    }
    d->hideAnimation->start();
}

QString Thumbnail::getImagePath() noexcept
{
    return d->imgPath;
}

void Thumbnail::mouseReleaseEvent(QMouseEvent *event)
{
    Q_EMIT clicked();
    event->accept();
}

Thumbnail::~Thumbnail() = default;
