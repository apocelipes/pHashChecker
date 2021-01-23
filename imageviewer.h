#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QStyle>

#include <string>
#include <vector>

#include "editableimage.h"

class Thumbnail;
class QPushButton;
class QFocusFrame;

class ImageViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ImageViewer(const std::vector<std::string> &images, QWidget *parent = nullptr);

signals:
    void currentIndexChanged(unsigned int current, unsigned int previous);

private:
    QWidget *imageThumbnailList;
    std::vector<Thumbnail*> thumbs;
    EditableImage *imageContent;
    unsigned int currentIndex{};
    QFocusFrame *thumbnailFocusBorder;

    void setDefaultSelection();
    void initThumbnailFocusBorder();
    QPushButton *createSideControlButton(QStyle::StandardPixmap pixmap, QWidget *parent = nullptr);
};

#endif // IMAGEVIEWER_H
