// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#include <QFocusFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSize>

#include "imageviewer.h"
#include "thumbnail.h"
#include "utils.h"

struct ImageViewerPrivate {
    ImageViewer *q = nullptr;
    QWidget *imageThumbnailList = nullptr;
    std::vector<Thumbnail*> thumbs;
    EditableImage *imageContent = nullptr;
    unsigned int currentIndex{};
    QFocusFrame *thumbnailFocusBorder = nullptr;

    void init(const std::vector<std::string> &images, ImageViewer *q_ptr);
    void initThumbnailFocusBorder()
    {
        thumbnailFocusBorder = new QFocusFrame{q};
        thumbnailFocusBorder->setAutoFillBackground(true);
        thumbnailFocusBorder->setStyleSheet("color:#30d5c8;");
    }

    [[nodiscard]] unsigned int decrCurrentIndex() noexcept
    {
        const auto oldIndex = currentIndex;
        currentIndex = currentIndex == 0u ? thumbs.size() - 1 : currentIndex - 1;
        return oldIndex;
    }

    void setDefaultSelection()
    {
        thumbs[0]->hideShadow();
        thumbnailFocusBorder->setWidget(thumbs[0]);
    }

    [[nodiscard]] QPushButton *createSideControlButton(QStyle::StandardPixmap pixmap, QWidget *btnParent) const
    {
        auto btn = new QPushButton(btnParent);
        btn->setIcon(q->style()->standardIcon(pixmap));
        btn->setAttribute(Qt::WA_StyledBackground);
        btn->setIconSize(QSize{50, 50});
        btn->setStyleSheet("background:rgba(0,0,0,0);border:0;");
        return btn;
    }

    void removeCurrentImage()
    {
        auto rmWidget = thumbs[currentIndex];
        imageThumbnailList->layout()->removeWidget(rmWidget);
        thumbs.erase(thumbs.begin() + currentIndex);
        rmWidget->deleteLater();
        if (thumbs.empty()) {
            imageContent->setImagePath("");
            Q_EMIT q->emptied();
            return;
        }
        if (currentIndex >= thumbs.size()) {
            currentIndex = thumbs.size() - 1;
        }
        thumbs[currentIndex]->hideShadow();
        imageContent->setImagePath(thumbs[currentIndex]->getImagePath());
        thumbnailFocusBorder->setWidget(thumbs[currentIndex]);
    }
};

inline void ImageViewerPrivate::init(const std::vector<std::string> &images, ImageViewer *q_ptr)
{
    q = q_ptr;
    if (images.empty()) {
        return;
    }
    thumbs.reserve(images.size());

    initThumbnailFocusBorder();

    for (const auto &img : images) {
        auto thumbnail = new Thumbnail{QString::fromStdString(img), q};
        QObject::connect(thumbnail, &Thumbnail::clicked, q, [this, thumbnail]() {
            auto index = Utils::indexOf(thumbs.cbegin(), thumbs.cend(), thumbnail);
            if (!index  || *index == currentIndex) {
                return;
            }
            Q_EMIT q->currentIndexChanged(*index, currentIndex);
            currentIndex = *index;
        });
        thumbs.emplace_back(thumbnail);
    }
    setDefaultSelection();

    imageContent = new EditableImage{thumbs[0]->getImagePath(), q};
    QObject::connect(q, &ImageViewer::currentIndexChanged, q, [this](unsigned int current, unsigned int prev) {
        thumbnailFocusBorder->setWidget(thumbs[current]);
        thumbs[prev]->showShadow();
        thumbs[current]->hideShadow();
        imageContent->setImagePath(thumbs[current]->getImagePath());
    });
    QObject::connect(imageContent, &EditableImage::trashMoved, q, &ImageViewer::removeCurrentImage);
    QObject::connect(imageContent, &EditableImage::deleted, q, &ImageViewer::removeCurrentImage);

    imageThumbnailList = new QWidget{q};
    auto listLayout = new QHBoxLayout;
    listLayout->setSpacing(6);
    for (auto thumbnail : thumbs) {
        listLayout->addWidget(thumbnail, 0, Qt::AlignCenter);
    }
    imageThumbnailList->setLayout(listLayout);

    auto contentLayout = new QHBoxLayout;
    auto prevButton = createSideControlButton(QStyle::SP_ArrowLeft, q);
    contentLayout->addWidget(prevButton, 0, Qt::AlignLeft);
    QObject::connect(prevButton, &QPushButton::clicked, q, [this](){
        if (thumbs.empty()) {
            return;
        }
        auto prevIndex = decrCurrentIndex();
        Q_EMIT q->currentIndexChanged(currentIndex, prevIndex);
    });
    contentLayout->addWidget(imageContent, 0, Qt::AlignCenter);
    auto nextButton = createSideControlButton(QStyle::SP_ArrowRight, q);
    contentLayout->addWidget(nextButton, 0, Qt::AlignRight);
    QObject::connect(nextButton, &QPushButton::clicked, q, [this](){
        if (thumbs.empty()) {
            return;
        }
        const auto newIndex = (currentIndex+1)%thumbs.size();
        Q_EMIT q->currentIndexChanged(newIndex, currentIndex);
        currentIndex = newIndex;
    });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(contentLayout, 2);
    mainLayout->addWidget(imageThumbnailList, 1, Qt::AlignCenter);
    q->setLayout(mainLayout);
}

ImageViewer::ImageViewer(const std::vector<std::string> &images, QWidget *parent)
    : QWidget(parent), d{new ImageViewerPrivate}
{
    d->init(images, this);
}

ImageViewer::~ImageViewer() noexcept = default;

void ImageViewer::removeCurrentImage()
{
    d->removeCurrentImage();
}
