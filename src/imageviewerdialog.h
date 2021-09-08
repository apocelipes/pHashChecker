// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef IMAGEVIEWERDIALOG_H
#define IMAGEVIEWERDIALOG_H

#include <QDialog>

#include <string>
#include <vector>

class ImageViewer;

class ImageViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewerDialog(const std::vector<std::vector<std::string>> &sameImageList);

private:
    std::vector<ImageViewer*> viewers;
};

#endif // IMAGEVIEWERDIALOG_H
