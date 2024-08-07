// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#pragma once

#include <QWidget>
#include <QString>

#include <memory>

class QMouseEvent;

class Thumbnail : public QWidget
{
    Q_OBJECT
public:
    explicit Thumbnail(QString path, QWidget *parent = nullptr) noexcept;
    ~Thumbnail() noexcept override;

    [[nodiscard]] QString getImagePath() const noexcept;

    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void clicked();

public Q_SLOTS:
    void showShadow() noexcept;
    void hideShadow() noexcept;

private:
    friend struct ThumbnailPrivate;
    std::unique_ptr<struct ThumbnailPrivate> d;
};
