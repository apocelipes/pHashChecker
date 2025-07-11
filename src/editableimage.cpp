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

#include <ankerl/unordered_dense.h>

#include "editableimage.h"
#include "hashdialog.h"
#include "utils/imageutils.h"
#include "utils/path.h"
#include "utils/sizeformat.h"
#include "utils/utils.h"

struct EditableImagePrivate {
    QMenu *contextMenu = nullptr;
    QString m_path;
    ankerl::unordered_dense::map<QString, QPixmap> cache;

    const QPixmap& getCachedPixmap(const QString &path) noexcept {
        if (!cache.contains(path)) {
            if (Utils::isFormatNeedConvert(path)) {
                cache.emplace(std::make_pair(path, Utils::convertToPixmap(path, EditableImageFixedWidth, EditableImageFixedHeight)));
            } else {
                cache.emplace(std::make_pair(path, QPixmap{path}.scaled(EditableImageFixedWidth, EditableImageFixedHeight)));
            }
        }
        return cache[path];
    }

    void removeCachedPixmap(const QString &path) noexcept {
        cache.erase(path);
    }
};

EditableImage::EditableImage(const QString &imgPath, QWidget *parent) noexcept
    : QLabel(parent), d{new EditableImagePrivate}
{
    setFixedSize(EditableImageFixedWidth, EditableImageFixedHeight);
    setImagePath(imgPath);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &EditableImage::showContextMenu);
    connect(this, &EditableImage::doubleClicked, this, &EditableImage::openImage);
}

void EditableImage::openImage() const noexcept
{
    const auto url = QUrl::fromLocalFile(Utils::getAbsPath(getImagePath()));
    QDesktopServices::openUrl(url);
}

EditableImage::~EditableImage() noexcept = default;

void EditableImage::initContextMenu() noexcept
{
    d->contextMenu = new QMenu{this};
    auto openAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("open"));
    connect(openAction, &QAction::triggered, this, &EditableImage::openImage);
    d->contextMenu->addAction(openAction);

    auto copyAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy data"));
    connect(copyAction, &QAction::triggered, this, [this]() noexcept {
        const auto &data = pixmap(Qt::ReturnByValue);
        QGuiApplication::clipboard()->setPixmap(data);
        Q_EMIT dataCopied(data);
    });
    d->contextMenu->addAction(copyAction);

    auto copyPathAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy path"));
    connect(copyPathAction, &QAction::triggered, this, [this]() noexcept {
        const auto absPath = Utils::getAbsPath(getImagePath());
        QGuiApplication::clipboard()->setText(absPath);
        Q_EMIT pathCopied(absPath);
    });
    d->contextMenu->addAction(copyPathAction);

    auto moveToTrashAction = new QAction(style()->standardIcon(QStyle::SP_TrashIcon), tr("move to trash"));
    connect(moveToTrashAction, &QAction::triggered, this, [this]() noexcept {
        QFile::moveToTrash(Utils::getAbsPath(getImagePath()));
        d->removeCachedPixmap(getImagePath());
        Q_EMIT trashMoved();
    });
    d->contextMenu->addAction(moveToTrashAction);

    auto deleteAction = new QAction(style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("delete"));
    connect(deleteAction, &QAction::triggered, this, [this]() noexcept {
        const auto absPath = Utils::getAbsPath(getImagePath());
        auto isDelete = QMessageBox::warning(this,
                                            tr("delete this image"),
                                            tr("do you want to delete %1 ?").arg(absPath),
                                            QMessageBox::Ok|QMessageBox::Cancel);
        if (isDelete == QMessageBox::Ok) {
            QFile::remove(absPath);
            d->removeCachedPixmap(getImagePath());
            Q_EMIT deleted();
        }
    });
    d->contextMenu->addAction(deleteAction);

    auto hashAction = new QAction{style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("hash")};
    connect(hashAction, &QAction::triggered, this, [this]() noexcept {
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
        setToolTip(isEmpty() ? tr("There's no image here") : Utils::getAbsPath(getImagePath()));
        return;
    }
    d->m_path = path;
    setPixmap(d->getCachedPixmap(d->m_path));
    setToolTip(tr("%1<br>size: %2").arg(Utils::getAbsPath(d->m_path), Utils::sizeFormat(info.size())));
    Q_EMIT pathChanged(d->m_path);
}

bool EditableImage::isEmpty() const noexcept
{
    return d->m_path.isEmpty();
}

void EditableImage::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_EMIT doubleClicked();
    event->accept();
}
