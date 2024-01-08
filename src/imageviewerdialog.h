// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef IMAGEVIEWERDIALOG_H
#define IMAGEVIEWERDIALOG_H

#include <QDialog>

#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>

class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class ImageViewer;

class ImageViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewerDialog(ankerl::unordered_dense::map<std::string, std::vector<std::string>> &sameImageList);

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
};

#endif // IMAGEVIEWERDIALOG_H
