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

Q_SIGNALS:
    void currentIndexChanged(unsigned int current, unsigned int previous);
    void emptied();

private Q_SLOTS:
    void removeCurrentImage();

private:
    QWidget *imageThumbnailList = nullptr;
    std::vector<Thumbnail*> thumbs;
    EditableImage *imageContent = nullptr;
    unsigned int currentIndex{};
    QFocusFrame *thumbnailFocusBorder = nullptr;

    void setDefaultSelection();
    void initThumbnailFocusBorder();
    QPushButton *createSideControlButton(QStyle::StandardPixmap pixmap, QWidget *parent = nullptr);

    unsigned int decrCurrentIndex() noexcept
    {
        auto oldIndex = currentIndex;
        currentIndex = currentIndex == 0u ? thumbs.size() - 1 : currentIndex - 1;
        return oldIndex;
    }
};

#endif // IMAGEVIEWER_H
