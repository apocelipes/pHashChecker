#include "editableimage.h"
#include "hashdialog.h"

#include <QtGlobal>
#include <QAction>
#include <QClipboard>
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
    setFixedSize(EditableImageFixedWidth, EditableImageFixedHeight);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &EditableImage::showContextMenu);
    connect(this, &EditableImage::doubleClicked, [this](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(getImagePath()));
    });
}

void EditableImage::initContextMenu()
{
    contextMenu = new QMenu{this};
    auto openAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("open"));
    connect(openAction, &QAction::triggered, [this](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(getImagePath()));
    });
    contextMenu->addAction(openAction);

    auto copyAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy data"));
    connect(copyAction, &QAction::triggered, [this](){
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        const auto &data = pixmap(Qt::ReturnByValue);
#else
        const auto &data = *pixmap();
#endif
        QGuiApplication::clipboard()->setPixmap(data);
        Q_EMIT dataCopied(data);
    });
    contextMenu->addAction(copyAction);

    auto copyPathAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy path"));
    connect(copyPathAction, &QAction::triggered, [this](){
        QGuiApplication::clipboard()->setText(getImagePath());
        Q_EMIT pathCopied(getImagePath());
    });
    contextMenu->addAction(copyPathAction);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    auto moveToTrashAction = new QAction(style()->standardIcon(QStyle::SP_TrashIcon), tr("move to trash"));
    connect(moveToTrashAction, &QAction::triggered, [this](){
        QFile::moveToTrash(getImagePath());
        Q_EMIT trashMoved();
    });
    contextMenu->addAction(moveToTrashAction);
#endif

    auto deleteAction = new QAction(style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("delete"));
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
    contextMenu->addAction(deleteAction);

    auto hashAction = new QAction{style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("hash")};
    connect(hashAction, &QAction::triggered, [this](){
        HashDialog dialog{getImagePath(), this};
        dialog.exec();
    });
    contextMenu->addAction(hashAction);
}

void EditableImage::showContextMenu(const QPoint &pos)
{
    if (contextMenu == nullptr) {
        initContextMenu();
    }

    const auto &actions = contextMenu->actions();
    for (auto action : actions) {
        action->setEnabled(!isEmpty());
    }
    contextMenu->popup(mapToGlobal(pos));
}
