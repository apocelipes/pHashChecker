// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#pragma once

#include <QWidget>

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
    ~ImageViewer() noexcept override;

Q_SIGNALS:
    void currentIndexChanged(unsigned int current, unsigned int previous);
    void emptied();

private Q_SLOTS:
    void removeCurrentImage() noexcept;

private:
    friend struct ImageViewerPrivate;
    std::unique_ptr<struct ImageViewerPrivate> d;
};
