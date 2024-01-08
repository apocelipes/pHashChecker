// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#include <QComboBox>
#include <QDialogButtonBox>
#include <QtGlobal>
#include <QVBoxLayout>
#include <QPushButton>

#include "imageviewerdialog.h"
#include "imageviewer.h"
#include "utils.h"

ImageViewerDialog::ImageViewerDialog(ankerl::unordered_dense::map<std::string, std::vector<std::string>> &sameImageList)
{
    mainLayout = new QVBoxLayout;
    comboBox = new QComboBox{this};
    results.reserve(sameImageList.size());
    unsigned int index = 1u;
    for (auto &images : sameImageList | std::views::values) {
        QString name = tr("Group %1").arg(index++);
        comboBox->addItem(name);
        results.emplace(std::move(name), std::move(images));
    }
    empty = new QLabel{tr("No data here!"), this};
    empty->setMinimumSize(900, 700);
    empty->setAlignment(Qt::AlignCenter);
    empty->hide();
    if (results.empty()) {
        current = empty;
    } else {
        createImageViewer(comboBox->currentText());
        current = viewers[comboBox->currentText()];
    }

    auto buttons = new QDialogButtonBox{this};
    prevBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowLeft), tr("prev")};
    connect(prevBtn, &QPushButton::clicked, [this](){
        auto index = comboBox->currentIndex();
        comboBox->setCurrentIndex(index - 1);
    });
    prevBtn->setEnabled(false);
    buttons->addButton(prevBtn, QDialogButtonBox::ActionRole);

    ignoreBtn = new QPushButton{style()->standardIcon(QStyle::SP_BrowserStop), tr("ignore this")};
    connect(ignoreBtn, &QPushButton::clicked, this, [this](){
        auto widget = viewers[comboBox->currentText()];
        auto index = comboBox->currentIndex();
        viewers.erase(comboBox->currentText());
        comboBox->removeItem(index);
        setCurrentWidgetByName(comboBox->currentText());
        widget->deleteLater();
    });
    buttons->addButton(ignoreBtn, QDialogButtonBox::ActionRole);

    nextBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowRight), tr("next")};
    connect(nextBtn, &QPushButton::clicked, comboBox, [this](){
        auto index = comboBox->currentIndex();
        comboBox->setCurrentIndex(index + 1);
    });
    buttons->addButton(nextBtn, QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Ok);

    auto buttonsSetEnable = [this](){
        auto index = comboBox->currentIndex();
        auto hasViewer = comboBox->count() != 0;
        prevBtn->setEnabled(hasViewer && index != 0);
        nextBtn->setEnabled(hasViewer && index != comboBox->count() - 1);
        ignoreBtn->setEnabled(hasViewer);
    };
    connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, buttonsSetEnable](int){
        setCurrentWidgetByName(comboBox->currentText());
        buttonsSetEnable();
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonsSetEnable();

    mainLayout->addWidget(comboBox, 0, Qt::AlignLeft);
    current->show();
    mainLayout->addWidget(current);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
    setWindowTitle(tr("Check Images"));
}

void ImageViewerDialog::createImageViewer(const QString &name) noexcept
{
    auto imageView = new ImageViewer{results[name], this};
    imageView->hide();
    connect(imageView, &ImageViewer::emptied, this, [this, name, imageView](){
        auto targetIndex = Utils::indexOf(viewers | std::views::values, imageView);
        if (!viewers.contains(name) || viewers[name] != imageView) {
            qWarning() << tr("target ImageViewer not found");
            return;
        }
        viewers.erase(name);
        comboBox->removeItem(static_cast<int>(*targetIndex));
        setCurrentWidgetByName(comboBox->currentText());
        imageView->deleteLater();
    });
    viewers[name] = imageView;
}

void ImageViewerDialog::replaceCurrentWidget(QWidget *newCurrent) noexcept
{
    assert(current != nullptr);
    current->hide();
    delete mainLayout->replaceWidget(current, newCurrent);
    newCurrent->show();
    current = newCurrent;
}

void ImageViewerDialog::setCurrentWidgetByName(const QString &name) noexcept
{
    if (name.isEmpty()) {
        ignoreBtn->setEnabled(false);
        replaceCurrentWidget(empty);
        return;
    }
    if (!viewers.contains(name)) {
        createImageViewer(name);
    }
    replaceCurrentWidget(viewers[name]);
}
