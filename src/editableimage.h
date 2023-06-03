// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef EDITABLEIMAGE_H
#define EDITABLEIMAGE_H

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QString>

#include <filesystem>
#include <memory>

constexpr int EditableImageFixedWidth = 850;
constexpr int EditableImageFixedHeight = 650;

class QMenu;

class EditableImage : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString imagePath READ getImagePath WRITE setImagePath NOTIFY pathChanged);
public:
    explicit EditableImage(const QString& imgPath, QWidget *parent = nullptr);
    ~EditableImage() noexcept override;

    [[nodiscard]] QString getImagePath() const noexcept;
    void setImagePath(const QString &path) noexcept;
    [[nodiscard]] bool isEmpty() const noexcept;

    void mouseDoubleClickEvent(QMouseEvent* event) override;

Q_SIGNALS:
    void doubleClicked();
    void deleted();
    void trashMoved();
    void dataCopied(const QPixmap &img);
    void pathCopied(const QString &imgPath);
    void pathChanged(const QString &imgPath);

private Q_SLOTS:
    void showContextMenu(const QPoint &pos);

private:
    friend struct EditableImagePrivate;
    std::unique_ptr<struct EditableImagePrivate> d;

    void initContextMenu();
};

#endif // EDITABLEIMAGE_H
