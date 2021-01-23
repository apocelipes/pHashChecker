#include "imageviewer.h"

#include <QFocusFrame>
#include <QPushButton>
#include <QSize>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "thumbnail.h"

ImageViewer::ImageViewer(const std::vector<std::string> &images, QWidget *parent)
    : QWidget(parent)
{
    initThumbnailFocusBorder();

    thumbs.reserve(images.size());
    unsigned int index = 0u;
    for (const auto &img : images) {
        auto thumbnail = new Thumbnail{QString::fromStdString(img), this};
        thumbnail->showShadow();
        connect(thumbnail, &Thumbnail::clicked, [this, index]() {
            Q_EMIT currentIndexChanged(index, currentIndex);
            currentIndex = index;
        });
        thumbs.emplace_back(thumbnail);
        ++index;
    }
    setDefaultSelection();

    imageContent = new EditableImage{thumbs[0]->getImagePath(), this};
    connect(this, &ImageViewer::currentIndexChanged, [this](unsigned int current, unsigned int prev) {
        thumbnailFocusBorder->setWidget(thumbs[current]);
        thumbs[prev]->showShadow();
        thumbs[current]->hideShadow();
        imageContent->setImagePath(thumbs[current]->getImagePath());
    });

    imageThumbnailList = new QWidget{this};
    auto listLayout = new QHBoxLayout;
    listLayout->setSpacing(6);
    for (auto thumbnail : thumbs) {
        listLayout->addWidget(thumbnail, 0, Qt::AlignCenter);
    }
    imageThumbnailList->setLayout(listLayout);

    auto contentLayout = new QHBoxLayout;
    auto prevButton = createSideControlButton(QStyle::SP_ArrowLeft);
    contentLayout->addWidget(prevButton, 0, Qt::AlignLeft);
    connect(prevButton, &QPushButton::clicked, [this](){
        if (currentIndex == 0u) {
            Q_EMIT currentIndexChanged(thumbs.size()-1, currentIndex);
            currentIndex = thumbs.size() - 1;
            return;
        }

        Q_EMIT currentIndexChanged(currentIndex - 1, currentIndex);
        --currentIndex;
    });
    contentLayout->addWidget(imageContent, 0, Qt::AlignCenter);
    auto nextButton = createSideControlButton(QStyle::SP_ArrowRight);
    contentLayout->addWidget(nextButton, 0, Qt::AlignRight);
    connect(nextButton, &QPushButton::clicked, [this](){
        Q_EMIT currentIndexChanged((currentIndex+1)%thumbs.size(), currentIndex);
        currentIndex = (currentIndex+1)%thumbs.size();
    });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(contentLayout, 2);
    mainLayout->addWidget(imageThumbnailList, 1, Qt::AlignCenter);
    setLayout(mainLayout);
}

QPushButton *ImageViewer::createSideControlButton(QStyle::StandardPixmap pixmap, QWidget *btnParent)
{
    auto btn = new QPushButton(btnParent);
    btn->setIcon(style()->standardIcon(pixmap));
    btn->setAttribute(Qt::WA_StyledBackground);
    btn->setIconSize(QSize{50, 50});
    btn->setStyleSheet("background:rgba(0,0,0,0);border:0;");
    return btn;
}

void ImageViewer::setDefaultSelection()
{
    thumbs[0]->hideShadow();
    thumbnailFocusBorder->setWidget(thumbs[0]);
}

void ImageViewer::initThumbnailFocusBorder()
{
    thumbnailFocusBorder = new QFocusFrame{this};
    thumbnailFocusBorder->setAutoFillBackground(true);
    thumbnailFocusBorder->setStyleSheet("color:#30d5c8;");
}
