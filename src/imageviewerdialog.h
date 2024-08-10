// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

#include <QDialog>

#include <string>
#include <vector>

#include "qstringhasher.hpp"

class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class ImageViewer;

class ImageViewerDialog : public QDialog
{
    Q_OBJECT
public:
    // take the ownership from MainWindow
    explicit ImageViewerDialog(SameImagesContainer sameImageList) noexcept;

private:
    ankerl::unordered_dense::map<QString, ImageViewer*> viewers;
    ankerl::unordered_dense::map<QString, std::vector<std::string>> results;
    QComboBox *comboBox = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    QLabel *empty = nullptr;
    QWidget *current = nullptr;
    QPushButton *prevBtn = nullptr;
    QPushButton *ignoreBtn = nullptr;
    QPushButton *nextBtn = nullptr;

    void createImageViewer(const QString &name) noexcept;
    void replaceCurrentWidget(QWidget *newCurrent) noexcept;
    void setCurrentWidgetByName(const QString &name) noexcept;
    void updateTitle() noexcept;
};
