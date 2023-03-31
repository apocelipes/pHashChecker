// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#ifndef PHASHCHECKER_NOTIFICATIONBAR_H
#define PHASHCHECKER_NOTIFICATIONBAR_H

#include <QFrame>
#include <QColor>
#include <QString>

#include <memory>

class QLabel;
class QPushButton;
class QGraphicsOpacityEffect;
class QPropertyAnimation;

class NotificationBar: public QFrame {
    Q_OBJECT
public:
    NotificationBar(const QColor &borderColor, const QColor &bgColor, QWidget *parent = nullptr) noexcept;
    ~NotificationBar() noexcept override;

    enum class NotificationType: int {
        INFO,
        ERROR,
        SUCCESS,
    };

    [[nodiscard]] static NotificationBar *createNotificationBar(
            NotificationBar::NotificationType type,
            const QString &msg = "",
            QWidget *parent = nullptr) noexcept;

public Q_SLOTS:
    void setColor(const QColor &borColor, const QColor &bgColor) noexcept;
    void setCloseButtonVisible(bool visible) noexcept;
    void setIcon(const QIcon &notifyIcon) noexcept;
    void setText(const QString &text) noexcept;

    void animatedShow();
    void animatedHide();
    void showAndHide(int remainMsecs = 5000);

    void hide();

private:
    friend struct NotificationBarPrivate;
    std::unique_ptr<struct NotificationBarPrivate> d;

    [[nodiscard]] static NotificationBar *createInfoBar(QWidget *parent = nullptr) noexcept;
    [[nodiscard]] static NotificationBar *createErrorBar(QWidget *parent = nullptr) noexcept;
    [[nodiscard]] static NotificationBar *createSuccessBar(QWidget *parent = nullptr) noexcept;
};

#endif //PHASHCHECKER_NOTIFICATIONBAR_H
