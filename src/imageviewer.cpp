// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#include <QFocusFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSize>
#include <QStyle>

#include "imageviewer.h"
#include "thumbnail.h"
#include "utils/utils.h"

struct ImageViewerPrivate {
    ImageViewer *q = nullptr;
    QWidget *imageThumbnailList = nullptr;
    std::vector<Thumbnail*> thumbs;
    EditableImage *imageContent = nullptr;
    unsigned int currentIndex{};
    QFocusFrame *thumbnailFocusBorder = nullptr;

    void init(const std::vector<std::string> &images, ImageViewer *q_ptr) noexcept;
    void initThumbnailFocusBorder() noexcept
    {
        static const std::array styles = {
            QStringLiteral(u"border: 2px solid #e57cdc;"),
            QStringLiteral(u"border: 2px solid #30d5c8;"),
        };
        thumbnailFocusBorder = new QFocusFrame{q};
        thumbnailFocusBorder->setStyleSheet(styles[QRandomGenerator::global()->generate() & 1]);
    }

    [[nodiscard]] unsigned int decrCurrentIndex() noexcept
    {
        const auto oldIndex = currentIndex;
        currentIndex = currentIndex == 0u ? thumbs.size() - 1 : currentIndex - 1;
        return oldIndex;
    }

    void setDefaultSelection() noexcept
    {
        thumbs[0]->hideShadow();
        thumbnailFocusBorder->setWidget(thumbs[0]);
    }

    [[nodiscard]] QPushButton *createSideControlButton(QStyle::StandardPixmap pixmap, QWidget *btnParent) const noexcept
    {
        auto btn = new QPushButton(btnParent);
        btn->setIcon(q->style()->standardIcon(pixmap));
        btn->setAttribute(Qt::WA_StyledBackground);
        btn->setIconSize(QSize{50, 50});
        btn->setStyleSheet(QStringLiteral(u"background:rgba(0,0,0,0);border:0;"));
        return btn;
    }

    void removeCurrentImage() noexcept
    {
        auto rmWidget = thumbs[currentIndex];
        imageThumbnailList->layout()->removeWidget(rmWidget);
        thumbs.erase(thumbs.begin() + currentIndex);
        rmWidget->deleteLater();
        if (thumbs.empty()) {
            imageContent->setImagePath(QString{});
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

inline void ImageViewerPrivate::init(const std::vector<std::string> &images, ImageViewer *q_ptr) noexcept
{
    q = q_ptr;
    if (images.empty()) {
        return;
    }
    thumbs.reserve(images.size());

    initThumbnailFocusBorder();

    for (const auto &img : images) {
        auto thumbnail = new Thumbnail{QString::fromStdString(img), q};
        QObject::connect(thumbnail, &Thumbnail::clicked, q, [this, thumbnail]() noexcept {
            auto index = Utils::indexOf(thumbs, thumbnail);
            if (!index || *index == currentIndex) {
                return;
            }
            Q_EMIT q->currentIndexChanged(*index, currentIndex);
            currentIndex = *index;
        });
        thumbs.emplace_back(thumbnail);
    }
    setDefaultSelection();

    imageContent = new EditableImage{thumbs[0]->getImagePath(), q};
    QObject::connect(q, &ImageViewer::currentIndexChanged, q, [this](unsigned int current, unsigned int prev) noexcept {
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
    QObject::connect(prevButton, &QPushButton::clicked, q, [this]() noexcept {
        if (thumbs.empty()) {
            return;
        }
        auto prevIndex = decrCurrentIndex();
        Q_EMIT q->currentIndexChanged(currentIndex, prevIndex);
    });
    prevButton->setShortcut(QKeySequence{Qt::Key_Left});

    contentLayout->addWidget(imageContent, 0, Qt::AlignCenter);

    auto nextButton = createSideControlButton(QStyle::SP_ArrowRight, q);
    contentLayout->addWidget(nextButton, 0, Qt::AlignRight);
    QObject::connect(nextButton, &QPushButton::clicked, q, [this]() noexcept {
        if (thumbs.empty()) {
            return;
        }
        const auto newIndex = (currentIndex+1) % thumbs.size();
        Q_EMIT q->currentIndexChanged(newIndex, currentIndex);
        currentIndex = newIndex;
    });
    nextButton->setShortcut(QKeySequence{Qt::Key_Right});

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(contentLayout, 2);
    mainLayout->addStretch();
    mainLayout->addWidget(imageThumbnailList, 1, Qt::AlignCenter);
    q->setLayout(mainLayout);
}

ImageViewer::ImageViewer(const std::vector<std::string> &images, QWidget *parent) noexcept
    : QWidget(parent), d{new ImageViewerPrivate}
{
    d->init(images, this);
}

ImageViewer::~ImageViewer() noexcept = default;

void ImageViewer::removeCurrentImage() noexcept
{
    d->removeCurrentImage();
}
