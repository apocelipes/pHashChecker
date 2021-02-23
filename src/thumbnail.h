#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QWidget>
#include <QString>
#include <QMouseEvent>
#include <QParallelAnimationGroup>

class QLabel;
class QGraphicsBlurEffect;
class QGraphicsOpacityEffect;

class Thumbnail : public QWidget
{
    Q_OBJECT
public:
    explicit Thumbnail(QString path, QWidget *parent = nullptr);

    void showShadow()
    {
        if (showAnimation == nullptr) {
            initShowShadowAnimation();
        }
        shadow->show();
        showAnimation->start();
    }

    void hideShadow()
    {
        if (hideAnimation == nullptr) {
            initHideShadowAnimation();
        }
        hideAnimation->start();
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

Q_SIGNALS:
    void clicked();

private:
    QString imgPath;
    QWidget *shadow = nullptr;
    QLabel *image = nullptr;
    QGraphicsBlurEffect *blurEffect = nullptr;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    QParallelAnimationGroup *showAnimation = nullptr;
    QParallelAnimationGroup *hideAnimation = nullptr;

    void initShowShadowAnimation();
    void initHideShadowAnimation();
};

#endif // THUMBNAIL_H
