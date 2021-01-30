#include "thumbnail.h"

#include <QPixmap>

#include <filesystem>

Thumbnail::Thumbnail(const QString &path, QWidget *parent)
    : QWidget(parent), imgPath{path}
{
    setFixedSize(100, 100);
    image = new QLabel(this);
    image->setScaledContents(true);
    image->setGeometry(geometry());
    if (std::filesystem::exists(imgPath.toStdString())) {
        QPixmap data{imgPath};
        image->setPixmap(data);
    }

    shadow = new QWidget(this);
    shadow->setWindowFlag(Qt::FramelessWindowHint);
    shadow->setAttribute(Qt::WA_StyledBackground);
    shadow->setStyleSheet("background:rgb(255,255,255);");
    shadow->setGeometry(geometry());
    setOpacity(DEFAULT_OPACITY);
    shadow->hide();
}
