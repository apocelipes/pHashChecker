// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

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
    NotificationBar(const QColor &borderColor, const QColor &bgColor, QWidget *parent = nullptr);
    ~NotificationBar() noexcept override;

    enum class NotificationType: int {
        INFO,
        ERROR,
        SUCCESS,
    };

public Q_SLOTS:
    void setColor(const QColor &borColor, const QColor &bgColor);

    void setCloseButtonVisible(bool visible);
    void setIcon(const QIcon &notifyIcon);
    void setText(const QString &text);

    void animatedShow();
    void animatedHide();
    void showAndHide(int remainMsecs = 5000);

    static NotificationBar *createNotificationBar(
            NotificationBar::NotificationType type,
            const QString &msg = "",
            QWidget *parent = nullptr);

private:
    friend struct NotificationBarPrivate;
    std::unique_ptr<struct NotificationBarPrivate> d;

    static NotificationBar *createInfoBar(QWidget *parent = nullptr);
    static NotificationBar *createErrorBar(QWidget *parent = nullptr);
    static NotificationBar *createSuccessBar(QWidget *parent = nullptr);
};

#endif //PHASHCHECKER_NOTIFICATIONBAR_H
