// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QWidget>
#include <QString>
#include <QMouseEvent>
#include <QParallelAnimationGroup>

#include <memory>

class QLabel;
class QGraphicsBlurEffect;
class QGraphicsOpacityEffect;

class Thumbnail : public QWidget
{
    Q_OBJECT
public:
    explicit Thumbnail(QString path, QWidget *parent = nullptr);
    ~Thumbnail() override;

    [[nodiscard]] QString getImagePath() noexcept;

    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void clicked();

public Q_SLOTS:
    void showShadow();
    void hideShadow();

private:
    friend struct ThumbnailPrivate;
    std::unique_ptr<struct ThumbnailPrivate> d;
};

#endif // THUMBNAIL_H
