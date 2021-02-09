#include "editableimage.h"
#include "hashdialog.h"

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
    setFixedSize(500, 400);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &EditableImage::showContextMenu);
    connect(this, &EditableImage::doubleClicked, [this](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(getImagePath()));
    });
}

void EditableImage::showContextMenu(const QPoint &pos)
{
    auto menu = new QMenu(this);
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);

    auto openAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("open"), menu);
    connect(openAction, &QAction::triggered, [this](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(getImagePath()));
    });
    menu->addAction(openAction);

    auto copyAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy data"), menu);
    connect(copyAction, &QAction::triggered, [this](){
        const auto &data = pixmap(Qt::ReturnByValue);
        QGuiApplication::clipboard()->setPixmap(data);
        Q_EMIT dataCopied(data);
    });
    menu->addAction(copyAction);

    auto copyPathAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("copy path"), menu);
    connect(copyPathAction, &QAction::triggered, [this](){
        QGuiApplication::clipboard()->setText(getImagePath());
        Q_EMIT pathCopied(getImagePath());
    });
    menu->addAction(copyPathAction);

    auto moveToTrashAction = new QAction(style()->standardIcon(QStyle::SP_TrashIcon), tr("move to trash"), menu);
    connect(moveToTrashAction, &QAction::triggered, [this](){
        QFile::moveToTrash(getImagePath());
        Q_EMIT trashMoved();
    });
    menu->addAction(moveToTrashAction);

    auto deleteAction = new QAction(style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("delete"), menu);
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

    auto hashAction = new QAction{style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("hash")};
    connect(hashAction, &QAction::triggered, [this](){
        HashDialog dialog{getImagePath(), this};
        dialog.exec();
    });
    menu->addAction(hashAction);

    auto actions = menu->actions();
    for (auto action = actions.begin(); action != actions.end(); ++action) {
        (*action)->setEnabled(!isEmpty());
    }
    menu->popup(mapToGlobal(pos));
}
