// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef IMAGEVIEWERDIALOG_H
#define IMAGEVIEWERDIALOG_H

#include <QDialog>

#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>

class ImageViewer;

class ImageViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewerDialog(const ankerl::unordered_dense::map<std::string, std::vector<std::string>> &sameImageList);

private:
    std::vector<ImageViewer*> viewers;
};

#endif // IMAGEVIEWERDIALOG_H
