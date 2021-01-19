#include "editableimage.h"

#include <QAction>
#include <QClipboard>
#include <QColor>
#include <QDesktopServices>
#include <QFile>
#include <QGuiApplication>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QStyle>
#include <QUrl>

EditableImage::EditableImage(const QString &imgPath, QWidget *parent)
    : QLabel(parent)
{
    setImagePath(imgPath);
    setScaledContents(true);
    setFixedSize(150, 300);
    QPixmap pic{getImagePath()};
    setPixmap(pic);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &EditableImage::showContextMenu);
    connect(this, &EditableImage::doubleClicked, [this](){
        QDesktopServices::openUrl(QUrl(getImagePath()));
    });
}

void EditableImage::showContextMenu(const QPoint &pos)
{
    auto menu = new QMenu(this);
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);

    auto openAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("open"), this);
    connect(openAction, &QAction::triggered, [this](){
        QDesktopServices::openUrl(QUrl(getImagePath()));
    });
    menu->addAction(openAction);
    auto copyAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy data"), this);
    connect(copyAction, &QAction::triggered, [this](){
        QGuiApplication::clipboard()->setPixmap(pixmap(Qt::ReturnByValue));
        Q_EMIT dataCopied(pixmap(Qt::ReturnByValue));
    });
    menu->addAction(copyAction);
    auto copyPathAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy path"), this);
    connect(copyPathAction, &QAction::triggered, [this](){
        QGuiApplication::clipboard()->setText(getImagePath());
        Q_EMIT pathCopied(getImagePath());
    });
    menu->addAction(copyPathAction);
    auto moveToTrashAction = new QAction(style()->standardIcon(QStyle::SP_TrashIcon), tr("move to trash"), this);
    connect(moveToTrashAction, &QAction::triggered, [this](){
        QFile::moveToTrash(getImagePath());
        Q_EMIT TrashMoved();
    });
    menu->addAction(moveToTrashAction);
    auto deleteAction = new QAction(style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("delete"), this);
    connect(deleteAction, &QAction::triggered, [this](){
        auto isDelete = QMessageBox::warning(this,
                                             tr("delete this image"),
                                             tr("do you want to delete it?"),
                                             QMessageBox::Ok|QMessageBox::Cancel);
        if (isDelete == QMessageBox::Ok) {
            QFile::remove(getImagePath());
            Q_EMIT deleted();
        }
    });
    menu->addAction(deleteAction);

    menu->popup(mapToGlobal(pos));
}
