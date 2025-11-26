// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QComboBox>
#include <QDialogButtonBox>
#include <QtGlobal>
#include <QVBoxLayout>
#include <QPushButton>

#include <ranges>

#include "imageviewerdialog.h"
#include "imageviewer.h"

ImageViewerDialog::ImageViewerDialog(SameImagesContainer sameImageList, QWidget *parent) noexcept
    : QDialog{parent}
{
    mainLayout = new QVBoxLayout;
    comboBox = new QComboBox{this};
    results.reserve(sameImageList.size());
    unsigned int index = 1U;
    for (auto &images : sameImageList | std::views::values) {
        QString name = tr("Group %1").arg(index++);
        comboBox->addItem(name);
        results.emplace(std::move(name), std::move(images));
    }
    empty = new QLabel{tr("No data here!"), this};
    empty->setMinimumSize(EditableImageFixedWidth + 50, EditableImageFixedHeight + 50);
    empty->setAlignment(Qt::AlignCenter);
    empty->hide();
    if (results.empty()) {
        current = empty;
    } else {
        const auto &name = comboBox->currentText();
        createImageViewer(name);
        current = viewers[name];
    }

    auto buttons = new QDialogButtonBox{this};
    prevBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowLeft), tr("prev")};
    connect(prevBtn, &QPushButton::clicked, this, [this]() noexcept {
        const auto index = comboBox->currentIndex();
        comboBox->setCurrentIndex(index - 1);
    });
    prevBtn->setEnabled(false);
    buttons->addButton(prevBtn, QDialogButtonBox::ActionRole);

    ignoreBtn = new QPushButton{style()->standardIcon(QStyle::SP_BrowserStop), tr("ignore this")};
    connect(ignoreBtn, &QPushButton::clicked, this, [this]() noexcept {
        const auto &oldName = comboBox->currentText();
        const auto comboIdx = comboBox->currentIndex();
        auto widget = viewers[oldName];
        viewers.erase(oldName);
        comboBox->removeItem(comboIdx);
        setCurrentWidgetByName(comboBox->currentText());
        widget->deleteLater();
    });
    buttons->addButton(ignoreBtn, QDialogButtonBox::ActionRole);

    nextBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowRight), tr("next")};
    connect(nextBtn, &QPushButton::clicked, comboBox, [this]() noexcept {
        const auto comboIdx = comboBox->currentIndex();
        comboBox->setCurrentIndex(comboIdx + 1);
    });
    buttons->addButton(nextBtn, QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Ok);

    auto buttonsSetEnable = [this]() noexcept {
        const auto comboIdx = comboBox->currentIndex();
        const auto hasViewer = comboBox->count() != 0;
        prevBtn->setEnabled(hasViewer && comboIdx != 0);
        nextBtn->setEnabled(hasViewer && comboIdx != comboBox->count() - 1);
        ignoreBtn->setEnabled(hasViewer);
    };
    connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, buttonsSetEnable](int) noexcept {
        setCurrentWidgetByName(comboBox->currentText());
        buttonsSetEnable();
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonsSetEnable();

    mainLayout->addWidget(comboBox, 0, Qt::AlignLeft);
    current->show();
    mainLayout->addWidget(current);
    auto btnLayout = new QHBoxLayout;
    btnLayout->addStretch(1);
    btnLayout->addWidget(buttons);
    mainLayout->addLayout(btnLayout);
    setLayout(mainLayout);
    updateTitle();
    setMaximumHeight(750);
    setModal(true);
}

void ImageViewerDialog::createImageViewer(const QString &name) noexcept
{
    auto imageView = new ImageViewer{results[name], this};
    results.erase(name);
    imageView->hide();
    connect(imageView, &ImageViewer::emptied, this, [this, name, imageView]() noexcept {
        const auto targetIndex = comboBox->findText(name);
        if (targetIndex < 0 || !viewers.contains(name) || viewers[name] != imageView) {
            qWarning() << tr("target ImageViewer not found");
            return;
        }
        viewers.erase(name);
        comboBox->removeItem(targetIndex);
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
    QWidget *widget = empty;
    if (name.isEmpty()) {
        ignoreBtn->setEnabled(false);
        goto end;
    }
    if (!viewers.contains(name)) {
        createImageViewer(name);
    }
    widget = viewers[name];
end:
    replaceCurrentWidget(widget);
    updateTitle();
}

void ImageViewerDialog::updateTitle() noexcept
{
    const auto count = comboBox->count();
    if (count <= 0) {
        setWindowTitle(tr("Check Results: No Results"));
        return;
    }

    setWindowTitle(tr("Check Results: %1/%2").arg(comboBox->currentIndex() + 1).arg(count));
}
