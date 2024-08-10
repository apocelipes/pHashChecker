// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#include <QtGlobal>
#include <QAction>
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QGuiApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPoint>
#include <QStyle>
#include <QUrl>

#include <filesystem>

#include "convertedimage.hpp"
#include "editableimage.h"
#include "hashdialog.h"
#include "utils/sizeformat.h"

struct EditableImagePrivate {
    QMenu *contextMenu = nullptr;
    QString m_path;
    std::optional<ConvertedImage> convertedImg;
};

EditableImage::EditableImage(const QString &imgPath, QWidget *parent) noexcept
    : QLabel(parent), d{new EditableImagePrivate}
{
    setImagePath(imgPath);
    setFixedSize(EditableImageFixedWidth, EditableImageFixedHeight);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &EditableImage::showContextMenu);
    connect(this, &EditableImage::doubleClicked, [this](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(getImagePath()));
    });
}

EditableImage::~EditableImage() noexcept = default;

void EditableImage::initContextMenu() noexcept
{
    d->contextMenu = new QMenu{this};
    auto openAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("open"));
    connect(openAction, &QAction::triggered, this, [this](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(getImagePath()));
    });
    d->contextMenu->addAction(openAction);

    auto copyAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy data"));
    connect(copyAction, &QAction::triggered, this, [this](){
        const auto &data = pixmap(Qt::ReturnByValue);
        QGuiApplication::clipboard()->setPixmap(data);
        Q_EMIT dataCopied(data);
    });
    d->contextMenu->addAction(copyAction);

    auto copyPathAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy path"));
    connect(copyPathAction, &QAction::triggered, this, [this](){
        QGuiApplication::clipboard()->setText(getImagePath());
        Q_EMIT pathCopied(getImagePath());
    });
    d->contextMenu->addAction(copyPathAction);

    auto moveToTrashAction = new QAction(style()->standardIcon(QStyle::SP_TrashIcon), tr("move to trash"));
    connect(moveToTrashAction, &QAction::triggered, this, [this](){
        QFile::moveToTrash(getImagePath());
        Q_EMIT trashMoved();
    });
    d->contextMenu->addAction(moveToTrashAction);

    auto deleteAction = new QAction(style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("delete"));
    connect(deleteAction, &QAction::triggered, this, [this](){
        auto isDelete = QMessageBox::warning(this,
                                             tr("delete this image"),
                                             tr("do you want to delete %1 ?").arg(d->m_path),
                                             QMessageBox::Ok|QMessageBox::Cancel);
        if (isDelete == QMessageBox::Ok) {
            QFile::remove(getImagePath());
            Q_EMIT deleted();
        }
    });
    d->contextMenu->addAction(deleteAction);

    auto hashAction = new QAction{style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("hash")};
    connect(hashAction, &QAction::triggered, this, [this](){
        HashDialog dialog{getImagePath(), this};
        dialog.exec();
    });
    d->contextMenu->addAction(hashAction);
}

void EditableImage::showContextMenu(const QPoint &pos) noexcept
{
    if (d->contextMenu == nullptr) {
        initContextMenu();
    }

    const auto &actions = d->contextMenu->actions();
    for (auto action : actions) {
        action->setEnabled(!isEmpty());
    }
    d->contextMenu->popup(mapToGlobal(pos));
}

QString EditableImage::getImagePath() const noexcept
{
    return d->m_path;
}

void EditableImage::setImagePath(const QString &path) noexcept
{
    if (path == d->m_path) {
        return;
    }
    const auto &info = QFileInfo{path};
    if (!info.exists()) {
        clear();
        d->convertedImg.reset();
        setToolTip(isEmpty() ? tr("There's no image here") : getImagePath());
        return;
    }
    d->m_path = path;
    auto newPath = d->m_path;
    if (Utils::isFormatNeedConvert(path)) {
        d->convertedImg.emplace(path, EditableImageFixedWidth, EditableImageFixedHeight, true);
        newPath = d->convertedImg->getImagePath();
    }
    QPixmap newImg{newPath};
    setPixmap(newImg.scaled(EditableImageFixedWidth, EditableImageFixedHeight));
    setToolTip(tr("%1<br>size: %2").arg(d->m_path).arg(Utils::sizeFormat(info.size())));
    Q_EMIT pathChanged(d->m_path);
}

bool EditableImage::isEmpty() const noexcept
{
    return d->m_path.isEmpty();
}

void EditableImage::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_EMIT doubleClicked();
    event->accept();
}
