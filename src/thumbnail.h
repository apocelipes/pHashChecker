#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QWidget>
#include <QString>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>

class QLabel;
class QGraphicsBlurEffect;

constexpr qreal DEFAULT_OPACITY = 0.8;
constexpr qreal DEFAULT_BLUR_RADIUS = 5.0;
constexpr int DEFAULT_ANIME_DURATION = 300; // ms

class Thumbnail : public QWidget
{
    Q_OBJECT
public:
    explicit Thumbnail(const QString &path, QWidget *parent = nullptr);

    void showShadow()
    {
        shadow->show();
        showAnimation->start();
    }

    void hideShadow()
    {
        hideAnimation->start();
    }

    qreal getOpacity()
    {
        return opacityEffect->opacity();
    }

    void setOpacity(const qreal opacity)
    {
        if (opacity < 0.0 || opacity > 1.0) {
            return;
        }

        opacityEffect->setOpacity(opacity);
        Q_EMIT opacityChanged(opacity);
    }

    QString getImagePath() noexcept
    {
        return imgPath;
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        Q_EMIT clicked();
        event->accept();
    }

signals:
    void clicked();
    void opacityChanged(qreal opacity);
private:
    QString imgPath;
    QWidget *shadow = nullptr;
    QLabel *image = nullptr;
    QGraphicsBlurEffect *blurEffect = nullptr;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    QParallelAnimationGroup *showAnimation = nullptr;
    QParallelAnimationGroup *hideAnimation = nullptr;

    void initAnimations();
};

#endif // THUMBNAIL_H
