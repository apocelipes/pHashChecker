#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>

constexpr qreal DEFAULT_OPACITY = 0.8;

class Thumbnail : public QWidget
{
    Q_OBJECT
public:
    explicit Thumbnail(const QString &path, QWidget *parent = nullptr);

    void showShadow()
    {
        shadow->show();
    }

    void hideShadow()
    {
        shadow->hide();
    }

    qreal getOpacity()
    {
        auto effect = qobject_cast<QGraphicsOpacityEffect*>(shadow->graphicsEffect());
        return effect->opacity();
    }

    void setOpacity(const qreal opacity)
    {
        if (opacity < 0.0 || opacity > 1.0) {
            return;
        }

        auto effect = qobject_cast<QGraphicsOpacityEffect*>(shadow->graphicsEffect());
        if (effect == nullptr) {
            effect = new QGraphicsOpacityEffect(shadow);
            shadow->setGraphicsEffect(effect);
        }
        effect->setOpacity(opacity);
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
    QWidget *shadow = nullptr;
    QLabel *image = nullptr;
    QString imgPath;
};

#endif // THUMBNAIL_H
